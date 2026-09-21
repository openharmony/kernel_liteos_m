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
 * 用例名称: ItPosixTime038
 * 用例描述: settimeofday 参数校验 + RTC 时区 hook(RtcSetTimezoneHook)路径
 *
 * 覆盖目标(lib/posix/src/time.c settimeofday :909-968):
 *   :913-916 tv == NULL 且 tz == NULL → -1 + EFAULT(参数校验 NULL)
 *   :918-921 tv->tv_usec >= 1000000 → -1 + EINVAL(settimeofday 自身校验行)
 *   tv->tv_usec == -1: 不触 :918(-1 < 1000000), 无 RtcSetTimeHook 时落
 *   :954-959 clock_settime → ValidTimeSpec(time_internal.h:80-88)拦负 nsec
 *   → clock_settime 置 EINVAL 后 settimeofday 透传 -1(errno 口径: EINVAL,
 *   校验实际发生在 clock_settime, 非本函数行)
 *   :923-939 tz 越界: tz_minuteswest > TIME_ZONE_MAX(720) 或 < TIME_ZONE_MIN(-840)
 *   → -1 + EINVAL(越界时不会触达 hook)
 *   :927-932(musl) 合法 tz 持锁写入 TIMEZONE(全局 timezone, 秒, 西正)
 *   :941-942 RtcSetTimezoneHook(TIMEZONE) 被调用, hook 收到"秒"口径的时区值
 *   签名(int settimeofday(const struct timeval *tv, const struct timezone *tz)):
 *   tv == NULL 只带合法 tz 是允许路径, 正好触达 :923-944 时区分支
 *
 * RTC hook 签名(lib/posix/include/rtc_time_hook.h 实测):
 *   INT32 RtcSetTimezoneHook(INT32 tz); 入参 tz 为"秒"(西正, 与全局 timezone 同口径)
 *
 * 断言口径:
 *   - tz_minuteswest = -480(UTC+8, 合法) → hook 收到 -480 * 60 == -28800 秒
 *   - 全局 timezone 变量被同步为 -28800(musl 分支 :931)
 *   - EXIT: 恢复 timezone 初值 + 注销 hook(全 NULL 成员结构体)
 */
#define _GNU_SOURCE /* musl: settimeofday/struct timezone 需要 _GNU_SOURCE 可见 */
#include "It_posix_time.h"
#include "rtc_time_hook.h"
#include "sys/time.h"

/* tzdst.c 定义的全局时区变量(秒, POSIX 西正: UTC+8 → -28800) */
extern long timezone;

/* 桩: 记录 RtcSetTimezoneHook 收到的时区值(秒口径) */
static INT32 g_tzHookRecv = 0;
static UINT32 g_tzHookCalls = 0;

static INT32 RtcSetTimezoneHookStub(INT32 tz)
{
    g_tzHookRecv = tz;
    g_tzHookCalls++;
    return 0;
}

