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
#include <fcntl.h>
#include <dirent.h>
#include "sys/stat.h"

/* *
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is PosixFsFuncTestSuite
 */
LITE_TEST_SUIT(Posix, PosixFs, PosixFsFuncTestSuite);

/* Corresponding to different platforms, only need to modify TEST_ROOT  */
#define TEST_ROOT            "/littlefs"

#define TEST_FILE_PTAH_RIGHT    TEST_ROOT"/FILE0"   /* file path, to open/rd/close */

#define FILE0                   "FILE0"             /* common file name used for testing  */
#define FILE1                   TEST_ROOT"/FILE1"   /* common file under test root path name used for testing */
#define DIR1                    TEST_ROOT"/DIR1/"   /* common path under test root path name used for testing */

#define DIRA                    TEST_ROOT"/a"
#define DIRAB                   TEST_ROOT"/a/b"
#define DIRAC                   TEST_ROOT"/a/c"


#define TEST_BUF_SIZE           40                  /* 40, common data for test, no special meaning */
#define TEST_SEEK_SIZE          10                  /* 10, common data for test, no special meaning */
#define TEST_RW_SIZE            20                  /* 20, common data for test, no special meaning */
#define TEST_LOOPUP_TIME        20                 /* 100, common data for test, no special meaning */
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
    TEST_ASSERT_EQUAL_STRING(TEST_ROOT, workDir);
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
    TEST_ASSERT_EQUAL_STRING(TEST_ROOT, workDir);
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

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_001
 * @tc.name     fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose001, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_002
 * @tc.name     fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose002, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_003
 * @tc.name     fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose003, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    fp = fopen(TEST_FILE_PTAH_RIGHT, "r");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_004
 * @tc.name     fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose004, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    fp = fopen(TEST_FILE_PTAH_RIGHT, "a");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_005
 * @tc.name     fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose005, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    fp = fopen(TEST_FILE_PTAH_RIGHT, "c");
    TEST_ASSERT_NULL(fp);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_006
 * @tc.name     remove the path before fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose006, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;

    remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_007
 * @tc.name     remove the path before fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose007, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;

    remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_008
 * @tc.name     remove the path before fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose008, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;

    remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "a+");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_009
 * @tc.name     remove the path before fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose009, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;

    remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "a");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_010
 * @tc.name     remove the path before fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose010, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;

    remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "wr");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_011
 * @tc.name     wrong input
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose011, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;

    fp = fopen(TEST_FILE_PTAH_RIGHT, NULL);
    TEST_ASSERT_NULL(fp);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FDOPEN_001
 * @tc.name     fdopen
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFdopen001, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    int fd = 0;

    remove(TEST_FILE_PTAH_RIGHT);
    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, 0666);
    TEST_ASSERT_TRUE(fd >= 0);

    fp = fdopen(fd, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FDOPEN_002
 * @tc.name     fdopen
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFdopen002, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    int fd = 0;

    remove(TEST_FILE_PTAH_RIGHT);
    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, 0666);
    TEST_ASSERT_TRUE(fd >= 0);

    fp = fdopen(fd, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FDOPEN_003
 * @tc.name     fdopen
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFdopen003, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    int fd = 0;

    remove(TEST_FILE_PTAH_RIGHT);
    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, 0666);
    TEST_ASSERT_TRUE(fd >= 0);

    fp = fdopen(fd, "a");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FDOPEN_004
 * @tc.name     fdopen
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFdopen004, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    int fd = 0;

    remove(TEST_FILE_PTAH_RIGHT);
    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, 0666);
    TEST_ASSERT_TRUE(fd >= 0);

    fp = fdopen(500, "w"); /* 500 is a wrong fd */
    // in some fs, may return ok, so return null or not is pass.
    if (NULL == fp) {
        close (fd);
        return;
    }

    fp->fd = fd;
    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_001
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek001, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_002
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek002, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, 0L, SEEK_SET);
    TEST_ASSERT_TRUE(ret != -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_003
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek003, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    TEST_ASSERT_TRUE(ret != -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, TEST_SEEK_SIZE);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_004
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek004, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_END);
    TEST_ASSERT_TRUE(ret != -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, TEST_SEEK_SIZE);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_005
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek005, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_CUR);
    TEST_ASSERT_TRUE(ret != -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, TEST_SEEK_SIZE);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_006
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek006, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, -1L, SEEK_SET);
    TEST_ASSERT_EQUAL_INT(ret, -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_007
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek007, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, -1L, SEEK_CUR);
    TEST_ASSERT_EQUAL_INT(ret, -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_008
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek008, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, -1L, SEEK_END);
    TEST_ASSERT_EQUAL_INT(ret, -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_009
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek009, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    /* wrong input, fs may return not error, no need to check return value */
    ret = fseek(fp, TEST_SEEK_SIZE, 5); /* 5, a wrong input */

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT(off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_010
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek010, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;
    int fd;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    fd = fp->fd;

    fp->fd = 500; /* 500 is a wrong fd */

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    TEST_ASSERT_EQUAL_INT(ret, -1);

    fp->fd = fd;

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_011
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek011, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_CUR);
    TEST_ASSERT_TRUE(ret != -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, (TEST_SEEK_SIZE + TEST_SEEK_SIZE));

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}


