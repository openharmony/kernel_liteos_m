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

#ifndef _LFS_API_H_
#define _LFS_API_H_

#include "bits/alltypes.h"
#include "sys/stat.h"

#include "dirent.h"
#include "lfs.h"
#include "lfs_util.h"
#include "memory.h"
#include "pthread.h"

#define INVALID_FD (-1)

typedef unsigned mode_t;

#ifndef VFS_ERROR
#define VFS_ERROR (-1)
#endif

#ifndef VFS_OK
#define VFS_OK  0
#endif

typedef struct {
    uint8_t useFlag;
    lfs_file_t file;
} LittleFsHandleStruct;

struct MountOps {
    int (*Mount)(const char *source, const char *target, const char *filesystemtype, unsigned long mountflags,
        const void *data);
    int (*Umount)(const char* target);
};

struct FsMap {
    const char *fileSystemtype;
    const struct MountOps *fsMops;
};

struct FileOps {
    int (*Open)(const char *path, int openFlag, int mode);
    int (*Close)(int fd);
    int (*Unlink)(const char *fileName);
    int (*Rmdir)(const char *dirName);
    int (*Mkdir)(const char *dirName, mode_t mode);
    struct dirent *(*Readdir)(DIR *dir);
    DIR *(*Opendir)(const char *dirName);
    int (*Closedir)(const DIR *dir);
    int (*Read)(int fd, void *buf, size_t len);
    int (*Write)(int fd, const void *buf, size_t len);
    int (*Seek)(int fd, off_t offset, int whence);
    int (*Getattr)(const char *path, struct stat *buf);
    int (*Rename)(const char *oldName, const char *newName);
    int (*Fsync)(int fd);
};

typedef struct {
    struct FileOps *fsVops;
} FileOpInfo;

typedef struct {
    uint8_t useFlag;
    lfs_dir_t dir;
} FileDirInfo;

#define LITTLE_FS_MAX_OPEN_FILES 100
#define LITTLE_FS_STANDARD_NAME_LENGTH 50

#define MAX_DEF_BUF_NUM 21
#define MAX_BUFFER_LEN 100
#define MAX_WRITE_FILE_LEN 500
#define MAX_READ_FILE_LEN 500
#define MAX_FILE_SYSTEM_LEN 2

#ifndef LFS_MAX_OPEN_DIRS
#define LFS_MAX_OPEN_DIRS 10
#endif

LittleFsHandleStruct *GetFreeFd(int *fd);

int InitMountInfo(const char *fileSystemType, const struct MountOps *fsMops);
int LfsMount(const char *source, const char *target, const char *fileSystemType, unsigned long mountflags,
    const void *data);

int LfsUmount(const char *target);
int LfsUnlink(const char *fileName);
int LfsMkdir(const char *dirName, mode_t mode);
int LfsRmdir(const char *dirName);
DIR *LfsOpendir(const char *dirName);
struct dirent *LfsReaddir(DIR *dir);
int LfsClosedir(const DIR *dir);
int LfsOpen(const char *path, int openFlag, int mode);
int LfsRead(int fd, void *buf, unsigned int len);
int LfsWrite(int fd, const void *buf, unsigned int len);
int LfsSeek(int fd, off_t offset, int whence);
int LfsClose(int fd);
int LfsRename(const char *oldName, const char *newName);
int LfsStat(const char *path, struct stat *buf);
int LfsFsync(int fd);

FileOpInfo GetFsOpInfo(void);
const struct FsMap *MountFindfs(const char *filesystemtype);

#endif /* _LFS_API_H_ */

