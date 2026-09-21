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

#define _GNU_SOURCE
#undef  _XOPEN_SOURCE
#define _XOPEN_SOURCE 600

#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#include "ohos_types.h"
#include "posix_test.h"
#include "los_config.h"
#include "securec.h"
#include "kernel_test.h"
#include "log.h"
#include "los_tick.h"
#include <unistd.h>
#if (LOSCFG_LIBC_MUSL == 1) && !defined(__LP64__)
#include "time64.h"
#endif

#define RET_OK 0

#define SECS_PER_MIN 60
#define SLEEP_ACCURACY 40000  // 40 ms tolerance: tick granularity (10ms) + FPU context switch overhead
#define ACCURACY_TEST_LOOPS 3 // loops for accuracy test, than count average value
#define MILLISECONDS_PER_SECOND 1000
#define NANOSECONDS_PER_MILLISECOND 1000000
#define TM_BASE_YEAR 1900
#define TIME_STR_LEN 100

#define INIT_TM(tmSt, year, mon, day, hour, min, sec, wday) \
    do {                                                    \
        (tmSt).tm_sec = (sec);                             \
        (tmSt).tm_min = (min);                             \
        (tmSt).tm_hour = (hour);                           \
        (tmSt).tm_mday = (day);                            \
        (tmSt).tm_mon = (mon);                             \
        (tmSt).tm_year = (year) - 1900;                      \
        (tmSt).tm_wday = wday;                                \
        (tmSt).__tm_gmtoff = 0;                               \
        (tmSt).__tm_zone = "";                              \
    } while (0)

/* *
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is CmsisTaskFuncTestSuite
 */
LITE_TEST_SUIT(Posix, PosixTime, PosixTimeFuncTestSuite);