static UINT32 Testcase(VOID)
{
    struct timeval tv = {0};
    struct timezone tz = {0};
    struct timezone tzRestore = {0};
    INT32 ret32;
    /* 保存初始时区, EXIT 恢复; 注销用全 NULL 成员结构体 */
    long tzSaved = timezone;
    struct RtcTimeHook cfgEmpty = {0};
    struct RtcTimeHook cfgTz = {0};

    cfgTz.RtcSetTimezoneHook = RtcSetTimezoneHookStub;

    /* 1. tv == NULL 且 tz == NULL → EFAULT(:913-916) */
    errno = 0;
    ret32 = settimeofday(NULL, NULL);
    ICUNIT_GOTO_EQUAL(ret32, -1, ret32, EXIT);
    ICUNIT_GOTO_EQUAL(errno, EFAULT, errno, EXIT);

    /* 2. tv_usec == 1000000 → EINVAL(settimeofday 自身校验 :918-921) */
    tv.tv_sec = 0;
    tv.tv_usec = 1000000; /* 1000000: 等于 1 秒, 越界 */
    errno = 0;
    ret32 = settimeofday(&tv, NULL);
    ICUNIT_GOTO_EQUAL(ret32, -1, ret32, EXIT);
    ICUNIT_GOTO_EQUAL(errno, EINVAL, errno, EXIT);

    /* 3. tv_usec == -1: :918 不拦, 无 RtcSetTimeHook 时透传 clock_settime,
     * ValidTimeSpec 拦负 nsec → -1 + EINVAL(时钟未被改动) */
    tv.tv_usec = -1;
    errno = 0;
    ret32 = settimeofday(&tv, NULL);
    ICUNIT_GOTO_EQUAL(ret32, -1, ret32, EXIT);
    ICUNIT_GOTO_EQUAL(errno, EINVAL, errno, EXIT);

    /* 4. tz 越界(hook 未注册, 即使注册了越界也不会触达 hook):
     * tz_minuteswest = 1000 > TIME_ZONE_MAX(720) → EINVAL(:936-938) */
    tz.tz_minuteswest = 1000; /* 1000: 超过 UTC-12:00 上限 720 分钟 */
    tz.tz_dsttime = 0;
    errno = 0;
    ret32 = settimeofday(NULL, &tz);
    ICUNIT_GOTO_EQUAL(ret32, -1, ret32, EXIT);
    ICUNIT_GOTO_EQUAL(errno, EINVAL, errno, EXIT);

    /* tz_minuteswest = -900 < TIME_ZONE_MIN(-840) → EINVAL(:936-938) */
    tz.tz_minuteswest = -900; /* -900: 超过 UTC+14:00 下限 -840 分钟 */
    errno = 0;
    ret32 = settimeofday(NULL, &tz);
    ICUNIT_GOTO_EQUAL(ret32, -1, ret32, EXIT);
    ICUNIT_GOTO_EQUAL(errno, EINVAL, errno, EXIT);

    /* 5. 注册 RtcSetTimezoneHook, tv == NULL 只带合法 tz:
     * UTC+8(-480 分钟) → :931 TIMEZONE = -480 * 60 = -28800,
     * :941-942 hook 收到秒口径 -28800 */
    LOS_RtcHookRegister(&cfgTz);
    tz.tz_minuteswest = -480; /* -480: UTC+8 */
    g_tzHookCalls = 0;
    errno = 0;
    ret32 = settimeofday(NULL, &tz);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT);
    ICUNIT_GOTO_EQUAL(g_tzHookCalls, 1, g_tzHookCalls, EXIT);
    ICUNIT_GOTO_EQUAL(g_tzHookRecv, -480 * 60, g_tzHookRecv, EXIT);
    ICUNIT_GOTO_EQUAL(timezone, -480 * 60, timezone, EXIT);

    /* 6. EXIT 恢复前验证: 通过 settimeofday 恢复初始时区(hook 再次收到原值) */
    tzRestore.tz_minuteswest = (int)(tzSaved / 60);
    tzRestore.tz_dsttime = 0;
    g_tzHookCalls = 0;
    ret32 = settimeofday(NULL, &tzRestore);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT);
    ICUNIT_GOTO_EQUAL(g_tzHookRecv, (INT32)tzSaved, g_tzHookRecv, EXIT);
    ICUNIT_GOTO_EQUAL(timezone, tzSaved, timezone, EXIT);

EXIT:
    /* 恢复: 先注销 RTC 时区 hook(OsEffectiveTimezone 不再走 hook), 再兜底恢复时区 */
    LOS_RtcHookRegister(&cfgEmpty);
    if (timezone != tzSaved) {
        tzRestore.tz_minuteswest = (int)(tzSaved / 60);
        tzRestore.tz_dsttime = 0;
        (VOID)settimeofday(NULL, &tzRestore);
    }
    return LOS_OK;
}

VOID ItPosixTime038(void)
{
    TEST_ADD_CASE("ItPosixTime038", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL0, TEST_FUNCTION);
}
