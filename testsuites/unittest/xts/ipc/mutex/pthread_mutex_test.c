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

LITE_TEST_SUIT(FUTEX, PthreadMutexApiTest, PthreadMutexApiTestSuite);

static BOOL PthreadMutexApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL PthreadMutexApiTestSuiteTearDown(void)
{
    return TRUE;
}

/********************************************* Test case dividing line ***********************************************/

struct PthreadMutexCond {
    int loopNum;
    int countMax;
    int count;
    int top;
    int bottom;
    pthread_cond_t notfull;
    pthread_cond_t notempty;
    pthread_mutex_t mutex;
};

struct PthreadMutexCond g_st1;

void *PthreadProduce(void *arg)
{
    int ret;
    for (int i = 0; i < g_st1.loopNum; i++) {
        ret = pthread_mutex_lock(&g_st1.mutex);
        ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
        // check full
        if ((g_st1.top + 1) % g_st1.countMax == g_st1.bottom) { /* 1, common data for test, no special meaning */
            ret = pthread_cond_wait(&g_st1.notempty, &g_st1.mutex);
            ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
        }
        // Produce
        g_st1.top = (g_st1.top + 1) % g_st1.countMax; /* 1, common data for test, no special meaning */
        g_st1.count++;

        ret = pthread_cond_signal(&g_st1.notempty);
        ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
        ret = pthread_mutex_unlock(&g_st1.mutex);
        ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
        usleep(10); /* 10, common data for test, no special meaning */
    }
    return arg;
}

void *PthreadConsume(void *arg)
{
    int ret;
    for (int i = 0; i < g_st1.loopNum; i++) {
        ret = pthread_mutex_lock(&g_st1.mutex);
        ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
        // check empty
        if (g_st1.top == g_st1.bottom) {
            ret = pthread_cond_wait(&g_st1.notempty, &g_st1.mutex);
            ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
        }
        // Consume
        g_st1.bottom = (g_st1.bottom + 1) % g_st1.countMax;
        g_st1.count--;

        ret = pthread_cond_signal(&g_st1.notempty);
        ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
        ret = pthread_mutex_unlock(&g_st1.mutex);
        ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);

        usleep(10); /* 10, common data for test, no special meaning */
    }
    return arg;
}

/**
 * @tc.number     SUB_KERNEL_FUTEX_MUTEX_ALL_0300
 * @tc.name       test pthread_mutex with condition variable, produce and consume
 * @tc.desc       [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadMutexApiTestSuite, testPthreadMutexCond, Function | MediumTest | Level3)
{
    int ret;
    pthread_t tid[2]; /* 2, common data for test, no special meaning */

    g_st1.count = 0;
    memset_s(tid, sizeof(tid), POSIX_IPC_NO_ERROR, sizeof(tid));
    
    ret = pthread_create(&tid[0], NULL, PthreadProduce, NULL);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_create(&tid[1], NULL, PthreadConsume, NULL); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_join(tid[0], NULL);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ret = pthread_join(tid[1], NULL); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, POSIX_IPC_NO_ERROR, ret);
    ICUNIT_ASSERT_EQUAL(g_st1.count, POSIX_IPC_NO_ERROR, g_st1.count);
    return 0;
}

RUN_TEST_SUITE(PthreadMutexApiTestSuite);

void PosixFutexMutexTest(void)
{
    RUN_ONE_TESTCASE(testPthreadMutexCond);
}