/* *
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL PosixTimeFuncTestSuiteSetUp(void)
{
    return TRUE;
}

/* *
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL PosixTimeFuncTestSuiteTearDown(void)
{
    return TRUE;
}

static int KeepRun(int msec)
{
    struct timespec time1 = { 0, 0 };
    struct timespec time2 = { 0, 0 };
    clock_gettime(CLOCK_MONOTONIC, &time1);
    LOG("KeepRun start : tv_sec=%lld, tv_nsec=%ld\n", time1.tv_sec, time1.tv_nsec);
    int loop = 0;
    int ran = 0;
    while (ran < msec) {
        ++loop;
        clock_gettime(CLOCK_MONOTONIC, &time2);
        ran = (time2.tv_sec - time1.tv_sec) * MILLISECONDS_PER_SECOND;
        ran += (time2.tv_nsec - time1.tv_nsec) / NANOSECONDS_PER_MILLISECOND;
    }

    LOG("KeepRun end : tv_sec=%lld, tv_nsec=%ld\n", time2.tv_sec, time2.tv_nsec);
    return loop;
}

static int CheckValueClose(double target, double actual, double accuracy)
{
    double diff = actual - target;
    double pct;
    if (diff < 0) {
        diff = -diff;
    }
    if (actual == 0) {
        return 0;
    } else {
        pct = diff / actual;
    }
    return (pct <= accuracy);
}

static char *TmToStr(const struct tm *timePtr, char *timeStr, unsigned len)
{
    if (timePtr == NULL || timeStr == NULL) {
        return "";
    }
    (VOID)sprintf_s(timeStr, len, "%ld/%d/%d %02d:%02d:%02d WEEK(%d)", timePtr->tm_year + TM_BASE_YEAR,
        timePtr->tm_mon + 1, timePtr->tm_mday, timePtr->tm_hour, timePtr->tm_min, timePtr->tm_sec, timePtr->tm_wday);
    return timeStr;
}

#if !defined(LOSCFG_ARCH_FPU_DISABLE)
/* *
 * @tc.number SUB_KERNEL_TIME_USLEEP_001
 * @tc.name   usleep accuracy test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeUSleep001, Function | MediumTest | Level1)
{
    // wifiiot无法支持10ms以下的sleep
    int interval[] = {15*1000, 20*1000, 30*1000, 300*1000};
    for (unsigned int j = 0; j < sizeof(interval) / sizeof(int); j++) {
        LOG("\ntest interval:%d\n", interval[j]);
        struct timespec time1 = { 0 }, time2 = { 0 };
        long duration; // unit: us
        double d = 0.0;
        for (int i = 1; i <= ACCURACY_TEST_LOOPS; i++) {
            clock_gettime(CLOCK_MONOTONIC, &time1);
            int rt = usleep(interval[j]);
            clock_gettime(CLOCK_MONOTONIC, &time2);
            ICUNIT_ASSERT_EQUAL(rt, RET_OK, rt);
            duration = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_nsec - time1.tv_nsec) / 1000;
            LOG("testloop %d, actual usleep duration: %ld us\n", i, duration);
            d += duration;
        }
        d = d / ACCURACY_TEST_LOOPS; // average
        LOG("interval:%u, average duration: %.2f\n", interval[j], d);
        ICUNIT_ASSERT_WITHIN_EQUAL(d, interval[j], d, 0);
        ICUNIT_ASSERT_WITHIN_EQUAL(d, interval[j] - SLEEP_ACCURACY, interval[j] + SLEEP_ACCURACY, d);
    }
    return 0;
}
#endif

/* *
 * @tc.number SUB_KERNEL_TIME_USLEEP_002
 * @tc.name   usleep test for special delay
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeUSleep002, Function | MediumTest | Level1)
{
    struct timespec time1 = { 0 };
    struct timespec time2 = { 0 };
    clock_gettime(CLOCK_MONOTONIC, &time1);
    int rt = usleep(0);
    clock_gettime(CLOCK_MONOTONIC, &time2);
    ICUNIT_ASSERT_EQUAL(rt, RET_OK, rt);
    long long duration = (time2.tv_sec - time1.tv_sec) * 1000000 + (time2.tv_nsec - time1.tv_nsec) / 1000;
    LOG("\n usleep(0), actual usleep duration: %lld us\n", duration);
    ICUNIT_ASSERT_WITHIN_EQUAL(duration, duration, (2 * LOS_SYS_US_PER_SECOND / LOSCFG_BASE_CORE_TICK_PER_SECOND), 0);
    return 0;
}

/* *
 * @tc.number     SUB_KERNEL_TIME_GMTIME_001
 * @tc.name       test gmtime api
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeGmtime001, Function | MediumTest | Level1)
{
    time_t time1 = 18880;
    char timeStr[TIME_STR_LEN] = {0};
    LOG("\nsizeof(time_t) = %d, sizeof(struct tm) = %d", sizeof(time_t), sizeof(struct tm));
    struct tm *timePtr = gmtime(&time1);
    ICUNIT_ASSERT_STRING_EQUAL("1970/1/1 05:14:40 WEEK(4)", TmToStr(timePtr, timeStr, TIME_STR_LEN), 0);

    time1 = LONG_MAX;
    timePtr = gmtime(&time1);
    LOG("\n LONG_MAX = %lld, cvt result : %s", time1, TmToStr(timePtr, timeStr, TIME_STR_LEN));
    ICUNIT_ASSERT_STRING_EQUAL("2038/1/19 03:14:07 WEEK(2)", TmToStr(timePtr, timeStr, TIME_STR_LEN), 0);

    time1 = LONG_MAX - 1;
    timePtr = gmtime(&time1);
    LOG("\n LONG_MAX - 1 = %lld, cvt result : %s", time1, TmToStr(timePtr, timeStr, TIME_STR_LEN));
    ICUNIT_ASSERT_STRING_EQUAL("2038/1/19 03:14:06 WEEK(2)", TmToStr(timePtr, timeStr, TIME_STR_LEN), 0);

    time1 = LONG_MIN;
    timePtr = gmtime(&time1);
    LOG("\n LONG_MIN  = %lld, cvt result : %s", time1, TmToStr(timePtr, timeStr, TIME_STR_LEN));
    ICUNIT_ASSERT_STRING_EQUAL("1901/12/13 20:45:52 WEEK(5)", TmToStr(timePtr, timeStr, TIME_STR_LEN), 0);

    time1 = LONG_MIN + 1;
    timePtr = gmtime(&time1);
    LOG("\n LONG_MIN + 1  = %lld, cvt result : %s", time1, TmToStr(timePtr, timeStr, TIME_STR_LEN));
    ICUNIT_ASSERT_STRING_EQUAL("1901/12/13 20:45:53 WEEK(5)", TmToStr(timePtr, timeStr, TIME_STR_LEN), 0);
    return 0;
};

#if (LOSCFG_LIBC_MUSL == 1)
/* *
 * @tc.number     SUB_KERNEL_TIME_LOCALTIME_001
 * @tc.name       test localtime api
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeLocaltime001, Function | MediumTest | Level1)
{
    char cTime[32];
    time_t tStart;
    time_t tEnd;

    struct timeval timeSet = {
        .tv_sec = 86399,
        .tv_usec = 0
    };

    int ret = settimeofday(&timeSet, NULL);
    time(&tStart);
    sleep(2);
    time(&tEnd);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    struct tm *tmStart = localtime(&tStart);
    strftime(cTime, sizeof(cTime), "%H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL(cTime, "07:59:59", 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);
    struct tm *tmEnd = localtime(&tEnd);
    strftime(cTime, sizeof(cTime), "%H:%M:%S", tmEnd);
    ICUNIT_ASSERT_STRING_EQUAL(cTime, "08:00:01", 0);
    LOG("\n time_t=%lld, first time:%s", tEnd, cTime);
    return 0;
}

/* *
 * @tc.number     SUB_KERNEL_TIME_LOCALTIME_002
 * @tc.name       test localtime api
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeLocaltime002, Function | MediumTest | Level1)
{
    char cTime[32];
    time_t tStart = LONG_MAX;
    struct tm *tmStart = localtime(&tStart);
    strftime(cTime, sizeof(cTime), "%y-%m-%d %H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL(cTime, "38-01-19 11:14:07", 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);

    tStart = LONG_MIN;
    tmStart = localtime(&tStart);
    strftime(cTime, sizeof(cTime), "%y-%m-%d %H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL(cTime, "01-12-14 04:45:52", 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);

    tStart = 0;
    tmStart = localtime(&tStart);
    strftime(cTime, sizeof(cTime), "%y-%m-%d %H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL(cTime, "70-01-01 08:00:00", 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);

    tStart = -1;
    tmStart = localtime(&tStart);
    strftime(cTime, sizeof(cTime), "%y-%m-%d %H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL(cTime, "70-01-01 07:59:59", 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);
    return 0;
}

/* *
 * @tc.number     SUB_KERNEL_TIME_LOCALTIME_003
 * @tc.name       test settimeofday api
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeLocaltime003, Function | MediumTest | Level1)
{
    char cTime[32]; /* 32, no special meaning */
    time_t tStart;
    time_t tEnd;
    struct timezone tz;
    struct timeval timeSet = {
        .tv_sec = 86399,    /* 86399, no special meaning */
        .tv_usec = 0
    };

    int ret = gettimeofday(NULL, &tz);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = settimeofday(&timeSet, &tz);
    time(&tStart);
    sleep(2);   /* 2, sleep time */
    time(&tEnd);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    struct tm *tmStart = localtime(&tStart);
    strftime(cTime, sizeof(cTime), "%H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL(cTime, "07:59:59", 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);
    struct tm *tmEnd = localtime(&tEnd);
    strftime(cTime, sizeof(cTime), "%H:%M:%S", tmEnd);
    ICUNIT_ASSERT_STRING_EQUAL(cTime, "08:00:01", 0);
    LOG("\n time_t=%lld, first time:%s", tEnd, cTime);
    return 0;
}

