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

#ifndef __POSIX_FS_TEST_H
#define __POSIX_FS_TEST_H

#include "posix_test.h"
#include <securec.h>
#include <stdio.h>
#include <libgen.h>
#include "ohos_types.h"
#include "los_config.h"
#include "vfs_files.h"
#include "vfs_mount.h"
#include "vfs_maps.h"
#include "kernel_test.h"
#include "log.h"
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include "limits.h"

#if (LOSCFG_SUPPORT_FATFS == 1)
#define TEST_ROOT            "system"
#endif

#if (LOSCFG_SUPPORT_LITTLEFS == 1)
#define TEST_ROOT            "/littlefs"
#endif

#define TEST_FILE_PTAH_RIGHT    TEST_ROOT"/FILE0"   /* file path, to open/rd/close */
#define FILE0                   "FILE0"             /* common file name used for testing  */
#define FILE1                   TEST_ROOT"/FILE1"   /* common file under test root path name used for testing */
#define FILE2                   TEST_ROOT"/FILE2"   /* common file under test root path name used for testing */
#define FILE3                   TEST_ROOT"/FILE3"   /* common file under test root path name used for testing */
#define FILE4                   TEST_ROOT"/FILE4"   /* common file under test root path name used for testing */
#define FILE5                   TEST_ROOT"/FILE5"   /* common file under test root path name used for testing */
#define FILE6                   TEST_ROOT"/FILE6"   /* common file under test root path name used for testing */
#define DIRA                    TEST_ROOT"/a"       /* common file under test root path name used for testing */
#define DIRB                    TEST_ROOT"/b"       /* common file under test root path name used for testing */
#define DIRC                    TEST_ROOT"/c"       /* common file under test root path name used for testing */
#define DIRD                    TEST_ROOT"/d"       /* common file under test root path name used for testing */

#define FILE_IN_DIRA            TEST_ROOT"/a/FILE0" /* common file under test root path name used for testing */
#define DIRAB                   TEST_ROOT"/a/b"     /* common file under test root path name used for testing */
#define DIRAC                   TEST_ROOT"/a/c"     /* common file under test root path name used for testing */

#define TEST_BUF_SIZE                   40          /* 40, common data for test, no special meaning */
#define TEST_SEEK_SIZE                  10          /* 10, common data for test, no special meaning */
#define TEST_RW_SIZE                    20          /* 20, common data for test, no special meaning */
#define TEST_LOOPUP_TIME                20          /* 100, common data for test, no special meaning */

#define ERROR_CONFIG_NFILE_DESCRIPTORS  88888       /* 88888, common data for test, no special meaning */
#define MODIFIED_FILE_SIZE              1024        /* 1024, common data for test, no special meaning */

#define TEST_MODE_NORMAL                0666        /* 0666, common data for test, no special meaning */
#define TEST_MODE_HIGH                  0777        /* 0777, common data for test, no special meaning */

#define POSIX_FS_IS_ERROR               (-1)          /* -1, common data for test, no special meaning */
#define POSIX_FS_NO_ERROR               0             /* 0, common data for test, no special meaning */

#endif /* __POSIX_FS_TEST_H */
