/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "alarm_test.h"

LITE_TEST_SUIT(TIME, TimeClockTimeTest, TimeClockTimeTestSuite);

static BOOL TimeClockTimeTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL TimeClockTimeTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_TIME_API_CLOCK_GETTIME_0100
 * @tc.name   test all supported clockid of clock_gettime
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeClockTimeTestSuite, testClockGettimeAll, Function | MediumTest | Level1)
{
    clockid_t cid = CLOCK_REALTIME;
    struct timespec time1 = {0, 0};
    int rt = clock_gettime(cid, &time1);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_TIME_API_CLOCK_SETTIME_0100
 * @tc.name   test clock_settime basic
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeClockTimeTestSuite, testClockSettime, Function | MediumTest | Level1)
{
    struct timespec time1 = {0, 0};
    sleep(1); /* 1, common data for test, no special meaning */
    int rt = clock_gettime(CLOCK_REALTIME, &time1);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    time_t sec = time1.tv_sec;
    time1.tv_sec -= 1; /* 1, common data for test, no special meaning */
    time1.tv_nsec = 1; /* 1, common data for test, no special meaning */
    rt = clock_settime(CLOCK_REALTIME, &time1);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    sleep(1); /* 1, common data for test, no special meaning */
    rt = clock_gettime(CLOCK_REALTIME, &time1);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    ICUNIT_ASSERT_EQUAL(time1.tv_sec, sec, time1.tv_sec);
    return 0;
}

/**
* @tc.number     SUB_KERNEL_TIME_API_GETTIMEOFDAY_0100
* @tc.name       test gettimeofday api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(TimeClockTimeTestSuite, testGettimeofday, Function | MediumTest | Level1)
{
    int ret;
    int sleepSec = 1; /* 1, common data for test, no special meaning */
    struct timeval tValStart = {0};
    struct timeval tValEnd = {0};
    struct timezone tZone;

    int ret1 = gettimeofday(&tValStart, &tZone);
    sleep(sleepSec);
    int ret2 = gettimeofday(&tValEnd, &tZone);
    ICUNIT_ASSERT_EQUAL(ret1, 0, ret1);
    ICUNIT_ASSERT_EQUAL(ret2, 0, ret2);

    ret = (int)(tValEnd.tv_sec - tValStart.tv_sec);
    ICUNIT_ASSERT_EQUAL(ret, sleepSec, ret);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, sleepSec, INT_MAX, ret);
    ret = (int)(tValEnd.tv_sec - tValStart.tv_sec);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, INT_MIN, sleepSec + 1, ret); /* 1, common data for test, no special meaning */
    return 0;
}

/**
* @tc.number     SUB_KERNEL_TIME_API_SETTIMEOFDAY_0100
* @tc.name       test settimeofday api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(TimeClockTimeTestSuite, testSettimeofday, Function | MediumTest | Level1)
{
    int ret;
    int setSec = 100; /* 100, common data for test, no special meaning */
    int sleepSec = 2; /* 2, common data for test, no special meaning */
    struct timeval tValStart = {0};
    struct timeval tValEnd = {0};
    struct timeval set = {.tv_sec = setSec, .tv_usec = 0};

    int ret1 = settimeofday(&set, NULL);
    int ret2 = gettimeofday(&tValStart, NULL);
    sleep(sleepSec);
    int ret3 = gettimeofday(&tValEnd, NULL);
    ICUNIT_ASSERT_EQUAL(ret1, 0, ret1);
    ICUNIT_ASSERT_EQUAL(ret2, 0, ret2);
    ICUNIT_ASSERT_EQUAL(ret3, 0, ret3);
    ICUNIT_ASSERT_EQUAL(tValStart.tv_sec, setSec, tValStart.tv_sec);

    ret = (int)(tValEnd.tv_sec - tValStart.tv_sec);
    ICUNIT_ASSERT_EQUAL(ret, sleepSec, ret);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, sleepSec, INT_MAX, ret);
    ret = (int)(tValEnd.tv_sec - tValStart.tv_sec);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, INT_MIN, sleepSec + 1, ret); /* 1, common data for test, no special meaning */
    return 0;
}

RUN_TEST_SUITE(TimeClockTimeTestSuite);

void ClockTimeTest(void)
{
    RUN_ONE_TESTCASE(testClockGettimeAll);
    RUN_ONE_TESTCASE(testClockSettime);
    RUN_ONE_TESTCASE(testGettimeofday);
    RUN_ONE_TESTCASE(testSettimeofday);
}
