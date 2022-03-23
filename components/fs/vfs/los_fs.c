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

#include "los_fs.h"
#include "los_config.h"
#include "fs_operations.h"
#if (LOSCFG_SUPPORT_FATFS == 1)
#include "fatfs.h"
#endif
#include "dirent.h"
#include "errno.h"
#include "fcntl.h"
#include "securec.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/mount.h"
#include "sys/statfs.h"
#include "sys/stat.h"
#include "unistd.h"

#ifdef LOSCFG_NET_LWIP_SACK
#define _BSD_SOURCE
#include "lwip/sockets.h"
#endif

#include "vfs_config.h"

#ifdef LOSCFG_RANDOM_DEV
#include "hks_client.h"
#define RANDOM_DEV_FD  CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS
#define RANDOM_DEV_PATH  "/dev/random"
#endif

#if (LOSCFG_POSIX_PIPE_API == 1)
#include "pipe_impl.h"
#ifdef LOSCFG_RANDOM_DEV
#define PIPE_DEV_FD (RANDOM_DEV_FD + 1)
#else
#define PIPE_DEV_FD (CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS)
#endif

int PollQueryFd(int fd, struct PollTable *table)
{
    if (fd >= PIPE_DEV_FD) {
        return PipePoll(fd, table);
    }

    return -ENODEV;
}
#endif

#define FREE_AND_SET_NULL(ptr) do { \
    free(ptr);                      \
    ptr = NULL;                     \
} while (0)

#ifdef LOSCFG_RANDOM_DEV
/**
 * @brief Get canonical form of a given path based on cwd(Current working directory).
 *
 * @param cwd Indicates the current working directory.
 * @param path Indicates the path to be canonicalization.
 * @param buf Indicates the pointer to the buffer where the result will be return.
 * @param bufSize Indicates the size of the buffer.
 * @return Returns the length of the canonical path.
 *
 * @attention if path is an absolute path, cwd is ignored. if cwd if not specified, it is assumed to be root('/').
 *            if the buffer is not big enough the result will be truncated, but the return value will always be the
 *            length of the canonical path.
 */
static size_t GetCanonicalPath(const char *cwd, const char *path, char *buf, size_t bufSize)
{
    size_t offset;
    if (!path) {
        path = "";
    }

    if (!cwd || path[0] == '/') {
        cwd = "";
    }

    offset = strlen("///") + 1; // three '/' and one '\0'
    size_t tmpLen = strlen(cwd) + strlen(path) + offset;
    char *tmpBuf = (char *)malloc(tmpLen);
    if (tmpBuf == NULL) {
        return FS_SUCCESS;
    }

    if (-1 == sprintf_s(tmpBuf, tmpLen, "/%s/%s/", cwd, path)) {
        free(tmpBuf);
        return FS_SUCCESS;
    }

    char *p;
    /* replace /./ to / */
    offset = strlen("/./") - 1;
    while ((p = strstr(tmpBuf, "/./")) != NULL) {
        if (EOK != memmove_s(p, tmpLen - (p - tmpBuf), p + offset, tmpLen - (p - tmpBuf) - offset)) {
            free(tmpBuf);
            return FS_SUCCESS;
        }
    }

    /* replace // to / */
    while ((p = strstr(tmpBuf, "//")) != NULL) {
        if (EOK != memmove_s(p, tmpLen - (p - tmpBuf), p + 1, tmpLen - (p - tmpBuf) - 1)) {
            free(tmpBuf);
            return FS_SUCCESS;
        }
    }

    /* handle /../ (e.g., replace /aa/bb/../ to /aa/) */
    offset = strlen("/../") - 1;
    while ((p = strstr(tmpBuf, "/../")) != NULL) {
        char *start = p;
        while (start > tmpBuf && *(start - 1) != '/') {
            --start;
        }
        if (EOK != memmove_s(start, tmpLen - (start - tmpBuf), p + offset, tmpLen - (p - tmpBuf) - offset)) {
            free(tmpBuf);
            return FS_SUCCESS;
        }
    }

    size_t totalLen = strlen(tmpBuf);
    /* strip the last / */
    if (totalLen > 1 && tmpBuf[totalLen - 1] == '/') {
        tmpBuf[--totalLen] = 0;
    }

    if (!buf || bufSize == 0) {
        free(tmpBuf);
        return totalLen;
    }

    if (EOK != memcpy_s(buf, bufSize, tmpBuf, (((totalLen + 1) > bufSize) ? bufSize : (totalLen + 1)))) {
        free(tmpBuf);
        return FS_SUCCESS;
    }

    buf[bufSize - 1] = 0;
    free(tmpBuf);
    return totalLen;
}
#endif

static struct FsMap g_fsmap[MAX_FILESYSTEM_LEN] = {0};
static struct FsMap *g_fs = NULL;

