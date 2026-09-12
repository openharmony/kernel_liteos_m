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
 * of conditions and the following disclaimer in the documentation and/or materials
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

#include "It_posix_pthread.h"

static volatile int g_cleanupCalled = 0;

static void cleanupHandler(void *arg)
{
    (void)arg;
    g_cleanupCalled = 1;
}

static void *threadFunc(void *arg)
{
    (void)arg;
    pthread_cleanup_push(cleanupHandler, NULL);
    pthread_exit(NULL);
    pthread_cleanup_pop(0);
    return NULL;
}

static UINT32 Testcase(VOID)
{
    int ret;
    pthread_t thread;

    g_cleanupCalled = 0;

    ret = pthread_create(&thread, NULL, threadFunc, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(thread, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(g_cleanupCalled, 1, g_cleanupCalled);

    return LOS_OK;
}

/**
 * @tc.name: ItPosixPthread027
 * @tc.desc: Test pthread_cleanup_push/pop called on pthread_exit
 * @tc.type: FUNC
 * @tc.require: issueI5WZI6
 */

VOID ItPosixPthread027(void)
{
    TEST_ADD_CASE("ItPosixPthread027", Testcase, TEST_POSIX, TEST_PTHREAD, TEST_LEVEL0, TEST_FUNCTION);
}
