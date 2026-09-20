/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "osTest.h"
#include "It_los_mem.h"
#include "los_timer.h"

/* 用例简要描述: LOS_TickTimerRegister 参数校验负路径全家桶（校验先于任何状态
 * 修改，全部安全拒绝）+ LOS_SysTickClockFreqAdjust 变频应用路径（加倍→调回
 * 恢复现场）。只调对外接口。 */

static UINT32 DummyInit013(HWI_PROC_FUNC tickHandler)
{
    (VOID)tickHandler;
    return LOS_OK;
}

static UINT64 DummyGetCycle013(UINT32 *period)
{
    (VOID)period;
    return 0;
}

static UINT64 DummyReload013(UINT64 time)
{
    return time;
}

static VOID DummyLock013(VOID)
{
}

static UINT32 FreqHandler013(UINTPTR param)
{
    return (UINT32)param;
}

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    ArchTickTimer badTimer;
    UINT32 curClock;

    /* 当前系统时钟 = 每 tick 周期数 × 每秒 tick 数（公共接口推导）。 */
    curClock = LOS_CyclePerTickGet() * LOSCFG_BASE_CORE_TICK_PER_SECOND;

    /* 构造"合法"模板：所有校验项默认通过（用于逐项破坏）。 */
    badTimer.freq = curClock;
    badTimer.irqNum = 0;
    badTimer.periodMax = 0xFFFFFFFF; // big enough.
    badTimer.init = DummyInit013;
    badTimer.getCycle = DummyGetCycle013;
    badTimer.reload = DummyReload013;
    badTimer.lock = DummyLock013;
    badTimer.unlock = DummyLock013;
    badTimer.tickHandler = NULL;

    /* F-Inv: timer 与 handler 双 NULL → LOS_ERRNO_SYS_PTR_NULL。 */
    ret = LOS_TickTimerRegister(NULL, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_PTR_NULL, ret);

    /* F-Inv: freq=0 → LOS_ERRNO_SYS_CLOCK_INVALID。 */
    badTimer.freq = 0;
    ret = LOS_TickTimerRegister(&badTimer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_CLOCK_INVALID, ret);

    /* F-Inv: irqNum 超出平台 HWI 上限 → LOS_ERRNO_TICK_CFG_INVALID。 */
    badTimer.freq = curClock;
    badTimer.irqNum = 0xFFFF; // 65535, far beyond any platform HWI limit.
    ret = LOS_TickTimerRegister(&badTimer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_TICK_CFG_INVALID, ret);

    /* F-Inv: periodMax=0 → LOS_ERRNO_TICK_CFG_INVALID。 */
    badTimer.irqNum = 0;
    badTimer.periodMax = 0;
    ret = LOS_TickTimerRegister(&badTimer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_TICK_CFG_INVALID, ret);

    /* F-Inv: init 回调为空 → LOS_ERRNO_SYS_HOOK_IS_NULL。 */
    badTimer.periodMax = 0xFFFFFFFF; // big enough.
    badTimer.init = NULL;
    ret = LOS_TickTimerRegister(&badTimer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_HOOK_IS_NULL, ret);

    /* F-Inv: 参数完全合法但系统 tick 已初始化 → 安全拒绝 IS_RUNNING。 */
    badTimer.init = DummyInit013;
    ret = LOS_TickTimerRegister(&badTimer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_TIMER_IS_RUNNING, ret);

    /* F-Inv: timer=NULL + 有效 handler，系统已运行 → IS_RUNNING。 */
    ret = LOS_TickTimerRegister(NULL, (HWI_PROC_FUNC)DummyLock013);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_TIMER_IS_RUNNING, ret);

    /* F-Inv: 变频 handler 返回 0 → LOS_ERRNO_SYS_CLOCK_INVALID。 */
    ret = LOS_SysTickClockFreqAdjust(FreqHandler013, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_CLOCK_INVALID, ret);

    /* F-Inv: 返回当前频率（无变化）→ LOS_ERRNO_SYS_CLOCK_INVALID。 */
    ret = LOS_SysTickClockFreqAdjust(FreqHandler013, (UINTPTR)curClock);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_CLOCK_INVALID, ret);

    /* F-Pos: 真实变频应用（加倍）→ LOS_OK。
     * 前置防御：加倍频率须不溢出 UINT32（freq 字段），且加倍后每 tick 周期数
     * 须不超出 24 位定时器（qemu SysTick）的 reload 上限 0xFFFFFF——
     * 任一不满足则本板配置不支持加倍构造，跳过变频应用段（负路径已覆盖）。 */
    if ((curClock <= (UINT32_MAX / 4)) && /* 2, ensure curClock * 2 fits in UINT32. */
        ((curClock * 2 / LOSCFG_BASE_CORE_TICK_PER_SECOND) <= 0xFFFFFF)) { /* 24-bit SysTick. */
        ret = LOS_SysTickClockFreqAdjust(FreqHandler013, (UINTPTR)(curClock * 2)); // 2, double.
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

        /* 现场恢复校验前先调回原频：断言失败也经 EXIT 保证频率恢复。 */
        ret = LOS_SysTickClockFreqAdjust(FreqHandler013, (UINTPTR)curClock);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

        /* 现场恢复校验：每 tick 周期数回到原值。 */
        ret = LOS_CyclePerTickGet();
        ICUNIT_GOTO_EQUAL(ret, curClock / LOSCFG_BASE_CORE_TICK_PER_SECOND, ret, EXIT);
    }

    return LOS_OK;

EXIT:
    /* 异常路径兜底：若仍处于加倍频率则调回原频，保证系统时钟不留痕。 */
    if (LOS_CyclePerTickGet() != curClock / LOSCFG_BASE_CORE_TICK_PER_SECOND) {
        (VOID)LOS_SysTickClockFreqAdjust(FreqHandler013, (UINTPTR)curClock);
    }
    return LOS_OK;
}

VOID ItLosTick013(VOID)
{
    TEST_ADD_CASE("ItLosTick013", TestCase, TEST_LOS, TEST_TICK, TEST_LEVEL1, TEST_FUNCTION);
}
