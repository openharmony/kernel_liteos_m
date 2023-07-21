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

LITE_TEST_SUIT(SCHED, PthreadSchedApi, PthreadSchedApiTestSuite);

static int g_policy = 0;
static int g_prioriy = 0;

static BOOL PthreadSchedApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadSchedApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_INHERIT_0100
 * @tc.name     test the default value of inheritsched.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrGetInheritsched, Function | MediumTest | Level1)
{
    int ret;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    int inheritsched = -1; /* -1, common data for test, no special meaning */
    int rt = pthread_attr_getinheritsched(&attr, &inheritsched);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    ICUNIT_ASSERT_EQUAL(inheritsched, PTHREAD_INHERIT_SCHED, inheritsched);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_INHERIT_0200
 * @tc.name     test set and get inheritsched.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetInheritsched, Function | MediumTest | Level1)
{
    int ret;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    int rt = pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    int inheritsched = -1; /* -1, common data for test, no special meaning */
    rt = pthread_attr_getinheritsched(&attr, &inheritsched);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    ICUNIT_ASSERT_EQUAL(inheritsched, PTHREAD_INHERIT_SCHED, inheritsched);

    rt = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    inheritsched = -1; /* -1, common data for test, no special meaning */
    rt = pthread_attr_getinheritsched(&attr, &inheritsched);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    ICUNIT_ASSERT_EQUAL(inheritsched, PTHREAD_EXPLICIT_SCHED, inheritsched);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_INHERIT_0300
 * @tc.name     pthread_attr_setinheritsched error test.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetInheritschedError, Function | MediumTest | Level1)
{
    int ret;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    int n = -GetRandom(100); /* 100, common data for test, no special meaning */
    int rt = pthread_attr_setinheritsched(&attr, n);
    ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);
    n = 2 + GetRandom(100); /* 2, 100, common data for test, no special meaning */
    rt = pthread_attr_setinheritsched(&attr, n);
    ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCHEDPARAM_0200
 * @tc.name     test set and get sched param.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetSchedParam, Function | MediumTest | Level1)
{
    int ret;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    struct sched_param param = {0};
    int rt = pthread_attr_getschedparam(&attr, &param);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);

    param.sched_priority = 22; /* 22, common data for test, no special meaning */
    rt = pthread_attr_setschedparam(&attr, &param);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);

    rt = pthread_attr_getschedparam(&attr, &param);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    ICUNIT_ASSERT_EQUAL(param.sched_priority, 22, param.sched_priority); /* 22, common data for test, no special meaning */
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCHEDPOLICY_0100
 * @tc.name     test the default value of sched policy.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrGetSchedPolicy, Function | MediumTest | Level1)
{
    int ret;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    int policy = -1; /* -1, common data for test, no special meaning */
    int rt = pthread_attr_getschedpolicy(&attr, &policy);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    ICUNIT_ASSERT_EQUAL(policy, SCHED_RR, policy);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCHEDPOLICY_0300
 * @tc.name     pthread_attr_setschedpolicy error test.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetSchedPolicyError, Function | MediumTest | Level1)
{
    int ret;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    int rt;

    int invalidPolicy[7] = {SCHED_OTHER, SCHED_BATCH, SCHED_IDLE, SCHED_DEADLINE, SCHED_RESET_ON_FORK}; /* 7, common data for test, no special meaning */
    invalidPolicy[5] = -GetRandom(10000); /* 5, 10000, common data for test, no special meaning */
    invalidPolicy[6] = GetRandom(10000) + 6; /* 6, 10000, common data for test, no special meaning */
    for (int i = 0; i < 7; i++) { /* 7, common data for test, no special meaning */
        rt = pthread_attr_setschedpolicy(&attr, invalidPolicy[i]);
        ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);
    }
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCOPE_0100
 * @tc.name     test the default value of sched scope.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrGetScope, Function | MediumTest | Level1)
{
    int ret;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    int scope = -1; /* -1, common data for test, no special meaning */
    int rt = pthread_attr_getscope(&attr, &scope);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    ICUNIT_ASSERT_EQUAL(scope, PTHREAD_SCOPE_SYSTEM, scope);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_SCHED_API_PATTR_SCOPE_0200
 * @tc.name     test set and get scope.
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadSchedApiTestSuite, testAttrSetScope, Function | MediumTest | Level1)
{
    int ret;
    pthread_attr_t attr;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    int rt = pthread_attr_setscope(&attr, PTHREAD_SCOPE_PROCESS);
    ICUNIT_ASSERT_EQUAL(rt, ENOTSUP, rt);

    rt = pthread_attr_setscope(&attr, -GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);

    rt = pthread_attr_setscope(&attr, GetRandom(10000) + 2); /* 2, 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(rt, EINVAL, rt);

    int scope = -1; /* -1, common data for test, no special meaning */
    rt = pthread_attr_getscope(&attr, &scope);
    ICUNIT_ASSERT_EQUAL(rt, 0, rt);
    ICUNIT_ASSERT_EQUAL(scope, PTHREAD_SCOPE_SYSTEM, scope);
    return 0;
}

RUN_TEST_SUITE(PthreadSchedApiTestSuite);

void PthreadSchedApiTest(void)
{
    RUN_ONE_TESTCASE(testAttrGetInheritsched);
    RUN_ONE_TESTCASE(testAttrSetInheritsched);
    RUN_ONE_TESTCASE(testAttrSetInheritschedError);
    RUN_ONE_TESTCASE(testAttrSetSchedParam);
    RUN_ONE_TESTCASE(testAttrGetSchedPolicy);
    RUN_ONE_TESTCASE(testAttrSetSchedPolicyError);
    RUN_ONE_TESTCASE(testAttrGetScope);
    RUN_ONE_TESTCASE(testAttrSetScope);
}
