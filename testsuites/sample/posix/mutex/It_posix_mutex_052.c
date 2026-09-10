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

#include "It_posix_mutex.h"

static UINT32 Testcase(VOID)
{
    int ret;
    int prioceiling;
    pthread_mutexattr_t attr;

    ret = pthread_mutexattr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_setprioceiling(&attr, 10);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_getprioceiling(&attr, &prioceiling);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(prioceiling, 10, prioceiling);

    ret = pthread_mutexattr_setprioceiling(&attr, LOS_TASK_PRIORITY_HIGHEST);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_getprioceiling(&attr, &prioceiling);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(prioceiling, LOS_TASK_PRIORITY_HIGHEST, prioceiling);

    ret = pthread_mutexattr_setprioceiling(&attr, LOS_TASK_PRIORITY_LOWEST);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_getprioceiling(&attr, &prioceiling);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(prioceiling, LOS_TASK_PRIORITY_LOWEST, prioceiling);

    ret = pthread_mutexattr_setprioceiling(NULL, 10);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_mutexattr_getprioceiling(NULL, &prioceiling);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_mutexattr_getprioceiling(&attr, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

/**
 * @tc.name: ItPosixMux052
 * @tc.desc: Test interface pthread_mutexattr_setprioceiling/getprioceiling
 * @tc.type: FUNC
 * @tc.require: issueI5WZI6
 */

VOID ItPosixMux052(void)
{
    TEST_ADD_CASE("ItPosixMux052", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