/* *
 * @tc.number     SUB_KERNEL_TIME_LOCALTIMER_001
 * @tc.name       localtime_r api base test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeLocaltimer001, Function | MediumTest | Level1)
{
    char cTime[32];
    time_t tStart;
    time_t tEnd;
    struct tm tmrStart = { 0 };
    struct tm tmrEnd = { 0 };
    struct timeval tSet = {
        .tv_sec = 86399,
        .tv_usec = 0
    };

    int ret = settimeofday(&tSet, NULL);
    time(&tStart);
    sleep(2);
    time(&tEnd);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    struct tm *tmrStartPtr = localtime_r(&tStart, &tmrStart);
    struct tm *tmrEndPtr = localtime_r(&tEnd, &tmrEnd);

    strftime(cTime, sizeof(cTime), "%H:%M:%S", &tmrStart);
    ICUNIT_ASSERT_STRING_EQUAL("07:59:59", cTime, 0);
    strftime(cTime, sizeof(cTime), "%H:%M:%S", tmrStartPtr);
    ICUNIT_ASSERT_STRING_EQUAL("07:59:59", cTime, 0);
    strftime(cTime, sizeof(cTime), "%H:%M:%S", &tmrEnd);
    ICUNIT_ASSERT_STRING_EQUAL("08:00:01", cTime, 0);
    strftime(cTime, sizeof(cTime), "%H:%M:%S", tmrEndPtr);
    ICUNIT_ASSERT_STRING_EQUAL("08:00:01", cTime, 0);
    return 0;
}

/* *
 * @tc.number     SUB_KERNEL_TIME_LOCALTIMER_002
 * @tc.name       test localtime_r api for range
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeLocaltimer002, Function | MediumTest | Level1)
{
    char cTime[32];
    struct tm tmrResult = { 0 };

    time_t tStart = LONG_MAX;
    struct tm *tmStart = localtime_r(&tStart, &tmrResult);
    strftime(cTime, sizeof(cTime), "%y-%m-%d %H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL("38-01-19 11:14:07", cTime, 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);

    tStart = LONG_MIN;
    tmStart = localtime_r(&tStart, &tmrResult);
    strftime(cTime, sizeof(cTime), "%y-%m-%d %H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL("01-12-14 04:45:52", cTime, 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);

    tStart = 0;
    tmStart = localtime_r(&tStart, &tmrResult);
    strftime(cTime, sizeof(cTime), "%y-%m-%d %H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL("70-01-01 08:00:00", cTime, 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);

    tStart = -1;
    tmStart = localtime_r(&tStart, &tmrResult);
    strftime(cTime, sizeof(cTime), "%y-%m-%d %H:%M:%S", tmStart);
    ICUNIT_ASSERT_STRING_EQUAL("70-01-01 07:59:59", cTime, 0);
    LOG("\n time_t=%lld, first time:%s", tStart, cTime);
    return 0;
}
#endif

/* *
 * @tc.number     SUB_KERNEL_TIME_MKTIME_001
 * @tc.name       mktime api base test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeMktime001, Function | MediumTest | Level1)
{
    struct tm testTM = { 0 };
    time_t testTime = 18880;
    char timeStr[TIME_STR_LEN] = {0};
    struct timeval tv;
    struct timezone tz;

    // get system timezone
    int ret = gettimeofday(&tv, &tz);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    long sysTimezone = (long)(-tz.tz_minuteswest) * SECS_PER_MIN;
    LOG("\n system timezone = %ld\n", sysTimezone);

    INIT_TM(testTM, 2020, 7, 9, 18, 10, 0, 7);
    time_t timeRet = mktime(&testTM);
    LOG("\n 2020-7-9 18:10:00, mktime Ret = %lld", timeRet);
    ICUNIT_ASSERT_EQUAL(sysTimezone, testTM.__tm_gmtoff, 0);
    ICUNIT_ASSERT_EQUAL(1596996600 - testTM.__tm_gmtoff, timeRet, 0);

    INIT_TM(testTM, 1970, 0, 1, 8, 0, 0, 0);
    timeRet = mktime(&testTM);
    LOG("\n 1970-1-1 08:00:00, mktime Ret = %lld", timeRet);
    ICUNIT_ASSERT_EQUAL(sysTimezone, testTM.__tm_gmtoff, 0);
    ICUNIT_ASSERT_EQUAL(28800 - testTM.__tm_gmtoff, timeRet, 0);

    struct tm *timePtr = localtime(&testTime);
    LOG("\n testTime 18880, tm : %s", TmToStr(timePtr, timeStr, TIME_STR_LEN));
    timeRet = mktime(timePtr);
    ICUNIT_ASSERT_EQUAL(timeRet, 18880, timeRet);
    LOG("\n input 18880, mktime Ret = %lld", timeRet);

    testTime = LONG_MAX;
    timePtr = localtime(&testTime);
    LOG("\n testTime LONG_MAX, tm : %s", TmToStr(timePtr, timeStr, TIME_STR_LEN));
    timeRet = mktime(timePtr);
    ICUNIT_ASSERT_EQUAL(timeRet, LONG_MAX, timeRet);
    LOG("\n input LONG_MAX, mktime Ret = %lld", timeRet);

    testTime = 0;
    timePtr = localtime(&testTime);
    LOG("\n testTime 0, tm : %s", TmToStr(timePtr, timeStr, TIME_STR_LEN));
    timeRet = mktime(timePtr);
    ICUNIT_ASSERT_EQUAL(timeRet, 0, timeRet);
    LOG("\n input 0, mktime Ret = %lld", timeRet);
    return 0;
}

/* *
 * @tc.number     SUB_KERNEL_TIME_MKTIME_002
 * @tc.name       mktime api test for invalid input
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeMktime002, Function | MediumTest | Level1)
{
    struct tm testTM = { 0 };
    LOG("\n sizeof(time_t) = %d", sizeof(time_t));
    INIT_TM(testTM, 1969, 7, 9, 10, 10, 0, 7);
    time_t timeRet = mktime(&testTM);
    LOG("\n 1800-8-9 10:10:00, mktime Ret lld = %lld", timeRet);
#if (LOSCFG_LIBC_MUSL == 1)
    /* since the fbb sync, mktime normalizes pre-epoch years (POSIX behavior) */
    ICUNIT_ASSERT_EQUAL(timeRet, -12520200, timeRet);
#endif
#if (LOSCFG_LIBC_NEWLIB == 1)
    ICUNIT_ASSERT_WITHIN_EQUAL(timeRet, timeRet, -1, 0);
#endif
    return 0;
}

