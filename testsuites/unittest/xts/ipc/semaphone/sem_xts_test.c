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

LITE_TEST_SUIT(IPC, SemApi, IpcSemApiXtsTestSuite);

static BOOL IpcSemApiXtsTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IpcSemApiXtsTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0100
 * @tc.name     Use sem_init initialize the semaphore with 0
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemInitA, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;
    int semValue = 0;
    int testValue = 0;

    ret = sem_init(&sem, 0, testValue);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(semValue, testValue, semValue);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0110
 * @tc.name     Use sem_init initialize the semaphore with 1
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemInitB, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;
    int semValue = 0;
    int testValue = 1; /* 1, common data for test, no special meaning */

    ret = sem_init(&sem, 0, testValue);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(semValue, testValue, semValue);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_INIT_0120
 * @tc.name     Use sem_init initialize the semaphore with 100
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemInitC, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;
    int semValue = 0;
    int testValue = 10; /* 10, common data for test, no special meaning */

    ret = sem_init(&sem, 0, testValue);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(semValue, testValue, semValue);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_POST_0100
 * @tc.name     sem_post increases the semaphore count
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemPost, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;
    int semValue = 0;

    ret = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 1, semValue); /* 1, common data for test, no special meaning */

    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 3, semValue); /* 3, common data for test, no special meaning */

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_WAIT_0100
 * @tc.name     sem_wait get semaphore
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemWait, Function | MediumTest | Level2)
{
    int ret;
    sem_t sem;
    int semValue = 0;

    ret = sem_init(&sem, 0, 3); /* 3, common data for test, no special meaning */
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 3, semValue); /* 3, common data for test, no special meaning */

    ret = sem_wait(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 2, semValue); /* 2, common data for test, no special meaning */

    ret = sem_wait(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_wait(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_DESTROY_0100
 * @tc.name     check sem_destroy function
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testSemdestroy, Function | MediumTest | Level3)
{
    int ret;
    sem_t sem;

    ret = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

void *ThreadChatF01(void *arg)
{
    int ret;
    sem_t *sem = (sem_t*)arg;
    int semValue = 0;

    ret = sem_getvalue(sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);
    ret = sem_wait(sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_CHAT_0100
 * @tc.name     Inter-thread communication, check sem_wait function
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testThreadChat, Function | MediumTest | Level3)
{
    pthread_t tid;
    sem_t sem;
    int reInt;
    int semValue = 0;

    reInt = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    reInt = pthread_create(&tid, NULL, ThreadChatF01, (void*)&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    usleep(20); /* 20, common data for test, no special meaning */
    reInt = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);

    reInt = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    reInt = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    usleep(20); /* 20, common data for test, no special meaning */
    reInt = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    ICUNIT_ASSERT_EQUAL(semValue, 1, semValue); /* 1, common data for test, no special meaning */

    reInt = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    reInt = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    return 0;
}

void *ThreadSemTimedWait(void *arg)
{
    int ret;
    struct timespec ts = {0};
    sem_t *sem = (sem_t*)arg;
    int semValue = 0;

    ret = sem_getvalue(sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec = ts.tv_sec + (ts.tv_nsec + KERNEL_100MS_BY_NS) / KERNEL_NS_PER_SECOND;
    ts.tv_nsec = (ts.tv_nsec + KERNEL_100MS_BY_NS) % KERNEL_NS_PER_SECOND;
    usleep(50); /* 50, common data for test, no special meaning */
    ret = sem_timedwait(sem, &ts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = sem_getvalue(sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    usleep(100); /* 100, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(semValue, 1, semValue); /* 1, common data for test, no special meaning */
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_CHAT_0300
 * @tc.name     Inter-thread communication, check sem_timedwait function
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testThreadSemTimedWait, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid;
    sem_t sem;
    int reInt;
    int semValue = 0;

    reInt = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    reInt = pthread_create(&tid, NULL, ThreadSemTimedWait, (void*)&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    usleep(10); /* 10, common data for test, no special meaning */
    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);
    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    reInt = pthread_join(tid, NULL);
    ret = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 1, semValue); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    ret = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}
 
void *ThreadNThreadWaitF01(void *arg)
{
    int ret;
    sem_t *sem = (sem_t*)arg;
    int semValue = 0;

    usleep(100); /* 100, common data for test, no special meaning */
    ret = sem_getvalue(sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);
    ret = sem_wait(sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return NULL;
}

void *ThreadNThreadWaitF02(void *arg)
{
    int ret;
    sem_t *sem = (sem_t*)arg;
    int semValue = 0;

    ret = sem_getvalue(sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);

    usleep(300); /* 300, common data for test, no special meaning */
    ret = sem_getvalue(sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);

    usleep(200); /* 200, common data for test, no special meaning */
    ret = sem_getvalue(sem, &semValue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(semValue, 1, semValue); /* 1, common data for test, no special meaning */
    ret = sem_wait(sem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_IPC_SEM_CHAT_0400
 * @tc.name     N threads wait, main thread post
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcSemApiXtsTestSuite, testNThreadWait, Function | MediumTest | Level4)
{
    pthread_t tid1;
    pthread_t tid2;
    sem_t sem;
    int reInt;
    int semValue = 0;

    reInt = sem_init(&sem, 0, 0);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    reInt = pthread_create(&tid1, NULL, ThreadNThreadWaitF01, (void*)&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    reInt = pthread_create(&tid2, NULL, ThreadNThreadWaitF02, (void*)&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    usleep(200); /* 200, common data for test, no special meaning */
    reInt = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);
    reInt = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    usleep(20); /* 20, common data for test, no special meaning */
    reInt = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);

    usleep(200); /* 200, common data for test, no special meaning */
    reInt = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    ICUNIT_ASSERT_EQUAL(semValue, 0, semValue);
    reInt = sem_post(&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);

    usleep(20); /* 20, common data for test, no special meaning */
    reInt = sem_getvalue(&sem, &semValue);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    ICUNIT_ASSERT_EQUAL(semValue, 1, semValue); /* 1, common data for test, no special meaning */

    reInt = pthread_join(tid1, NULL);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    reInt = pthread_join(tid2, NULL);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    reInt = sem_destroy(&sem);
    ICUNIT_ASSERT_EQUAL(reInt, 0, reInt);
    return 0;
}

RUN_TEST_SUITE(IpcSemApiXtsTestSuite);

void PosixSemXtsFuncTest()
{
    RUN_ONE_TESTCASE(testSemInitA);
    RUN_ONE_TESTCASE(testSemInitB);
    RUN_ONE_TESTCASE(testSemInitC);
    RUN_ONE_TESTCASE(testSemPost);
    RUN_ONE_TESTCASE(testSemWait);
    RUN_ONE_TESTCASE(testSemdestroy);
    RUN_ONE_TESTCASE(testThreadChat);
}
