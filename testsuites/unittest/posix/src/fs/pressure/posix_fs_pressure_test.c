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

#include "posix_fs_test.h"
#include "vfs_config.h"

#define MAX_OPEN_FILE_NUM               16
#define PRESSURE_RUN_TIMES              200
#define PRESSURE_RUN_TIMES003           150
#define READ_WRITE_BUF_SIZE             1024
#define PATH_MAX_NAME_LEN               PATH_MAX
#define MAX_OPEN_DIRS_NUM               LOSCFG_MAX_OPEN_DIRS
#define TEMP_FILE_PATH                  TEST_ROOT"/FILE7"
#define TEMP_DIR_PATH                   TEST_ROOT"/DIRE"

/* *
 * @tc.number   SUB_KERNEL_FS_FRESSURE001
 * @tc.name     open close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsPressure001, Function | MediumTest | Level1)
{
    int32_t ret;
    int32_t fd = -1;
    char tmpFileName[PATH_MAX_NAME_LEN + 1] = TEST_ROOT"/";

    int32_t len = strlen(TEST_ROOT);
    for (int32_t i = len + 1; i < PATH_MAX_NAME_LEN - 1; i++) {
        tmpFileName[i] = 'F';
    }
    tmpFileName[PATH_MAX_NAME_LEN - 1] = '\0';

    for (int32_t times = 0; times < PRESSURE_RUN_TIMES; times++) {
        fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
        ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

        ret = close(fd);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

        ret = unlink(tmpFileName);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }

    tmpFileName[PATH_MAX_NAME_LEN - 1] = 'E';
    tmpFileName[PATH_MAX_NAME_LEN] = '\0';
    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_EQUAL(errno, ENAMETOOLONG, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    return POSIX_FS_NO_ERROR;

EXIT1:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_PRESSURE002
 * @tc.name     open write lseek read close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsPressure002, Function | MediumTest | Level1)
{
    off_t off;
    int32_t ret;
    int32_t fd = -1;
    bool flag = true;
    const char tmpFileName[TEST_BUF_SIZE] = FILE1;
    char *writeBuf = (char *)malloc(READ_WRITE_BUF_SIZE * sizeof(char));
    char *readBuf = (char *)malloc(READ_WRITE_BUF_SIZE * sizeof(char));

    (void)memset_s(writeBuf, READ_WRITE_BUF_SIZE, 'w', READ_WRITE_BUF_SIZE);

    for (int32_t times = 0; times < PRESSURE_RUN_TIMES; times++) {
        (void)memset_s(readBuf, READ_WRITE_BUF_SIZE, 'r', READ_WRITE_BUF_SIZE);
        fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
        ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

        ret = write(fd, writeBuf, READ_WRITE_BUF_SIZE / 2);     /* 2, common data for test, no special meaning */
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        ret = write(fd, writeBuf, READ_WRITE_BUF_SIZE / 2);     /* 2, common data for test, no special meaning */
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

        off = lseek(fd, 0, SEEK_SET);   /* 0, offset distance */
        ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT1);

        for (int32_t i = 0; i < 8; i++) {   /* 8, number of reads */
            ret = read(fd, readBuf + i * 128, 128); /* 128, length per read */
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        }

        for (int32_t i = 0; i < READ_WRITE_BUF_SIZE; i++) {
            ICUNIT_GOTO_EQUAL(writeBuf[i], readBuf[i], POSIX_FS_IS_ERROR, EXIT1);
        }

        ret = close(fd);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

        ret = unlink(tmpFileName);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }

    return POSIX_FS_NO_ERROR;

