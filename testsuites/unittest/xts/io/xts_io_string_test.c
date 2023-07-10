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

LITE_TEST_SUIT(IO, IoStringApi, IoStringApiTestSuite);

static BOOL IoStringApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoStringApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_0300
 * @tc.name   strcmp basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrcmp, Function | MediumTest | Level1)
{
    int ret = strcmp("abcdef", "ABCDEF");
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 0, INT_MAX, ret);

    ret = strcmp("123456", "654321");
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, INT_MIN, 0, ret);

    ret = strcmp("~!@#$%^&*()_+", "~!@#$%^&*()_+");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_0400
 * @tc.name   strncmp basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrncmp, Function | MediumTest | Level1)
{
    const char dest[] = "abcdef";
    const char src[] = "ABCDEF";
    int ret = strncmp(dest, src, 1); /* 1 common data for test, no special meaning */
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 0, INT_MAX, ret);

    ret = strncmp("123456", "654321", 6); /* 6 common data for test, no special meaning */
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, INT_MIN, 0, ret);

    ret = strncmp("~!@#$%^&*()_+", "~!@#$%^&*()_+", 8); /* 8 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_0700
 * @tc.name   stpcpy basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStpcpy, Function | MediumTest | Level1)
{
    const char *src = "hello world !";
    char dest[50] = {0}; /* 50 common data for test, no special meaning */
    char *ret = stpcpy(dest, src);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(dest, src, dest);

    const char *srcT = "~!@#$%%^&*()_+";
    char destT[50] = {0}; /* 50 common data for test, no special meaning */
    ret = stpcpy(destT, srcT);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_STRING_EQUAL(destT, srcT, destT);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_0800
 * @tc.name   stpncpy basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStpncpy, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    char dest[50] = {0}; /* 50 common data for test, no special meaning */
    char *ret = stpncpy(dest, src, 5); /* 5 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(ret, "", ret);
    ICUNIT_ASSERT_STRING_EQUAL(dest, "hello", dest);

    char srcT[] = "~!@#$%%^&*()_+";
    char destT[50] = {0}; /* 50 common data for test, no special meaning */
    ret = stpncpy(destT, srcT, 15); /* 15 common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(ret, "", ret);
    ICUNIT_ASSERT_STRING_EQUAL(destT, "~!@#$%%^&*()_+", destT);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1000
 * @tc.name   strcpy basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrcpy, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    char dest[20] = {0}; /* 20 common data for test, no special meaning */
    int ret = strcpy_s(dest, sizeof(dest), src);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    char srcT[] = "This is String1";
    char destT[20] = {0}; /* 20 common data for test, no special meaning */
    ret = strcpy_s(destT, sizeof(destT), srcT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1100
 * @tc.name   strncpy basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrncpy, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    char dest[20] = {0};
    int ret = strncpy_s(dest, sizeof(dest), src, 6); /* 6 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    char destT[] = "~!@#$%^&*()_+";
    ret = strncpy_s(destT, sizeof(destT), "hello world", 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1400
 * @tc.name   strcat basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrcat, Function | MediumTest | Level1)
{
    char dest[50] = {0}; /* 50 common data for test, no special meaning */
    int ret = strcat_s(dest, sizeof(dest), "hello world !");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    char destT[50] = "hello world ! || "; /* 50 common data for test, no special meaning */
    ret = strcat_s(destT, sizeof(destT), "This is String1");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1500
 * @tc.name   strncat basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrncat, Function | MediumTest | Level1)
{
    char dest[50] = "hello world || "; /* 50 common data for test, no special meaning */
    int ret = strncat_s(dest, sizeof(dest), "This is String1", 7); /* 7 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    char destT[20] = "|/*"; /* 20 common data for test, no special meaning */
    ret = strncat_s(destT, sizeof(destT), "~!@#$%^&*()_+", 13); /* 13 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1600
 * @tc.name   strchr basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrchr, Function | MediumTest | Level1)
{
    char src[] = "hello !! world";
    char *ret = strchr(src, '!');
    ICUNIT_ASSERT_STRING_EQUAL(ret, "!! world", ret);

    ret = strchr(src, '?');
    ICUNIT_ASSERT_STRING_EQUAL(ret, NULL, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1700
 * @tc.name   strrchr basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrrchr, Function | MediumTest | Level1)
{
    char src[] = "hello world & HEELO & WORLD";
    char *ret = strrchr(src, '&');
    ICUNIT_ASSERT_STRING_EQUAL(ret, "& WORLD", ret);

    ret = strrchr(src, '?');
    ICUNIT_ASSERT_STRING_EQUAL(ret, NULL, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1800
 * @tc.name   strnlen basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrnlen, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    unsigned int ltSrcSize = strlen(src) - 2; /* 2 common data for test, no special meaning */
    unsigned int gtSrcSize = strlen(src) + 2; /* 2 common data for test, no special meaning */

    size_t ret = strnlen(src, ltSrcSize);
    ICUNIT_ASSERT_EQUAL(ret, ltSrcSize, ret);

    ret = strnlen(src, gtSrcSize);
    ICUNIT_ASSERT_EQUAL(ret, strlen(src), ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_1900
 * @tc.name   strtok basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrtok, Function | MediumTest | Level1)
{
    char src[] = "hello world & This is-String1";
    char *ret = strtok(src, "&");
    ICUNIT_ASSERT_STRING_EQUAL(ret, src, ret);

    ret = strtok(NULL, "-");
    ICUNIT_ASSERT_STRING_EQUAL(ret, " This is", ret);

    char srcS[] = "hello world !";
    ret = strtok(srcS, "?");
    ICUNIT_ASSERT_STRING_EQUAL(ret, srcS, ret);

    char srcT[50] = {0}; /* 50 common data for test, no special meaning */
    ret = strtok(srcT, "~");
    ICUNIT_ASSERT_STRING_EQUAL(ret, NULL, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_2200
 * @tc.name   strdup basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrdup, Function | MediumTest | Level1)
{
    char src[] = "hello world !";
    char *ret = strdup(src);
    ICUNIT_ASSERT_STRING_EQUAL(ret, src, ret);

    char srcS[] = "This is String1";
    ret = strdup(srcS);
    ICUNIT_ASSERT_STRING_EQUAL(ret, "This is String1", ret);
    free(ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_2500
 * @tc.name   strcspn basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrcspn, Function | MediumTest | Level1)
{
    const char dest[] = "hello world !";
    const char src[] = "!";
    size_t ret = strcspn(dest, src);
    ICUNIT_ASSERT_EQUAL(ret, 12U, ret);

    const char srcS[] = "a";
    ret = strcspn(dest, srcS);
    ICUNIT_ASSERT_EQUAL(ret, 13U, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_2600
 * @tc.name   strspn basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrspn, Function | MediumTest | Level1)
{
    const char dest[] = "hello world !";
    const char src[] = "heAlo";
    size_t ret = strspn(dest, src);
    ICUNIT_ASSERT_EQUAL(ret, 5U, ret);
    ICUNIT_ASSERT_EQUAL(ret, 5U, ret);

    const char destS[] = "this is string";
    const char srcS[] = "abc";
    ret = strspn(destS, srcS);
    ICUNIT_ASSERT_EQUAL(ret, 0U, ret);

    const char srcT[] = "helo\0 wal";
    ret = strspn(dest, srcT);
    ICUNIT_ASSERT_EQUAL(ret, 5U, ret);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_IO_STRING_2700
 * @tc.name   strstr basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoStringApiTestSuite, testStrstr, Function | MediumTest | Level1)
{
    const char dest[] = "hello world !";
    const char src[] = "heAlo";
    const char *ret = strstr(dest, src);
    ICUNIT_ASSERT_STRING_EQUAL(ret, NULL, ret);

    const char destS[] = "string this is string";
    const char srcS[] = "string";
    ret = strstr(destS, srcS);
    ICUNIT_ASSERT_STRING_EQUAL(ret, "string this is string", ret);

    const char srcT[] = "\0hello";
    ret = strstr(dest, srcT);
    ICUNIT_ASSERT_STRING_EQUAL(ret, "hello world !", ret);
    return 0;
}

RUN_TEST_SUITE(IoStringApiTestSuite);

void XtsIoStringFuncTest(void)
{
    RUN_ONE_TESTCASE(testStrcmp);
    RUN_ONE_TESTCASE(testStrncmp);
    RUN_ONE_TESTCASE(testStpcpy);
    RUN_ONE_TESTCASE(testStpncpy);
    RUN_ONE_TESTCASE(testStrcpy);
    RUN_ONE_TESTCASE(testStrncpy);
    RUN_ONE_TESTCASE(testStrcat);
    RUN_ONE_TESTCASE(testStrncat);
    RUN_ONE_TESTCASE(testStrchr);
    RUN_ONE_TESTCASE(testStrrchr);
    RUN_ONE_TESTCASE(testStrnlen);
    RUN_ONE_TESTCASE(testStrtok);
    RUN_ONE_TESTCASE(testStrdup);
    RUN_ONE_TESTCASE(testStrcspn);
    RUN_ONE_TESTCASE(testStrspn);
    RUN_ONE_TESTCASE(testStrstr);
}
