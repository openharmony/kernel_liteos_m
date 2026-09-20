/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "It_los_mem.h"

/*
 * ItLosTick012 - LOS_TickTimerRegister 校验矩阵 / LOS_SysTickClockFreqAdjust 同值早返 /
 *
 * timer 恢复机制结论(源码实测,决定本用例安全边界):
 *  - tick timer init 为 LOS_SYS_INIT(SYS_INIT_LEVEL_ARCH, SYNC_1),boot 期已执行成功,
 *    g_sysTimerIsInit=TRUE(los_tick.c:201);
 *  - LOS_TickTimerRegister(timer!=NULL) 先过 TickTimerCheck,其最后一道校验
 *    (los_tick.c:158-160)在系统启动后必返回 LOS_ERRNO_SYS_TIMER_IS_RUNNING,
 *    memcpy_s 主路径(:224-241)与同指针早返(:229-232)均不可达;
 *  - LOS_TickTimerRegister(NULL,handler) 同样被 :243-245 的 IS_RUNNING 拒绝,
 *    handler 设置路径(:247-254)不可达;
 *  - 因此本用例所有注册调用均被拒绝且不改变任何系统状态,EXIT 无需恢复 timer。
 *
 * 覆盖点(kernel/base/los_tick.c,基线 tick 56% 的未覆盖行):
 *  - LOS_TickTimerRegister(:209-255): 010 已测双 NULL/零 freq,此处补
 *      freq<每秒 tick 数(:140) / irqNum 超上限(:144-146) / periodMax=0(:148-150) /
 *      函数指针缺失(:152-156) / 全合法被 IS_RUNNING 拒绝(:158-160) /
 *      (NULL,handler) 被拒(:243-245)。TickTimerCheck(:132-163)经此全分支触达。
 *  - LOS_SysTickClockFreqAdjust(:257-296): 011 已测 NULL handler/零 freq,此处补
 *      handler 返回当前频率的同值分支(:274 freq==g_sysClock)→ 早返不改变频率。
 *      真实变频主路径会 reload+改全局时钟频率并影响调度时间换算,
 *      副作用不可逆,归档不测(011 注释已判"漂移崩溃")。
 */

/* 哑 timer 回调: 仅用于通过 TickTimerCheck 非空校验,系统启动后注册被拒,不会被真正调用 */
static UINT32 ItTickTimerDummyInit(HWI_PROC_FUNC handler)
{
    (VOID)handler;
    return LOS_OK;
}

static UINT64 ItTickTimerDummyGetCycle(UINT32 *period)
{
    if (period != NULL) {
        *period = 0;
    }
    return 0;
}

static UINT64 ItTickTimerDummyReload(UINT64 period)
{
    return period;
}

static VOID ItTickTimerDummyLock(VOID)
{
}

static VOID ItTickTimerDummyUnlock(VOID)
{
}

static VOID ItTickDummyHandler(VOID)
{
}

/* 同值 handler: 返回当前 g_sysClock(=CyclePerTick*TICK_PER_SECOND),命中 :274 同值早返 */
static UINT32 ItTickFreqSameHandler(UINTPTR param)
{
    (VOID)param;
    return LOS_CyclePerTickGet() * LOSCFG_BASE_CORE_TICK_PER_SECOND;
}

/* 用例简要描述: LOS_TickTimerRegister 全校验分支+FreqAdjust 同值早返(纯 LOS_ 公开接口) */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT64 sysClock;
    ArchTickTimer timer;

    /* F-Inv: freq 低于每秒 tick 数(100) → LOS_ERRNO_SYS_CLOCK_INVALID(los_tick.c:140,
       与 010 的 freq=0 行分支不同) */
    timer.freq = LOSCFG_BASE_CORE_TICK_PER_SECOND - 1;
    timer.irqNum = 0;
    timer.periodMax = 0xFFFFFFFF;
    timer.init = ItTickTimerDummyInit;
    timer.getCycle = ItTickTimerDummyGetCycle;
    timer.reload = ItTickTimerDummyReload;
    timer.lock = ItTickTimerDummyLock;
    timer.unlock = ItTickTimerDummyUnlock;
    timer.tickHandler = NULL;
    ret = LOS_TickTimerRegister(&timer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_CLOCK_INVALID, ret);

    /* F-Inv: irqNum 超平台上限 → LOS_ERRNO_TICK_CFG_INVALID(los_tick.c:144-146);
       注意 irqNum 为 INT32,直接赋 0xFFFF 会符号截断为 -1 反而合法,故用上限+1 */
    timer.freq = LOS_CyclePerTickGet() * LOSCFG_BASE_CORE_TICK_PER_SECOND;
    timer.irqNum = (INT32)LOSCFG_PLATFORM_HWI_LIMIT + 1;
    ret = LOS_TickTimerRegister(&timer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_TICK_CFG_INVALID, ret);

    /* F-Inv: periodMax=0 → LOS_ERRNO_TICK_CFG_INVALID(los_tick.c:148-150) */
    timer.irqNum = 0;
    timer.periodMax = 0;
    ret = LOS_TickTimerRegister(&timer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_TICK_CFG_INVALID, ret);

    /* F-Inv: init 回调缺失 → LOS_ERRNO_SYS_HOOK_IS_NULL(los_tick.c:152-156) */
    timer.periodMax = 0xFFFFFFFF;
    timer.init = NULL;
    ret = LOS_TickTimerRegister(&timer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_HOOK_IS_NULL, ret);

    /* F-Inv: 全部参数合法 → 系统启动后 g_sysTimerIsInit=TRUE,
       TickTimerCheck 返回 LOS_ERRNO_SYS_TIMER_IS_RUNNING(los_tick.c:158-160):
       证明 memcpy 主路径(:224-241)不可达,系统 timer 不会被改写 */
    timer.init = ItTickTimerDummyInit;
    ret = LOS_TickTimerRegister(&timer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_TIMER_IS_RUNNING, ret);

    /* F-Inv: timer=NULL 且 handler 非空 → 同样被 IS_RUNNING 拒绝(los_tick.c:243-245),
       handler 设置路径(:247-254)不可达,真实 tickHandler 不受影响 */
    ret = LOS_TickTimerRegister(NULL, ItTickDummyHandler);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_TIMER_IS_RUNNING, ret);

    /* F-Inv: handler 返回当前频率(同值) → LOS_ERRNO_SYS_CLOCK_INVALID
       (los_tick.c:274 的 freq==g_sysClock 分支),仅经历 lock/unlock 真实 timer,
       不修改频率/不重装定时器 */
    ret = LOS_SysTickClockFreqAdjust(ItTickFreqSameHandler, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_CLOCK_INVALID, ret);

    sysClock = LOS_CyclePerTickGet() * LOSCFG_BASE_CORE_TICK_PER_SECOND;
    ICUNIT_ASSERT_NOT_EQUAL(sysClock, 0, sysClock);

    return LOS_OK;
}

VOID ItLosTick012(VOID)
{
    TEST_ADD_CASE("ItLosTick012", TestCase, TEST_LOS, TEST_TICK, TEST_LEVEL1, TEST_FUNCTION);
}
