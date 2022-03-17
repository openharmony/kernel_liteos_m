/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_config.h"
#include "stdio.h"
#include "stdarg.h"
#include <sys/ioctl.h>

#ifdef LOSCFG_LIBC_NEWLIB_FS
#include "los_fs.h"
#else
#include "sys/stat.h"
#endif

#ifdef LOSCFG_LIBC_NEWLIB_FS
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

int _open(const char *path, int oflag, ...)
{
    va_list vaList;
    va_start(vaList, oflag);
    int ret;
    ret = LOS_Open(path, oflag);
    va_end(vaList);
    return ret;
}

int _close(int fd)
{
    return LOS_Close(fd);
}

ssize_t _read(int fd, void *buf, size_t nbyte)
{
    return LOS_Read(fd, buf, nbyte);
}

ssize_t _write(int fd, const void *buf, size_t nbyte)
{
    return LOS_Write(fd, buf, nbyte);
}

off_t _lseek(int fd, off_t offset, int whence)
{
    return LOS_Lseek(fd, offset, whence);
}

int _unlink(const char *path)
{
    return LOS_Unlink(path);
}

int _fstat(int fd, struct stat *buf)
{
    return LOS_Fstat(fd, buf);
}

int _stat(const char *path, struct stat *buf)
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

int ioctl(int fd, int req, ...)
{
    va_list ap;
    va_start(ap, req);
    int ret;
    ret = LOS_Ioctl(fd, req, ap);
    va_end(ap);
    return ret;
}

ssize_t pread(int fd, void *buf, size_t nbyte, off_t offset)
{
    return LOS_Pread(fd, buf, nbyte, offset);
}

ssize_t pwrite(int fd, const void *buf, size_t nbyte, off_t offset)
{
    return LOS_Pwrite(fd, buf, nbyte, offset);
}

#else /* #ifdef LOSCFG_FS_VFS */

int _open(const char *path, int oflag, ...)
{
    return -1;
}

int _close(int fd)
{
    return -1;
}

ssize_t _read(int fd, void *buf, size_t nbyte)
{
    return -1;
}

ssize_t _write(int fd, const void *buf, size_t nbyte)
{
    return -1;
}

off_t _lseek(int fd, off_t offset, int whence)
{
    return -1;
}

int _unlink(const char *path)
{
    return -1;
}

int _fstat(int fd, struct stat *buf)
{
    return -1;
}

int _stat(const char *path, struct stat *buf)
{
    return -1;
}

#endif
