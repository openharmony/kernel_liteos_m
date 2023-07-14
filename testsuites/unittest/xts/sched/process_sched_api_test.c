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

#include "sched_api_test.h"

LITE_TEST_SUIT(SCHED, ProcessSchedApi, ProcessSchedApiTestSuite);

static BOOL ProcessSchedApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL ProcessSchedApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_GET_PRIORITY_MAX_0200
 * @tc.name     sched_get_priority_max api error test with unsupport policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ProcessSchedApiTestSuite, testSchedGetPriorityMaxError0200, Function | MediumTest | Level3)
{
    int invalidPolicy[] = {SCHED_FIFO, SCHED_OTHER, SCHED_BATCH, SCHED_IDLE, SCHED_DEADLINE, SCHED_RESET_ON_FORK};
    int testLoop = sizeof(invalidPolicy) / sizeof(int);
    for (int i = 0; i < testLoop; i++) {
        errno = 0;
        int prio = sched_get_priority_max(invalidPolicy[i]);
        ICUNIT_ASSERT_EQUAL(prio, -1, prio); /* -1, common data for test, no special meaning */
        ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    }
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_GET_PRIORITY_MAX_0300
 * @tc.name     sched_get_priority_max api error test with invalid policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ProcessSchedApiTestSuite, testSchedGetPriorityMaxError0300, Function | MediumTest | Level3)
{
    int invalidPolicyVal;
    int prio;

    invalidPolicyVal = -GetRandom(10000); /* 10000, common data for test, no special meaning */
    errno = 0;
    prio = sched_get_priority_max(invalidPolicyVal);
    ICUNIT_ASSERT_EQUAL(prio, -1, prio); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    invalidPolicyVal = GetRandom(10000) + SCHED_DEADLINE; /* 10000, common data for test, no special meaning */
    errno = 0;
    prio = sched_get_priority_max(invalidPolicyVal);
    ICUNIT_ASSERT_EQUAL(prio, -1, prio); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_GET_PRIORITY_MIN_0200
 * @tc.name     sched_get_priority_min api error test with unsupport policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ProcessSchedApiTestSuite, testSchedGetPriorityMinError0200, Function | MediumTest | Level3)
{
    int invalidPolicy[] = {SCHED_FIFO, SCHED_OTHER, SCHED_BATCH, SCHED_IDLE, SCHED_DEADLINE, SCHED_RESET_ON_FORK};
    int testLoop = sizeof(invalidPolicy) / sizeof(int);
    for (int i = 0; i < testLoop; i++) {
        errno = 0;
        int prio = sched_get_priority_min(invalidPolicy[i]);
        ICUNIT_ASSERT_EQUAL(prio, -1, prio); /* -1, common data for test, no special meaning */
        ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    }
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_GET_PRIORITY_MIN_0300
 * @tc.name     sched_get_priority_min api error test with invalid policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(ProcessSchedApiTestSuite, testSchedGetPriorityMinError0300, Function | MediumTest | Level3)
{
    int invalidPolicyVal;
    int prio;
    invalidPolicyVal = -GetRandom(10000); /* 10000, common data for test, no special meaning */
    errno = 0;
    prio = sched_get_priority_min(invalidPolicyVal);
    ICUNIT_ASSERT_EQUAL(prio, -1, prio); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    invalidPolicyVal = GetRandom(10000) + SCHED_DEADLINE; /* 10000, common data for test, no special meaning */
    errno = 0;
    prio = sched_get_priority_min(invalidPolicyVal);
    ICUNIT_ASSERT_EQUAL(prio, -1, prio); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    return 0;
}

RUN_TEST_SUITE(ProcessSchedApiTestSuite);

void ProcessSchedApiTest(void)
{
    RUN_ONE_TESTCASE(testSchedGetPriorityMaxError0200);
    RUN_ONE_TESTCASE(testSchedGetPriorityMaxError0300);
    RUN_ONE_TESTCASE(testSchedGetPriorityMinError0200);
    RUN_ONE_TESTCASE(testSchedGetPriorityMinError0300);
}
