/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "ohos_types.h"
#include "posix_test.h"
#include "los_config.h"
#include "kernel_test.h"
#include "ctype.h"
#include "stdlib.h"
#include "string.h"
#include "errno.h"
#include "limits.h"
#include "log.h"

/* *
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is CmsisTaskFuncTestSuite
 */
LITE_TEST_SUIT(Posix, Posixtimer, PosixStdlibStrtolTest);

/* *
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL PosixStdlibStrtolTestSetUp(void)
{
    return TRUE;
}

/* *
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL PosixStdlibStrtolTestTearDown(void)
{
    LOG("+-------------------------------------------+\n");
    return TRUE;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_001
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol001, Function | MediumTest | Level1)
{
    char nPtr16[] = " 10";
    char *endPtr16 = NULL;
    long ret = strtol(nPtr16, &endPtr16, 16);
    if (ret == 16) {
        LOG("[DEMO] posix stdlib test case 1:strtol(base=16) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr16, endPtr16);
    } else {
        LOG("[DEMO] posix stdlib test case 1:strtol(base=16) ret:%ld,%s fail.\n", ret, nPtr16);
    }
    ICUNIT_ASSERT_EQUAL(ret, 16, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr16, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_002
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol002, Function | MediumTest | Level1)
{
    char nPtr16[] = "0x10";
    char *endPtr16 = NULL;
    long ret = strtol(nPtr16, &endPtr16, 0);
    if (ret == 16) {
        LOG("[DEMO] posix stdlib test case 2:strtol(base=16) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr16, endPtr16);
    } else {
        LOG("[DEMO] posix stdlib test case 2:strtol(base=16) ret:%ld,%s fail.\n", ret, nPtr16);
    }
    ICUNIT_ASSERT_EQUAL(ret, 16, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr16, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_003
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol003, Function | MediumTest | Level1)
{
    char nPtr10[] = "10";
    char *endPtr10 = NULL;
    long ret = strtol(nPtr10, &endPtr10, 10);
    if (ret == 10) {
        LOG("[DEMO] posix stdlib test case 3:strtol(base=10) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr10, endPtr10);
    } else {
        LOG("[DEMO] posix stdlib test case 3:strtol(base=10) ret:%ld,%s fail.\n", ret, nPtr10);
    }
    ICUNIT_ASSERT_EQUAL(ret, 10, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr10, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_004
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol004, Function | MediumTest | Level1)
{
    char nPtr10[] = "-10";
    char *endPtr10 = NULL;
    long ret = strtol(nPtr10, &endPtr10, 10);
    if (ret == -10) {
        LOG("[DEMO] posix stdlib test case 4:strtol(base=10) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr10, endPtr10);
    } else {
        LOG("[DEMO] posix stdlib test case 4:strtol(base=10) ret:%ld,%s fail.\n", ret, nPtr10);
    }
    ICUNIT_ASSERT_EQUAL(ret, -10, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr10, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_005
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol005, Function | MediumTest | Level1)
{
    char nPtr10_3[] = "10";
    char *endPtr10_3 = NULL;
    long ret = strtol(nPtr10_3, &endPtr10_3, 0);
    if (ret == 10) {
        LOG("[DEMO] posix stdlib test case 5:strtol(base=0) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr10_3, endPtr10_3);
    } else {
        LOG("[DEMO] posix stdlib test case 5:strtol(base=0) ret:%ld,%s fail.\n", ret, nPtr10_3);
    }
    ICUNIT_ASSERT_EQUAL(ret, 10, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr10_3, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_006
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol006, Function | MediumTest | Level1)
{
    char nPtr8[] = "10";
    char *endPtr8 = NULL;
    long ret = strtol(nPtr8, &endPtr8, 8);
    if (ret == 8) {
        LOG("[DEMO] posix stdlib test case 6:strtol(base=8) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr8, endPtr8);
    } else {
        LOG("[DEMO] posix stdlib test case 6:strtol(base=8) ret:%ld,%s fail.\n", ret, nPtr8);
    }
    ICUNIT_ASSERT_EQUAL(ret, 8, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr8, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_007
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol007, Function | MediumTest | Level1)
{
    char nPtr8_2[] = "010";
    char *endPtr8_2 = NULL;
    long ret = strtol(nPtr8_2, &endPtr8_2, 8);
    if (ret == 8) {
        LOG("[DEMO] posix stdlib test case 7:strtol(base=8) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr8_2, endPtr8_2);
    } else {
        LOG("[DEMO] posix stdlib test case 7:strtol(base=8) ret:%ld,%s fail.\n", ret, nPtr8_2);
    }
    ICUNIT_ASSERT_EQUAL(ret, 8, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr8_2, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_008
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol008, Function | MediumTest | Level1)
{
    char nPtr8_3[] = "010";
    char *endPtr8_3 = NULL;
    long ret = strtol(nPtr8_3, &endPtr8_3, 0);
    if (ret == 8) {
        LOG("[DEMO] posix stdlib test case 8:strtol(base=8) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr8_3, endPtr8_3);
    } else {
        LOG("[DEMO] posix stdlib test case 8:strtol(base=8) ret:%ld,%s fail.\n", ret, nPtr8_3);
    }
    ICUNIT_ASSERT_EQUAL(ret, 8, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr8_3, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_009
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol009, Function | MediumTest | Level1)
{
    char nPtr2[] = "10";
    char *endPtr2 = NULL;
    long ret = strtol(nPtr2, &endPtr2, 2);
    if (ret == 2) {
        LOG("[DEMO] posix stdlib test case 9:strtol(base=2) ret:%ld,%s, endPtr:%s ok.\n", ret, nPtr2, endPtr2);
    } else {
        LOG("[DEMO] posix stdlib test case 9:strtol(base=2) ret:%ld,%s fail.\n", ret, nPtr2);
    }
    ICUNIT_ASSERT_EQUAL(ret, 2, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr2, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_010
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol010, Function | MediumTest | Level1)
{
    char nPtr[] = "12 0110 0XDEFE 0666 -1.6";
    char *endPtr = NULL;
    long ret = strtol(nPtr, &endPtr, 10);
    if (ret == 12) {
        LOG("[DEMO] posix stdlib test case 10:strtol(base=10) ret:%ld, %s, endPtr:%s ok.\n", ret, endPtr, endPtr);
    } else {
        LOG("[DEMO] posix stdlib test case 10:strtol(base=10) ret:%ld, %s fail.\n", ret, endPtr);
    }
    ICUNIT_ASSERT_EQUAL(ret, 12, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " 0110 0XDEFE 0666 -1.6", 0);
    return 0;
}

#if (LOSCFG_LIBC_MUSL == 1)
/* *
 * @tc.number    : TEST_STDLIB_STRTOL_011
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol011, Function | MediumTest | Level1)
{
    char nPtr[] = "12 1.5";
    char *endPtr = NULL;
    long ret = strtol(nPtr, &endPtr, 65);
    if (ret == 0) {
        LOG("[DEMO] posix stdlib test case 11:strtol(base=65) ret:%ld, %s, endPtr:%s ok.\n", ret, endPtr, endPtr);
    } else {
        LOG("[DEMO] posix stdlib test case 11:strtol(base=65) ret:%ld, %s fail.\n", ret, endPtr);
    }
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "12 1.5", 0);
    return 0;
}
#endif

LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol012, Function | MediumTest | Level1)
{
    char nPtr[] = "2147483647 -2147483648";
    char *endPtr = NULL;
    long ret = strtol(nPtr, &endPtr, 10);
    if (ret == 2147483647) {
        LOG("[DEMO] posix stdlib test case 12:strtol(base=10) ret:%ld, %s, endPtr:%s ok.\n", ret, endPtr, endPtr);
    } else {
        LOG("[DEMO] posix stdlib test case 12:strtol(base=10) ret:%ld, %s fail.\n", ret, endPtr);
    }
    ICUNIT_ASSERT_EQUAL(ret, 2147483647, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, " -2147483648", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_ATOL_002
 * @tc.name      : convert string to long integer
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtol013, Function | MediumTest | Level1)
{
    char nPtr[] = " -2147483648";
    char *endPtr = NULL;
    long ret = strtol(nPtr, &endPtr, 10);
    if (ret == -2147483648) {
        LOG("[DEMO] posix stdlib test case 13:strtol(base=10) ret:%ld, %s, endPtr:%s ok.\n", ret, endPtr, endPtr);
    } else {
        LOG("[DEMO] posix stdlib test case 13:strtol(base=10) ret:%ld, %s fail.\n", ret, endPtr);
    }
    ICUNIT_ASSERT_EQUAL(ret, -2147483648, ret);
    ICUNIT_ASSERT_STRING_EQUAL(endPtr, "", 0);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_014
 * @tc.name      : strtol overflow is clamped to LONG_MAX/LONG_MIN and sets ERANGE
 * @tc.desc      : [C- SOFTWARE -0200]
 * 移植自 musl libc-test/src/functional/strtol.c:36-44（sizeof(long)==4 分支）
 * 及 :61-66（sizeof(long)==8 分支）：上溢 clamp 到边界值、errno 置 ERANGE、
 * endptr 停在被完整消费的数字串尾。
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtolOverflow001, Function | MediumTest | Level1)
{
    char *endPtr = NULL;
    long ret;

    if (sizeof(long) == 4) {
        /* 32 位 long：正向上溢出 2147483648 clamp 到 LONG_MAX 并置 ERANGE */
        char nPtrPos[] = "2147483648";
        errno = 0;
        ret = strtol(nPtrPos, &endPtr, 10);
        ICUNIT_ASSERT_EQUAL(ret, LONG_MAX, ret);
        ICUNIT_ASSERT_EQUAL(errno, ERANGE, errno);
        ICUNIT_ASSERT_EQUAL(endPtr - nPtrPos, 10, (int)(endPtr - nPtrPos));

        /* 32 位 long：负向下溢出 -2147483649 clamp 到 LONG_MIN 并置 ERANGE */
        char nPtrNeg[] = "-2147483649";
        errno = 0;
        ret = strtol(nPtrNeg, &endPtr, 10);
        ICUNIT_ASSERT_EQUAL(ret, LONG_MIN, ret);
        ICUNIT_ASSERT_EQUAL(errno, ERANGE, errno);
        ICUNIT_ASSERT_EQUAL(endPtr - nPtrNeg, 11, (int)(endPtr - nPtrNeg));
    } else {
        /* 64 位 long：对应 musl :61-66 的 64 位边界值 */
        char nPtrPos[] = "9223372036854775808";
        errno = 0;
        ret = strtol(nPtrPos, &endPtr, 10);
        ICUNIT_ASSERT_EQUAL(ret, LONG_MAX, ret);
        ICUNIT_ASSERT_EQUAL(errno, ERANGE, errno);
        ICUNIT_ASSERT_EQUAL(endPtr - nPtrPos, 19, (int)(endPtr - nPtrPos));

        char nPtrNeg[] = "-9223372036854775809";
        errno = 0;
        ret = strtol(nPtrNeg, &endPtr, 10);
        ICUNIT_ASSERT_EQUAL(ret, LONG_MIN, ret);
        ICUNIT_ASSERT_EQUAL(errno, ERANGE, errno);
        ICUNIT_ASSERT_EQUAL(endPtr - nPtrNeg, 20, (int)(endPtr - nPtrNeg));
    }
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_015
 * @tc.name      : strtol with various bases (36/2/16) and invalid base 37
 * @tc.desc      : [C- SOFTWARE -0200]
 * 移植自 musl libc-test/src/functional/strtol.c:118-131：base36 单字符 'z'==35、
 * 全二进制串按 base2 求值、base16 下 "0xz" 精确停位（endptr 偏移 1）、
 * base37 非法返回 0 且 endptr 不动且 errno 置 EINVAL。
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtolBase001, Function | MediumTest | Level1)
{
    char nPtr[] = "0xz";
    char *endPtr = NULL;
    long ret;

    /* base36：'z' 数值为 35 */
    ret = strtol("z", NULL, 36);
    ICUNIT_ASSERT_EQUAL(ret, 35, ret);

    /* base2：全二进制串等于对应十六进制值 0x12345678 */
    ret = strtol("00010010001101000101011001111000", NULL, 2);
    ICUNIT_ASSERT_EQUAL(ret, 0x12345678, ret);

    /* base16："0xz" 中 'x' 后无有效十六进制数字，只消费前缀 "0"，endptr 偏移 1 */
    ret = strtol(nPtr, &endPtr, 16);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(endPtr - nPtr, 1, (int)(endPtr - nPtr));

    /* base37 非法：返回 0、endptr 不动、errno 置 EINVAL（__intscan 直透） */
    char nPtr37[] = "123";
    errno = 0;
    ret = strtol(nPtr37, &endPtr, 37);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(endPtr - nPtr37, 0, (int)(endPtr - nPtr37));
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    return 0;
}

