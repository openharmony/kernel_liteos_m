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

LITE_TEST_SUIT(IO, IoStdlibApi, IoStdlibApiTestSuite);

static BOOL IoStdlibApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoStdlibApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0400
 * @tc.name   strtof basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtof, Function | MediumTest | Level1)
{
    char *endPtr = NULL;
    float ret = strtof("  -3.40E+38 hello", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, (float)(-3.40E+38), ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " hello", endPtr);

    ret = strtof("  3.40E+38 ===", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, (float)(3.40E+38), ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " ===", endPtr);

    ret = strtof("-9.6e17  this   123", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, (float)(-9.6e17), ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "  this   123", endPtr);

    ret = strtof("this is string", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "this is string", endPtr);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0500
 * @tc.name   strtod basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtod, Function | MediumTest | Level1)
{
    char *endPtr = NULL;
    double ret = strtod("  -1.79E+308 hello", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, -1.79E+308, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " hello", endPtr);

    ret = strtod("1.79E+308  ===", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, 1.79E+308, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "  ===", endPtr);

    ret = strtod("-9.6e17  this   123", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, -9.6e17, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "  this   123", endPtr);

    ret = strtod("this is string", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "this is string", endPtr);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0600
 * @tc.name   strtold basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtold, Function | MediumTest | Level1)
{
    char *endPtr = NULL;
    long double ret = strtold("  2.22507e-308 hello", &endPtr);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " hello", endPtr);

    ret = strtold("  1.79769e+308 ===", &endPtr);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " ===", endPtr);

    ret = strtold("-9.6e17  this   123", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, -9.6e17, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "  this   123", endPtr);

    ret = strtold("this is string", &endPtr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "this is string", endPtr);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0700
 * @tc.name   strtol basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtol, Function | MediumTest | Level1)
{
    char nPtr[] = "12 0110 0XDEFE 0666 -1.6";
    char *endPtr = NULL;
    long ret = strtol(nPtr, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 12, ret); /* 12 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0110 0XDEFE 0666 -1.6", endPtr);

    ret = strtol(endPtr, &endPtr, 2); /* 2 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 6, ret); /* 6 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0XDEFE 0666 -1.6", endPtr);

    ret = strtol(endPtr, &endPtr, 16); /* 16 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0XDEFE, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0666 -1.6", endPtr);

    ret = strtol(endPtr, &endPtr, 8); /* 8 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0666, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " -1.6", endPtr);

    ret = strtol(endPtr, &endPtr, 65); /* 65 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " -1.6", endPtr);

    ret = strtol(endPtr, &endPtr, 0);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, ".6", endPtr);

    ret = strtol(endPtr, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, ".6", endPtr);

    char nPtr1[] = "2147483647 2147483648 -2147483648 -2147483649";
    ret = strtol(nPtr1, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 2147483647, ret); /* 2147483647 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 2147483648 -2147483648 -2147483649", endPtr);

    ret = strtol(endPtr, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 2147483647, ret); /* 2147483647 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " -2147483648 -2147483649", endPtr);

    ret = strtol(endPtr, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -2147483648, ret); /* -2147483648 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " -2147483649", endPtr);

    ret = strtol(endPtr, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -2147483648, ret); /* -2147483648 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "", endPtr);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_0800
 * @tc.name   strtoul basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtoul, Function | MediumTest | Level1)
{
    char nPtr[] = "12 0110 0XDEFE 0666 4294967295 4294967296 12.34";
    char *endPtr = NULL;
    unsigned long ret = strtoul(nPtr, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 12UL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0110 0XDEFE 0666 4294967295 4294967296 12.34", endPtr);

    ret = strtoul(endPtr, &endPtr, 2); /* 2 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 6UL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0XDEFE 0666 4294967295 4294967296 12.34", endPtr);

    ret = strtoul(endPtr, &endPtr, 16); /* 16 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0XDEFEUL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0666 4294967295 4294967296 12.34", endPtr);

    ret = strtoul(endPtr, &endPtr, 8); /* 8 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0666UL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 4294967295 4294967296 12.34", endPtr);

    ret = strtoul(endPtr, &endPtr, 0);
    ICUNIT_ASSERT_EQUAL(ret, 4294967295UL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 4294967296 12.34", endPtr);

    ret = strtoul(endPtr, &endPtr, 0);
    ICUNIT_ASSERT_EQUAL(ret, 4294967295UL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 12.34", endPtr);

    ret = strtoul(endPtr, &endPtr, 65); /* 65 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0UL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 12.34", endPtr);

    ret = strtoul(endPtr, &endPtr, 0);
    ICUNIT_ASSERT_EQUAL(ret, 12UL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, ".34", endPtr);

    ret = strtoul(endPtr, &endPtr, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0UL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, ".34", endPtr);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STDLIB_1000
 * @tc.name   strtoull basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStdlibApiTestSuite, testStrtoull, Function | MediumTest | Level1)
{
    char nPtr[] = "12 0110 0XDEFE 0666 1.6";
    char *endPtr = NULL;
    unsigned long long ret = strtoull(nPtr, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 12ULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0110 0XDEFE 0666 1.6", endPtr);

    ret = strtoull(endPtr, &endPtr, 2); /* 2 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 6ULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0XDEFE 0666 1.6", endPtr);

    ret = strtoull(endPtr, &endPtr, 16); /* 16 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0XDEFEULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0666 1.6", endPtr);

    ret = strtoull(endPtr, &endPtr, 8); /* 8 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0666ULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 1.6", endPtr);

    ret = strtoull(endPtr, &endPtr, 65); /* 65 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0ULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 1.6", endPtr);

    ret = strtoull(endPtr, &endPtr, 0);
    ICUNIT_ASSERT_EQUAL(ret, 1ULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, ".6", endPtr);

    ret = strtoull(endPtr, &endPtr, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0ULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, ".6", endPtr);

    char nPtr1[] = "18446744073709551615 18446744073709551616";
    ret = strtoull(nPtr1, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 18446744073709551615ULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 18446744073709551616", endPtr);

    ret = strtoull(endPtr, &endPtr, 10); /* 10 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 18446744073709551615ULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "", endPtr);
    return 0;
}

RUN_TEST_SUITE(IoStdlibApiTestSuite);

void XtsIoStdlibFuncTest(void)
{
    RUN_ONE_TESTCASE(testStrtof);
    RUN_ONE_TESTCASE(testStrtod);
    RUN_ONE_TESTCASE(testStrtold);
    RUN_ONE_TESTCASE(testStrtol);
    RUN_ONE_TESTCASE(testStrtoul);
    RUN_ONE_TESTCASE(testStrtoull);
}
