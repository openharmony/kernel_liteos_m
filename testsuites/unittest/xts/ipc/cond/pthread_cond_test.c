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

pthread_mutex_t g_mtx3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_cond3 = PTHREAD_COND_INITIALIZER;
static volatile int g_condReady = 0;

LITE_TEST_SUIT(FUTEX, PthreadCondApiTest, PthreadCondApiTestSuite);

static BOOL PthreadCondApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadCondApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_COND_INIT_0100
 * @tc.name     pthread_cond_init initializes condition variables
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadCondApiTestSuite, testPthreadCondInit, Function | MediumTest | Level2)
{
    int ret;
    pthread_condattr_t condAttr;

    ret = pthread_condattr_init(&condAttr);
    ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT);

    pthread_cond_t cond1;
    ret = pthread_cond_init(&cond1, &condAttr);
    ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT_ATTR);

    pthread_cond_t cond2;
    ret = pthread_cond_init(&cond2, NULL);
    ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT_COND1);

    ret = pthread_cond_destroy(&cond2);
    ICUNIT_TRACK_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);

    ret = pthread_cond_destroy(&cond1);
    ICUNIT_TRACK_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);

    ret = pthread_condattr_destroy(&condAttr);
    ICUNIT_TRACK_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    return 0;

EXIT_COND1:
    (VOID)pthread_cond_destroy(&cond1);
EXIT_ATTR:
    (VOID)pthread_condattr_destroy(&condAttr);
EXIT:
    return LOS_NOK;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_COND_DESTROY_0100
 * @tc.name     pthread_cond_destroy destroy condition variables
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadCondApiTestSuite, testPthreadCondDestroy, Function | MediumTest | Level3)
{
    int ret;
    pthread_condattr_t condAttr;
    ret = pthread_condattr_init(&condAttr);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);

    pthread_cond_t cond1;
    ret = pthread_cond_init(&cond1, &condAttr);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_cond_destroy(&cond1);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);

    pthread_cond_t cond2;
    ret = pthread_cond_init(&cond2, NULL);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_cond_destroy(&cond2);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);

    pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
    ret = pthread_cond_destroy(&cond3);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    return 0;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_CONDATTR_INIT_0100
 * @tc.name     Init and destroy operations
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadCondApiTestSuite, testPthreadCondattrInit, Function | MediumTest | Level2)
{
    int ret;
    pthread_condattr_t condAttr;

    ret = pthread_condattr_init(&condAttr);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_condattr_destroy(&condAttr);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_condattr_init(&condAttr);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    return 0;
}

// pthread_cond_broadcast
void *ThreadPthreadCondBroadcast1(void *arg)
{
    int ret;
    int *testIntP = (int *)arg;
    ret = pthread_mutex_lock(&g_mtx3);
    ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT1);
    *testIntP = TEST_INTP_SIZE;
    g_condReady = 1; /* set predicate before broadcast */
    ret = pthread_cond_broadcast(&g_cond3);
    ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT);
    ret = pthread_mutex_unlock(&g_mtx3);
    ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT);
    return arg;
EXIT:
    pthread_cond_broadcast(&g_cond3);
    pthread_mutex_unlock(&g_mtx3);
EXIT1:
    return NULL;
}

// pthread_cond_wait
void *ThreadPthreadCondBroadcast2(void *arg)
{
    int ret;
    int *testIntP = (int *)arg;
    ret = pthread_mutex_lock(&g_mtx3);
    ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT1);
    /* Loop on predicate so a late-arriving waiter does not block forever
     * even if the broadcast already consumed (CLR'd) the event before this
     * thread entered LOS_EventRead. */
    while (g_condReady == 0) {
        ret = pthread_cond_wait(&g_cond3, &g_mtx3);
        ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT);
    }
    (*testIntP)++;
    ret = pthread_mutex_unlock(&g_mtx3);
    ICUNIT_GOTO_EQUAL(ret, POSIX_IPC_NO_ERROR, ret, EXIT);
    return arg;
EXIT:
    pthread_mutex_unlock(&g_mtx3);
EXIT1:
    return NULL;
}

/**
 * @tc.number   SUB_KERNEL_PTHREAD_COND_BROADCAST_0100
 * @tc.name     Use pthread_cond_broadcast to release conditional semaphore
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadCondApiTestSuite, testPthreadCondBroadcast, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid[3]; /* 3, common data for test, no special meaning */
    int testInt = 0;

    /* Reset predicate and explicitly init cond to avoid lazy-init race
     * when multiple waiters enter pthread_cond_wait simultaneously. */
    g_condReady = 0;
    ret = pthread_cond_init(&g_cond3, NULL);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);

    ret = pthread_create(&tid[0], NULL, ThreadPthreadCondBroadcast1, (void*)&testInt);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_create(&tid[1], NULL, ThreadPthreadCondBroadcast2, (void*)&testInt); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_create(&tid[2], NULL, ThreadPthreadCondBroadcast2, (void*)&testInt); /* 2, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);

    usleep(100); /* 100, common data for test, no special meaning */
    int index = (int)(sizeof(tid) / sizeof(tid[0]));
    for (int i = 0; i < index; i++) {
        ret = pthread_join(tid[i], NULL);
        ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    }
EXIT:
    ret = pthread_cond_destroy(&g_cond3);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_mutex_destroy(&g_mtx3);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ICUNIT_ASSERT_EQUAL(testInt, TEST_SEEK_SIZE, testInt);
    return 0;
}

RUN_TEST_SUITE(PthreadCondApiTestSuite);

void PosixFutexCondTest(void)
{
    RUN_ONE_TESTCASE(testPthreadCondInit);
    RUN_ONE_TESTCASE(testPthreadCondDestroy);
    RUN_ONE_TESTCASE(testPthreadCondattrInit);
#if defined(LOSCFG_PLATFORM_HI3322)
    RUN_ONE_TESTCASE(testPthreadCondBroadcast);
#endif
}