EXIT1:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_ORESSURE003
 * @tc.name     open fstat stat mkdir unlink rmdir close
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsPressure003, Function | MediumTest | Level1)
{
    int res;
    int32_t ret;
    bool flagMkdir = true;
    struct stat buf = { 0 };
    int32_t j, k, fd[MAX_OPEN_FILE_NUM];
    char tmpDirName[TEST_BUF_SIZE] = { 0 };
    char tmpFileName[TEST_BUF_SIZE] = { 0 };

    for (int32_t times = 0; times < PRESSURE_RUN_TIMES003; times++) {
        for (j = 0; j < MAX_OPEN_FILE_NUM; j++) {
            res = sprintf_s(tmpFileName, TEST_BUF_SIZE, "%s%02d", TEMP_FILE_PATH, j);
            if (res < 0) {
                printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
                goto EXIT3;
            }
            fd[j] = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
            ICUNIT_GOTO_NOT_EQUAL(fd[j], POSIX_FS_IS_ERROR, fd[j], EXIT3);
        }
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            ret = fstat(fd[i], &buf);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT3);
            ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT3);
        }
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            ret = close(fd[i]);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT2);
        }
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            res = sprintf_s(tmpFileName, TEST_BUF_SIZE, "%s%02d", TEMP_FILE_PATH, i);
            if (res < 0) {
                printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
                goto EXIT1;
            }
            ret = stat(tmpFileName, &buf);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
            ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT1);
        }
        for (k = 0; k < MAX_OPEN_DIRS_NUM; k++) {
            res = sprintf_s(tmpDirName, TEST_BUF_SIZE, "%s%02d", TEMP_DIR_PATH, k);
            if (res < 0) {
                printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
                goto EXIT1;
            }
            ret = mkdir(tmpDirName, TEST_MODE_HIGH);
            if (ret == POSIX_FS_IS_ERROR) {
                flagMkdir = false;
            }
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        }
        for (int32_t i = 0; i < MAX_OPEN_DIRS_NUM; i++) {
            res = sprintf_s(tmpDirName, TEST_BUF_SIZE, "%s%02d", TEMP_DIR_PATH, i);
            if (res < 0) {
                printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
                goto EXIT1;
            }
            ret = stat(tmpDirName, &buf);
            ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
            ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFDIR, POSIX_FS_IS_ERROR, EXIT1);
        }
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            res = sprintf_s(tmpFileName, TEST_BUF_SIZE, "%s%02d", TEMP_FILE_PATH, i);
            if (res < 0) {
                printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
                return POSIX_FS_IS_ERROR;
            }
            ret = unlink(tmpFileName);
            ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        }
        for (int32_t i = 0; i < MAX_OPEN_DIRS_NUM; i++) {
            res = sprintf_s(tmpDirName, TEST_BUF_SIZE, "%s%02d", TEMP_DIR_PATH, i);
            if (res < 0) {
                printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
                return POSIX_FS_IS_ERROR;
            }
            ret = rmdir(tmpDirName);
            ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        }
        for (int32_t i = 0; i < MAX_OPEN_FILE_NUM; i++) {
            res = sprintf_s(tmpFileName, TEST_BUF_SIZE, "%s%02d", TEMP_FILE_PATH, i);
            if (res < 0) {
                printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
                return POSIX_FS_IS_ERROR;
            }
            ret = stat(tmpFileName, &buf);
            ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        }
        for (int32_t i = 0; i < MAX_OPEN_DIRS_NUM; i++) {
            res = sprintf_s(tmpDirName, TEST_BUF_SIZE, "%s%02d", TEMP_DIR_PATH, i);
            if (res < 0) {
                printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
                return POSIX_FS_IS_ERROR;
            }
            ret = stat(tmpDirName, &buf);
            ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        }
    }
    return POSIX_FS_NO_ERROR;
EXIT3:
    for (int32_t m = 0; m < j; m++) {
        ret = close(fd[m]);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
    goto EXIT1;
EXIT2:
    for (int32_t m = j; m < MAX_OPEN_FILE_NUM; m++) {
        ret = close(fd[m]);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
EXIT1:
    for (int32_t m = 0; m < j; m++) {
        res = sprintf_s(tmpFileName, TEST_BUF_SIZE, "%s%02d", TEMP_FILE_PATH, m);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            return POSIX_FS_IS_ERROR;
        }
        ret = unlink(tmpFileName);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
    if (flagMkdir == false) {
        goto EXIT;
    }
    return POSIX_FS_NO_ERROR;
EXIT:
    for (int32_t m = 0; m < k; m++) {
        res = sprintf_s(tmpDirName, TEST_BUF_SIZE, "%s%02d", TEMP_DIR_PATH, m);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            return POSIX_FS_IS_ERROR;
        }
        ret = rmdir(tmpDirName);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
    return POSIX_FS_NO_ERROR;
}

void PosixFsPressureTest(void)
{
    RUN_ONE_TESTCASE(TestFsPressure001);
    RUN_ONE_TESTCASE(TestFsPressure002);
    RUN_ONE_TESTCASE(TestFsPressure003);
}
