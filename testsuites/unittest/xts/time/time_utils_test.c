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

time_t g_time = 18880; /* 18880, common data for test, no special meaning */
size_t g_zero = 0;

LITE_TEST_SUIT(TIME, TimeUtilsTest, TimeUtilsTestSuite);

static BOOL TimeUtilsTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL TimeUtilsTestSuiteTearDown(void)
{
    return TRUE;
}

/**
* @tc.number     SUB_KERNEL_TIME_API_MKTIME_0100
* @tc.name       test mktime api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(TimeUtilsTestSuite, testMktime, Function | MediumTest | Level2)
{
    struct tm *localTime = NULL;
    struct tm timeptr = {0};
    struct timeval tv;
    struct timezone tz;
	
    int ret = gettimeofday(&tv, &tz);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    long sysTimezone = (long)(-tz.tz_minuteswest) * SECS_PER_MIN;

    INIT_TM(timeptr, 2000, 6, 9, 10, 10, 0, 7); /* 2000, 6, 9, 10, 7, common data for test, no special meaning */
    time_t timeRet = mktime(&timeptr);
    ICUNIT_ASSERT_EQUAL(sysTimezone, timeptr.__tm_gmtoff, sysTimezone);
    ICUNIT_ASSERT_EQUAL(timeRet, 963137400 - timeptr.__tm_gmtoff, timeRet); /* 963137400, common data for test, no special meaning */
    localTime = localtime(&g_time);
    ICUNIT_ASSERT_NOT_EQUAL(localTime, NULL, localTime);
    time_t timep = mktime(localTime);
    ICUNIT_ASSERT_EQUAL(timep, 18880, timep); /* 18880, common data for test, no special meaning */
    return 0;
}

RUN_TEST_SUITE(TimeUtilsTestSuite);

void TimeUtilsTest(void)
{
    RUN_ONE_TESTCASE(testMktime);
}
