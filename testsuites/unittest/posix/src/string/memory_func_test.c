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

#include <string.h>
#include <stdlib.h>
#include "ohos_types.h"
#include "posix_test.h"
#include "los_config.h"
#include "kernel_test.h"
#include "log.h"

/* *
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is PosixMemFuncTestSuite
 */
LITE_TEST_SUIT(Posix, PosixMemory, PosixMemFuncTestSuite);


/* *
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL PosixMemFuncTestSuiteSetUp(void)
{
    return TRUE;
}

/* *
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL PosixMemFuncTestSuiteTearDown(void)
{
    printf("+Hello this is a memory function test+\n");
    return TRUE;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCPY_OPERATION_001
 * @tc.name      : Memony operation for memcpy test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcpy001, Function | MediumTest | Level1)
{
    void *retValue = NULL;
    char source[] = {"This File is About Memony Operation Test , Please Carefully Check Result As Below\r\n"};
    char dest[1024] = {0};
    retValue = memcpy(dest, source, sizeof(source) / sizeof(source[0]));

    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);

    retValue = memcpy(source, dest, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);

    ICUNIT_ASSERT_STRING_EQUAL(dest, source, 0);

    int len;
    char chr = 'A';
    char buf[1024];

    len = sizeof(buf) / sizeof(buf[0]);
    for (int i = 0; i < len; i++) {
        buf[i] = chr + rand() % 26;
    }

    memcpy(&buf[16], &buf[0], 16);
    for (int i = 0; i < 16; i++) {
        ICUNIT_ASSERT_EQUAL(buf[i + 16], buf[i], 0);
        if (buf[i + 16] != buf[i]) {
            printf("String Copy error \r\n");
            break;
        }
    }
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCPY_OPERATION_002
 * @tc.name      : Memony operation for memcpy test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcpy002, Function | MediumTest | Level1)
{
    void *retValue = NULL;

    char source[] = {"memory refers to the computer hardware devices used to store information for"
        " immediate use in a computer\r\n"};
    char dest[1024] = {0};
    retValue = memcpy(dest, source, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);

    retValue = memcpy(source, dest, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(dest, source, 0);

    char chr = 'A';
    int i, len, failure;
    char src[1024];
    char dst[1024];

    len = rand() % 1024;
    for (i = 0; i < len; i++) {
        src[i] = chr + i % 26;
    }

    memcpy(dst, src, len);
    failure = 0;
    for (i = 0; i < len; i++) {
        if (dst[i] != src[i]) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMSET_OPERATION_001
 * @tc.name      : Memony operation for memset test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemset001, Function | MediumTest | Level1)
{
    void *retValue = NULL;
    char source[1024] = {"memory refers to the computer hardware devices used to store information for"
        " immediate use in a computer\r\n"};
    char ch = rand() % 26 + 'A';
    retValue = memset(source, ch, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);
    ICUNIT_ASSERT_EQUAL(retValue, source, 0);

    for (int i = 0; i < (sizeof(source) / sizeof(source[0])); i++) {
        ICUNIT_ASSERT_EQUAL(source[i], ch, 0);
    }
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCMP_OPERATION_001
 * @tc.name      : Memony operation for memcmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcmp001, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"memory refers to the computer hardware devices used to store information for "
        "immediate use in a computer\r\n"};
    char dest[] = {"memory refers to the computer hardware devices used to store information for "
        "immediate use in a computer\r\n"};

    retValue = memcmp(source, dest, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_EQUAL(retValue, 0, retValue);


    char orign[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    char lt[8]    = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x77};
    char eq[8]    = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    char gt[8]    = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x99};

    int ret;
    int len = sizeof(orign);

    ret = memcmp(orign, lt, len);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 1, ret, 0);

    ret = memcmp(eq, orign, len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = memcmp(orign, gt, len);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, ret, -1, 0);

    ret = memcmp(gt, orign, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCMP_OPERATION_002
 * @tc.name      : Memony operation for memcmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcmp002, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"memory refers to the computer hardware devices used to store information for "
        "immediate use in a computer\r\n"};
    char dest[] = {"Hello, Richard, how are you?\r\n"};
    retValue = memcmp(source, dest, sizeof(dest) / sizeof(dest[0]));
    ICUNIT_ASSERT_WITHIN_EQUAL(retValue, 1, retValue, 0);

    int ret = memcmp(L"CBCDEFG", L"BBCDEFG", 7);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 1, ret, 0);

    ret = memcmp(L"ABCDEFG", L"abcdefg", 2);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, ret, -1, 0);

    ret = memcmp(L"ABCDEFG", L"ABCDEFG", 6);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCMP_OPERATION_003
 * @tc.name      : Memony operation for memcmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcmp003, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"00000\r\n"};
    char dest[] = {"memory refers to the computer hardware devices used to store information for "
        "immediate use in a computer\r\n"};
    retValue = memcmp(source, dest, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_WITHIN_EQUAL(retValue, retValue, -1, 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_REALLOC_OPERATION_001
 * @tc.name      : Memony operation for realloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemRealloc001, Function | MediumTest | Level1)
{
    char *source = (char *)malloc(20);
    source = (char *)realloc(source, 0);
    ICUNIT_ASSERT_EQUAL(source, NULL, 0);

    size_t k, len, mlen, rlen;
    for (int i = 0; i < 5; i++) {
        char *mem = NULL;
        char *data = NULL;
        int failure = 0;
        const char testChar = 0x36;

        mlen = 10;
        mem = malloc(mlen);
        ICUNIT_ASSERT_NOT_EQUAL(mem, NULL, 0);

        (void)memset_s(mem, mlen, testChar, mlen);
        rlen = rand() % (1024) + mlen;
        char *mem1 = realloc(mem, rlen);
        if (mem1 == NULL) {
            free(mem);
        }
        ICUNIT_ASSERT_NOT_EQUAL(mem1, NULL, 0);

        len = mlen <= rlen ? mlen : rlen;

        data = (char *)mem1;
        for (k = 0; k < len; k++) {
            if (data[k] != testChar) {
                failure = 1;
            }
        }

        free(mem1);
        ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    }
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_REALLOC_OPERATION_002
 * @tc.name      : Memony operation for realloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemRealloc002, Function | MediumTest | Level1)
{
    char *source = (char *)malloc(20);

    char *newData = (char *)realloc(source, 40);
    ICUNIT_ASSERT_NOT_EQUAL(newData, NULL, 0);
    if (newData != NULL) {
        source = newData;
    }
    free(source);

    const size_t len = 1024;
    const size_t large = 4096;

    void *mem = malloc(len);
    ICUNIT_ASSERT_NOT_EQUAL(mem, NULL, 0);

    void *reMem = realloc(mem, large);
    ICUNIT_ASSERT_NOT_EQUAL(reMem, NULL, 0);

    if (reMem != NULL) {
        mem = reMem;
    }
    free(mem);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_REALLOC_OPERATION_003
 * @tc.name      : Memony operation for realloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemRealloc003, Function | MediumTest | Level1)
{
    char *retValue = NULL;

    retValue = (char *)realloc(retValue, 20);
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_REALLOC_OPERATION_004
 * @tc.name      : Memony operation for realloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemRealloc004, Function | MediumTest | Level1)
{
    char *source = (char *)malloc(20);

    char *newData = (char *)realloc(source, 10);
    ICUNIT_ASSERT_NOT_EQUAL(newData, NULL, 0);

    if (newData != NULL) {
        source = newData;
    }
    free(source);
    return 0;
};


RUN_TEST_SUITE(PosixMemFuncTestSuite);

void PosixStringMemTest03()
{
    LOG("begin PosixStringMemTest03....");
    RUN_ONE_TESTCASE(testOsMemMemcpy001);
    RUN_ONE_TESTCASE(testOsMemMemcpy002);
    RUN_ONE_TESTCASE(testOsMemMemset001);
    RUN_ONE_TESTCASE(testOsMemMemcmp001);
    RUN_ONE_TESTCASE(testOsMemMemcmp002);
    RUN_ONE_TESTCASE(testOsMemMemcmp003);
    RUN_ONE_TESTCASE(testOsMemRealloc001);
    RUN_ONE_TESTCASE(testOsMemRealloc002);
    RUN_ONE_TESTCASE(testOsMemRealloc003);
    RUN_ONE_TESTCASE(testOsMemRealloc004);

    return;
}