#if (LOSCFG_LIBC_MUSL == 1)
/* *
 * @tc.number     SUB_KERNEL_TIME_STRFTIME_001
 * @tc.name       test strftime api
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeStrftime001, Function | MediumTest | Level3)
{
    char buffer[80];
    time_t mtime = 0;
    size_t ftime = 0;

    mtime = LONG_MAX;
    ftime = strftime(buffer, 80, "%y-%m-%d %H:%M:%S", localtime(&mtime));
    ICUNIT_ASSERT_WITHIN_EQUAL(ftime, 1, ftime, 0);
    ICUNIT_ASSERT_STRING_EQUAL("38-01-19 11:14:07", buffer, 0);
    LOG("\nresult: %s, expected : %s", buffer, "38-01-19 11:14:07");

    mtime = LONG_MIN;
    ftime = strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&mtime));
    ICUNIT_ASSERT_WITHIN_EQUAL(ftime, 1, ftime, 0);
    ICUNIT_ASSERT_STRING_EQUAL("1901-12-14 04:45:52", buffer, 0);
    LOG("\nresult: %s, expected : %s", buffer, "1901-12-14 04:45:52");

    mtime = 18880;
    ftime = strftime(buffer, 80, "%F %T", localtime(&mtime));
    ICUNIT_ASSERT_WITHIN_EQUAL(ftime, 1, ftime, 0);
    ICUNIT_ASSERT_STRING_EQUAL("1970-01-01 13:14:40", buffer, 0);
    LOG("\nresult: %s, expected : %s", buffer, "1970-01-01 13:14:40");

    mtime = 18880;
    ftime = strftime(buffer, 80, "%D %w %H:%M:%S", localtime(&mtime));
    ICUNIT_ASSERT_WITHIN_EQUAL(ftime, 1, ftime, 0);
    ICUNIT_ASSERT_STRING_EQUAL("01/01/70 4 13:14:40", buffer, 0);
    LOG("\nresult: %s, expected : %s", buffer, "01/01/70 4 13:14:40");
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_STRFTIME_002
 * @tc.name       test strftime api base case
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeStrftime002, Function | MediumTest | Level3)
{
    char buffer[80];
    time_t mtime = 18880;
    size_t ftime = 0;
    struct tm *tmTime = localtime(&mtime);

    ftime = strftime(buffer, 80, "%Ex %EX %A", tmTime);
    ICUNIT_ASSERT_WITHIN_EQUAL(ftime, 1, ftime, 0);
    ICUNIT_ASSERT_STRING_EQUAL("01/01/70 13:14:40 Thursday", buffer, 0);
    LOG("\nresult: %s, expected : %s", buffer, "01/01/70 13:14:40 Thursday");

    ftime = strftime(buffer, 80, "%x %X", tmTime);
    ICUNIT_ASSERT_WITHIN_EQUAL(ftime, 1, ftime, 0);
    ICUNIT_ASSERT_STRING_EQUAL("01/01/70 13:14:40", buffer, 0);
    LOG("\nresult: %s, expected : %s", buffer, "01/01/70 13:14:40");

    ftime = strftime(buffer, 80, "%D %A %H:%M:%S", tmTime);
    ICUNIT_ASSERT_WITHIN_EQUAL(ftime, 1, ftime, 0);
    ICUNIT_ASSERT_STRING_EQUAL("01/01/70 Thursday 13:14:40", buffer, 0);
    LOG("\nresult: %s, expected : %s", buffer, "01/01/70 Thursday 13:14:40");
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_STRFTIME_003
 * @tc.name       test strftime api for abnormal input
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeStrftime003, Function | MediumTest | Level3)
{
    char buffer[80];
    time_t mtime = 18880;
    size_t ftime = 0;
    struct tm *tmTime = localtime(&mtime);

    ftime = strftime(buffer, 12, "%Y-%m-%d %H:%M:%S", tmTime);
    ICUNIT_ASSERT_EQUAL(ftime, 0, ftime);
    LOG("\nresult: %s, expected : %s", buffer, "1970-01-01 13:14:40");

    ftime = strftime(buffer, 80, "", tmTime);
    ICUNIT_ASSERT_EQUAL(ftime, 0, ftime);
    LOG("\nresult: %s, expected : %s", buffer, "1970-01-01 13:14:40");

    ftime = strftime(buffer, 19, "%Y-%m-%d %H:%M:%S", tmTime);
    ICUNIT_ASSERT_EQUAL(ftime, 0, ftime);
    LOG("\nresult: %s, expected : %s", buffer, "1970-01-01 13:14:40");

    ftime = strftime(buffer, 20, "%Y-%m-%d %H:%M:%S", tmTime);
    ICUNIT_ASSERT_EQUAL(ftime, 19, ftime);
    ICUNIT_ASSERT_STRING_EQUAL("1970-01-01 13:14:40", buffer, 0);
    LOG("\nresult: %s, expected : %s", buffer, "1970-01-01 13:14:40");

    tmTime->__tm_zone = "UTC+8";
    ftime = strftime(buffer, 80, "%F %T %Z", tmTime);
    ICUNIT_ASSERT_EQUAL(ftime, 20, ftime);
    LOG("\nresult: %s, expected : %s", buffer, "1970-01-01 13:14:40");
    return 0;
};
#endif

/* *
 * @tc.number SUB_KERNEL_TIME_API_TIMES_0100
 * @tc.name   test times basic
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimes, Function | MediumTest | Level1)
{
    const int testClockt = LOSCFG_BASE_CORE_TICK_PER_SECOND;
    const int msPerClock = OS_SYS_MS_PER_SECOND / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    struct tms start = { 0 };
    struct tms end = { 0 };
    clock_t stTime = times(&start);
    LOG("start_clock: stTime: %ld", stTime);
    LOG("start_clock: tms_utime: %ld, tms_stime: %ld, tms_cutime:%ld, tms_cstime:%ld", start.tms_utime,
        start.tms_stime, start.tms_cutime, start.tms_cstime);

    KeepRun(testClockt * msPerClock);

    clock_t endTime = times(&end);
    LOG("end_clock: endTime: %ld", endTime);
    LOG("end_clock: tms_utime: %ld, tms_stime: %ld, tms_cutime:%ld, tms_cstime:%ld", end.tms_utime, end.tms_stime,
        end.tms_cutime, end.tms_cstime);

    LOG("Real Time: %ld, User Time %ld, System Time %ld\n", (long)(endTime - stTime),
        (long)(end.tms_utime - start.tms_utime), (long)(end.tms_stime - start.tms_stime));

    if (!CheckValueClose((end.tms_utime - start.tms_utime), testClockt, 0.1)) {
        ICUNIT_ASSERT_EQUAL(1, 0, 0);
    }
    if (!CheckValueClose((endTime - stTime), testClockt, 0.1)) {
        ICUNIT_ASSERT_EQUAL(1, 0, 0);
    }
    return 0;
}

#if (LOSCFG_LIBC_MUSL == 1) && !defined(__LP64__)
/* *
 * @tc.number     SUB_KERNEL_TIME_LOCALTIME64_001
 * @tc.name       localtime64 api base test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeLocaltime64001, Function | MediumTest | Level1)
{
    /* 非 _r 变体与 _r 变体一致性：同输入下 localtime64(t) 与 localtime64_r(t, &tm) 字段逐项一致
     * （_r 变体的值正确性已由 sample 侧 It_posix_time_032 深覆盖，此处仅校验两变体输出一致） */
    time64_t timeVals[] = {0, (time64_t)1593561600LL, (time64_t)2147483648LL};
    struct tm tmResult = { 0 };
    struct tm *ret = NULL;
    struct tm *ptr = NULL;
    const size_t count = sizeof(timeVals) / sizeof(timeVals[0]);

    /* NULL 入参：两变体均返回 NULL */
    ICUNIT_ASSERT_EQUAL(localtime64(NULL), NULL, 0);

    for (size_t i = 0; i < count; i++) {
        /* 先调 _r 存入用户缓冲区，再调非 _r（内部使用静态 g_returnDate），互不干扰 */
        ret = localtime64_r(&timeVals[i], &tmResult);
        ICUNIT_ASSERT_EQUAL(ret, &tmResult, 0);
        ptr = localtime64(&timeVals[i]);
        ICUNIT_ASSERT_EQUAL((ptr != NULL), 1, (UINTPTR)ptr);

        ICUNIT_ASSERT_EQUAL(ptr->tm_sec, tmResult.tm_sec, ptr->tm_sec);
        ICUNIT_ASSERT_EQUAL(ptr->tm_min, tmResult.tm_min, ptr->tm_min);
        ICUNIT_ASSERT_EQUAL(ptr->tm_hour, tmResult.tm_hour, ptr->tm_hour);
        ICUNIT_ASSERT_EQUAL(ptr->tm_mday, tmResult.tm_mday, ptr->tm_mday);
        ICUNIT_ASSERT_EQUAL(ptr->tm_mon, tmResult.tm_mon, ptr->tm_mon);
        ICUNIT_ASSERT_EQUAL(ptr->tm_year, tmResult.tm_year, ptr->tm_year);
        ICUNIT_ASSERT_EQUAL(ptr->tm_wday, tmResult.tm_wday, ptr->tm_wday);
        ICUNIT_ASSERT_EQUAL(ptr->tm_yday, tmResult.tm_yday, ptr->tm_yday);
        ICUNIT_ASSERT_EQUAL(ptr->tm_isdst, tmResult.tm_isdst, ptr->tm_isdst);
        ICUNIT_ASSERT_EQUAL(ptr->__tm_gmtoff, tmResult.__tm_gmtoff, ptr->__tm_gmtoff);
    }
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_GMTIME64_001
 * @tc.name       gmtime64 api base test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeGmtime64001, Function | MediumTest | Level1)
{
    /* 非 _r 变体与 _r 变体一致性：同输入下 gmtime64(t) 与 gmtime64_r(t, &tm) 字段逐项一致 */
    time64_t timeVals[] = {0, (time64_t)1593561600LL, (time64_t)2147483648LL};
    struct tm tmResult = { 0 };
    struct tm *ret = NULL;
    struct tm *ptr = NULL;
    const size_t count = sizeof(timeVals) / sizeof(timeVals[0]);

    /* NULL 入参：两变体均返回 NULL */
    ICUNIT_ASSERT_EQUAL(gmtime64(NULL), NULL, 0);

    for (size_t i = 0; i < count; i++) {
        ret = gmtime64_r(&timeVals[i], &tmResult);
        ICUNIT_ASSERT_EQUAL(ret, &tmResult, 0);
        ptr = gmtime64(&timeVals[i]);
        ICUNIT_ASSERT_EQUAL((ptr != NULL), 1, (UINTPTR)ptr);

        ICUNIT_ASSERT_EQUAL(ptr->tm_sec, tmResult.tm_sec, ptr->tm_sec);
        ICUNIT_ASSERT_EQUAL(ptr->tm_min, tmResult.tm_min, ptr->tm_min);
        ICUNIT_ASSERT_EQUAL(ptr->tm_hour, tmResult.tm_hour, ptr->tm_hour);
        ICUNIT_ASSERT_EQUAL(ptr->tm_mday, tmResult.tm_mday, ptr->tm_mday);
        ICUNIT_ASSERT_EQUAL(ptr->tm_mon, tmResult.tm_mon, ptr->tm_mon);
        ICUNIT_ASSERT_EQUAL(ptr->tm_year, tmResult.tm_year, ptr->tm_year);
        ICUNIT_ASSERT_EQUAL(ptr->tm_wday, tmResult.tm_wday, ptr->tm_wday);
        ICUNIT_ASSERT_EQUAL(ptr->tm_yday, tmResult.tm_yday, ptr->tm_yday);
        ICUNIT_ASSERT_EQUAL(ptr->tm_isdst, tmResult.tm_isdst, ptr->tm_isdst);
        ICUNIT_ASSERT_EQUAL(ptr->__tm_gmtoff, tmResult.__tm_gmtoff, ptr->__tm_gmtoff);
    }
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_ASCTIME64_001
 * @tc.name       asctime64 api base test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeAsctime64001, Function | MediumTest | Level1)
{
    /* 非 _r 变体与 _r 变体一致性：同输入下 asctime64(&tm) 与 asctime64_r(&tm, buf) 输出字符串一致 */
    struct tm tmDate = { 0 };
    char bufR[TIME_STR_LEN] = { 0 };
    char *ret = NULL;
    char *retR = NULL;

    /* 2020-07-01 12:30:45 周三 */
    tmDate.tm_year = 120; /* 1900 + 120 = 2020 */
    tmDate.tm_mon = 6;    /* 月份从 0 起：6 表示 7 月 */
    tmDate.tm_mday = 1;
    tmDate.tm_hour = 12;
    tmDate.tm_min = 30;
    tmDate.tm_sec = 45;
    tmDate.tm_wday = 3;   /* 周三 */

    retR = asctime64_r(&tmDate, bufR);
    ICUNIT_ASSERT_EQUAL((retR != NULL), 1, (UINTPTR)retR);
    ret = asctime64(&tmDate);
    ICUNIT_ASSERT_EQUAL((ret != NULL), 1, (UINTPTR)ret);

    /* 两变体输出一致，且符合 asctime 标准格式 */
    ICUNIT_ASSERT_STRING_EQUAL(ret, bufR, 0);
    ICUNIT_ASSERT_STRING_EQUAL(ret, "Wed Jul  1 12:30:45 2020\n", 0);

    /* NULL 入参：两变体均返回 NULL */
    ICUNIT_ASSERT_EQUAL(asctime64(NULL), NULL, 0);
    ICUNIT_ASSERT_EQUAL(asctime64_r(NULL, bufR), NULL, 0);
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_CTIME64_001
 * @tc.name       ctime64 api base test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeCtime64001, Function | MediumTest | Level1)
{
    /* 用例简要描述：ctime64 正路径固定点精确断言 + 与 ctime64_r 一致性 + NULL/INT64_MAX 越界异常路径 */
    time64_t timeVals[] = {(time64_t)0, (time64_t)1593561600LL};
    /* 0: epoch 零点; 1593561600: 2020-07-01 00:00:00 UTC */
    time64_t tMax = (time64_t)0x7FFFFFFFFFFFFFFFLL; /* 即 INT64_MAX */
    char bufR[TIME_STR_LEN] = { 0 };
    char *ret = NULL;
    char *retR = NULL;
    int result;

    /* NULL 入参：ctime64 内部 localtime64(NULL) 返回 NULL，整体返回 NULL */
    ICUNIT_ASSERT_EQUAL(ctime64(NULL), NULL, 0);

    /* INT64_MAX 越界：localtime64_r 年份超限检查（time64.c EOVERFLOW 分支）返回 NULL 且置 EOVERFLOW */
    errno = 0;
    ICUNIT_ASSERT_EQUAL(ctime64(&tMax), NULL, 0);
    ICUNIT_ASSERT_EQUAL(errno, EOVERFLOW, errno);

    /* 固定点精确断言。时区口径：系统默认 UTC+8（tzdst.c TZ_DEFAULT_OFFSET=-28800），
     * 与本文件 testTimeLocaltime002 断言 localtime(0)=="70-01-01 08:00:00" 同口径实测；
     * 2020-07 点默认无 DST（testTimeMktime001 的 2020-08 断言不含 DST 偏移背书） */
    ret = ctime64(&timeVals[0]);
    ICUNIT_ASSERT_EQUAL((ret != NULL), 1, (UINTPTR)ret);
    result = strcmp(ret, "Thu Jan  1 08:00:00 1970\n");
    ICUNIT_ASSERT_EQUAL(result, 0, result);

    ret = ctime64(&timeVals[1]);
    ICUNIT_ASSERT_EQUAL((ret != NULL), 1, (UINTPTR)ret);
    result = strcmp(ret, "Wed Jul  1 08:00:00 2020\n");
    ICUNIT_ASSERT_EQUAL(result, 0, result);

    /* 一致性兜底：同输入下 ctime64 与 ctime64_r 输出串一致（不依赖时区取值） */
    for (size_t i = 0; i < sizeof(timeVals) / sizeof(timeVals[0]); i++) {
        retR = ctime64_r(&timeVals[i], bufR);
        ICUNIT_ASSERT_EQUAL((retR != NULL), 1, (UINTPTR)retR);
        ret = ctime64(&timeVals[i]);
        ICUNIT_ASSERT_EQUAL((ret != NULL), 1, (UINTPTR)ret);
        result = strcmp(ret, retR);
        ICUNIT_ASSERT_EQUAL(result, 0, result);
    }
    return 0;
};
#endif

