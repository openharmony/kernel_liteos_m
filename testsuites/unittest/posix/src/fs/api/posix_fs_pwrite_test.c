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

/* *
 * @tc.number   SUB_KERNEL_FS_PRWRITE_OK
 * @tc.name     pwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsPwriteOK, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };
    char writeBuf[TEST_BUF_SIZE] = "fsPwrtiteTest";

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = pwrite(fd, writeBuf, TEST_BUF_SIZE, 0);   /* 0, offset distance */
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_PRWRITE_EFAULT
 * @tc.name     pwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsPwriteEFAULT, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };
    char *writeBuf = NULL;

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = pwrite(fd, writeBuf, TEST_BUF_SIZE, 0);   /* 0, offset distance */
    ICUNIT_ASSERT_EQUAL(errno, EFAULT, POSIX_FS_IS_ERROR);
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_PRWRITE_EACCES
 * @tc.name     pwrite
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsPwriteEACCES, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = { FILE1 };
    char writeBuf[TEST_BUF_SIZE] = "fsPwriteTest";

    int32_t fd = open(tmpFileName, O_CREAT | O_RDONLY, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = pwrite(fd, writeBuf, TEST_BUF_SIZE, 0);   /* 0, offset distance */
    ICUNIT_ASSERT_EQUAL(errno, EACCES, POSIX_FS_IS_ERROR);
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

void PosixFsPwriteTest(void)
{
    RUN_ONE_TESTCASE(TestFsPwriteOK);
    RUN_ONE_TESTCASE(TestFsPwriteEFAULT);
    RUN_ONE_TESTCASE(TestFsPwriteEACCES);
}
