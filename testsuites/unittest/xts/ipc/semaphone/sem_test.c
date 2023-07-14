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

static int g_semTestStep = 0;

LITE_TEST_SUIT(IPC, SemApi, IpcSemApiTestSuite);

static BOOL IpcSemApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IpcSemApiTestSuiteTearDown(void)
{
    return TRUE;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemInit0100, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;
    int testValue[3] = {0, 1, 10}; /* 3, 1, 10 common data for test, no special meaning */

    int index = (int)(sizeof(testValue) / sizeof(int));
    for (int i = 0; i < index; i++) {
        
        ret = sem_init((sem_t *)&sem, 0, testValue[0]);
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);

        ret = sem_destroy(&sem);
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }
    return 0;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemPost0100, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;

    ret = sem_init((sem_t *)&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemWait0100, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;

    ret = sem_init((sem_t *)&sem, 0, 3); /* 3, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_wait(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_wait(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_wait(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

void *ThreadChat(void *arg)
{
    int ret;
    sem_t *sem = (sem_t *)arg;

    ICUNIT_ASSERT_EQUAL(g_semTestStep, 0, g_semTestStep);

    g_semTestStep = 1; /* 1, common data for test, no special meaning */
    ret = sem_wait(sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    g_semTestStep = 2; /* 2, common data for test, no special meaning */
    return NULL;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testThreadChat0100, Function | MediumTest | Level3)
{
    pthread_t tid;
    sem_t sem;
    int ret;
    struct timespec req;
    g_semTestStep = 0;

    ret = sem_init((sem_t *)&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&tid, NULL, ThreadChat, (void *)&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    req.tv_sec = 0;
    req.tv_nsec = TEN_CONT * NANO_MS;
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(g_semTestStep, 1, g_semTestStep); /* 1, common data for test, no special meaning */

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(g_semTestStep, 2, g_semTestStep); /* 2, common data for test, no special meaning */

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(g_semTestStep, 2, g_semTestStep); /* 2, common data for test, no special meaning */

    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

void *ThreadNThreadWait1(void *arg)
{
    int ret;
    sem_t *sem = (sem_t *)arg;
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = HUNDRED_CONT * NANO_MS;

    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    g_semTestStep = 1; /* 1, common data for test, no special meaning */
    ret = sem_wait(sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    g_semTestStep = 2; /* 2, common data for test, no special meaning */
    return NULL;
}

void *ThreadNThreadWait2(void *arg)
{
    int ret;
    sem_t *sem = (sem_t *)arg;
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = 300 * NANO_MS; /* 300, common data for test, no special meaning */
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    g_semTestStep = 3; /* 3, common data for test, no special meaning */

    req.tv_nsec = 200 * NANO_MS; /* 200, common data for test, no special meaning */
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = sem_wait(sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    g_semTestStep = 4; /* 4, common data for test, no special meaning */
    return NULL;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testThreadChat0400, Function | MediumTest | Level4)
{
    pthread_t tid1;
    pthread_t tid2;
    sem_t sem;
    int ret;
    struct timespec req;

    req.tv_sec = 0;
    req.tv_nsec = 200 * NANO_MS; /* 200, common data for test, no special meaning */
    g_semTestStep = 0;

    ret = sem_init((sem_t *)&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&tid1, NULL, ThreadNThreadWait1, (void *)&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&tid2, NULL, ThreadNThreadWait2, (void *)&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(g_semTestStep, 1, g_semTestStep); /* 1, common data for test, no special meaning */

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    req.tv_nsec = 20 * NANO_MS; /* 20, common data for test, no special meaning */
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(g_semTestStep, 2, g_semTestStep); /* 2, common data for test, no special meaning */

    req.tv_nsec = 200 * NANO_MS; /* 200, common data for test, no special meaning */
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(g_semTestStep, 3, g_semTestStep); /* 3, common data for test, no special meaning */

    ret = sem_post(&sem);
    req.tv_nsec = 20 * NANO_MS; /* 20, common data for test, no special meaning */
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(g_semTestStep, 3, g_semTestStep); /* 3, common data for test, no special meaning */

    ret = pthread_join(tid1, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(tid2, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(g_semTestStep, 4, g_semTestStep); /* 4, common data for test, no special meaning */

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemInitAbnormal0200, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;
    unsigned int gtSemMax = (unsigned int)SEM_VALUE_MAX + 1; /* 1, common data for test, no special meaning */
    
    ret = sem_init(&sem, 0, SEM_VALUE_MAX);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_init(&sem, 0, gtSemMax);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = sem_init(&sem, 0, 1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemPostAbnormal, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;

    ret = sem_init(&sem, 0, SEM_VALUE_MAX);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemTimedWaitAbnormalA, Function | MediumTest | Level3)
{
    int ret;
    struct timespec ts;
    sem_t sem;

    ret = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ts.tv_sec = 0;
    ts.tv_nsec = -2; /* -2, common data for test, no special meaning */
    ret = sem_timedwait(&sem, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

LITE_TEST_CASE(IpcSemApiTestSuite, testSemTimedWaitAbnormalB, Function | MediumTest | Level3)
{
    int ret;
    struct timespec ts;
    sem_t sem;

    ret = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ts.tv_sec = time(NULL);
    ts.tv_nsec = NANO_S;
    ret = sem_timedwait(&sem, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

RUN_TEST_SUITE(IpcSemApiTestSuite);

void PosixSemFuncTest()
{
    RUN_ONE_TESTCASE(testSemInit0100);
    RUN_ONE_TESTCASE(testSemPost0100);
    RUN_ONE_TESTCASE(testSemWait0100);
    RUN_ONE_TESTCASE(testThreadChat0100);
    RUN_ONE_TESTCASE(testThreadChat0400);
    RUN_ONE_TESTCASE(testSemInitAbnormal0200);
    RUN_ONE_TESTCASE(testSemPostAbnormal);
    RUN_ONE_TESTCASE(testSemTimedWaitAbnormalA);
    RUN_ONE_TESTCASE(testSemTimedWaitAbnormalB);
}