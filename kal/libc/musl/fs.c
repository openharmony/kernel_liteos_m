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
#define _GNU_SOURCE

#include "los_config.h"
#include <errno.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef LOSCFG_LIBC_MUSL_FS
#include "los_fs.h"

int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data)
{
    return LOS_FsMount(source, target, filesystemtype, mountflags, data);
}

int umount(const char *target)
{
    return LOS_FsUmount(target);
}

int umount2(const char *target, int flag)
{
    return LOS_FsUmount2(target, flag);
}

int open(const char *path, int oflag, ...)
{
    va_list vaList;
    va_start(vaList, oflag);
    int ret;
    ret = LOS_Open(path, oflag, vaList);
    va_end(vaList);
    return ret;
}

int close(int fd)
{
    return LOS_Close(fd);
}

ssize_t read(int fd, void *buf, size_t nbyte)
{
    return LOS_Read(fd, buf, nbyte);
}

ssize_t write(int fd, const void *buf, size_t nbyte)
{
    return LOS_Write(fd, buf, nbyte);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return LOS_Lseek(fd, offset, whence);
}

int unlink(const char *path)
{
    return LOS_Unlink(path);
}

int fstat(int fd, struct stat *buf)
{
    return LOS_Fstat(fd, buf);
}

int stat(const char *path, struct stat *buf)
{
    return LOS_Stat(path, buf);
}

int fsync(int fd)
{
    return LOS_Fsync(fd);
}

int mkdir(const char *path, mode_t mode)
{
    return LOS_Mkdir(path, mode);
}

DIR *opendir(const char *dirName)
{
    return LOS_Opendir(dirName);
}

struct dirent *readdir(DIR *dir)
{
    return LOS_Readdir(dir);
}

int closedir(DIR *dir)
{
    return LOS_Closedir(dir);
}

int rmdir(const char *path)
{
    return LOS_Unlink(path);
}

int rename(const char *oldName, const char *newName)
{
    return LOS_Rename(oldName, newName);
}

int statfs(const char *path, struct statfs *buf)
{
    return LOS_Statfs(path, buf);
}

int ftruncate(int fd, off_t length)
{
    return LOS_Ftruncate(fd, length);
}

ssize_t pread(int fd, void *buf, size_t nbyte, off_t offset)
{
    return LOS_Pread(fd, buf, nbyte, offset);
}

ssize_t pwrite(int fd, const void *buf, size_t nbyte, off_t offset)
{
    return LOS_Pwrite(fd, buf, nbyte, offset);
}

int access(const char *path, int mode)
{
    struct stat st;

    if (stat(path, &st) < 0) {
        return -1;
    }
    if ((st.st_mode & S_IFDIR) || (st.st_mode & S_IFREG)) {
        return 0;
    }
    if ((mode & W_OK) && !(st.st_mode & S_IWRITE)) {
        return -1;
    }

    return 0;
}

int fcntl(int fd, int cmd, ...)
{
    int ret;
    va_list vaList;

    va_start(vaList, cmd);
    ret = OsFcntl(fd, cmd, vaList);
    va_end(vaList);
    return ret;
}

int ioctl(int fd, int req, ...)
{
    int ret;
    va_list vaList;

    va_start(vaList, req);
    ret = OsIoctl(fd, req, vaList);
    va_end(vaList);
    return ret;
}

#else /* #ifdef LOSCFG_FS_VFS */

int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data)
{
    return -1;
}

int umount(const char *target)
{
    return -1;
}

int umount2(const char *target, int flag)
{
    return -1;
}

int open(const char *path, int oflag, ...)
{
    return -1;
}

int close(int fd)
{
    return -1;
}

ssize_t read(int fd, void *buf, size_t nbyte)
{
    return -1;
}

ssize_t write(int fd, const void *buf, size_t nbyte)
{
    return -1;
}

off_t lseek(int fd, off_t offset, int whence)
{
    return -1;
}

int unlink(const char *path)
{
    return -1;
}

int fstat(int fd, struct stat *buf)
{
    return -1;
}

int stat(const char *path, struct stat *buf)
{
    return -1;
}

int fsync(int fd)
{
    return -1;
}

int mkdir(const char *path, mode_t mode)
{
    return -1;
}

DIR *opendir(const char *dirName)
{
    return NULL;
}

struct dirent *readdir(DIR *dir)
{
    return NULL;
}

int closedir(DIR *dir)
{
    return -1;
}

int rmdir(const char *path)
{
    return -1;
}

int rename(const char *oldName, const char *newName)
{
    return -1;
}

int statfs(const char *path, struct statfs *buf)
{
    return -1;
}

int ftruncate(int fd, off_t length)
{
    return -1;
}

ssize_t pread(int fd, void *buf, size_t nbyte, off_t offset)
{
    return -1;
}

ssize_t pwrite(int fd, const void *buf, size_t nbyte, off_t offset)
{
    return -1;
}

int access(const char *path, int mode)
{
    return -1;
}

int fcntl(int fd, int cmd, ...)
{
    return -1;
}

int ioctl(int fd, int req, ...)
{
    return -1;
}
#endif
