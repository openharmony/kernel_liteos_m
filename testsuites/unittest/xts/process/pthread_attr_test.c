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

#include "pthread_test.h"

LITE_TEST_SUIT(PROCESS, PthreadAttr, PthreadAttrTestSuite);

static BOOL PthreadAttrTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadAttrTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_ATTR_SETDETACHSTATE_0100
 * @tc.name     basic test about pthread_attr_setdetachstate
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadAttrTestSuite, testPthreadAttrSetdetachstate, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid;
    pthread_attr_t attr;
    int param;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_attr_getdetachstate(&attr, &param);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(param, PTHREAD_CREATE_DETACHED, param);
    ret = pthread_create(&tid, &attr, ThreadPublic, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_attr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_attr_getdetachstate(&attr, &param);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(param, PTHREAD_CREATE_JOINABLE, param);
    ret = pthread_create(&tid, &attr, ThreadPublic, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_attr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_ATTR_SETSTACKSIZE_0200
 * @tc.name     test pthread_attr_setstacksize EINVAL
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadAttrTestSuite, testPthreadAttrSetstacksizeEINVAL, Function | MediumTest | Level3)
{
    int ret;
    pthread_attr_t attr;
    size_t stackSize;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_attr_getstacksize(&attr, &stackSize);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    stackSize = PTHREAD_STACK_MIN - 1; /* 1, common data for test, no special meaning */
    ret = pthread_attr_setstacksize(&attr, stackSize);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);
    ret = pthread_attr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

RUN_TEST_SUITE(PthreadAttrTestSuite);

void PthreadAttrTest(void)
{
    RUN_ONE_TESTCASE(testPthreadAttrSetdetachstate);
    RUN_ONE_TESTCASE(testPthreadAttrSetstacksizeEINVAL);
}

