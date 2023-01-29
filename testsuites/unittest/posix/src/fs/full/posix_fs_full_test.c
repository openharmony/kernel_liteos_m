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

#define TEST_OPEN_DIR_NUM           16
#define TEST_OPEN_FILE_NUM          16
#define WRITE_BUF_SIZE              128
#define READ_WRITE_BUF_SIZE         1024
#define TEMP_DIRE                   TEST_ROOT"/e"
#define TEMP_DIRE_FILE              TEST_ROOT"/e/eFile"
#define TEMP_DIRF                   TEST_ROOT"/f"
#define TEMP_DIRF_DIR               TEST_ROOT"/f/fDir"
#define TEST_DIRG                   TEST_ROOT"/g"

/* *
 * @tc.number   SUB_KERNEL_FS_FULL001
 * @tc.name     open close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull001, Function | MediumTest | Level1)
{
    int32_t ret;
    int32_t fd1 = -1;
    int32_t fd2 = -1;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };

    fd1 = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd1, POSIX_FS_IS_ERROR, fd1);

#if (LOSCFG_SUPPORT_FATFS == 1)
    fd2 = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_EQUAL(fd2, POSIX_FS_IS_ERROR, fd2);
#endif

    ret = close(fd1);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    fd2 = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd2, POSIX_FS_IS_ERROR, fd2);

    ret = close(fd2);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL002
 * @tc.name     open write read lseek read close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull002, Function | MediumTest | Level1)
{
    off_t off;
    int32_t ret;
    int32_t fd = -1;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };
    char *writeBuf = (char *)malloc(READ_WRITE_BUF_SIZE * sizeof(char));
    char *readBuf = (char *)malloc(READ_WRITE_BUF_SIZE * sizeof(char));

    (void)memset_s(writeBuf, READ_WRITE_BUF_SIZE, 'w', READ_WRITE_BUF_SIZE);
    (void)memset_s(readBuf, READ_WRITE_BUF_SIZE, 'r', READ_WRITE_BUF_SIZE);

    fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf, READ_WRITE_BUF_SIZE / 2);     /* 2, number of writes */
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ret = write(fd, writeBuf, READ_WRITE_BUF_SIZE / 2);     /* 2, number of writes */
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    off = lseek(fd, 0, SEEK_SET);   /* 0, offset distance */
    ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);

    for (int32_t i = 0; i < 8; i++) {            /* 8, number of reads */
        ret = read(fd, readBuf + i * 128, 128);  /* 128, 128, length per read */
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }

    for (int32_t j = 0; j < READ_WRITE_BUF_SIZE; j++) {
        ICUNIT_GOTO_EQUAL(writeBuf[j], readBuf[j], POSIX_FS_IS_ERROR, EXIT);
    }

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL003
 * @tc.name     mkdir opendir closedir rmdir fstat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull003, Function | MediumTest | Level1)
{
    int res;
    DIR *dir = NULL;
    int32_t index = 0;
    bool flag = true;
    struct dirent *dirmsg = NULL;
    struct stat buf = { 0 };
    int32_t i, fd[TEST_OPEN_FILE_NUM];
    char tmpFileName[TEST_BUF_SIZE] = { 0 };
    char fileName[TEST_BUF_SIZE] = { 0 };

    int32_t ret = mkdir(TEMP_DIRE, TEST_MODE_NORMAL);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    for (i = 0; i < TEST_OPEN_FILE_NUM; i++) {
        res = sprintf_s(tmpFileName, TEST_BUF_SIZE, "%s%02d", TEMP_DIRE_FILE, i);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            goto EXIT1;
        }
        fd[i] = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
        flag = fd[i] == -1 ? false : true;
        ICUNIT_GOTO_NOT_EQUAL(fd[i], POSIX_FS_IS_ERROR, fd[i], EXIT1);
    }
    dir = opendir(TEMP_DIRE);
    flag = dir == NULL ? false : true;
    ICUNIT_GOTO_NOT_EQUAL(dir, NULL, POSIX_FS_IS_ERROR, EXIT1);
    while ((dirmsg = readdir(dir)) != NULL) {
        res = sprintf_s(fileName, TEST_BUF_SIZE, "%s%02d", "eFile", index);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            goto EXIT1;
        }
        ret = strcmp(dirmsg->d_name, fileName);
        ICUNIT_GOTO_EQUAL(ret, POSIX_FS_NO_ERROR, ret, EXIT1);
        ret = fstat(fd[index], &buf);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT1);
        index++;
    }
    ICUNIT_GOTO_EQUAL(index, TEST_OPEN_FILE_NUM, POSIX_FS_IS_ERROR, EXIT1);
