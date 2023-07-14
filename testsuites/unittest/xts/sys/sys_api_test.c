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

#include "sys_api.h"
#include "xts_test.h"

LITE_TEST_SUIT(SYSTEM, SysApiTest, SysApiTestSuite);

static BOOL SysApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL SysApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
* @tc.number    SUB_KERNEL_SYS_STRERROR_0100
* @tc.name      test strerror
* @tc.desc      [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(SysApiTestSuite, testStrerror, Function | MediumTest | Level1)
{
    ICUNIT_ASSERT_STRING_EQUAL(strerror(-1), "No error information", strerror(-1)); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(strerror(0), "No error information", strerror(0));
    ICUNIT_ASSERT_STRING_EQUAL(strerror(2), "No such file or directory", strerror(2)); /* 2, common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(strerror(10), "No child process", strerror(10)); /* 10, common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(strerror(65536), "No error information", strerror(65536)); /* 65536, common data for test, no special meaning */
}

RUN_TEST_SUITE(SysApiTestSuite);

void SysApiFuncTest(void)
{
    RUN_ONE_TESTCASE(testStrerror);
}
