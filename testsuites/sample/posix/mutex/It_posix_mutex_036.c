/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

#include "It_posix_mutex.h"

#define THREAD_NUM 5
#define LOOPS 4

static g_value;
static pthread_mutex_t g_mutex036 = PTHREAD_MUTEX_INITIALIZER;

/* pthread_mutex_lock 1-1.c
 * Test that pthread_mutex_lock()
 *   shall lock the mutex object referenced by 'mutex'.  If the mutex is
 *   already locked, the calling thread shall block until the mutex becomes
 *   available. This operation shall return with the mutex object referenced
 *   by 'mutex' in the locked state with the calling thread as its owner.

 * Steps:
 *   -- Initialize a mutex to protect a global variable 'value'
 *   -- Create N threads. Each is looped M times to acquire the mutex,
 *      increase the value, and then release the mutex.
 *   -- Check if the value has increased properly (M*N); a broken mutex
 *      implementation may cause lost augments.
 *
 */

static void *TaskF01(void *parm)
{
    int i, tmp;
    int rc;

    /* Loopd M times to acquire the mutex, increase the value,
       and then release the mutex. */
    for (i = 0; i < LOOPS; ++i) {
        rc = pthread_mutex_lock(&g_mutex036);
        if (rc != 0) {
            return (void *)(LOS_NOK);
        }

        tmp = g_value;
        tmp = tmp + 1;
        sleep(1);
        g_value = tmp;

        rc = pthread_mutex_unlock(&g_mutex036);
        if (rc != 0) {
            return (void *)(LOS_NOK);
        }
        sleep(1);
    }

    return (void *)(LOS_OK);
}

static UINT32 Testcase(VOID)
{
    int i, rc;
    pthread_attr_t pta;
    pthread_t threads[THREAD_NUM];

    g_value = 0;

    rc = pthread_attr_init(&pta);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    rc = pthread_attr_setdetachstate(&pta, PTHREAD_CREATE_JOINABLE);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    /* Create threads */
    for (i = 0; i < THREAD_NUM; ++i) {
        rc = pthread_create(&threads[i], &pta, TaskF01, NULL);
        ICUNIT_ASSERT_EQUAL(rc, 0, rc);
    }

    /* Wait to join all threads */
    for (i = 0; i < THREAD_NUM; ++i) {
        rc = pthread_join(threads[i], NULL);
        ICUNIT_ASSERT_EQUAL(rc, 0, rc);
    }

    rc = pthread_attr_destroy(&pta);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    rc = pthread_mutex_destroy(&g_mutex036);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    /* Check if the final value is as expected */
    ICUNIT_ASSERT_EQUAL(g_value, (THREAD_NUM) * LOOPS, g_value);

    return LOS_OK;
}

/**
 * @tc.name: ItPosixMux035
 * @tc.desc: Test interface pthread_mutex_unlock
 * @tc.type: FUNC
 * @tc.require: issueI5WZI6
 */

VOID ItPosixMux036(void)
{
    TEST_ADD_CASE("ItPosixMux036", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
