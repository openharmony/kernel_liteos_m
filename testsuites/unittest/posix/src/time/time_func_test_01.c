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

#define RET_OK 0

#define SECS_PER_MIN 60
#define SLEEP_ACCURACY 21000  // 20 ms, with 1ms deviation
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
    sprintf_s(timeStr, len, "%ld/%d/%d %02d:%02d:%02d WEEK(%d)", timePtr->tm_year + TM_BASE_YEAR, timePtr->tm_mon + 1,
        timePtr->tm_mday, timePtr->tm_hour, timePtr->tm_min, timePtr->tm_sec, timePtr->tm_wday);
    return timeStr;
}

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
    ICUNIT_ASSERT_WITHIN_EQUAL(duration, duration, 1000, 0);
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
    ICUNIT_ASSERT_EQUAL(timeRet, -1, timeRet);
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

    if (!CheckValueClose((end.tms_utime - start.tms_utime), testClockt, 0.02)) {
        ICUNIT_ASSERT_EQUAL(1, 0, 0);
    }
    if (!CheckValueClose((endTime - stTime), testClockt, 0.02)) {
        ICUNIT_ASSERT_EQUAL(1, 0, 0);
    }
    return 0;
}

RUN_TEST_SUITE(PosixTimeFuncTestSuite);

void PosixTimeFuncTest(void)
{
    LOG("begin PosixTimeFuncTest....\n");

    RUN_ONE_TESTCASE(testTimeUSleep001);
    RUN_ONE_TESTCASE(testTimeUSleep002);

    RUN_ONE_TESTCASE(testTimeGmtime001);
#if (LOSCFG_LIBC_MUSL == 1)
    RUN_ONE_TESTCASE(testTimeLocaltime001);
    RUN_ONE_TESTCASE(testTimeLocaltime002);
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
    RUN_ONE_TESTCASE(testTimes);
    return;
}
