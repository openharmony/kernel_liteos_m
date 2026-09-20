/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * 用例名称: ItPosixTime037
 * 用例描述: time() 的 RTC hook 路径 + LOS_RtcHookRegister 注册/注销契约
 *
 * 覆盖目标(lib/posix/src/time.c):
 *   - time() :512-542, 重点 :518-530
 *     :519     RtcGetTimeHook(&usec) 成功(返回 0) → sec = usec / 1e6
 *     :527-528 timer != NULL → *timer = sec(与返回值一致)
 *     :520-523 hook 返回非 0 → 回退 GetCurrentTime()(g_rtcTimeBase + LOS_Tick2MS(tickDelta))
 *     :531-541 hook 注销后走 clock_gettime(CLOCK_REALTIME) 分支
 *   - LOS_RtcHookRegister :97-107
 *     :99-101  cfg == NULL 为 no-op(实测: 不能用 NULL 注销, 已注册 hook 保持不变)
 *     :102-106 成员整体覆盖式拷贝(注销需注册"全 NULL 成员"结构体)
 *   - settimeofday 的 RtcSetTimeHook 分支 :947-953(顺带覆盖, 配合失败回退分支
 *     给 g_rtcTimeBase 赋值; :949-950 msec/usec 双口径换算)
 *   - GetCurrentTime :498-510(无 RtcGetTickHook 时 tickDelta=0, sec = g_rtcTimeBase/1000)
 *
 * RTC hook 签名(lib/posix/include/rtc_time_hook.h 实测):
 *   INT32 RtcGetTimeHook(UINT64 *usec);  *usec 输出微秒, 返回 0 成功/非 0 失败
 *   INT32 RtcSetTimeHook(UINT64 msec, UINT64 *usec); 入参 msec=毫秒, *usec=微秒, 返回 0 成功
 *   struct RtcTimeHook 共 5 个成员, LOS_RtcHookRegister 一次性整体覆盖
 *
 * 断言口径:
 *   - 成功 hook 返回固定 usec = 1593561600 * 1000000(2020-07-01 00:00:00 UTC)
 *     → time(NULL) == 1593561600; time(&t) 时 *t == 返回值
 *   - 失败 hook(返回 1) + 先经 settimeofday hook 路径设 g_rtcTimeBase = 1600000000 * 1000(ms)
 *     → time() 回退分支 sec = g_rtcTimeBase / 1000 == 1600000000(精确断言, 无 GetTickHook)
 *   - EXIT 注销: 注册全 NULL 成员结构体(LOS_RtcHookRegister(NULL) 无法注销)
 */
#define _GNU_SOURCE /* musl: settimeofday 原型需要 _GNU_SOURCE 可见 */
#include "It_posix_time.h"
#include "rtc_time_hook.h"
#include "sys/time.h"

/* RTC 固定时间: 1593561600 = 2020-07-01 00:00:00 UTC, 微秒口径 */
#define RTC_TEST_USEC       (1593561600ULL * 1000000ULL)
#define RTC_TEST_SEC        1593561600
/* 回退分支基准时间: 1600000000 秒, 与 RTC 固定值区分开 */
#define RTC_FALLBACK_SEC    1600000000

/* 桩: 记录 RtcSetTimeHook 收到的双口径参数 */
static UINT64 g_setHookMsec = 0;
static UINT64 g_setHookUsec = 0;

/* 成功 hook: 输出固定微秒值 */
static INT32 RtcGetTimeOkHook(UINT64 *usec)
{
    *usec = RTC_TEST_USEC;
    return 0;
}

/* 失败 hook: 返回非 0, 触发 time() 的 GetCurrentTime 回退分支 */
static INT32 RtcGetTimeErrHook(UINT64 *usec)
{
    (VOID)usec;
    return 1;
}

/* 成功 hook: 记录 settimeofday 传入的 msec 与 usec 双口径参数 */
static INT32 RtcSetTimeOkHook(UINT64 msec, UINT64 *usec)
{
    g_setHookMsec = msec;
    g_setHookUsec = *usec;
    return 0;
}