EXIT1:
    for (int32_t j = 0; j < i; j++) {
        res = sprintf_s(tmpFileName, TEST_BUF_SIZE, "%s%02d", TEMP_DIRE_FILE, j);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            return POSIX_FS_IS_ERROR;
        }
        ret = close(fd[j]);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
        ret = unlink(tmpFileName);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
    if (flag == false) {
        goto EXIT;
    }
    ret = closedir(dir);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    ret = rmdir(TEMP_DIRE);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL004
 * @tc.name     mkdir readdir opendir stat closedir rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull004, Function | MediumTest | Level1)
{
    int res;
    int32_t i;
    int32_t index = 0;
    DIR *dir = NULL;
    struct dirent *dirmsg = NULL;
    struct stat buf = { 0 };
    char dirName[TEST_BUF_SIZE] = { 0 };
    char tmpDirName[TEST_BUF_SIZE] = { 0 };

    int32_t ret = mkdir(TEMP_DIRF, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    for (i = 0; i < TEST_OPEN_DIR_NUM; i++) {
        res = sprintf_s(tmpDirName, TEST_BUF_SIZE, "%s%02d", TEMP_DIRF_DIR, i);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            goto EXIT;
        }
        ret = mkdir(tmpDirName, TEST_MODE_HIGH);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    }

    dir = opendir(TEMP_DIRF);
    ICUNIT_GOTO_NOT_EQUAL(dir, NULL, POSIX_FS_IS_ERROR, EXIT1);
    while ((dirmsg = readdir(dir)) != NULL) {
        res = sprintf_s(dirName, TEST_BUF_SIZE, "%s%02d", "fDir", index);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            goto EXIT1;
        }
        ret = strcmp(dirmsg->d_name, dirName);
        ICUNIT_GOTO_EQUAL(ret, POSIX_FS_NO_ERROR, ret, EXIT1);
        res = sprintf_s(tmpDirName, TEST_BUF_SIZE, "%s%02d", TEMP_DIRF_DIR, index);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            goto EXIT1;
        }
        ret = stat(tmpDirName, &buf);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
        ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFDIR, POSIX_FS_IS_ERROR, EXIT1);
        index++;
    }
    ICUNIT_GOTO_EQUAL(index, TEST_OPEN_DIR_NUM, POSIX_FS_IS_ERROR, EXIT1);
EXIT1:
    ret = closedir(dir);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    for (int32_t j = 0; j < i; j++) {
        res = sprintf_s(tmpDirName, TEST_BUF_SIZE, "%s%02d", TEMP_DIRF_DIR, j);
        if (res < 0) {
            printf("[%s:%d] sprintf_s failed\n", __func__, __LINE__);
            return POSIX_FS_IS_ERROR;
        }
        ret = rmdir(tmpDirName);
        ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    }
    ret = rmdir(TEMP_DIRF);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL005
 * @tc.name     read write lseek close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull005, Function | MediumTest | Level1)
{
    off_t off;
    int32_t ret;
    int32_t fd = -1;
    bool flag = true;
    char readBuf = 'r';
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };
    char *writeBuf = (char*)malloc(WRITE_BUF_SIZE * sizeof(char));

    (void)memset_s(writeBuf, WRITE_BUF_SIZE, 'w', WRITE_BUF_SIZE);

    fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf, WRITE_BUF_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    off = lseek(fd, 0, SEEK_SET);
    ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);

    for (int i = 1; i <= TEST_OPEN_FILE_NUM; i++) {
        ret = read(fd, &readBuf, 1);
        ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
        if (readBuf != 'w') {
            flag = false;
            break;
        }
        readBuf = 'r';
        off = lseek(fd, 7, SEEK_CUR);   /* 7, offset distance */
        ICUNIT_GOTO_NOT_EQUAL(off, POSIX_FS_IS_ERROR, off, EXIT);
    }

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL006
 * @tc.name     open fstat close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull006, Function | MediumTest | Level1)
{
    int32_t ret;
    struct stat buf = { 0 };
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = fstat(fd, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL007
 * @tc.name     open stat close unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull007, Function | MediumTest | Level1)
{
    int32_t ret;
    struct stat buf = { 0 };
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = stat(tmpFileName, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = stat(tmpFileName, &buf);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL008
 * @tc.name     mkdir stat rmdir
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull008, Function | MediumTest | Level1)
{
    char dirName[TEST_BUF_SIZE] = { TEST_DIRG };
    struct stat buf = { 0 };

    int32_t ret = mkdir(dirName, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = stat(dirName, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFDIR, POSIX_FS_IS_ERROR, EXIT1);

    ret = rmdir(dirName);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

    ret = stat(dirName, &buf);
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    return POSIX_FS_NO_ERROR;

EXIT1:
    ret = rmdir(dirName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    return POSIX_FS_IS_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FULL009
 * @tc.name     open close stat rename unlink
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFull009, Function | MediumTest | Level1)
{
    int32_t ret;
    struct stat buf = { 0 };
    char tmpFileName1[TEST_BUF_SIZE] = { FILE5 };
    char tmpFileName2[TEST_BUF_SIZE] = { FILE6 };

    int32_t fd = open(tmpFileName1, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = close(fd);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT1);

    ret = stat(tmpFileName1, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT);

    ret = rename(tmpFileName1, tmpFileName2);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    ret = stat(tmpFileName1, &buf);
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    ret = stat(tmpFileName2, &buf);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf.st_mode & S_IFMT, S_IFREG, POSIX_FS_IS_ERROR, EXIT);

    ret = unlink(tmpFileName1);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName2);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;

EXIT1:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);
EXIT:
    ret = unlink(tmpFileName1);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

void PosixFsFullTest(void)
{
    RUN_ONE_TESTCASE(TestFsFull001);
    RUN_ONE_TESTCASE(TestFsFull002);
#if (LOSCFG_SUPPORT_FATFS == 1)
    RUN_ONE_TESTCASE(TestFsFull003);
    RUN_ONE_TESTCASE(TestFsFull004);
#endif
    RUN_ONE_TESTCASE(TestFsFull005);
    RUN_ONE_TESTCASE(TestFsFull006);
    RUN_ONE_TESTCASE(TestFsFull007);
    RUN_ONE_TESTCASE(TestFsFull008);
    RUN_ONE_TESTCASE(TestFsFull009);
}
