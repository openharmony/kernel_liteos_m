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

#ifndef __NEED_mode_t
#define __NEED_mode_t
#endif

#include "posix_fs_test.h"

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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_005
 * @tc.name     fopen and fclose
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose005, Function | MediumTest | Level1)
{
    FILE *fp = NULL;
    fp = fopen(TEST_FILE_PTAH_RIGHT, "c");
    ICUNIT_ASSERT_EQUAL(fp, NULL, 0);
    return 0;
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "a+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "a");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "wr");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FOPEN_FCLOSE_011
 * @tc.name     wrong input
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFopenFclose011, Function | MediumTest | Level1)
{
    FILE *fp = NULL;

    fp = fopen(TEST_FILE_PTAH_RIGHT, NULL);
    ICUNIT_ASSERT_EQUAL(fp, NULL, 0);                                  
    return 0;
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_WITHIN_EQUAL(fd, 0, fd, 0);

    fp = fdopen(fd, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_WITHIN_EQUAL(fd, 0, fd, -1);

    fp = fdopen(fd, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_WITHIN_EQUAL(fd, 0, fd, -1);

    fp = fdopen(fd, "a");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

#if (LOSCFG_LIBC_MUSL == 1)
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

    (void)remove(TEST_FILE_PTAH_RIGHT);
    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_WITHIN_EQUAL(fd, 0, fd, -1);

    fp = fdopen(500, "w"); /* 500 is a wrong fd */
    // in some fs, may return ok, so return null or not is pass.
    if (NULL == fp) {
        close (fd);
        return 0;
    }

    fp->fd = fd;
    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}
#endif
/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_001
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek001, Function | MediumTest | Level1)
{
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, 0L, SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL,   0);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, TEST_SEEK_SIZE, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_END);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, TEST_SEEK_SIZE, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_CUR);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, TEST_SEEK_SIZE, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
}

#if (LOSCFG_SUPPORT_LITTLEFS == 1)
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, -1L, SEEK_SET);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
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

    remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, -1L, SEEK_CUR);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_008
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek008, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    long off = 0;

    remove(TEST_FILE_PTAH_RIGHT);
    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, -1L, SEEK_END);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
}
#endif

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_009
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek009, Function | MediumTest | Level1)
{
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    /* wrong input, fs may return not error, no need to check return value */
    (void)fseek(fp, TEST_SEEK_SIZE, 5); /* 5, a wrong input */

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
}

#if (LOSCFG_LIBC_MUSL == 1)
/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_010
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek010, Function | MediumTest | Level1)
{
    int ret = 0;
    FILE *fp = NULL;
    int fd;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    fd = fp->fd;

    fp->fd = 500; /* 500 is a wrong fd */

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    fp->fd = fd;

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}
#endif
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_CUR);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, (TEST_SEEK_SIZE + TEST_SEEK_SIZE), (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, (TEST_SEEK_SIZE + TEST_SEEK_SIZE), SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, TEST_SEEK_SIZE, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
}

#if (LOSCFG_SUPPORT_LITTLEFS == 1)
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_END);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, TEST_SEEK_SIZE, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
}
#endif

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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_SET);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    rewind(fp);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);
  
    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_CUR);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    rewind(fp);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;

}

