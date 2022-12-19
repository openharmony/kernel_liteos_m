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

#define TEST_FILE1     TEST_ROOT"/file1.txt"
#define TEST_FILE2     TEST_ROOT"/file2.txt"
#define TEST_FILE3     TEST_ROOT"/file3.txt"
#define TEST_FILE4     TEST_ROOT"/file4.txt"

/* *
 * @tc.number   SUB_KERNEL_FS_RENAME_OK
 * @tc.name     rename
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsRenameOK, Function | MediumTest | Level1)
{
    int32_t ret;
    const char fileNameOld[TEST_BUF_SIZE] = { TEST_FILE1 };
    const char fileNameNew[TEST_BUF_SIZE] = { TEST_FILE2 };

    int32_t fd = open(fileNameOld, O_CREAT | O_RDWR);
    ICUNIT_ASSERT_NOT_EQUAL(fd, POSIX_FS_IS_ERROR, fd);

    ret = close(fd);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = rename(fileNameOld, fileNameNew);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    ret = unlink(fileNameNew);
    ICUNIT_ASSERT_NOT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RENAME_EINVAL001
 * @tc.name     rename
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsRenameEINVAL001, Function | MediumTest | Level1)
{
    const char *fileNameOld = NULL;
    const char fileNameNew[TEST_BUF_SIZE] = { TEST_FILE3 };

    int32_t ret = rename(fileNameOld, fileNameNew);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RENAME_EINVAL002
 * @tc.name     rename
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsRenameEINVAL002, Function | MediumTest | Level1)
{
    const char fileNameOld[TEST_BUF_SIZE] = { TEST_FILE4 };
    const char *fileNameNew = NULL;

    int32_t ret = rename(fileNameOld, fileNameNew);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

/* *
 * @tc.number   SUB_KERNEL_FS_RENAME_EINVAL003
 * @tc.name     rename
 * @tc.desc     [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixFsFuncTestSuite, TestFsRenameEINVAL003, Function | MediumTest | Level1)
{
    int32_t ret;
    const char fileNameOld[TEST_BUF_SIZE] = { TEST_FILE3 };
    const char fileNameNew[TEST_BUF_SIZE] = { TEST_FILE4 };

    struct MountPoint *mountBak = g_mountPoints;
    g_mountPoints = NULL;
    ret = rename(fileNameOld, fileNameNew);
    g_mountPoints = mountBak;
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, POSIX_FS_IS_ERROR);
    ICUNIT_ASSERT_EQUAL(ret, POSIX_FS_IS_ERROR, ret);

    return POSIX_FS_NO_ERROR;
}

void PosixFsRenameTest(void)
{
    RUN_ONE_TESTCASE(TestFsRenameOK);
    RUN_ONE_TESTCASE(TestFsRenameEINVAL001);
    RUN_ONE_TESTCASE(TestFsRenameEINVAL002);
    RUN_ONE_TESTCASE(TestFsRenameEINVAL003);
}
