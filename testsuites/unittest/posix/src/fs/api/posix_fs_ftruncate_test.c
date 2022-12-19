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
 * @tc.number   SUB_KERNEL_FS_FTRUNCATE_OK
 * @tc.name     ftruncate
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFtruncateOK, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = FILE2;
    const char writeBuf[TEST_BUF_SIZE] = "hello";

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = write(fd, writeBuf, TEST_BUF_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

    ret = ftruncate(fd, MODIFIED_FILE_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTRUNCATE_EBADF001
 * @tc.name     ftruncate
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFtruncateEBADF001, Function | MediumTest | Level1)
{
    int32_t ret = ftruncate(-1, MODIFIED_FILE_SIZE);    /* -1, bad fd */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = ftruncate(ERROR_CONFIG_NFILE_DESCRIPTORS, MODIFIED_FILE_SIZE);
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTRUNCATE_EBADF002
 * @tc.name     ftruncate
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFtruncateEBADF002, Function | MediumTest | Level1)
{
    int32_t ret = ftruncate(0, MODIFIED_FILE_SIZE);      /* 0, used for stdin */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = ftruncate(1, MODIFIED_FILE_SIZE);      /* 1, used for stdout */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = ftruncate(2, MODIFIED_FILE_SIZE);      /* 2, used for stderr */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTRUNCATE_EINVAL
 * @tc.name     ftruncate
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFtruncateEINVAL, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = FILE1;

    int32_t fd = open(tmpFileName, O_CREAT | O_RDWR, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = ftruncate(fd, -1);    /* -1, length after modification */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, POSIX_FS_IS_ERROR);
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_FTRUNCATE_EACCES
 * @tc.name     ftruncate
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsFtruncateEACCES, Function | MediumTest | Level1)
{
    int32_t ret;
    const char tmpFileName[TEST_BUF_SIZE] = FILE1;

    int32_t fd = open(tmpFileName, O_CREAT | O_RDONLY, TEST_MODE_HIGH);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = ftruncate(fd, MODIFIED_FILE_SIZE);
    ICUNIT_ASSERT_EQUAL(errno, EACCES, POSIX_FS_IS_ERROR);
    ICUNIT_GOTO_EQUAL(ret, POSIX_FS_IS_ERROR, ret, EXIT);

EXIT:
    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(tmpFileName);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

void PosixFsFtruncateTest(void)
{
#if (LOSCFG_SUPPORT_FATFS == 1)
    RUN_ONE_TESTCASE(TestFsFtruncateOK);
#endif
    RUN_ONE_TESTCASE(TestFsFtruncateEBADF001);
    RUN_ONE_TESTCASE(TestFsFtruncateEBADF002);
    RUN_ONE_TESTCASE(TestFsFtruncateEINVAL);
    RUN_ONE_TESTCASE(TestFsFtruncateEACCES);
}
