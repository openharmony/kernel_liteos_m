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

/* pthread_mutex_destroy 4-2.c
 * Test that when a pthread_mutex_destroy is called on a
 *  locked mutex, it fails and returns EBUSY

 * Steps:
 * 1. Create a mutex
 * 2. Lock the mutex
 * 3. Try to destroy the mutex
 * 4. Check that this may fail with EBUSY
 */

static UINT32 Testcase(VOID)
{
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    int rc;

    /* Lock the mutex */
    rc = pthread_mutex_lock(&mutex);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    /* Try to destroy the locked mutex */
    rc = pthread_mutex_destroy(&mutex);
    ICUNIT_GOTO_EQUAL(rc, EBUSY, rc, EXIT);

    rc = pthread_mutex_unlock(&mutex);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    rc = pthread_mutex_destroy(&mutex);
    ICUNIT_ASSERT_EQUAL(rc, 0, rc);

    return LOS_OK;

EXIT:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixMux015
 * @tc.desc: Test interface pthread_mutex_lock
 * @tc.type: FUNC
 * @tc.require: issueI5WZI6
 */

VOID ItPosixMux015(void)
{
    TEST_ADD_CASE("ItPosixMux015", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