static UINT32 Testcase(VOID)
{
    time_t timerVal = 0;
    time_t retSec;
    struct timeval tv = {0};
    INT32 ret32;
    /* 注销用: 全 NULL 成员结构体(实测 LOS_RtcHookRegister(NULL) 是 no-op, 无法注销) */
    struct RtcTimeHook cfgEmpty = {0};
    /* cfg1: 仅注册 GetTimeHook(成功桩) */
    struct RtcTimeHook cfgGetOk = {0};
    /* cfg2: 失败 GetTimeHook + 成功 SetTimeHook(用于给 g_rtcTimeBase 赋值) */
    struct RtcTimeHook cfgGetErr = {0};

    cfgGetOk.RtcGetTimeHook = RtcGetTimeOkHook;
    cfgGetErr.RtcGetTimeHook = RtcGetTimeErrHook;
    cfgGetErr.RtcSetTimeHook = RtcSetTimeOkHook;

    /* 1. 注册成功 GetTimeHook → time() 走 :519/:525, sec = usec / 1e6 */
    LOS_RtcHookRegister(&cfgGetOk);
    retSec = time(NULL);
    ICUNIT_GOTO_EQUAL(retSec, (time_t)RTC_TEST_SEC, retSec, EXIT);

    /* 2. time(&timer) → *timer == 返回值(:527-528) */
    timerVal = 0;
    retSec = time(&timerVal);
    ICUNIT_GOTO_EQUAL(retSec, (time_t)RTC_TEST_SEC, retSec, EXIT);
    ICUNIT_GOTO_EQUAL(timerVal, (time_t)RTC_TEST_SEC, timerVal, EXIT);

    /* 3. LOS_RtcHookRegister(NULL) 为 no-op(:99-101): hook 仍生效 */
    LOS_RtcHookRegister(NULL);
    retSec = time(NULL);
    ICUNIT_GOTO_EQUAL(retSec, (time_t)RTC_TEST_SEC, retSec, EXIT);

    /* 4. 注册失败 GetTimeHook + SetTimeHook(整体覆盖式注册):
     * settimeofday 合法值走 :947-953 hook 路径(不动系统时钟),
     * g_rtcTimeBase = tv_sec * 1000 + tv_usec / 1000(:949),
     * hook 收到 msec(毫秒)与 *usec(微秒)双口径(:949-950) */
    LOS_RtcHookRegister(&cfgGetErr);
    tv.tv_sec = RTC_FALLBACK_SEC;
    tv.tv_usec = 0;
    errno = 0;
    ret32 = settimeofday(&tv, NULL);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT);
    ICUNIT_GOTO_EQUAL(g_setHookMsec, (UINT64)RTC_FALLBACK_SEC * 1000, g_setHookMsec, EXIT);
    ICUNIT_GOTO_EQUAL(g_setHookUsec, (UINT64)RTC_FALLBACK_SEC * 1000000, g_setHookUsec, EXIT);

    /* 5. 失败 hook(返回 1) → time() 回退分支 :520-523:
     * 无 RtcGetTickHook → GetCurrentTime 的 tickDelta = 0(:503-509),
     * sec = g_rtcTimeBase / 1000 == RTC_FALLBACK_SEC(精确断言) */
    timerVal = 0;
    retSec = time(&timerVal);
    ICUNIT_GOTO_EQUAL(retSec, (time_t)RTC_FALLBACK_SEC, retSec, EXIT);
    ICUNIT_GOTO_EQUAL(timerVal, (time_t)RTC_FALLBACK_SEC, timerVal, EXIT);

    /* 6. 注销(全 NULL 成员结构体, :102-106 整体覆盖) → time() 走 :531-541
     * clock_gettime 分支; 步骤 4 未动系统时钟, 此处仅弱断言非 -1 */
    LOS_RtcHookRegister(&cfgEmpty);
    retSec = time(NULL);
    ICUNIT_GOTO_NOT_EQUAL(retSec, (time_t)-1, retSec, EXIT);

EXIT:
    /* 恢复: 确保所有 RTC hook 注销(幂等), 不影响后续用例 */
    LOS_RtcHookRegister(&cfgEmpty);
    return LOS_OK;
}

VOID ItPosixTime037(void)
{
    TEST_ADD_CASE("ItPosixTime037", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL0, TEST_FUNCTION);
}