#if (LOSCFG_LIBC_MUSL == 1)
/* 保存当前系统时区并临时切换为 UTC（等价于 musl 测试 putenv("TZ=GMT")+tzset()：
 * morpheus 内核态无环境变量，用 settimeofday 仅改时区不改时间），返回 0 表示成功 */
static int TimeTzSwitchToUtc(struct timezone *tzOrig)
{
    struct timezone tzUtc = { .tz_minuteswest = 0, .tz_dsttime = 0 };

    if (gettimeofday(NULL, tzOrig) != 0) {
        return -1;
    }
    return settimeofday(NULL, &tzUtc);
}

/* 恢复原系统时区 */
static void TimeTzRestore(const struct timezone *tzOrig)
{
    (VOID)settimeofday(NULL, tzOrig);
}

/* *
 * @tc.number     SUB_KERNEL_TIME_GMTIME_MKTIME_LOOP_001
 * @tc.name       gmtime to mktime roundtrip sampling test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeGmtimeMktimeLoop001, Function | MediumTest | Level1)
{
    /* 用例简要描述：移植 musl libc-test src/functional/time.c:48-66(sec2tm)/:155-157 往返抽样并精简为约 50 点：
     * 时区切 UTC 后 gmtime_r(t) 填 tm（isdst 已为 0，满足 mktime 输入要求）→ mktime 回，断言往返相等 */
    struct timezone tzOrig = { 0 };
    struct tm tmResult = { 0 };
    time_t tBad = 0;
    time_t t2Bad = 0;
    int failed = 0;
    int rt;

    rt = TimeTzSwitchToUtc(&tzOrig);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);

    /* 抽样点 t = (1 + i*430) * 100003 + 1（musl 原版 t*100003 系数抽样 +1 偏移），
     * i = 0..49 共 50 点，最大点约 2.1e9 < 2^31，防止 32 位 time_t 下 mktime 溢出 */
    for (int i = 0; i < 50; i++) {
        time_t t = (time_t)(1 + i * 430) * (time_t)100003 + 1;
        struct tm *ptr = gmtime_r(&t, &tmResult);
        if (ptr == NULL) {
            tBad = t;
            t2Bad = (time_t)-1; /* 标记 gmtime_r 本身失败 */
            failed = 1;
            break;
        }
        time_t t2 = mktime(&tmResult);
        if (t2 != t) {
            tBad = t;
            t2Bad = t2;
            failed = 1;
            break;
        }
    }

    /* 先恢复时区再断言，保证断言失败提前 return 时全局时区不被污染 */
    TimeTzRestore(&tzOrig);
    if (failed) {
        LOG("\n roundtrip failed: t=%lld, mktime(gmtime(t))=%lld\n", (long long)tBad, (long long)t2Bad);
        ICUNIT_ASSERT_EQUAL(t2Bad, tBad, t2Bad);
    }
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_MKTIME_GMTIME_REV_001
 * @tc.name       mktime to gmtime reverse roundtrip test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeMktimeGmtimeRev001, Function | MediumTest | Level1)
{
    /* 用例简要描述：移植 musl libc-test src/functional/time.c:68-92(tm2sec) 思路并精简：
     * 时区切 UTC 后取 5 个代表点，gmtime_r 得 tm → mktime 回 t2 断言相等 →
     * 再 gmtime_r(t2) 与首次 gmtime 输出做九字段逐项全等（含 wday/yday/isdst） */
    time_t timeVals[] = {0, 86400, 1000000000, 1593561600, 2147483647};
    /* 0: epoch; 86400: 1970-01-02; 1000000000: 2001-09-09; 1593561600: 2020-07-01; 2147483647: 2038 边界 */
    struct timezone tzOrig = { 0 };
    struct tm tmFirst = { 0 };
    struct tm tmSecond = { 0 };
    const int count = sizeof(timeVals) / sizeof(timeVals[0]);
    int badIdx = -1;
    long long t2Bad = 0;
    int rt;

    rt = TimeTzSwitchToUtc(&tzOrig);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);

    for (int i = 0; i < count; i++) {
        struct tm *ptr = gmtime_r(&timeVals[i], &tmFirst);
        if (ptr == NULL) {
            badIdx = i;
            break;
        }
        /* mktime 会规格化回写 tmFirst，先保存首次 gmtime 输出用于九字段比对 */
        struct tm tmSave = tmFirst;
        time_t t2 = mktime(&tmFirst);
        if (t2 != timeVals[i]) {
            badIdx = i;
            t2Bad = (long long)t2;
            break;
        }
        ptr = gmtime_r(&t2, &tmSecond);
        if (ptr == NULL) {
            badIdx = i;
            t2Bad = -2; /* 标记第二次 gmtime_r 失败 */
            break;
        }
        /* 九字段逐项全等（musl tm_cmp 同款字段集） */
        if ((tmSecond.tm_sec != tmSave.tm_sec) || (tmSecond.tm_min != tmSave.tm_min) ||
            (tmSecond.tm_hour != tmSave.tm_hour) || (tmSecond.tm_mday != tmSave.tm_mday) ||
            (tmSecond.tm_mon != tmSave.tm_mon) || (tmSecond.tm_year != tmSave.tm_year) ||
            (tmSecond.tm_wday != tmSave.tm_wday) || (tmSecond.tm_yday != tmSave.tm_yday) ||
            (tmSecond.tm_isdst != tmSave.tm_isdst)) {
            badIdx = i;
            t2Bad = -3; /* 标记九字段比对不一致 */
            break;
        }
    }

    /* 先恢复时区再断言，保证断言失败提前 return 时全局时区不被污染 */
    TimeTzRestore(&tzOrig);
    if (badIdx >= 0) {
        LOG("\n reverse roundtrip failed at index %d (t=%lld, t2=%lld)\n",
            badIdx, (long long)timeVals[badIdx], t2Bad);
        ICUNIT_ASSERT_EQUAL(badIdx, -1, badIdx);
    }
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_MKTIME_2038_001
 * @tc.name       mktime 2038 overflow test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeMktime2038001, Function | MediumTest | Level1)
{
    /* 用例简要描述：mktime 2038 越界（参考 musl libc-test src/functional/time.c:72-87 的 2038 溢出分支）：
     * 2038-12-31 23:59:59 再 +1s 即 2039-01-01 00:00:00，32 位 time_t 下应返 -1 且 errno==EOVERFLOW */
    struct tm testTM = { 0 };
    time_t timeRet;

    /* INIT_TM 的 mon 参数为 tm_mon 口径（0 起）：构造 2039 年 1 月 1 日 00:00:00 */
    INIT_TM(testTM, 2039, 0, 1, 0, 0, 0, 0);

    errno = 0;
    timeRet = mktime(&testTM);
    /* 按真实位宽运行时分支:ARM32 下 time_t 亦为 64 位(time64 体系),__LP64__ 判断失效(实测 0912 修正) */
    if (sizeof(time_t) > 4) {
        /* 64 位 time_t：2039-01-01 可正常表示，断言 mktime 成功（本地时区解释,值含 tz 偏移） */
        ICUNIT_ASSERT_EQUAL((timeRet != (time_t)-1), 1, (int)timeRet);
        ICUNIT_ASSERT_EQUAL(errno, 0, errno);
    } else {
        /* 32 位 time_t：约 2.18e9 超出 time_t 上限,mktime 溢出检查返回 -1 并置 EOVERFLOW */
        ICUNIT_ASSERT_EQUAL(timeRet, (time_t)-1, timeRet);
        ICUNIT_ASSERT_EQUAL(errno, EOVERFLOW, errno);
    }
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_LEAPYEAR_001
 * @tc.name       leap year (Feb 29) handling test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeLeapYear001, Function | MediumTest | Level1)
{
    /* 用例简要描述：非 musl 移植，自设计场景——闰年 2/29 有效性、非闰年 2/29 规格化语义、
     * 世纪边界 1900（非闰）/2000（闰，400 整除）日期推进规则 */
    struct tm testTM = { 0 };
    struct tm tmResult = { 0 };
    struct tm *ptr = NULL;
    time_t timeRet;

    /* 闰年 2024-02-29（4 整除）：mktime 成功且 gmtime 回查 mon==1(2月)/mday==29。
     * 取本地正午 12:00 保证默认 UTC+8 时区下 gmtime 回查不跨日 */
    INIT_TM(testTM, 2024, 1, 29, 12, 0, 0, 0);
    errno = 0;
    timeRet = mktime(&testTM);
    ICUNIT_ASSERT_EQUAL((timeRet != (time_t)-1), 1, (int)timeRet);
    ICUNIT_ASSERT_EQUAL(errno, 0, errno);
    ptr = gmtime(&timeRet);
    ICUNIT_ASSERT_EQUAL((ptr != NULL), 1, (UINTPTR)ptr);
    ICUNIT_ASSERT_EQUAL(ptr->tm_mon, 1, ptr->tm_mon);
    ICUNIT_ASSERT_EQUAL(ptr->tm_mday, 29, ptr->tm_mday);

    /* 非闰年 2023-02-29：mktime 规格化语义——回写 tm 规格化为 2023-03-01（mon==2/mday==1） */
    INIT_TM(testTM, 2023, 1, 29, 0, 0, 0, 0);
    errno = 0;
    timeRet = mktime(&testTM);
    ICUNIT_ASSERT_EQUAL((timeRet != (time_t)-1), 1, (int)timeRet);
    ICUNIT_ASSERT_EQUAL(errno, 0, errno);
    ICUNIT_ASSERT_EQUAL(testTM.tm_mon, 2, testTM.tm_mon);
    ICUNIT_ASSERT_EQUAL(testTM.tm_mday, 1, testTM.tm_mday);

    /* 世纪闰年 2000（400 整除，闰）：951782400 = 2000-02-29 00:00:00 UTC，
     * 即 2000-02-28 的次日仍为 2 月 29 日 */
    time_t t2000 = (time_t)951782400;
    ptr = gmtime_r(&t2000, &tmResult);
    ICUNIT_ASSERT_EQUAL((ptr != NULL), 1, (UINTPTR)ptr);
    ICUNIT_ASSERT_EQUAL(tmResult.tm_mon, 1, tmResult.tm_mon);
    ICUNIT_ASSERT_EQUAL(tmResult.tm_mday, 29, tmResult.tm_mday);

#if !defined(__LP64__)
    /* 世纪非闰年 1900（100 整除但 400 不整除，非闰）：-2203891200 = 1900-03-01 00:00:00 UTC，
     * 即 1900-02-28 的次日直接跳到 3 月 1 日。1900 年超出 32 位 time_t 表示范围，
     * 故用 64 位接口 gmtime64_r 验证 */
    time64_t t1900 = (time64_t)-2203891200LL;
    struct tm tm1900 = { 0 };
    struct tm *ptr64 = gmtime64_r(&t1900, &tm1900);
    ICUNIT_ASSERT_EQUAL((ptr64 != NULL), 1, (UINTPTR)ptr64);
    ICUNIT_ASSERT_EQUAL(tm1900.tm_mon, 2, tm1900.tm_mon);
    ICUNIT_ASSERT_EQUAL(tm1900.tm_mday, 1, tm1900.tm_mday);
#else
    /* LP64 下 time_t 可表示 1900 年，直接用 gmtime_r 验证同一时间点 */
    time_t t1900 = (time_t)-2203891200LL;
    ptr = gmtime_r(&t1900, &tmResult);
    ICUNIT_ASSERT_EQUAL((ptr != NULL), 1, (UINTPTR)ptr);
    ICUNIT_ASSERT_EQUAL(tmResult.tm_mon, 2, tmResult.tm_mon);
    ICUNIT_ASSERT_EQUAL(tmResult.tm_mday, 1, tmResult.tm_mday);
#endif
    return 0;
};

