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

/**
 * @defgroup los_vfs fs
 * @ingroup kernel
 */

#ifndef _LOS_VFS_H_
#define _LOS_VFS_H_

#include "los_config.h"
#include "dirent.h"
#include "sys/mount.h"
#include "sys/statfs.h"
#include "sys/stat.h"
#include "unistd.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

int LOS_Open(const char *path, int oflag, ...);

int LOS_Close(int fd);

ssize_t LOS_Read(int fd, void *buf, size_t nbyte);

ssize_t LOS_Write(int fd, const void *buf, size_t nbyte);

off_t LOS_Lseek(int fd, off_t offset, int whence);

int LOS_Unlink(const char *path);

int LOS_Fstat(int fd, struct stat *buf);

int LOS_Stat(const char *path, struct stat *buf);

int LOS_Fsync(int fd);

int LOS_Mkdir(const char *path, mode_t mode);

DIR *LOS_Opendir(const char *dirName);

struct dirent *LOS_Readdir(DIR *dir);

int LOS_Closedir(DIR *dir);

int LOS_Rmdir(const char *path);

int LOS_Rename(const char *oldName, const char *newName);

int LOS_Statfs(const char *path, struct statfs *buf);

int LOS_Ftruncate(int fd, off_t length);

int LOS_FsUmount2(const char *target, int flag);

int LOS_FsUmount(const char *target);

int LOS_FsMount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data);

ssize_t LOS_Pread(int fd, void *buf, size_t nbyte, off_t offset);

ssize_t LOS_Pwrite(int fd, const void *buf, size_t nbyte, off_t offset);

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_FS_H_ */