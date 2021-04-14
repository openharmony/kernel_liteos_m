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

#ifndef __NEED_mode_t
#define __NEED_mode_t
#endif

#include <securec.h>
#include <stdio.h>
#include <libgen.h>
#include "ohos_types.h"
#include "hctest.h"
#include "los_config.h"
#include "kernel_test.h"
#include "log.h"
#include <bits/alltypes.h>

/* *
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is PosixFsFuncTestSuite
 */
LITE_TEST_SUIT(Posix, PosixFs, PosixFsFuncTestSuite);

/* *
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL PosixFsFuncTestSuiteSetUp(void)
{
    return TRUE;
}

/* *
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL PosixFsFuncTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

#define FILE0 "FILE0"
#define FILE1 "/opt/test/FILE1"
#define DIR1 "/opt/test/DIR1/"

/* *
 * @tc.number   SUB_KERNEL_FS_DIRNAME_001
 * @tc.name     dirname basic function test
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsDirname001, Function | MediumTest | Level1)
{
    char path[] = FILE0;
    char *workDir = dirname((char *)path);
    TEST_ASSERT_NOT_NULL(workDir);
    TEST_ASSERT_EQUAL_STRING(".", workDir);
}

/* *
 * @tc.number   SUB_KERNEL_FS_DIRNAME_002
 * @tc.name     dirname basic function test
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsDirname002, Function | MediumTest | Level1)
{
    char path[] = FILE1;
    char *workDir = dirname((char *)path);
    TEST_ASSERT_NOT_NULL(workDir);
    TEST_ASSERT_EQUAL_STRING("/opt/test", workDir);
}

/* *
 * @tc.number   SUB_KERNEL_FS_DIRNAME_003
 * @tc.name     dirname basic function test
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsDirname003, Function | MediumTest | Level1)
{
    // get dir
    char path[] = DIR1;
    char *workDir = dirname((char *)path);
    TEST_ASSERT_NOT_NULL(workDir);
    TEST_ASSERT_EQUAL_STRING("/opt/test", workDir);
}

/* *
 * @tc.number   SUB_KERNEL_FS_DIRNAME_004
 * @tc.name     dirname basic function test for special input
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsDirname004, Function | MediumTest | Level1)
{
    // get dir
    char *workDir = dirname("");
    TEST_ASSERT_NOT_NULL(workDir);
    TEST_ASSERT_EQUAL_STRING(".", workDir);

    workDir = dirname(NULL);
    TEST_ASSERT_NOT_NULL(workDir);
    TEST_ASSERT_EQUAL_STRING(".", workDir);

    workDir = dirname("/");
    TEST_ASSERT_NOT_NULL(workDir);
    TEST_ASSERT_EQUAL_STRING("/", workDir);

    workDir = dirname("..");
    TEST_ASSERT_NOT_NULL(workDir);
    TEST_ASSERT_EQUAL_STRING(".", workDir);

    workDir = dirname(".");
    TEST_ASSERT_NOT_NULL(workDir);
    TEST_ASSERT_EQUAL_STRING(".", workDir);
}

RUN_TEST_SUITE(PosixFsFuncTestSuite);


void PosixFsFuncTest()
{
    LOG("begin PosixFsFuncTest....");
    RUN_ONE_TESTCASE(testFsDirname001);
    RUN_ONE_TESTCASE(testFsDirname002);
    RUN_ONE_TESTCASE(testFsDirname003);
    RUN_ONE_TESTCASE(testFsDirname004);

    return;
}