/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_012
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek012, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, (TEST_SEEK_SIZE + TEST_SEEK_SIZE), SEEK_SET);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    TEST_ASSERT_TRUE(ret != -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, TEST_SEEK_SIZE);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_013
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek013, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_END);
    TEST_ASSERT_TRUE(ret != -1);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, TEST_SEEK_SIZE);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_014
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek014, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    TEST_ASSERT_TRUE(ret != -1);

    rewind(fp);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_015
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek015, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_CUR);
    TEST_ASSERT_TRUE(ret != -1);

    rewind(fp);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_016
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek016, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_END);
    TEST_ASSERT_TRUE(ret != -1);

    rewind(fp);

    off = ftell(fp);
    TEST_ASSERT_EQUAL_INT((int)off, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_001
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs001, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";
    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fputs(chr1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_002
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs002, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";
    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "a");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fputs(chr1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_003
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs003, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";
    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fputs(chr1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_004
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs004, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";
    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fputs(chr1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    (void)fseek (fp, 0L, SEEK_SET);
    ret = fread(str, strlen(chr1) + 1, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = strcmp(str, "hello");
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_005
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs005, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";
    char str[TEST_BUF_SIZE] = {0};
    int i;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = fputs(chr1, fp);
        TEST_ASSERT_TRUE(ret != -1);
    }

    ret = ftell(fp);
    TEST_ASSERT_EQUAL_INT(ret, TEST_LOOPUP_TIME * strlen(chr1));

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_001
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite001, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fwrite(chr1, strlen(chr1) + 1, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_002
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite002, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fwrite(0, 0, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_003
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite003, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "r");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fread(chr1, strlen(chr1) + 1, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_004
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite004, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "r");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fread(0, 0, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_005
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite005, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "a");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fwrite(chr1, strlen(chr1) + 1, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_006
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite006, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fwrite(chr1, strlen(chr1), 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    (void)fseek(fp, 0L, SEEK_SET);

    ret = fread(str, TEST_RW_SIZE, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fwrite(chr2, strlen(chr2), 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    (void)fseek(fp, 0L, SEEK_SET);
    ret = fread(str, TEST_RW_SIZE, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = strcmp(str, "helloworld");
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);

}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_007
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite007, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    long off = 0;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    ret = fwrite(chr1, strlen(chr1), 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = fwrite(chr2, strlen(chr2), 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    (void)fseek(fp, 0L, SEEK_SET);

    ret = fread(str, TEST_RW_SIZE, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = strcmp(str, "helloworld");
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_008
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite008, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    int i;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = fwrite(chr1, strlen(chr1), 1, fp);
        TEST_ASSERT_TRUE(ret != -1);
    }

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = fwrite(chr2, strlen(chr2), 1, fp);
        TEST_ASSERT_TRUE(ret != -1);
    }

    (void)fseek(fp, (TEST_LOOPUP_TIME - 2) * strlen(chr1), SEEK_SET); /* 2, means will read chr1 2times */

    ret = fread(str, TEST_RW_SIZE, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = strncmp(str, "hellohelloworldworld", TEST_RW_SIZE);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_009
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite009, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    int i;

    const char chr1[TEST_BUF_SIZE] = "12345";
    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    TEST_ASSERT_NOT_NULL(fp);

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = fwrite(chr1, strlen(chr1), 1, fp);
        TEST_ASSERT_TRUE(ret != -1);
    }

    (void)fseek(fp, TEST_SEEK_SIZE, SEEK_SET);

    ret = fread(str, TEST_RW_SIZE, 1, fp);
    TEST_ASSERT_TRUE(ret != -1);

    ret = strncmp(str, "1234512345123451234512345", TEST_RW_SIZE);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = fclose(fp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_READDIR_001
 * @tc.name     readdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsReaddir001, Function | MediumTest | Level1)
{
    DIR *dirp;
    struct dirent *dResult;
    int ret = 0;

    ret = mkdir(DIRA, 0777);
    dirp = opendir(DIRA);
    TEST_ASSERT_NOT_NULL(dirp);

    dResult = readdir(dirp);
    TEST_ASSERT_NOT_NULL(dResult);

    ret = closedir(dirp);
    TEST_ASSERT_EQUAL_INT(ret, 0);
    ret = rmdir(DIRA);
}

/* *
 * @tc.number   SUB_KERNEL_FS_READDIR_002
 * @tc.name     readdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsReaddir002, Function | MediumTest | Level1)
{
    DIR *dirp;
    struct dirent *dResult;
    off_t tellDir0;
    off_t tellDir1;
    int ret = 0;

    ret = mkdir(DIRA, 0777);
    dirp = opendir((DIRA));
    TEST_ASSERT_NOT_NULL(dirp);

    dResult = readdir(dirp);
    TEST_ASSERT_NOT_NULL(dirp);
    tellDir0 = dResult->d_off;

    dResult = readdir(dirp);
    TEST_ASSERT_NOT_NULL(dirp);
    tellDir1 = dResult->d_off;

    TEST_ASSERT_TRUE(tellDir0 == tellDir1);
    ret = rmdir(DIRA);
}

/* *
 * @tc.number   SUB_KERNEL_FS_REMOVE_001
 * @tc.name     remove
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRemove001, Function | MediumTest | Level1)
{
    FILE *fp = NULL;
    int ret = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    fclose(fp);
    ret = remove(TEST_FILE_PTAH_RIGHT);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_REMOVE_002
 * @tc.name     remove
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRemove002, Function | MediumTest | Level1)
{
    int ret = 0;

    ret = mkdir(DIRA, 0777);
    TEST_ASSERT_EQUAL_INT(ret, 0);
    ret = remove(DIRA);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_RMDIR_001
 * @tc.name     rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRmdir001, Function | MediumTest | Level1)
{
    int ret = 0;

    ret = mkdir(DIRA, 0777);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = mkdir(DIRAB, 0777);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = rmdir(DIRAB);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = rmdir(DIRA);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_RMDIR_002
 * @tc.name     rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRmdir002, Function | MediumTest | Level1)
{
    int ret = 0;

    ret = mkdir(DIRA, 0777);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = mkdir(DIRAB, 0777);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = mkdir(DIRAC, 0777);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = rmdir(DIRAB);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = rmdir(DIRAC);
    TEST_ASSERT_EQUAL_INT(ret, 0);

    ret = rmdir(DIRA);
    TEST_ASSERT_EQUAL_INT(ret, 0);
}

/* *
 * @tc.number   SUB_KERNEL_FS_UNLINK_001
 * @tc.name     unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsUnlink001, Function | MediumTest | Level1)
{
    int ret = 0;
    int fd = 0;
    char tmpFileName[]= FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR, 0777);
    TEST_ASSERT_TRUE(ret != -1);

    (void)close(fd);

    ret = unlink(tmpFileName);
    TEST_ASSERT_TRUE(ret != -1);
}

/* *
 * @tc.number   SUB_KERNEL_FS_STAT_001
 * @tc.name     unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsStat001, Function | MediumTest | Level1)
{
    struct stat buf;
    int fd = 0;
    char tmpFileName[]= FILE1;
    int ret = 0;

    remove(FILE1);
    fd = open(tmpFileName, O_CREAT | O_RDWR, 0777);
    TEST_ASSERT_TRUE(ret != -1);

    (void)close(fd);

    ret = stat(tmpFileName, &buf);
    TEST_ASSERT_TRUE(ret != -1);
}

/* *
 * @tc.number   SUB_KERNEL_FS_STAT_002
 * @tc.name     unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsStat002, Function | MediumTest | Level1)
{
    struct stat buf;
    int fd = 0;
    char tmpFileName[]= FILE1;
    int ret = 0;
    ssize_t size = 0;
    char writeBuf[TEST_BUF_SIZE] = "write test";

    remove(FILE1);
    fd = open(tmpFileName, O_CREAT | O_RDWR, 0777);
    TEST_ASSERT_TRUE(ret != -1);

    size = write(fd, writeBuf, sizeof(writeBuf));
    TEST_ASSERT_TRUE(ret != -1);

    (void)close(fd);

    ret = stat(tmpFileName, &buf);
    TEST_ASSERT_TRUE(ret != -1);

    TEST_ASSERT_TRUE(buf.st_size == sizeof(writeBuf));
}

/* *
 * @tc.number   SUB_KERNEL_FS_STAT_003
 * @tc.name     unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsStat003, Function | MediumTest | Level1)
{
    struct stat buf;
    int fd;
    char tmpFileName[]= FILE1;
    int ret = 0;
    ssize_t size;
    char writeBuf[TEST_BUF_SIZE] = "write test";

    (void)memset_s(&buf, sizeof(buf), 0, sizeof(buf));
    fd = open(tmpFileName, O_CREAT | O_RDWR, 0777);
    TEST_ASSERT_TRUE(ret != -1);

    size = write(fd, writeBuf, sizeof(writeBuf));
    TEST_ASSERT_TRUE(ret != -1);
    (void)close(fd);

    ret = stat(tmpFileName, &buf);
    TEST_ASSERT_TRUE(ret != -1);

    TEST_ASSERT_EQUAL_INT(buf.st_rdev, 0);
}

/* *
 * extern lseek is necessary, Otherwise it will cause stacking errors
 */
extern off_t lseek(int fd, off_t offset, int whence);

/* *
 * @tc.number   SUB_KERNEL_FS_WRITE_001
 * @tc.name     unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsWrite001, Function | MediumTest | Level1)
{
    off_t reLseek;
    int fd = 0;
    char writeBuf[TEST_BUF_SIZE];
    int ret = 0;
    char tmpFileName[]= FILE1;

    for (int i = 0; i < TEST_BUF_SIZE; i++) {
        writeBuf[i] = '1';
    }

    fd = open(tmpFileName, O_CREAT | O_RDWR, 0777);
    TEST_ASSERT_TRUE(ret != -1);

    ret = write(fd, writeBuf, TEST_RW_SIZE);
    TEST_ASSERT_TRUE(ret != -1);

    reLseek = lseek(fd, 0, SEEK_CUR);

    ret = write(fd, writeBuf, TEST_RW_SIZE);
    TEST_ASSERT_TRUE(ret != -1);

    reLseek = lseek(fd, 0, SEEK_CUR);

    TEST_ASSERT_TRUE((TEST_RW_SIZE + TEST_RW_SIZE) == (unsigned int)reLseek);

    (void)close(fd);
}

/* *
 * @tc.number   SUB_KERNEL_FS_WRITE_002
 * @tc.name     unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsWrite002, Function | MediumTest | Level1)
{
    int fd = 0;
    char writeBuf[TEST_BUF_SIZE] = "0123456789";
    int ret = 0;
    struct stat statbuf;
    char tmpFileName[]= FILE1;
    int i;

    remove(FILE1);
    fd = open(tmpFileName, O_CREAT | O_RDWR, 0777);
    TEST_ASSERT_TRUE(ret != -1);

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = write(fd, writeBuf, sizeof(writeBuf));
    }
    TEST_ASSERT_TRUE(ret != -1);

    (void)close(fd);

    ret = stat(tmpFileName, &statbuf);
    TEST_ASSERT_TRUE(ret != -1);

    TEST_ASSERT_TRUE(statbuf.st_size == sizeof(writeBuf) * TEST_LOOPUP_TIME);
}

RUN_TEST_SUITE(PosixFsFuncTestSuite);

void PosixFsFuncTest()
{
    LOG("begin PosixFsFuncTest....\r\n");

    RUN_ONE_TESTCASE(testFsUnlink001);

    RUN_ONE_TESTCASE(testFsStat001);
    RUN_ONE_TESTCASE(testFsStat002);
    RUN_ONE_TESTCASE(testFsStat003);

    RUN_ONE_TESTCASE(testFsWrite001);
    RUN_ONE_TESTCASE(testFsWrite002);

    RUN_ONE_TESTCASE(testFsDirname001);
    RUN_ONE_TESTCASE(testFsDirname002);
    RUN_ONE_TESTCASE(testFsDirname003);
    RUN_ONE_TESTCASE(testFsDirname004);

    RUN_ONE_TESTCASE(testFsReaddir001);
    RUN_ONE_TESTCASE(testFsReaddir002);

    RUN_ONE_TESTCASE(testFsRemove001);
    RUN_ONE_TESTCASE(testFsRemove002);

    RUN_ONE_TESTCASE(testFsRmdir001);
    RUN_ONE_TESTCASE(testFsRmdir002);

    RUN_ONE_TESTCASE(testFsFopenFclose001);
    RUN_ONE_TESTCASE(testFsFopenFclose002);
    RUN_ONE_TESTCASE(testFsFopenFclose003);
    RUN_ONE_TESTCASE(testFsFopenFclose004);
    RUN_ONE_TESTCASE(testFsFopenFclose005);
    RUN_ONE_TESTCASE(testFsFopenFclose006);
    RUN_ONE_TESTCASE(testFsFopenFclose007);
    RUN_ONE_TESTCASE(testFsFopenFclose008);
    RUN_ONE_TESTCASE(testFsFopenFclose009);
    RUN_ONE_TESTCASE(testFsFopenFclose010);
    RUN_ONE_TESTCASE(testFsFopenFclose011);

    RUN_ONE_TESTCASE(testFsFdopen001);
    RUN_ONE_TESTCASE(testFsFdopen002);
    RUN_ONE_TESTCASE(testFsFdopen003);
    RUN_ONE_TESTCASE(testFsFdopen004);

    RUN_ONE_TESTCASE(testFsFtellFseek001);
    RUN_ONE_TESTCASE(testFsFtellFseek002);
    RUN_ONE_TESTCASE(testFsFtellFseek003);
    RUN_ONE_TESTCASE(testFsFtellFseek004);
    RUN_ONE_TESTCASE(testFsFtellFseek005);
    RUN_ONE_TESTCASE(testFsFtellFseek006);
    RUN_ONE_TESTCASE(testFsFtellFseek007);
    RUN_ONE_TESTCASE(testFsFtellFseek008);
    RUN_ONE_TESTCASE(testFsFtellFseek009);
    RUN_ONE_TESTCASE(testFsFtellFseek010);
    RUN_ONE_TESTCASE(testFsFtellFseek011);
    RUN_ONE_TESTCASE(testFsFtellFseek012);
    RUN_ONE_TESTCASE(testFsFtellFseek013);
    RUN_ONE_TESTCASE(testFsFtellFseek014);
    RUN_ONE_TESTCASE(testFsFtellFseek015);
    RUN_ONE_TESTCASE(testFsFtellFseek016);

    RUN_ONE_TESTCASE(testFsFputs001);
    RUN_ONE_TESTCASE(testFsFputs002);
    RUN_ONE_TESTCASE(testFsFputs003);
    RUN_ONE_TESTCASE(testFsFputs004);
    RUN_ONE_TESTCASE(testFsFputs005);

    RUN_ONE_TESTCASE(testFsFreadFwrite001);
    RUN_ONE_TESTCASE(testFsFreadFwrite002);
    RUN_ONE_TESTCASE(testFsFreadFwrite003);
    RUN_ONE_TESTCASE(testFsFreadFwrite004);
    RUN_ONE_TESTCASE(testFsFreadFwrite005);
    RUN_ONE_TESTCASE(testFsFreadFwrite006);
    RUN_ONE_TESTCASE(testFsFreadFwrite007);
    RUN_ONE_TESTCASE(testFsFreadFwrite008);
    RUN_ONE_TESTCASE(testFsFreadFwrite009);

    return;
}