/* *
 * @tc.number    : TEST_STDLIB_STRTOL_016
 * @tc.name      : strtol skips leading whitespace and handles sign-only/empty strings
 * @tc.desc      : [C- SOFTWARE -0200]
 * 移植自 musl libc-test/src/functional/strtol.c:133-137 思想：前导空白被跳过后
 * 按给定 base 解析（"  015437" base8 == 015437）；"+"/"-"/空串无转换发生，
 * 返回 0 且 endptr 不动（与 musl 对 "  15437" 偏移 7 的断言同构，此处为带 0 前缀变体）。
 */
LITE_TEST_CASE(PosixStdlibStrtolTest, testStdlibStrtolWs001, Function | MediumTest | Level1)
{
    char nPtr[] = "  015437";
    char *endPtr = NULL;
    long ret;

    /* 前导空白跳过后按八进制解析，endptr 指向串尾（偏移 8） */
    ret = strtol(nPtr, &endPtr, 8);
    ICUNIT_ASSERT_EQUAL(ret, 015437, ret);
    ICUNIT_ASSERT_EQUAL(endPtr - nPtr, 8, (int)(endPtr - nPtr));

    /* 仅正号：无转换发生，返回 0 且 endptr 不动 */
    char nPtrPlus[] = "+";
    ret = strtol(nPtrPlus, &endPtr, 10);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(endPtr - nPtrPlus, 0, (int)(endPtr - nPtrPlus));

    /* 仅负号：无转换发生，返回 0 且 endptr 不动 */
    char nPtrMinus[] = "-";
    ret = strtol(nPtrMinus, &endPtr, 10);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(endPtr - nPtrMinus, 0, (int)(endPtr - nPtrMinus));

    /* 空串：无转换发生，返回 0 且 endptr 不动 */
    char nPtrEmpty[] = "";
    ret = strtol(nPtrEmpty, &endPtr, 10);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(endPtr - nPtrEmpty, 0, (int)(endPtr - nPtrEmpty));
    return 0;
}

