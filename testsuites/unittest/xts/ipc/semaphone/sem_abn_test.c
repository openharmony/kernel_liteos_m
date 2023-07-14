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

#include "xts_ipc.h"

LITE_TEST_SUIT(IPC, SemAbnormalTestSuite, SemAbnormalTestSuite);

static BOOL SemAbnormalTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL SemAbnormalTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0200
 * @tc.name     Use sem_init initialized value when value is SEM_VALUE_MAX
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemInitAbnormalSemvaluemax, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;
    int semValue = 0;

    ret = sem_init(&sem, 0, SEM_VALUE_MAX);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(SEM_VALUE_MAX, semValue, SEM_VALUE_MAX);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0210
 * @tc.name     Use sem_init initialized value when value is greater than SEM_VALUE_MAX
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemInitAbnormalGtsemvaluemax, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;
    unsigned int gtSemMax = (unsigned int)SEM_VALUE_MAX + 1; /* 1, common data for test, no special meaning */

    ret = sem_init(&sem, 0, gtSemMax);
    ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
EXIT:
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0220
 * @tc.name     Use sem_init initialized value twice
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemInitAbnormalInitTwice, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;

    ret = sem_init(&sem, 0, 1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_POST_0200
 * @tc.name     sem_post increases the semaphore count near the maximum value
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemPostAbnormal, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;

    ret = sem_init(&sem, 0, SEM_VALUE_MAX);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = sem_post(&sem);
    ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT);
    ICUNIT_GOTO_EQUAL(errno, EOVERFLOW, errno, EXIT); /* -1, common data for test, no special meaning */
    
EXIT:
    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_TIMEDWAIT_0200
 * @tc.name     sem_timedwait get semaphore, wait time abnormal, tv_nsec less than 0
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemTimedwaitAbnormalA, Function | MediumTest | Level3)
{
    int ret;
    struct timespec ts = {0};
    sem_t sem;
    int semValue = 0;

    ret = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ts.tv_sec = time(NULL);
    ts.tv_nsec = -2; /* -2, common data for test, no special meaning */
    ret = sem_timedwait(&sem, &ts);
    ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT); /* -1, common data for test, no special meaning */
    ICUNIT_GOTO_EQUAL(errno, EINVAL, errno, EXIT);

EXIT:
    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);
    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_TIMEDWAIT_0300
 * @tc.name     sem_timedwait get semaphore, wait time abnormal
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(SemAbnormalTestSuite, testSemTimedwaitAbnormalB, Function | MediumTest | Level3)
{
    int ret;
    struct timespec ts = {0};
    sem_t sem;
    int semValue = 0;

    ret = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ts.tv_sec = time(NULL);
    ts.tv_nsec = KERNEL_NS_PER_SECOND;
    ret = sem_timedwait(&sem, &ts);
    ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT); /* -1, common data for test, no special meaning */
    ICUNIT_GOTO_EQUAL(errno, EINVAL, errno, EXIT);
EXIT:
    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);
    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

RUN_TEST_SUITE(SemAbnormalTestSuite);

void PosixSemAbnFuncTest()
{
    RUN_ONE_TESTCASE(testSemInitAbnormalSemvaluemax);
    RUN_ONE_TESTCASE(testSemInitAbnormalInitTwice);
    RUN_ONE_TESTCASE(testSemTimedwaitAbnormalA);
    RUN_ONE_TESTCASE(testSemTimedwaitAbnormalB);
}
