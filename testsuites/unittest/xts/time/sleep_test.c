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

static const char *ALL_CLOCKS_NAME[CLOCK_TAI + 1] = {
    "CLOCK_REALTIME",
    "CLOCK_MONOTONIC",
    "CLOCK_PROCESS_CPUTIME_ID",
    "CLOCK_THREAD_CPUTIME_ID",
    "CLOCK_MONOTONIC_RAW",
    "CLOCK_REALTIME_COARSE",
    "CLOCK_MONOTONIC_COARSE",
    "CLOCK_BOOTTIME",
    "CLOCK_REALTIME_ALARM",
    "CLOCK_BOOTTIME_ALARM",
    "CLOCK_SGI_CYCLE",
    "CLOCK_TAI",
};

LITE_TEST_SUIT(TIME, TimeSleepTest, TimeSleepTestSuite);

static BOOL TimeSleepTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL TimeSleepTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_TIME_API_CLOCK_NANOSLEEP_0200
 * @tc.name   clock_nanosleep fail test - non-support clock_id
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeSleepTestSuite, testClockNanosleepInvalidID, Reliability | SmallTest | Level2)
{
    clockid_t cid = 1; /* 1, common data for test, no special meaning */
    const char *cname = ALL_CLOCKS_NAME[cid];
    struct timespec req = {0, 100}; /* 100, common data for test, no special meaning */
    struct timespec rem = {0};
    int rt = clock_nanosleep(cid, 0, &req, &rem);
    if (cid == CLOCK_SGI_CYCLE) {
        ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);
    }
    return 0;
}

/**
 * @tc.number SUB_KERNEL_TIME_API_CLOCK_NANOSLEEP_0300
 * @tc.name   clock_nanosleep fail test - invalid parameter
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(TimeSleepTestSuite, testClockNanosleepInvalidPara, Reliability | SmallTest | Level2)
{
    struct timespec req = {0, 100}; /* 100, common data for test, no special meaning */
    struct timespec rem = {0};
    int rt;

    int id = GetRandom(1000) + 12; /* 1000, 12, common data for test, no special meaning */
    rt = clock_nanosleep(id, 0, &req, &rem);
    ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);

    id = -GetRandom(1000) - 12; /* 1000, 12, common data for test, no special meaning */
    rt = clock_nanosleep(id, 0, &req, &rem);
    ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);

    int flag = TIMER_ABSTIME;
    rt = clock_nanosleep(CLOCK_REALTIME, flag, &req, &rem);
    ICUNIT_ASSERT_EQUAL(rt, ENOTSUP, rt);
    flag = GetRandom(100) + 1; /* 100, 1, common data for test, no special meaning */
    rt = clock_nanosleep(CLOCK_REALTIME, flag, &req, &rem);
    ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);
    flag = -GetRandom(100) - 1; /* 100, 1, common data for test, no special meaning */
    rt = clock_nanosleep(CLOCK_REALTIME, flag, &req, &rem);
    ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);

    req.tv_sec = -1; /* -1, common data for test, no special meaning */
    req.tv_nsec = 1; /* 1, common data for test, no special meaning */
    rt = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    req.tv_sec = 1; /* 1, common data for test, no special meaning */
    req.tv_nsec = -1; /* -1, common data for test, no special meaning */
    rt = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    req.tv_sec = 1; /* 1, common data for test, no special meaning */
    req.tv_nsec = 1000 * 1000 * 1000 + 1; /* 1000, 1, common data for test, no special meaning */
    rt = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    return 0;
}

RUN_TEST_SUITE(TimeSleepTestSuite);

void SleepTest(void)
{
    RUN_ONE_TESTCASE(testClockNanosleepInvalidID);
    RUN_ONE_TESTCASE(testClockNanosleepInvalidPara);
}