RUN_TEST_SUITE(PosixStdlibStrtolTest);

void PosixStdlibStrtolFuncTest()
{
    LOG("begin PosixStdlibStrtolFuncTest....");
    RUN_ONE_TESTCASE(testStdlibStrtol001);
    RUN_ONE_TESTCASE(testStdlibStrtol002);
    RUN_ONE_TESTCASE(testStdlibStrtol003);
    RUN_ONE_TESTCASE(testStdlibStrtol004);
    RUN_ONE_TESTCASE(testStdlibStrtol005);
    RUN_ONE_TESTCASE(testStdlibStrtol006);
    RUN_ONE_TESTCASE(testStdlibStrtol007);
    RUN_ONE_TESTCASE(testStdlibStrtol008);
    RUN_ONE_TESTCASE(testStdlibStrtol009);
    RUN_ONE_TESTCASE(testStdlibStrtol010);
#if (LOSCFG_LIBC_MUSL == 1)
    RUN_ONE_TESTCASE(testStdlibStrtol011);
#endif
    RUN_ONE_TESTCASE(testStdlibStrtol012);
    RUN_ONE_TESTCASE(testStdlibStrtol013);
    RUN_ONE_TESTCASE(testStdlibStrtolOverflow001);
    RUN_ONE_TESTCASE(testStdlibStrtolBase001);
    RUN_ONE_TESTCASE(testStdlibStrtolWs001);

    return;
}