static void InitMountInfo(void)
{
#if (LOSCFG_SUPPORT_FATFS == 1)
    extern struct MountOps g_fatfsMnt;
    extern struct FileOps g_fatfsFops;
    g_fsmap[0].fileSystemtype = strdup("fat");
    g_fsmap[0].fsMops = &g_fatfsMnt;
    g_fsmap[0].fsFops = &g_fatfsFops;
#endif
#if (LOSCFG_SUPPORT_LITTLEFS == 1)
    extern struct MountOps g_lfsMnt;
    extern struct FileOps g_lfsFops;
    g_fsmap[1].fileSystemtype = strdup("littlefs");
    g_fsmap[1].fsMops = &g_lfsMnt;
    g_fsmap[1].fsFops = &g_lfsFops;
#endif
}

static struct FsMap *MountFindfs(const char *fileSystemtype)
{
    struct FsMap *m = NULL;

    for (int i = 0; i < MAX_FILESYSTEM_LEN; i++) {
        m = &(g_fsmap[i]);
        if (m->fileSystemtype && strcmp(fileSystemtype, m->fileSystemtype) == 0) {
            return m;
        }
    }

    return NULL;
}

int LOS_FsMount(const char *source, const char *target,
                const char *filesystemtype, unsigned long mountflags,
                const void *data)
{
    static int initFlag = 0;

    if (initFlag == 0) {
        InitMountInfo();
        initFlag = 1;
    }

    g_fs = MountFindfs(filesystemtype);
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }

    if (g_fs->fsMops == NULL || g_fs->fsMops->Mount == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }

    return g_fs->fsMops->Mount(source, target, filesystemtype, mountflags, data);
}

int LOS_FsUmount(const char *target)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsMops == NULL || g_fs->fsMops->Umount == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsMops->Umount(target);
}

int LOS_FsUmount2(const char *target, int flag)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsMops == NULL || g_fs->fsMops->Umount2 == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsMops->Umount2(target, flag);
}

int LOS_Open(const char *path, int oflag, ...)
{
#ifdef LOSCFG_RANDOM_DEV
    unsigned flags = O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_CREAT | O_LARGEFILE | O_TRUNC | O_EXCL | O_DIRECTORY;
    if ((unsigned)oflag & ~flags) {
        errno = EINVAL;
        return FS_FAILURE;
    }

    size_t pathLen = strlen(path) + 1;
    if ((unsigned)pathLen > PATH_MAX) {
        errno = EINVAL;
        return FS_FAILURE;
    }
    char *canonicalPath = (char *)malloc(pathLen);
    if (!canonicalPath) {
        errno = ENOMEM;
        return FS_FAILURE;
    }
    if (GetCanonicalPath(NULL, path, canonicalPath, pathLen) == 0) {
        FREE_AND_SET_NULL(canonicalPath);
        errno = ENOMEM;
        return FS_FAILURE;
    }

    if (strcmp(canonicalPath, RANDOM_DEV_PATH) == 0) {
        FREE_AND_SET_NULL(canonicalPath);
        if ((O_ACCMODE & (unsigned)oflag) != O_RDONLY) {
            errno = EPERM;
            return FS_FAILURE;
        }
        if ((unsigned)oflag & O_DIRECTORY) {
            errno = ENOTDIR;
            return FS_FAILURE;
        }
        return RANDOM_DEV_FD;
    }
    if (strcmp(canonicalPath, "/") == 0 || strcmp(canonicalPath, "/dev") == 0) {
        FREE_AND_SET_NULL(canonicalPath);
        if ((unsigned)oflag & O_DIRECTORY) {
            errno = EPERM;
            return FS_FAILURE;
        }
        errno = EISDIR;
        return FS_FAILURE;
    }
    FREE_AND_SET_NULL(canonicalPath);
#endif

#if (LOSCFG_POSIX_PIPE_API == 1)
    if ((path != NULL) && !strncmp(path, PIPE_DEV_PATH, strlen(PIPE_DEV_PATH))) {
        return PipeOpen(path, oflag, PIPE_DEV_FD);
    }
#endif

    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Open == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Open(path, oflag);
}

int LOS_Close(int fd)
{
#ifdef LOSCFG_RANDOM_DEV
    if (fd == RANDOM_DEV_FD) {
        return FS_SUCCESS;
    }
#endif
#ifdef LOSCFG_NET_LWIP_SACK
    if (fd >= CONFIG_NFILE_DESCRIPTORS && fd < (CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS)) {
        return closesocket(fd);
    }
#endif

#if (LOSCFG_POSIX_PIPE_API == 1)
    if (fd >= PIPE_DEV_FD) {
        return PipeClose(fd);
    }
#endif

    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Close == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Close(fd);
}

