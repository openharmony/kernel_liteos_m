/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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
#include <time.h>

/* *
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is PosixStringFuncTestSuite
 */
LITE_TEST_SUIT(Posix, Posixstring, PosixStringFuncTestSuite);


/* *
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL PosixStringFuncTestSuiteSetUp(void)
{
    return TRUE;
}

/* *
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL PosixStringFuncTestSuiteTearDown(void)
{
    printf("+Hello this is a String  function test+\n");
    return TRUE;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_STRING_OPERATION_001
 * @tc.name      : Memony operation for strcmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrstrcmp001, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"Compiler exited with error"};
    char dest[] = {"Compiler exited with error"};

    retValue = strcmp(source, dest);
    ICUNIT_ASSERT_EQUAL(retValue, 0, retValue);

    int ret = strcmp("abcdef", "ABCDEF");
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 1, ret, 0);

    ret = strcmp("123456", "654321");
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, ret, -1, 0);
    ICUNIT_ASSERT_EQUAL(strcmp("~!@#$%^&*()_+", "~!@#$%^&*()_+"), 0, 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strcmp_OPERATION_002
 * @tc.name      : Memony operation for strncmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrstrcmp002, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"Compiler exited with error"};
    char dest[] = {"00000000000"};

    retValue = strcmp(source, dest);
    ICUNIT_ASSERT_WITHIN_EQUAL(retValue, 1, retValue, 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strcmp_OPERATION_003
 * @tc.name      : Memony operation for strncmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrstrcmp003, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"0000000"};
    char dest[] = {"Compiler exited with error"};

    retValue = strcmp(source, dest);
    ICUNIT_ASSERT_WITHIN_EQUAL(retValue, retValue, -1, 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strdup_OPERATION_001
 * @tc.name      : Memony operation for strdup test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrdup001, Function | MediumTest | Level1)
{
    char source[] = {"Compiler exited with error"};
    char *dest;

    dest = strdup(source);
    ICUNIT_ASSERT_NOT_EQUAL(dest, NULL, 0);
    printf("The Result Display :%s\r\n", dest);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(dest, source, sizeof(source) / sizeof(source[0]), 0);

    char src[] = "hello world !";
    char *ret = strdup(src);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(ret, src, sizeof(src) / sizeof(src[0]), 0);
    free(ret);

    char srcS[] = "This is String1";
    ret = strdup(srcS);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(ret, "This is String1", sizeof(srcS) / sizeof(srcS[0]), 0);
    free(ret);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strdup_OPERATION_002
 * @tc.name      : Memony operation for strdup test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrdup002, Function | MediumTest | Level1)
{
    char source[] = {"export MY_TEST_PATH=/opt/hadoop-2.6.5"};
    char *dest;

    dest = strdup(source);
    ICUNIT_ASSERT_NOT_EQUAL(dest, NULL, 0);
    printf("The Result Display :%s\r\n", dest);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(dest, source, sizeof(source) / sizeof(source[0]), 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strcspn_OPERATION_001
 * @tc.name      : Memony operation for strcspn test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrcspn001, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"export MY_TEST_PATH=/opt/hadoop-2.6.5"};
    char dest1[] = {"H"};

    retValue = strcspn(source, dest1);
    ICUNIT_ASSERT_EQUAL(retValue, 18U, retValue);

    const char dest[] = "hello world !";
    const char src[] = "!";
    size_t ret = strcspn(dest, src);
    ICUNIT_ASSERT_EQUAL(ret, 12U, ret);

    const char srcS[] = "a";
    ret = strcspn(dest, srcS);
    ICUNIT_ASSERT_EQUAL(ret, 13U, ret);
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_strcspn_OPERATION_002
 * @tc.name      : Memony operation for strcspn test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrcspn002, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"Compiler exited with error"};
    char dest[] = {"or"};

    retValue = strcspn(source, dest);
    ICUNIT_ASSERT_EQUAL(retValue, 1, retValue);
    return 0;
};

#ifndef LOSCFG_COMPILER_ICCARM
/* *
 * @tc.number    : SUB_KERNEL_POSIX_strptime_OPERATION_001
 * @tc.name      : Memony operation for strptime test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrptime001, Function | MediumTest | Level1)
{
    struct tm tmData;
    (void)memset_s(&tmData, sizeof(struct tm), 0, sizeof(struct tm));
    char *ret = strptime("2020-10-29 21:24:00abc", "%Y-%m-%d %H:%M:%S", &tmData);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(ret, "abc", 3, 0);
    ICUNIT_ASSERT_EQUAL(tmData.tm_year, 120, tmData.tm_year);
    ICUNIT_ASSERT_EQUAL(tmData.tm_mon, 9, tmData.tm_mon);
    ICUNIT_ASSERT_EQUAL(tmData.tm_mday, 29, tmData.tm_mday);
    ICUNIT_ASSERT_EQUAL(tmData.tm_hour, 21, tmData.tm_hour);
    ICUNIT_ASSERT_EQUAL(tmData.tm_min, 24, tmData.tm_min);
    return 0;
}
#endif


RUN_TEST_SUITE(PosixStringFuncTestSuite);

void PosixStringFuncTest03()
{
    LOG("begin PosixStringFuncTest03....");
    RUN_ONE_TESTCASE(testStrstrcmp001);
    RUN_ONE_TESTCASE(testStrstrcmp002);
    RUN_ONE_TESTCASE(testStrstrcmp003);
    RUN_ONE_TESTCASE(testStrStrdup001);
    RUN_ONE_TESTCASE(testStrStrdup002);
    RUN_ONE_TESTCASE(testStrStrcspn001);
    RUN_ONE_TESTCASE(testStrStrcspn002);
#ifndef LOSCFG_COMPILER_ICCARM
    RUN_ONE_TESTCASE(testStrStrptime001);
#endif

    return;
}