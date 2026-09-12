/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
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
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED BY ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "It_posix_rwlock.h"

static UINT32 Testcase(VOID)
{
    int ret;
    pthread_rwlock_t rwl;
    pthread_rwlockattr_t attr;
    int pshared = -1;

    ret = pthread_rwlockattr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlockattr_getpshared(&attr, &pshared);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(pshared, PTHREAD_PROCESS_PRIVATE, pshared, EXIT);

    ret = pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    ICUNIT_GOTO_EQUAL(ret, ENOSYS, ret, EXIT);

    ret = pthread_rwlockattr_getpshared(NULL, &pshared);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_rwlockattr_getpshared(&attr, NULL);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_rwlock_init(&rwl, &attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlock_destroy(&rwl);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

EXIT:
    pthread_rwlockattr_destroy(&attr);
    return LOS_OK;
}

VOID ItPosixRwlock009(void)
{
    TEST_ADD_CASE("ItPosixRwlock009", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