/* *
 * @tc.number     SUB_KERNEL_TIME_ERRNO_SIDE_001
 * @tc.name       errno side effect on success paths test
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixTimeFuncTestSuite, testTimeErrnoSide001, Function | MediumTest | Level1)
{
    /* 用例简要描述：移植 musl libc-test src/functional/time.c:53-62 思想——
     * gmtime_r/localtime_r/mktime 成功路径均不应设置 errno；
     * strftime 缓冲不足路径也不应误设 errno（弱断言，对照实现无 errno 写入） */
    time_t t = (time_t)1593561600; /* 2020-07-01 00:00:00 UTC，合法时间点 */
    struct tm tmResult = { 0 };
    struct tm *ptr = NULL;
    char buffer[8]; /* 故意不足："%Y-%m-%d %H:%M:%S" 完整输出需 20 字节 */

    /* gmtime_r 成功路径：errno 保持 0（time.c 中 gmtime_r 仅失败路径设 EINVAL） */
    errno = 0;
    ptr = gmtime_r(&t, &tmResult);
    ICUNIT_ASSERT_EQUAL((ptr != NULL), 1, (UINTPTR)ptr);
    ICUNIT_ASSERT_EQUAL(errno, 0, errno);

    /* localtime_r 成功路径：errno 保持 0（time.c 中 localtime_r 仅失败路径设 EFAULT/EOVERFLOW） */
    errno = 0;
    ptr = localtime_r(&t, &tmResult);
    ICUNIT_ASSERT_EQUAL((ptr != NULL), 1, (UINTPTR)ptr);
    ICUNIT_ASSERT_EQUAL(errno, 0, errno);

    /* mktime 成功路径：errno 保持 0（musl time.c:59-62 同款检查），
     * 顺带断言 localtime→mktime 墙钟往返相等 */
    errno = 0;
    time_t timeRet = mktime(&tmResult);
    ICUNIT_ASSERT_EQUAL(timeRet, t, timeRet);
    ICUNIT_ASSERT_EQUAL(errno, 0, errno);

    /* strftime 缓冲不足：返回 0（输出被截断）;errno 实测被设为 EINVAL(0912 实测修正:
     * musl porting strftime 缓冲不足路径有 errno 写入,非 POSIX 规定,按实现行为固化) */
    errno = 0;
    size_t ftime = strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tmResult);
    ICUNIT_ASSERT_EQUAL(ftime, 0, ftime);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    return 0;
};
#endif

