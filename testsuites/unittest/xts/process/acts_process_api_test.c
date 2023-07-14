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

LITE_TEST_SUIT(PROCESS, ProcessApi, ProcessApiTestSuite);

static BOOL ProcessApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL ProcessApiTestSuiteTearDown(void)
{
    return TRUE;
}

static void *ThreadFunc(void* arg)
{
    return NULL;
}

/**
* @tc.number     SUB_KERNEL_NDKAPI_PROCESS_PTHREAD_SETNAME_NP_1000
* @tc.name       test pthread_setname_np api para stringlong
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(ProcessApiTestSuite, testPthreadSetnameNp1000, Function | MediumTest | Level1) {
    pthread_t thisThread;
    int returnVal = pthread_create(&thisThread, NULL, ThreadFunc, NULL);
    returnVal = pthread_setname_np(thisThread, "funcThreadNamelongName");
    ICUNIT_ASSERT_NOT_EQUAL(returnVal, 0, returnVal);
    ICUNIT_ASSERT_EQUAL(returnVal, ERANGE, returnVal);
    return 0;
}

RUN_TEST_SUITE(ProcessApiTestSuite);

void ActsProcessApiTest(void)
{
    RUN_ONE_TESTCASE(testPthreadSetnameNp1000);
}

