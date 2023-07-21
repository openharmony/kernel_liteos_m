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

LITE_TEST_SUIT(PROCESS, PthreadBasicApi, PthreadBasicApiTestSuite);

static BOOL PthreadBasicApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadBasicApiTestSuiteTearDown(void)
{
    return TRUE;
}

void *ThreadPthreadCreateBasic(void *arg)
{
    char *s = (char*)arg;
    ICUNIT_ASSERT_STRING_EQUAL(s, "1234567890 !@#$%^&*()_= ZXCVBNM [];'./>?:\" +-*/qwertyuiopasdfghjklzxcvbnm", s);
    return arg;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_CREATE_0100
 * @tc.name     pthread_create create a thread
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadCreateBasic, Function | MediumTest | Level2)
{
    int ret;
    pthread_t tid;
    char str[] = "1234567890 !@#$%^&*()_= ZXCVBNM [];'./>?:\" +-*/qwertyuiopasdfghjklzxcvbnm";
    ret = pthread_create(&tid, NULL, ThreadPthreadCreateBasic, (void*)str);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_JOIN_0200
 * @tc.name     Test the function of pthread_join to get the return value
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testJoinReturn, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid;
    int num = 4; /* 4, common data for test, no special meaning */
    void *joinRe = NULL;
    ret = pthread_create(&tid, NULL, ThreadPublic, (void*)&num);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, &joinRe);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    int *p = (int*)joinRe;
    ICUNIT_ASSERT_EQUAL(&num, p, &num);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_JOIN_0300
 * @tc.name     Test the function about pthread_join, but child thread Exited
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testJoinExited, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, ThreadPublic, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    usleep(50); /* 50, common data for test, no special meaning */
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

void *ThreadPthreadExitThread(void *arg)
{
    pthread_exit(arg);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_EXIT_0100
 * @tc.name     Test the return function of pthread_exit in the child thread
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadExitThread, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid;
    int num = 4; /* 4, common data for test, no special meaning */
    void *joinRe = NULL;
    ret = pthread_create(&tid, NULL, ThreadPthreadExitThread, (void*)&num);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, &joinRe);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    int *p = (int*)joinRe;
    ICUNIT_ASSERT_EQUAL(&num, p, &num);
    return 0;
}

void FunPthreadExit(void *arg)
{
    pthread_exit(arg);
}

void *ThreadPthreadExitFunction(void *arg)
{
    FunPthreadExit(arg);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_EXIT_0200
 * @tc.name     Test the return function of pthread_exit in the child thread function
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadExitFunction, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid;
    int num = 4; /* 4, common data for test, no special meaning */
    void *joinRe = NULL;
    ret = pthread_create(&tid, NULL, ThreadPthreadExitFunction, (void*)&num);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, &joinRe);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    int *p = (int*)joinRe;
    ICUNIT_ASSERT_EQUAL(&num, p, &num);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_DETACH_0100
 * @tc.name     Use pthread_detach to detach child threads
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadDetach, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, ThreadPublic, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_detach(tid);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);
    return 0;
}

void *ThreadPthreadEqual(void *arg)
{
    int ret;
    pthread_t *tid = (pthread_t*)arg;
    ret = pthread_equal(*tid, pthread_self());
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);
    usleep(10); /* 10, common data for test, no special meaning */
    return arg;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_EQUAL_0100
 * @tc.name     Use pthread_equal checks process equality
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadBasicApiTestSuite, testPthreadEqual, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid;
    ret = pthread_create(&tid, NULL, ThreadPthreadEqual, (void*)&tid);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_equal(tid, pthread_self());
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

RUN_TEST_SUITE(PthreadBasicApiTestSuite);

void PthreadBasicApiTest(void)
{
    RUN_ONE_TESTCASE(testPthreadCreateBasic);
    RUN_ONE_TESTCASE(testJoinReturn);
    RUN_ONE_TESTCASE(testJoinExited);
    RUN_ONE_TESTCASE(testPthreadExitThread);
    RUN_ONE_TESTCASE(testPthreadExitFunction);
    RUN_ONE_TESTCASE(testPthreadDetach);
    RUN_ONE_TESTCASE(testPthreadEqual);
}