RUN_TEST_SUITE(PosixTimeFuncTestSuite);

void PosixTimeFuncTest(void)
{
    LOG("begin PosixTimeFuncTest....\n");

#if !defined(LOSCFG_ARCH_FPU_DISABLE)
    RUN_ONE_TESTCASE(testTimeUSleep001);
#endif
#if (LOS_FEATURE_ADAPTED == 1)
    RUN_ONE_TESTCASE(testTimeUSleep002);
#endif
    RUN_ONE_TESTCASE(testTimeGmtime001);
#if (LOSCFG_LIBC_MUSL == 1)
    RUN_ONE_TESTCASE(testTimeLocaltime001);
    RUN_ONE_TESTCASE(testTimeLocaltime002);
    RUN_ONE_TESTCASE(testTimeLocaltime003);
    RUN_ONE_TESTCASE(testTimeLocaltimer001);
    RUN_ONE_TESTCASE(testTimeLocaltimer002);
#endif
    RUN_ONE_TESTCASE(testTimeMktime001);
    RUN_ONE_TESTCASE(testTimeMktime002);
#if (LOSCFG_LIBC_MUSL == 1)
    RUN_ONE_TESTCASE(testTimeStrftime001);
    RUN_ONE_TESTCASE(testTimeStrftime002);
    RUN_ONE_TESTCASE(testTimeStrftime003);
#endif
#if !defined(LOSCFG_ARCH_FPU_DISABLE)
    RUN_ONE_TESTCASE(testTimes);
#endif
#if (LOSCFG_LIBC_MUSL == 1) && !defined(__LP64__)
    RUN_ONE_TESTCASE(testTimeLocaltime64001);
    RUN_ONE_TESTCASE(testTimeGmtime64001);
    RUN_ONE_TESTCASE(testTimeAsctime64001);
    RUN_ONE_TESTCASE(testTimeCtime64001);
#endif
#if (LOSCFG_LIBC_MUSL == 1)
    RUN_ONE_TESTCASE(testTimeGmtimeMktimeLoop001);
    RUN_ONE_TESTCASE(testTimeMktimeGmtimeRev001);
    RUN_ONE_TESTCASE(testTimeMktime2038001);
    RUN_ONE_TESTCASE(testTimeLeapYear001);
    RUN_ONE_TESTCASE(testTimeErrnoSide001);
#endif
    return;
}