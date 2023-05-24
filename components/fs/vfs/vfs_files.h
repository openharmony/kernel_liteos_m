/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _VFS_FILES_H_
#define _VFS_FILES_H_

#include "dirent.h"
#include "sys/stat.h"
#include "unistd.h"
#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define FILE_STATUS_NOT_USED 0
#define FILE_STATUS_INITING 1
#define FILE_STATUS_READY 2
#define FILE_STATUS_CLOSING 3

struct FileOps;
struct File;
struct Dir;
struct MountPoint;

struct FileOps {
    int     (*open)(struct File *, const char *, int);
    int     (*close)(struct File *);
    ssize_t (*read)(struct File *, char *, size_t);
    ssize_t (*write)(struct File *, const char *, size_t);
    off_t   (*lseek)(struct File *, off_t, int);
    int     (*stat)(struct MountPoint *, const char *, struct stat *);
    int     (*truncate)(struct File *, off_t);
    int     (*unlink)(struct MountPoint *, const char *);
    int     (*rename)(struct MountPoint *, const char *, const char *);
    int     (*ioctl)(struct File *, int, unsigned long);
    int     (*sync)(struct File *);
    int     (*opendir)(struct Dir *, const char *);
    int     (*readdir)(struct Dir *, struct dirent *);
    int     (*closedir)(struct Dir *);
    int     (*mkdir)(struct MountPoint *, const char *);
    int     (*rmdir)(struct MountPoint *, const char *);
};

struct File {
    const struct FileOps *fFops;
    UINT32 fFlags;
    UINT32 fStatus;
    off_t fOffset;
    INT32 fOwner;
    struct MountPoint *fMp;
    void *fData; /* file system operations handle, fatfs FIL, etc. */
    const char *fullPath;
};

struct Dir {
    struct MountPoint *dMp;
    struct dirent      dDent;
    off_t              dOffset;
    void              *dData;
};

int FileToFd(const struct File *file);
struct File *FdToFile(int fd);
struct File *VfsFileGet(void);
struct File *VfsFileGetSpec(int fd);
void VfsFilePut(struct File *file);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _VFS_FILES_H_ */
