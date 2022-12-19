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

/* *
 * @tc.number   SUB_KERNEL_FS_FSTAT_OK
 * @tc.name     fstat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFstatOK, Function | MediumTest | Level1)
{
    int32_t ret;
    struct stat buf = { 0 };
    const char tmpFileName[TEST_BUF_SIZE] = FILE1;

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = fstat(fd, &buf);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FSTAT_EBADF001
 * @tc.name     fstat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFstatEBADF001, Function | MediumTest | Level1)
{
    struct stat buf = { 0 };

    int32_t ret = fstat(-1, &buf);  /* -1, bad fd  */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FSTAT_EBADF002
 * @tc.name     fstat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFstatEBADF002, Function | MediumTest | Level1)
{
    struct stat buf = { 0 };

    int32_t ret = fstat(ERROR_CONFIG_NFILE_DESCRIPTORS, &buf);
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FSTAT_EBADF003
 * @tc.name     fstat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFstatEBADF003, Function | MediumTest | Level1)
{
    struct stat buf = { 0 };

    int32_t ret = fstat(0, &buf);   /* 0, used for stdin */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = fstat(1, &buf);   /* 1, used for stdout */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = fstat(2, &buf);   /* 2, used for stderr */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FSTAT_EINVAL
 * @tc.name     fstat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFstatEINVAL, Function | MediumTest | Level1)
{
    int32_t ret;
    struct stat *buf = NULL;
    const char tmpFileName[TEST_BUF_SIZE] = FILE1;

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = fstat(fd, buf);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FSTAT_ENOENT
 * @tc.name     fstat
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFstatENOENT, Function | MediumTest | Level1)
{
    int32_t ret;
    struct stat buf = { 0 };
    const char tmpFileName[TEST_BUF_SIZE] = FILE1;

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    struct MountPoint *mountBak = g_mountPoints;
    g_mountPoints = NULL;
    ret = fstat(fd, &buf);
    g_mountPoints = mountBak;
    ICUNIT_ASSERT_EQUAL(errno, ENOENT, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

void PosixFsFstatTest(void)
{
    RUN_ONE_TESTCASE(TestFsFstatOK);
    RUN_ONE_TESTCASE(TestFsFstatEBADF001);
    RUN_ONE_TESTCASE(TestFsFstatEBADF002);
    RUN_ONE_TESTCASE(TestFsFstatEBADF003);
    RUN_ONE_TESTCASE(TestFsFstatEINVAL);
    RUN_ONE_TESTCASE(TestFsFstatENOENT);
}
