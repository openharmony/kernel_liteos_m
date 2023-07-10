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

#include "xts_io.h"

LITE_TEST_SUIT(IO, IoStringsApi, IoStringsApiTestSuite);

static BOOL IoStringsApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoStringsApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_IO_STRINGS_0200
 * @tc.name   strncasecmp basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringsApiTestSuite, testStrncasecmp, Function | MediumTest | Level1)
{
    int ret = strncasecmp("abcdefg", "abcdEFg", 7); /* 7 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = strncasecmp("abcdefg", "abcdEF", 7); /* 7 common data for test, no special meaning */
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 0, INT_MAX, ret);

    ret = strncasecmp("abcdef", "abcdEFg", 7); /* 7 common data for test, no special meaning */
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, INT_MIN, 0, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRINGS_0300
 * @tc.name   strcasecmp basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringsApiTestSuite, testStrcasecmp, Function | MediumTest | Level2)
{
    int ret = strcasecmp("abcdefg", "abcdEFg");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = strcasecmp("abcdefg", "abcdEF");
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 0, INT_MAX, ret);

    ret = strcasecmp("abcdef", "abcdEFg");
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, INT_MIN, 0, ret);
    return 0;
}

RUN_TEST_SUITE(IoStringsApiTestSuite);

void XtsIoStringsFuncTest(void)
{
    RUN_ONE_TESTCASE(testStrncasecmp);
    RUN_ONE_TESTCASE(testStrcasecmp);
}