/* *
 * @tc.number   SUB_KERNEL_FS_FTELL_FSEEK_016
 * @tc.name     ftell and fseek
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFtellFseek016, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    long off = 0;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fseek(fp, TEST_SEEK_SIZE, SEEK_END);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    rewind(fp);

    off = ftell(fp);
    ICUNIT_GOTO_EQUAL((int)off, 0, (int)off, EXIT);

EXIT:
    (void)fclose(fp);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_001
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs001, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fputs(chr1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_002
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs002, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";

    fp = fopen(TEST_FILE_PTAH_RIGHT, "a");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fputs(chr1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_003
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs003, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fputs(chr1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_004
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs004, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";
    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fputs(chr1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    (void)fseek (fp, 0L, SEEK_SET);
    ret = fread(str, strlen(chr1) + 1, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = strcmp(str, "hello");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FPUTS_005
 * @tc.name     fputs
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFputs005, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";
    int i;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = fputs(chr1, fp);
        ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    }

    ret = ftell(fp);
    ICUNIT_ASSERT_EQUAL(ret, TEST_LOOPUP_TIME * strlen(chr1), ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_001
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite001, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;

    const char chr1[TEST_BUF_SIZE] = "hello";

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fwrite(chr1, strlen(chr1) + 1, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_002
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite002, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fwrite(0, 0, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_003
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite003, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";

    fp = fopen(TEST_FILE_PTAH_RIGHT, "r");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fread((void *)chr1, strlen(chr1) + 1, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_004
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite004, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;

    fp = fopen(TEST_FILE_PTAH_RIGHT, "r");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fread(0, 0, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_005
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite005, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;
    const char chr1[TEST_BUF_SIZE] = "hello";

    fp = fopen(TEST_FILE_PTAH_RIGHT, "a");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fwrite(chr1, strlen(chr1) + 1, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_006
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite006, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fwrite(chr1, strlen(chr1), 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    (void)fseek(fp, 0L, SEEK_SET);

    ret = fread(str, TEST_RW_SIZE, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fwrite(chr2, strlen(chr2), 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    (void)fseek(fp, 0L, SEEK_SET);
    ret = fread(str, TEST_RW_SIZE, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = strcmp(str, "helloworld");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;

}

/* *
 * @tc.number   SUB_KERNEL_FS_FWRITE_007
 * @tc.name     fread and fwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFreadFwrite007, Function | MediumTest | Level1)
{
    int ret;
    FILE *fp = NULL;

    const char chr1[TEST_BUF_SIZE] = "hello";
    const char chr2[TEST_BUF_SIZE] = "world";

    char str[TEST_BUF_SIZE] = {0};

    fp = fopen(TEST_FILE_PTAH_RIGHT, "w+");
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    ret = fwrite(chr1, strlen(chr1), 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = fwrite(chr2, strlen(chr2), 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    (void)fseek(fp, 0L, SEEK_SET);

    ret = fread(str, TEST_RW_SIZE, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = strcmp(str, "helloworld");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = fwrite(chr1, strlen(chr1), 1, fp);
        ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    }

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = fwrite(chr2, strlen(chr2), 1, fp);
        ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    }

    (void)fseek(fp, (TEST_LOOPUP_TIME - 2) * strlen(chr1), SEEK_SET); /* 2, means will read chr1 2times */

    ret = fread(str, TEST_RW_SIZE, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = strncmp(str, "hellohelloworldworld", TEST_RW_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(fp, NULL, 0);

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = fwrite(chr1, strlen(chr1), 1, fp);
        ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    }

    (void)fseek(fp, TEST_SEEK_SIZE, SEEK_SET);

    ret = fread(str, TEST_RW_SIZE, 1, fp);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ret = strncmp(str, "1234512345123451234512345", TEST_RW_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = fclose(fp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPENDIR_001
 * @tc.name     opendir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpendir001, Function | MediumTest | Level1)
{
    DIR *dir = NULL;

    dir = opendir(TEST_ROOT);
    ICUNIT_ASSERT_NOT_EQUAL(dir, NULL, 0);

    (void)closedir(dir);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPENDIR_002
 * @tc.name     opendir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpendir002, Function | MediumTest | Level1)
{
    DIR *dir = NULL;

    remove(DIRA);
    dir = opendir(DIRA);
    ICUNIT_ASSERT_EQUAL(dir, NULL, 0);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPENDIR_003
 * @tc.name     opendir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpendir003, Function | MediumTest | Level1)
{
    DIR *dir = NULL;

    dir = opendir(NULL);
    ICUNIT_ASSERT_EQUAL(dir, NULL, 0);

    return 0;
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
    int ret;

    (void)mkdir(DIRA, TEST_MODE_NORMAL);
    (void)mkdir(DIRAB, TEST_MODE_NORMAL);

    dirp = opendir(DIRA);
    ICUNIT_ASSERT_NOT_EQUAL(dirp, NULL, 0);

    dResult = readdir(dirp);
    ICUNIT_ASSERT_NOT_EQUAL(dResult, NULL, 0);

    ret = closedir(dirp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    (void)rmdir(DIRAB);
    (void)rmdir(DIRA);
    return 0;
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

    (void)mkdir(DIRA, TEST_MODE_NORMAL);
    (void)mkdir(DIRAB, TEST_MODE_NORMAL);
    (void)mkdir(DIRAC, TEST_MODE_NORMAL);

    dirp = opendir(DIRA);
    ICUNIT_ASSERT_NOT_EQUAL(dirp, NULL, 0);

    dResult = readdir(dirp);
    ICUNIT_ASSERT_NOT_EQUAL(dResult, NULL, 0);
    tellDir0 = dResult->d_off;

    dResult = readdir(dirp);
    ICUNIT_ASSERT_NOT_EQUAL(dResult, NULL, 0);
    tellDir1 = dResult->d_off;
    ICUNIT_ASSERT_EQUAL(tellDir0, tellDir1, -1);

    (void)rmdir(DIRAC);
    (void)rmdir(DIRAB);
    (void)rmdir(DIRA);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_READDIR_003
 * @tc.name     readdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsReaddir003, Function | MediumTest | Level1)
{
    DIR *dirp;
    struct dirent *dResult;

    dirp = opendir(NULL);
    ICUNIT_ASSERT_EQUAL(dirp, NULL, 0);

    dResult = readdir(dirp);
    ICUNIT_ASSERT_EQUAL(dResult, NULL, 0);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_REMOVE_001
 * @tc.name     remove
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRemove001, Function | MediumTest | Level1)
{
    int fd;
    int ret = 0;

    fd = open(TEST_FILE_PTAH_RIGHT, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    close(fd);
    ret = remove(TEST_FILE_PTAH_RIGHT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_REMOVE_002
 * @tc.name     remove
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRemove002, Function | MediumTest | Level1)
{
    int ret = 0;

    ret = mkdir(DIRA, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = remove(DIRA);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_REMOVE_003
 * @tc.name     remove
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRemove003, Function | MediumTest | Level1)
{
    int ret = 0;
    ret = remove(DIRA);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_REMOVE_004
 * @tc.name     remove
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRemove004, Function | MediumTest | Level1)
{
    int ret = 0;

    (void)mkdir(DIRA, TEST_MODE_NORMAL);

    ret = mkdir(DIRAC, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = remove(DIRA);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    ret = remove(DIRAC);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = remove(DIRA);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RMDIR_001
 * @tc.name     rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRmdir001, Function | MediumTest | Level1)
{
    int ret = 0;

    (void)mkdir(DIRA, TEST_MODE_NORMAL);

    ret = mkdir(DIRAB, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = rmdir(DIRAB);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = rmdir(DIRA);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RMDIR_002
 * @tc.name     rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRmdir002, Function | MediumTest | Level1)
{
    int ret = 0;

    (void)mkdir(DIRA, TEST_MODE_NORMAL);

    ret = mkdir(DIRAB, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mkdir(DIRAC, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = rmdir(DIRAB);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = rmdir(DIRAC);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = rmdir(DIRA);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RMDIR_003
 * @tc.name     rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRmdir003, Function | MediumTest | Level1)
{
    int ret;

    (void)mkdir(DIRA, TEST_MODE_NORMAL);

    ret = mkdir(DIRAB, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = rmdir(DIRA);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    ret = rmdir(DIRAB);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = rmdir(DIRA);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RMDIR_004
 * @tc.name     rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRmdir004, Function | MediumTest | Level1)
{
    int ret;

    ret = rmdir(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_UNLINK_001
 * @tc.name     unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsUnlink001, Function | MediumTest | Level1)
{
    int ret;
    int fd;
    char tmpFileName[]= FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    (void)close(fd);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_UNLINK_002
 * @tc.name     unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsUnlink002, Function | MediumTest | Level1)
{
    int ret;

    ret = unlink(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_STAT_001
 * @tc.name     stat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsStat001, Function | MediumTest | Level1)
{
    struct stat buf;
    int fd;
    char tmpFileName[]= FILE1;
    int ret;

    remove(FILE1);
    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    (void)close(fd);

    ret = stat(tmpFileName, &buf);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_STAT_002
 * @tc.name     stat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsStat002, Function | MediumTest | Level1)
{
    struct stat buf;
    int fd = 0;
    char tmpFileName[]= FILE1;
    int ret;
    ssize_t size;
    char writeBuf[TEST_BUF_SIZE] = "write test";

    remove(FILE1);
    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    size = write(fd, writeBuf, sizeof(writeBuf));
    (void)close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(size, -1, size);

    ret = stat(tmpFileName, &buf);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(buf.st_size, sizeof(writeBuf), -1);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_STAT_003
 * @tc.name     stat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsStat003, Function | MediumTest | Level1)
{
    struct stat buf;
    int fd;
    char tmpFileName[]= FILE1;
    int ret;
    ssize_t size;
    char writeBuf[TEST_BUF_SIZE] = "write test";

    (void)memset_s(&buf, sizeof(buf), 0, sizeof(buf));
    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    size = write(fd, writeBuf, sizeof(writeBuf));
    (void)close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(size, -1, size);

    ret = stat(tmpFileName, &buf);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);

    ICUNIT_ASSERT_EQUAL(buf.st_rdev, 0, buf.st_rdev);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RENAME_001
 * @tc.name     rename
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRename001, Function | MediumTest | Level1)
{
    int ret;
    int fd;

    (void)remove(FILE1);
    (void)remove(FILE2);

    fd = open(FILE1, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);
    (void)close(fd);

    ret = rename(FILE1, FILE2);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = remove(FILE1);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    ret = remove(FILE2);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RENAME_002
 * @tc.name     rename
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRename002, Function | MediumTest | Level1)
{
    int ret;
    int fd;

    (void)remove(FILE1);

    fd = open(FILE1, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);
    (void)close(fd);

    ret = rename(FILE1, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RENAME_003
 * @tc.name     rename
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsRename003, Function | MediumTest | Level1)
{
    int ret;

    (void)remove(FILE1);

    ret = rename(NULL, FILE1);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPEN_001
 * @tc.name     open
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpen001, Function | MediumTest | Level1)
{
    int fd;

    (void)remove(FILE1);

    fd = open(FILE1, O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(fd, -1, fd);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPEN_002
 * @tc.name     open
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpen002, Function | MediumTest | Level1)
{
    int fd;

    (void)remove(FILE_IN_DIRA);
    (void)mkdir(DIRA, TEST_MODE_NORMAL);

    fd = open(FILE_IN_DIRA, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);
    (void)close(fd);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPEN_004
 * @tc.name     open
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpen003, Function | MediumTest | Level1)
{
    int fd;
    ssize_t size;
    char writeBuf[TEST_BUF_SIZE] = "write test";

    (void)remove(FILE1);

    fd = open(FILE1, O_CREAT | O_RDONLY, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);
    size = write(fd, writeBuf, sizeof(writeBuf));
    ICUNIT_ASSERT_WITHIN_EQUAL(size, size, 0, -1);
    (void)close(fd);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPEN_004
 * @tc.name     open
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpen004, Function | MediumTest | Level1)
{
    int fd;

    (void)rmdir(DIRA);
    fd = open(FILE_IN_DIRA, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPEN_005
 * @tc.name     open
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpen005, Function | MediumTest | Level1)
{
    int fd;

    (void)rmdir(DIRA);
    fd = open(NULL, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(fd, -1, fd);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPEN_006
 * @tc.name     open ro + ro
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpen006, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    int32_t fd1 = open(tmpFileName, O_RDONLY, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd1);

    int32_t fd2 = open(tmpFileName, O_RDONLY, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd2, POSIX_FS_IS_ERROR, fd2);

    int32_t fd3 = open(tmpFileName, O_RDONLY, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd3, POSIX_FS_IS_ERROR, fd3);

    ret = close(fd1);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    ret = close(fd2);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    ret = close(fd3);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPEN_007
 * @tc.name     open rw + rw
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpen007, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    int32_t fd1 = open(tmpFileName, O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd1);

    int32_t fd2 = open(tmpFileName, O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(fd2, POSIX_FS_IS_ERROR, fd2);
    ICUNIT_ASSERT_EQUAL(errno, EBUSY, POSIX_FS_IS_ERROR);

    ret = close(fd1);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_OPEN_008
 * @tc.name     open
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsOpen008, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    int32_t fd1 = open(tmpFileName, O_RDONLY, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd1, POSIX_FS_IS_ERROR, fd1);

    int32_t fd2 = open(tmpFileName, O_RDONLY, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd2, POSIX_FS_IS_ERROR, fd2);

    ret = close(fd1);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = close(fd2);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    fd1 = open(tmpFileName, O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd1, POSIX_FS_IS_ERROR, fd1);

    fd2 = open(tmpFileName, O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(fd2, POSIX_FS_IS_ERROR, fd2);

    int32_t fd3 = open(tmpFileName, O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_EQUAL(fd3, POSIX_FS_IS_ERROR, fd3);

    ret = close(fd1);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}



/* *
 * @tc.number   SUB_KERNEL_FS_CLOSE_001
 * @tc.name     close
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsClose001, Function | MediumTest | Level1)
{
    int ret;
    int fd;

    (void)remove(FILE1);

    fd = open(FILE1, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    (void)close(fd);
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);
    return 0;

}

/* *
 * extern lseek is necessary, Otherwise it will cause stacking errors
 */
extern off_t lseek(int fd, off_t offset, int whence);

/* *
 * @tc.number   SUB_KERNEL_FS_WRITE_001
 * @tc.name     write
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsWrite001, Function | MediumTest | Level1)
{
    off_t reLseek;
    int fd = 0;
    char writeBuf[TEST_BUF_SIZE];
    int ret = 0;

    for (int i = 0; i < TEST_BUF_SIZE; i++) {
        writeBuf[i] = '1';
    }

    fd = open(FILE1, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    ret = write(fd, writeBuf, TEST_RW_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT);

    (void)lseek(fd, 0, SEEK_CUR);

    ret = write(fd, writeBuf, TEST_RW_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT);

    reLseek = lseek(fd, 0, SEEK_CUR);
    ICUNIT_GOTO_EQUAL((unsigned int)reLseek, (TEST_RW_SIZE + TEST_RW_SIZE), (unsigned int)reLseek, EXIT);

EXIT:
    (void)close(fd);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_WRITE_002
 * @tc.name     write
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsWrite002, Function | MediumTest | Level1)
{
    int fd = 0;
    char writeBuf[TEST_BUF_SIZE] = "0123456789";
    int ret = 0;
    struct stat statbuf;
    int i;

    remove(FILE1);
    fd = open(FILE1, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    for (i = 0; i < TEST_LOOPUP_TIME; i++) {
        ret = write(fd, writeBuf, sizeof(writeBuf));
        ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT);
    }
    (void)close(fd);

    ret = stat(FILE1, &statbuf);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(statbuf.st_size, sizeof(writeBuf) * TEST_LOOPUP_TIME, -1);
    return 0;
EXIT:
    (void)close(fd);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_WRITE_003
 * @tc.name     write
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsWrite003, Function | MediumTest | Level1)
{
    int fd = 0;
    char writeBuf[TEST_BUF_SIZE];
    char readBuf[TEST_BUF_SIZE];
    int ret = 0;

    for (int i = 0; i < TEST_BUF_SIZE; i++) {
        writeBuf[i] = '1';
    }

    fd = open(FILE1, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    ret = write(fd, writeBuf, TEST_RW_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT);

    (void)lseek(fd, 0, SEEK_CUR);

    ret = read(fd, readBuf, TEST_RW_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT);

    (void)close(fd);

    ret = strncmp(writeBuf, readBuf, TEST_RW_SIZE);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);
    return 0;

EXIT:
    (void)close(fd);
    return 0;
}


#if (LOSCFG_LIBC_MUSL == 1)
/* *
 * @tc.number   SUB_KERNEL_FS_DIRNAME_001
 * @tc.name     dirname basic function test
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsDirname001, Function | MediumTest | Level1)
{
    char path[] = FILE0;
    char *workDir = dirname((char *)path);
    ICUNIT_ASSERT_NOT_EQUAL(workDir, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(workDir, ".", 0);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(workDir, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(workDir, TEST_ROOT, -1);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_DIRNAME_003
 * @tc.name     dirname basic function test
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsDirname003, Function | MediumTest | Level1)
{
    // get dir
    char path[] = DIRA;
    char *workDir = dirname((char *)path);
    ICUNIT_ASSERT_NOT_EQUAL(workDir, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(workDir, TEST_ROOT, -1);
    return 0;
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
    ICUNIT_ASSERT_NOT_EQUAL(workDir, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(workDir ,".", 0);

    workDir = dirname(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(workDir, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(workDir, ".", 0);

    workDir = dirname("/");
    ICUNIT_ASSERT_NOT_EQUAL(workDir, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(workDir, "/", 0);

    workDir = dirname("..");
    ICUNIT_ASSERT_NOT_EQUAL(workDir, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(workDir, ".", 0);

    workDir = dirname(".");
    ICUNIT_ASSERT_NOT_EQUAL(workDir, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(workDir, ".", 0);
    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FCNTL_001
 * @tc.name     fcntl
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFcntl001, Function | MediumTest | Level1)
{
    int fd = 0;
    int flags = 0;
    char tmpFileName[]= FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    flags = fcntl(fd, F_GETFL);
    ICUNIT_TRACK_EQUAL(flags, O_CREAT | O_RDWR, 0);

    (void)close(fd);

    return 0;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FCNTL_002
 * @tc.name     fcntl
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, testFsFcntl002, Function | MediumTest | Level1)
{
    int fd = 0;
    int flags = 0;
    char tmpFileName[]= FILE1;

    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(fd, -1, fd);

    (void)fcntl(fd, F_SETFL, flags | O_APPEND);
    flags = fcntl(fd, F_GETFL);
    ICUNIT_TRACK_EQUAL(flags, O_CREAT | O_RDWR | O_APPEND, 0);

    (void)close(fd);

    return 0;
}
#endif

RUN_TEST_SUITE(PosixFsFuncTestSuite);

void PosixFsAPITest(void)
{
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
    RUN_ONE_TESTCASE(testFsFtellFseek001);
    RUN_ONE_TESTCASE(testFsFtellFseek002);
    RUN_ONE_TESTCASE(testFsFtellFseek003);
    RUN_ONE_TESTCASE(testFsFtellFseek004);
    RUN_ONE_TESTCASE(testFsFtellFseek005);
    RUN_ONE_TESTCASE(testFsFtellFseek009);
    RUN_ONE_TESTCASE(testFsFtellFseek011);
    RUN_ONE_TESTCASE(testFsFtellFseek012);
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
    RUN_ONE_TESTCASE(testFsOpendir001);
    RUN_ONE_TESTCASE(testFsOpendir002);
    RUN_ONE_TESTCASE(testFsOpendir003);
    RUN_ONE_TESTCASE(testFsReaddir001);
    RUN_ONE_TESTCASE(testFsReaddir002);
    RUN_ONE_TESTCASE(testFsReaddir003);
    RUN_ONE_TESTCASE(testFsRemove001);
    RUN_ONE_TESTCASE(testFsRemove002);
    RUN_ONE_TESTCASE(testFsRemove003);
    RUN_ONE_TESTCASE(testFsRemove004);
    RUN_ONE_TESTCASE(testFsRmdir001);
    RUN_ONE_TESTCASE(testFsRmdir002);
    RUN_ONE_TESTCASE(testFsRmdir003);
    RUN_ONE_TESTCASE(testFsRmdir004);
    RUN_ONE_TESTCASE(testFsUnlink001);
    RUN_ONE_TESTCASE(testFsUnlink002);
    RUN_ONE_TESTCASE(testFsRename001);
    RUN_ONE_TESTCASE(testFsRename002);
    RUN_ONE_TESTCASE(testFsRename003);
    RUN_ONE_TESTCASE(testFsStat001);
    RUN_ONE_TESTCASE(testFsStat002);
    RUN_ONE_TESTCASE(testFsStat003);
    RUN_ONE_TESTCASE(testFsOpen001);
    RUN_ONE_TESTCASE(testFsOpen002);
    RUN_ONE_TESTCASE(testFsOpen003);
    RUN_ONE_TESTCASE(testFsOpen004);
    RUN_ONE_TESTCASE(testFsOpen005);
#if (LOSCFG_SUPPORT_FATFS == 1)
    RUN_ONE_TESTCASE(testFsOpen006);
    RUN_ONE_TESTCASE(testFsOpen007);
    RUN_ONE_TESTCASE(testFsOpen008);
#endif
    RUN_ONE_TESTCASE(testFsClose001);
    RUN_ONE_TESTCASE(testFsWrite001);
    RUN_ONE_TESTCASE(testFsWrite002);
    RUN_ONE_TESTCASE(testFsWrite003);

#if (LOSCFG_SUPPORT_LITTLEFS == 1)
    RUN_ONE_TESTCASE(testFsFtellFseek006);
    RUN_ONE_TESTCASE(testFsFtellFseek007);
    RUN_ONE_TESTCASE(testFsFtellFseek008);
    RUN_ONE_TESTCASE(testFsFtellFseek013);
#endif

#if (LOSCFG_LIBC_MUSL == 1)
    RUN_ONE_TESTCASE(testFsFdopen004);
    RUN_ONE_TESTCASE(testFsFtellFseek010);
    RUN_ONE_TESTCASE(testFsDirname001);
    RUN_ONE_TESTCASE(testFsDirname002);
    RUN_ONE_TESTCASE(testFsDirname003);
    RUN_ONE_TESTCASE(testFsDirname004);
    RUN_ONE_TESTCASE(testFsFcntl001);
    RUN_ONE_TESTCASE(testFsFcntl002);
#endif
}

void PosixFsFuncTest()
{
    PosixFsAPITest();
    PosixFsOpenTest();
    PosixFsCloseTest();
    PosixFsOpendirTest();
    PosixFsClosedirTest();
    PosixFsReadTest();
    PosixFsWriteTest();
    PosixFsReaddirTest();
    PosixFsMkdirTest();
    PosixFsRmdirTest();
    PosixFsLseekTest();
    PosixFsUnlinkTest();
    PosixFsStatTest();
    PosixFsFstatTest();
    PosixFsFsyncTest();
    PosixFsRenameTest();
#if (LOSCFG_SUPPORT_FATFS == 1)
    PosixFsStatfsTest();
#endif
    PosixFsFtruncateTest();
    PosixFsPreadTest();
    PosixFsPwriteTest();
    PosixFsAccessTest();
    PosixFsFullTest();
    PosixFsPressureTest();
    return;
}