ssize_t LOS_Read(int fd, void *buf, size_t nbyte)
{
#ifdef LOSCFG_RANDOM_DEV
    if (fd == RANDOM_DEV_FD) {
        if (nbyte == 0) {
            return FS_SUCCESS;
        }
        if (buf == NULL) {
            errno = EINVAL;
            return FS_FAILURE;
        }
        if (nbyte > 1024) { /* 1024, max random_size */
            nbyte = 1024; /* hks_generate_random: random_size must <= 1024 */
        }
        struct hks_blob key = {HKS_BLOB_TYPE_RAW, (uint8_t *)buf, nbyte};
        if (hks_generate_random(&key) != 0) {
            errno = EIO;
            return FS_FAILURE;
        }
        return (ssize_t)nbyte;
    }
#endif
#ifdef LOSCFG_NET_LWIP_SACK
    if (fd >= CONFIG_NFILE_DESCRIPTORS && fd < (CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS)) {
        return recv(fd, buf, nbyte, 0);
    }
#endif

#if (LOSCFG_POSIX_PIPE_API == 1)
    if (fd >= PIPE_DEV_FD) {
        return PipeRead(fd, buf, nbyte);
    }
#endif

    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Read == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Read(fd, buf, nbyte);
}

ssize_t LOS_Write(int fd, const void *buf, size_t nbyte)
{
#ifdef LOSCFG_RANDOM_DEV
    if (fd == RANDOM_DEV_FD) {
        errno = EBADF; /* "/dev/random" is readonly */
        return FS_FAILURE;
    }
#endif
#ifdef LOSCFG_NET_LWIP_SACK
    if (fd >= CONFIG_NFILE_DESCRIPTORS && fd < (CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS)) {
        return send(fd, buf, nbyte, 0);
    }
#endif

#if (LOSCFG_POSIX_PIPE_API == 1)
    if (fd >= PIPE_DEV_FD) {
        return PipeWrite(fd, buf, nbyte);
    }
#endif

    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Write == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Write(fd, buf, nbyte);
}

off_t LOS_Lseek(int fd, off_t offset, int whence)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Seek == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Seek(fd, offset, whence);
}

int LOS_Unlink(const char *path)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Unlink == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Unlink(path);
}

int LOS_Fstat(int fd, struct stat *buf)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Fstat == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Fstat(fd, buf);
}

int LOS_Stat(const char *path, struct stat *buf)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Getattr == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Getattr(path, buf);
}

int LOS_Fsync(int fd)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Fsync == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Fsync(fd);
}

int LOS_Mkdir(const char *path, mode_t mode)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Mkdir == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Mkdir(path, mode);
}

DIR *LOS_Opendir(const char *dirName)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return NULL;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Opendir == NULL) {
        errno = ENOSYS;
        return NULL;
    }
    return g_fs->fsFops->Opendir(dirName);
}

struct dirent *LOS_Readdir(DIR *dir)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return NULL;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Readdir == NULL) {
        errno = ENOSYS;
        return NULL;
    }
    return g_fs->fsFops->Readdir(dir);
}

int LOS_Closedir(DIR *dir)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Closedir == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Closedir(dir);
}

int LOS_Rmdir(const char *path)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Rmdir == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Rmdir(path);
}

int LOS_Rename(const char *oldName, const char *newName)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Rename == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Rename(oldName, newName);
}

int LOS_Statfs(const char *path, struct statfs *buf)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsMops == NULL || g_fs->fsMops->Statfs == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsMops->Statfs(path, buf);
}

int LOS_Ftruncate(int fd, off_t length)
{
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Ftruncate == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Ftruncate(fd, length);
}

ssize_t LOS_Pread(int fd, void *buf, size_t nbyte, off_t offset)
{
#ifdef LOSCFG_RANDOM_DEV
    if (fd == RANDOM_DEV_FD) {
        if (nbyte == 0) {
            return FS_SUCCESS;
        }
        if (buf == NULL) {
            errno = EINVAL;
            return FS_FAILURE;
        }
        if (nbyte > 1024) { /* 1024, max random_size */
            nbyte = 1024; /* hks_generate_random: random_size must <= 1024 */
        }
        struct hks_blob key = {HKS_BLOB_TYPE_RAW, (uint8_t *)buf, nbyte};
        if (hks_generate_random(&key) != 0) {
            errno = EIO;
            return FS_FAILURE;
        }
        return (ssize_t)nbyte;
    }
#endif
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Pread == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Pread(fd, buf, nbyte, offset);
}

ssize_t LOS_Pwrite(int fd, const void *buf, size_t nbyte, off_t offset)
{
#ifdef LOSCFG_RANDOM_DEV
    if (fd == RANDOM_DEV_FD) {
        errno = EBADF; /* "/dev/random" is readonly */
        return FS_FAILURE;
    }
#endif
    if (g_fs == NULL) {
        errno = ENODEV;
        return FS_FAILURE;
    }
    if (g_fs->fsFops == NULL || g_fs->fsFops->Pwrite == NULL) {
        errno = ENOSYS;
        return FS_FAILURE;
    }
    return g_fs->fsFops->Pwrite(fd, buf, nbyte, offset);
}
