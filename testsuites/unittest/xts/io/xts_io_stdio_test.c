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

LITE_TEST_SUIT(IO, IoStdio, IoStdioTestSuite);

static BOOL IoStdioTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoStdioTestSuiteTearDown(void)
{
    return TRUE;
}

int FormatVsnptf(char *format, ...)
{
    va_list vArgList;
    va_start(vArgList, format);
    char str[50] = {0}; /* 50 common data for test, no special meaning */
    int ret = vsnprintf_s(str, sizeof(str), sizeof(str), format, vArgList);
    va_end(vArgList);
    return ret;
}

/**
 * @tc.number SUB_KERNEL_IO_STDIO_2100
 * @tc.name   vsnprintf basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdioTestSuite, testVsnprintf, Function | MediumTest | Level1)
{
    int ret = FormatVsnptf((char *)"%s has %d words", "hello world", 11); /* 11 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 24, ret); /* 24 common data for test, no special meaning */

    ret = FormatVsnptf((char *)"%f and %c as well as %ld\n", 2.2, 'c', 6); /* 2.2, 6 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 28, ret); /* 28 common data for test, no special meaning */
    return 0;
}

RUN_TEST_SUITE(IoStdioTestSuite);

void XtsIoStdioFuncTest(void)
{
    RUN_ONE_TESTCASE(testVsnprintf);
}
