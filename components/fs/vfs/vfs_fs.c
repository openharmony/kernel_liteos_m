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

#define _GNU_SOURCE 1
#include "los_fs.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/uio.h>
#include "errno.h"
#include "fcntl.h"
#include "los_mux.h"
#include "los_debug.h"
#include "los_sched.h"
#include "limits.h"
#include "securec.h"
#include "vfs_config.h"
#include "vfs_files.h"
#include "vfs_maps.h"
#include "vfs_mount.h"
#include "vfs_operations.h"

#if (LOSCFG_LIBC_NEWLIB == 1)
#define ALIAS_OF(of) __attribute__((alias(#of)))
#define FUNC_ALIAS(real_func, new_alias, args_list, return_type) \
    return_type new_alias args_list ALIAS_OF(real_func)
#else
#define FUNC_ALIAS(real_func, new_alias, args_list, return_type)
#endif

#ifdef LOSCFG_NET_LWIP_SACK
#include "lwipopts.h"
#include "lwip/sockets.h"
#define CONFIG_NSOCKET_DESCRIPTORS  LWIP_CONFIG_NUM_SOCKETS
#else
#define CONFIG_NSOCKET_DESCRIPTORS  0
#endif

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
    LOSCFG_FS_FREE_HOOK(ptr);       \
    ptr = NULL;                     \
} while (0)

#define LOS_FCNTL   (O_NONBLOCK | O_NDELAY | O_APPEND | O_SYNC)
#define IOV_MAX_CNT 4

UINT32 g_fsMutex;
static UINT32 g_dirNum = 0;

int LOS_FsLock(void)
{
    if (!OsCheckKernelRunning()) {
        return LOS_OK;
    }
    if (LOS_MuxPend(g_fsMutex, (UINT32)LOSCFG_FS_LOCK_TIMEOUT) != LOS_OK) {
        PRINT_ERR("LOS_FsLock failed!");
        return (int)LOS_NOK;
    }

    return LOS_OK;
}

void LOS_FsUnlock(void)
{
    if (!OsCheckKernelRunning()) {
        return;
    }
    (void)LOS_MuxPost(g_fsMutex);
}

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

    if ((!cwd) || (path[0] == '/')) {
        cwd = "";
    }

    offset = strlen("///") + 1; // three '/' and one '\0'
    size_t tmpLen = strlen(cwd) + strlen(path) + offset;
    char *tmpBuf = (char *)LOSCFG_FS_MALLOC_HOOK(tmpLen);
    if (tmpBuf == NULL) {
        return LOS_OK;
    }

    if (-1 == sprintf_s(tmpBuf, tmpLen, "/%s/%s/", cwd, path)) {
        LOSCFG_FS_FREE_HOOK(tmpBuf);
        return LOS_OK;
    }

    char *p;
    /* replace /./ to / */
    offset = strlen("/./") - 1;
    while ((p = strstr(tmpBuf, "/./")) != NULL) {
        if (EOK != memmove_s(p, tmpLen - (p - tmpBuf), p + offset, tmpLen - (p - tmpBuf) - offset)) {
            LOSCFG_FS_FREE_HOOK(tmpBuf);
            return LOS_OK;
        }
    }

    /* replace // to / */
    while ((p = strstr(tmpBuf, "//")) != NULL) {
        if (EOK != memmove_s(p, tmpLen - (p - tmpBuf), p + 1, tmpLen - (p - tmpBuf) - 1)) {
            LOSCFG_FS_FREE_HOOK(tmpBuf);
            return LOS_OK;
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
            LOSCFG_FS_FREE_HOOK(tmpBuf);
            return LOS_OK;
        }
    }

    size_t totalLen = strlen(tmpBuf);
    /* strip the last / */
    if (totalLen > 1 && tmpBuf[totalLen - 1] == '/') {
        tmpBuf[--totalLen] = 0;
    }

    if ((!buf) || (bufSize == 0)) {
        LOSCFG_FS_FREE_HOOK(tmpBuf);
        return totalLen;
    }

    if (EOK != memcpy_s(buf, bufSize, tmpBuf, (((totalLen + 1) > bufSize) ? bufSize : (totalLen + 1)))) {
        LOSCFG_FS_FREE_HOOK(tmpBuf);
        return LOS_OK;
    }

    buf[bufSize - 1] = 0;
    LOSCFG_FS_FREE_HOOK(tmpBuf);
    return totalLen;
}
#endif

static inline int VfsPathCheck(const char *path, bool isFile)
{
    size_t len;
    if ((path == NULL) || (path[0] == '\0')) {
        VFS_ERRNO_SET(EINVAL);
        return (int)LOS_NOK;
    }

    len = strlen(path);
    if (len >= PATH_MAX) {
        VFS_ERRNO_SET(ENAMETOOLONG);
        return (int)LOS_NOK;
    }

    if (isFile && path[len - 1] == '/') {
        VFS_ERRNO_SET(EINVAL);
        return (int)LOS_NOK;
    }

    return LOS_OK;
}

static int VfsOpen(const char *path, int flags)
{
    size_t len;
    struct File *file = NULL;
    int fd = -1;
    const char *pathInMp = NULL;
    struct MountPoint *mp = NULL;

    if (VfsPathCheck(path, TRUE) != LOS_OK) {
        return fd;
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return fd;
    }

    mp = VfsMpFind(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0') ||
        (mp->mFs->fsFops == NULL) || (mp->mFs->fsFops->open == NULL)) {
        /* path is not in any mountpoint */
        VFS_ERRNO_SET(ENOENT);
        LOS_FsUnlock();
        return fd;
    }

    if ((mp->mWriteEnable == FALSE) &&
        (flags & (O_CREAT | O_WRONLY | O_RDWR))) {
        /* can't create file in read only mp */
        VFS_ERRNO_SET(EACCES);
        LOS_FsUnlock();
        return fd;
    }

    file = VfsFileGet();
    if (file == NULL) {
        VFS_ERRNO_SET(ENFILE);
        LOS_FsUnlock();
        return fd;
    }

    len = strlen(path) + 1;
    file->fullPath = LOSCFG_FS_MALLOC_HOOK(len);
    if (file->fullPath == NULL) {
        VFS_ERRNO_SET(ENOMEM);
        VfsFilePut(file);
        LOS_FsUnlock();
        return (int)LOS_NOK;
    }
    (void)strcpy_s((char *)file->fullPath, len, path);

    file->fFlags = (UINT32)flags;
    file->fOffset = 0;
    file->fData = NULL;
    file->fFops = mp->mFs->fsFops;
    file->fMp = mp;
    file->fOwner = LOS_CurTaskIDGet();

    if (file->fFops->open(file, pathInMp, flags) == 0) {
        mp->mRefs++;
        fd = FileToFd(file);
        file->fStatus = FILE_STATUS_READY; /* file now ready to use */
    } else {
        LOSCFG_FS_FREE_HOOK((void *)file->fullPath);
        VfsFilePut(file);
    }

    LOS_FsUnlock();
    return fd;
}

/* attach to a file and then set new status */

static struct File *VfsAttachFile(int fd, UINT32 status)
{
    struct File *file = NULL;

    if ((fd < MIN_START_FD) || (fd >= CONFIG_NFILE_DESCRIPTORS)) {
        VFS_ERRNO_SET(EBADF);
        return NULL;
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EFAULT);
        return NULL;
    }

    file = FdToFile(fd);
    if ((file == NULL) || (file->fMp == NULL)) {
        VFS_ERRNO_SET(EBADF);
        LOS_FsUnlock();
        return NULL;
    }

    if (file->fStatus != FILE_STATUS_READY) {
        VFS_ERRNO_SET(EBADF);
        LOS_FsUnlock();
        return NULL;
    }

    file->fStatus = status;
    return file;
}

static struct File *VfsAttachFileReady(int fd)
{
    return VfsAttachFile(fd, FILE_STATUS_READY);
}

static struct File *VfsAttachFileWithStatus(int fd, int status)
{
    return VfsAttachFile(fd, (UINT32)status);
}

static void VfsDetachFile(const struct File *file)
{
    (void)file;
    LOS_FsUnlock();
}

static int VfsClose(int fd)
{
    struct File *file = NULL;
    int ret = (int)LOS_NOK;

    file = VfsAttachFileWithStatus(fd, FILE_STATUS_CLOSING);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFops != NULL) && (file->fFops->close != NULL)) {
        ret = file->fFops->close(file);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    if ((ret == 0) && (file->fMp != NULL)) {
        file->fMp->mRefs--;
    }

    if (file->fullPath != NULL) {
        LOSCFG_FS_FREE_HOOK((void *)file->fullPath);
    }

    VfsFilePut(file);

    VfsDetachFile(file);

    return ret;
}

static ssize_t VfsRead(int fd, char *buff, size_t bytes)
{
    struct File *file = NULL;
    ssize_t ret = (ssize_t)-1;

    if (buff == NULL) {
        VFS_ERRNO_SET(EFAULT);
        return ret;
    }

    if (bytes == 0) {
        return 0;
    }

    file = VfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFlags & O_ACCMODE) == O_WRONLY) {
        VFS_ERRNO_SET(EACCES);
    } else if ((file->fFops != NULL) && (file->fFops->read != NULL)) {
        ret = file->fFops->read(file, buff, bytes);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    /* else ret will be -1 */
    VfsDetachFile(file);

    return ret;
}

static ssize_t VfsWrite(int fd, const void *buff, size_t bytes)
{
    struct File *file = NULL;
    ssize_t ret = (ssize_t)LOS_NOK;

    if ((buff == NULL) || (bytes == 0)) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    file = VfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFlags & O_ACCMODE) == O_RDONLY) {
        VFS_ERRNO_SET(EACCES);
    } else if ((file->fFops != NULL) && (file->fFops->write != NULL)) {
        ret = file->fFops->write(file, buff, bytes);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    /* else ret will be -1 */
    VfsDetachFile(file);

    return ret;
}

static int VfsIoctl(int fd, int func, va_list ap)
{
    unsigned long arg;
    struct File *file = NULL;
    int ret = (int)LOS_NOK;

    arg = va_arg(ap, unsigned long);
    file = VfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFops != NULL) && (file->fFops->ioctl != NULL)) {
        ret = file->fFops->ioctl(file, func, arg);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    VfsDetachFile(file);

    return ret;
}

static int VfsVfcntl(struct File *filep, int cmd, va_list ap)
{
    int ret;
    UINT32 flags;

    if ((filep == NULL) || (filep->fFops == NULL)) {
        return -EBADF;
    }

    if (cmd == F_GETFL) {
        ret = (int)(filep->fFlags);
    } else if (cmd == F_SETFL) {
        flags = (UINT32)va_arg(ap, int);
        flags &= LOS_FCNTL;
        filep->fFlags &= ~LOS_FCNTL;
        filep->fFlags |= flags;
        ret = LOS_OK;
    } else {
        ret = -ENOSYS;
    }
    return ret;
}

static int MapToPosixRet(int ret)
{
    return ((ret) < 0 ? -1 : (ret));
}

/* POSIX interface */
int open(const char *path, int flags, ...)
{
    if (path == NULL) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }
#ifdef LOSCFG_RANDOM_DEV
    unsigned flagMask = O_RDONLY | O_WRONLY | O_RDWR | O_APPEND | O_CREAT | O_LARGEFILE \
                        | O_TRUNC | O_EXCL | O_DIRECTORY;
    if ((unsigned)flags & ~flagMask) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }

    size_t pathLen = strlen(path) + 1;
    if ((unsigned)pathLen > PATH_MAX) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }

    char *canonicalPath = (char *)LOSCFG_FS_MALLOC_HOOK(pathLen);
    if (!canonicalPath) {
        errno = ENOMEM;
        return (int)LOS_NOK;
    }
    if (GetCanonicalPath(NULL, path, canonicalPath, pathLen) == 0) {
        FREE_AND_SET_NULL(canonicalPath);
        errno = ENOMEM;
        return (int)LOS_NOK;
    }

    if (strcmp(canonicalPath, RANDOM_DEV_PATH) == 0) {
        FREE_AND_SET_NULL(canonicalPath);
        if ((O_ACCMODE & (unsigned)flags) != O_RDONLY) {
            errno = EPERM;
            return (int)LOS_NOK;
        }
        if ((unsigned)flags & O_DIRECTORY) {
            errno = ENOTDIR;
            return (int)LOS_NOK;
        }
        return RANDOM_DEV_FD;
    }
    if ((strcmp(canonicalPath, "/") == 0) ||
        (strcmp(canonicalPath, "/dev") == 0)) {
        FREE_AND_SET_NULL(canonicalPath);
        if ((unsigned)flags & O_DIRECTORY) {
            errno = EPERM;
            return (int)LOS_NOK;
        }
        errno = EISDIR;
        return (int)LOS_NOK;
    }
    FREE_AND_SET_NULL(canonicalPath);
#endif
#if (LOSCFG_POSIX_PIPE_API == 1)
    if (!strncmp(path, PIPE_DEV_PATH, strlen(PIPE_DEV_PATH))) {
        return PipeOpen(path, flags, PIPE_DEV_FD);
    }
#endif

    int ret = VfsOpen(path, flags);
    return MapToPosixRet(ret);  
}
FUNC_ALIAS(open, _open, (const char *path, int flags, ...), int);

int close(int fd)
{
#ifdef LOSCFG_RANDOM_DEV
    if (fd == RANDOM_DEV_FD) {
        return LOS_OK;
    }
#endif
#ifdef LOSCFG_NET_LWIP_SACK
    if (fd >= CONFIG_NFILE_DESCRIPTORS && fd < (CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS)) {
        return closesocket(fd);
    }
#endif /* LOSCFG_NET_LWIP_SACK */
#if (LOSCFG_POSIX_PIPE_API == 1)
    if (fd >= PIPE_DEV_FD) {
        return PipeClose(fd);
    }
#endif

    int ret = (int)LOS_NOK;
    if (fd >= MIN_START_FD && fd < CONFIG_NFILE_DESCRIPTORS) {
        ret = VfsClose(fd);
    }
    return MapToPosixRet(ret);
}
FUNC_ALIAS(close, _close, (int fd), int);

ssize_t read(int fd, void *buff, size_t bytes)
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
        return recv(fd, buff, bytes, 0);
    }
#endif /* LOSCFG_NET_LWIP_SACK */

#if (LOSCFG_POSIX_PIPE_API == 1)
    if (fd >= PIPE_DEV_FD) {
        return PipeRead(fd, buff, bytes);
    }
#endif

    ssize_t ret = (ssize_t)LOS_NOK;
    if (fd >= MIN_START_FD && fd < CONFIG_NFILE_DESCRIPTORS) {
        ret = VfsRead(fd, buff, bytes);
    }

   return MapToPosixRet(ret);
}
FUNC_ALIAS(read, _read, (int fd, void *buff, size_t bytes), ssize_t);

ssize_t write(int fd, const void *buff, size_t bytes)
{
#ifdef LOSCFG_RANDOM_DEV
    if (fd == RANDOM_DEV_FD) {
        errno = EBADF; /* "/dev/random" is readonly */
        return (ssize_t)LOS_NOK;
    }
#endif
#ifdef LOSCFG_NET_LWIP_SACK
    if (fd >= CONFIG_NFILE_DESCRIPTORS &&
        fd < (CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS)) {
        return send(fd, buff, bytes, 0);
    }
#endif /* LOSCFG_NET_LWIP_SACK */

#if (LOSCFG_POSIX_PIPE_API == 1)
    if (fd >= PIPE_DEV_FD) {
        return PipeWrite(fd, buff, bytes);
    }
#endif

    ssize_t ret = (ssize_t)LOS_NOK;
    if (fd >= MIN_START_FD && fd < CONFIG_NFILE_DESCRIPTORS) {
        ret = VfsWrite(fd, buff, bytes);
    }

    return MapToPosixRet(ret);
}
FUNC_ALIAS(write, _write, (int fd, const void *buff, size_t bytes), ssize_t);

off_t lseek(int fd, off_t off, int whence)
{
    struct File *file;
    off_t ret = (off_t)LOS_NOK;

    file = VfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFops == NULL) || (file->fFops->lseek == NULL)) {
        ret = file->fOffset;
    } else {
        ret = file->fFops->lseek(file, off, whence);
    }

    VfsDetachFile(file);
    return ret;
}
FUNC_ALIAS(lseek, _lseek, (int fd, off_t off, int whence), off_t);

int stat(const char *path, struct stat *stat)
{
    struct MountPoint *mp = NULL;
    const char *pathInMp = NULL;
    int ret = (int)LOS_NOK;

    if (VfsPathCheck(path, FALSE) != LOS_OK) {
        return MapToPosixRet(ret);
    }

    if (stat == NULL) {
        VFS_ERRNO_SET(EINVAL);
        return MapToPosixRet(ret);
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet(ret);
    }

    mp = VfsMpFind(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0')) {
        VFS_ERRNO_SET(ENOENT);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    if (mp->mFs->fsFops->stat != NULL) {
        ret = mp->mFs->fsFops->stat(mp, pathInMp, stat);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    LOS_FsUnlock();
    return MapToPosixRet(ret);
}
FUNC_ALIAS(stat, _stat, (const char *path, struct stat *stat), int);

int statfs(const char *path, struct statfs *buf)
{
    struct MountPoint *mp = NULL;
    const char *pathInMp = NULL;
    int ret = (int)LOS_NOK;

    if (VfsPathCheck(path, FALSE) != LOS_OK) {
        return MapToPosixRet(ret);
    }

    if (buf == NULL) {
        VFS_ERRNO_SET(EINVAL);
        return MapToPosixRet(ret);
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet(ret);
    }

    mp = VfsMpFind(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0')) {
        VFS_ERRNO_SET(ENOENT);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    if (mp->mFs->fsFops->stat != NULL) {
        ret = mp->mFs->fsMops->statfs(pathInMp, buf);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    LOS_FsUnlock();
    return MapToPosixRet(ret);
}

int unlink(const char *path)
{
    struct MountPoint *mp = NULL;
    const char *pathInMp = NULL;
    int ret = (int)LOS_NOK;

    if (VfsPathCheck(path, FALSE) != LOS_OK) {
        return MapToPosixRet(ret);
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet(ret);
    }

    mp = VfsMpFind(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0') ||
        (mp->mFs->fsFops->unlink == NULL)) {
        VFS_ERRNO_SET(ENOENT);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    ret = mp->mFs->fsFops->unlink(mp, pathInMp);

    LOS_FsUnlock();
    return MapToPosixRet(ret);
}
FUNC_ALIAS(unlink, _unlink, (const char *path), int);

int rename(const char *oldpath, const char *newpath)
{
    struct MountPoint *mpOld = NULL;
    struct MountPoint *mpNew = NULL;
    const char *pathInMpOld = NULL;
    const char *pathInMpNew = NULL;
    int ret = (int)LOS_NOK;

    if (VfsPathCheck(oldpath, FALSE) != LOS_OK) {
        return MapToPosixRet(ret);
    }
    if (VfsPathCheck(newpath, FALSE) != LOS_OK) {
        return MapToPosixRet(ret);
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet(ret);
    }

    mpOld = VfsMpFind(oldpath, &pathInMpOld);

    if (pathInMpOld == NULL) {
        VFS_ERRNO_SET(EINVAL);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    if ((mpOld == NULL) || (*pathInMpOld == '\0') ||
        (mpOld->mFs->fsFops->unlink == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    mpNew = VfsMpFind(newpath, &pathInMpNew);
    if ((mpNew == NULL) || (pathInMpNew == NULL) || (*pathInMpNew == '\0') || (mpNew->mFs->fsFops->unlink == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    if (mpOld != mpNew) {
        VFS_ERRNO_SET(EXDEV);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    if (mpOld->mFs->fsFops->rename != NULL) {
        ret = mpOld->mFs->fsFops->rename(mpOld, pathInMpOld, pathInMpNew);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    LOS_FsUnlock();
    return MapToPosixRet(ret);
}

int fsync(int fd)
{
    struct File *file;
    int ret = (int)LOS_NOK;

    file = VfsAttachFileReady(fd);
    if (file == NULL) {
        return MapToPosixRet(ret);
    }

    if (file->fMp->mWriteEnable == FALSE) {
        VFS_ERRNO_SET(EACCES);
        VfsDetachFile(file);
        return MapToPosixRet(ret);
    }

    if ((file->fFops != NULL) && (file->fFops->sync != NULL)) {
        ret = file->fFops->sync(file);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    VfsDetachFile(file);
    return MapToPosixRet(ret);
}

DIR *opendir(const char *path)
{
    struct MountPoint *mp = NULL;
    const char *pathInMp = NULL;
    struct Dir *dir = NULL;
    UINT32 ret;

    if (VfsPathCheck(path, FALSE) != LOS_OK) {
        return NULL;
    }

    dir = (struct Dir *)LOSCFG_FS_MALLOC_HOOK(sizeof(struct Dir));
    if (dir == NULL) {
        VFS_ERRNO_SET(ENOMEM);
        return NULL;
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        LOSCFG_FS_FREE_HOOK(dir);
        return NULL;
    }

    if (g_dirNum >= LOSCFG_MAX_OPEN_DIRS) {
        VFS_ERRNO_SET(ENFILE);
        LOS_FsUnlock();
        LOSCFG_FS_FREE_HOOK(dir);
        return NULL;
    }

    mp = VfsMpFind(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL)) {
        VFS_ERRNO_SET(ENOENT);
        LOS_FsUnlock();
        LOSCFG_FS_FREE_HOOK(dir);
        return NULL;
    }

    if (mp->mFs->fsFops->opendir == NULL) {
        VFS_ERRNO_SET(ENOTSUP);
        LOS_FsUnlock();
        LOSCFG_FS_FREE_HOOK(dir);
        return NULL;
    }

    dir->dMp = mp;
    dir->dOffset = 0;

    ret = (UINT32)mp->mFs->fsFops->opendir(dir, pathInMp);
    if (ret == 0) {
        mp->mRefs++;
        g_dirNum++;
    } else {
        LOSCFG_FS_FREE_HOOK(dir);
        dir = NULL;
    }

    LOS_FsUnlock();
    return (DIR *)dir;
}

struct dirent *readdir(DIR *dir)
{
    struct dirent *ret = NULL;
    struct Dir *d = (struct Dir *)dir;

    if ((dir == NULL) || (d->dMp == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        return NULL;
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return NULL;
    }

    if ((d->dMp->mFs != NULL) && (d->dMp->mFs->fsFops != NULL) &&
        (d->dMp->mFs->fsFops->readdir != NULL)) {
        if (d->dMp->mFs->fsFops->readdir(d, &d->dDent) == 0) {
            ret = &d->dDent;
        }
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    LOS_FsUnlock();

    return ret;
}

int closedir(DIR *dir)
{
    struct MountPoint *mp = NULL;
    int ret = (int)LOS_NOK;
    struct Dir *d = (struct Dir *)dir;

    if ((d == NULL) || (d->dMp == NULL)) {
        VFS_ERRNO_SET(EBADF);
        return MapToPosixRet(ret);
    }

    mp = d->dMp;

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet(ret);
    }

    if ((d->dMp->mFs != NULL) && (d->dMp->mFs->fsFops != NULL) &&
        (d->dMp->mFs->fsFops->closedir != NULL)) {
        ret = d->dMp->mFs->fsFops->closedir(d);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    if (ret == 0) {
        mp->mRefs--;
        g_dirNum--;
    } else {
        VFS_ERRNO_SET(EBADF);
    }

    LOS_FsUnlock();
    LOSCFG_FS_FREE_HOOK(d);
    d = NULL;
    return MapToPosixRet(ret);
}

int mkdir(const char *path, mode_t mode)
{
    struct MountPoint *mp = NULL;
    const char *pathInMp = NULL;
    int ret = (int)LOS_NOK;
    (void)mode;

    if (VfsPathCheck(path, FALSE) != LOS_OK) {
        return MapToPosixRet(ret);
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet(ret);
    }

    mp = VfsMpFind(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0')) {
        VFS_ERRNO_SET(ENOENT);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    if (mp->mFs->fsFops->mkdir != NULL) {
        ret = mp->mFs->fsFops->mkdir(mp, pathInMp);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    LOS_FsUnlock();
    return MapToPosixRet(ret);
}

int rmdir(const char *path)
{
    struct MountPoint *mp = NULL;
    const char *pathInMp = NULL;
    int ret = (int)LOS_NOK;

    if (path == NULL) {
        VFS_ERRNO_SET(EINVAL);
        return MapToPosixRet(ret);
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet(ret);
    }

    mp = VfsMpFind(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0') ||
        (mp->mFs->fsFops->rmdir == NULL)) {
        VFS_ERRNO_SET(ENOENT);
        LOS_FsUnlock();
        return MapToPosixRet(ret);
    }

    ret = mp->mFs->fsFops->rmdir(mp, pathInMp);

    LOS_FsUnlock();
    return MapToPosixRet(ret);
}

int lstat(const char *path, struct stat *buffer)
{
    return stat(path, buffer);
}

int fstat(int fd, struct stat *buf)
{
    struct File *filep;
    int ret;
    filep = VfsAttachFileReady(fd);
    if ((filep == NULL) || (filep->fMp == NULL) || filep->fullPath == NULL) {
        return (int)LOS_NOK;
    }
    ret = stat(filep->fullPath, buf);
    VfsDetachFile(filep);
    return ret;
}
FUNC_ALIAS(fstat, _fstat, (int fd, struct stat *buf), int);

int fcntl(int fd, int cmd, ...)
{
    struct File *filep = NULL;
    int ret;
    va_list ap;

    va_start(ap, cmd);
    if (fd < CONFIG_NFILE_DESCRIPTORS) {
        filep = VfsAttachFileReady(fd);
        ret = VfsVfcntl(filep, cmd, ap);   
        VfsDetachFile(filep);
    } else {
#ifndef LOSCFG_NET_LWIP_SACK
        ret = -EBADF;
#else
        int arg = va_arg(ap, int);
        ret = lwip_fcntl(fd, (long)cmd, arg);
        va_end(ap);
        return ret;
#endif /* LOSCFG_NET_LWIP_SACK */
    }

    if (ret < 0) {
        VFS_ERRNO_SET(-ret);
        ret = (int)LOS_NOK;
    }
    va_end(ap);
    return ret;
}

int ioctl(int fd, int req, ...)
{
    int ret;
    va_list ap;

    va_start(ap, req);
    if (fd < CONFIG_NFILE_DESCRIPTORS) {
        ret = VfsIoctl(fd, req, ap);
    } else {
#ifndef LOSCFG_NET_LWIP_SACK
        ret = -EBADF;
#else
        UINTPTR arg = va_arg(ap, UINTPTR);
        ret = lwip_ioctl(fd, (long)req, (void *)arg);
#endif /* LOSCFG_NET_LWIP_SACK */
    }

    va_end(ap);
    return ret;
}

ssize_t readv(int fd, const struct iovec *iovBuf, int iovcnt)
{
    int i;
    errno_t ret;
    char *buf = NULL;
    char *curBuf = NULL;
    char *readBuf = NULL;
    size_t bufLen = 0;
    size_t bytesToRead;
    ssize_t totalBytesRead;
    size_t totalLen;
    const struct iovec *iov = (const struct iovec *)iovBuf;

    if ((iov == NULL) || (iovcnt <= 0) || (iovcnt > IOV_MAX_CNT)) {
        return (ssize_t)LOS_NOK;
    }

    for (i = 0; i < iovcnt; ++i) {
        if ((SSIZE_MAX - bufLen) < iov[i].iov_len) {
            return (ssize_t)LOS_NOK;
        }
        bufLen += iov[i].iov_len;
    }
    if (bufLen == 0) {
        return (ssize_t)LOS_NOK;
    }
    totalLen = bufLen * sizeof(char);
    buf = (char *)LOSCFG_FS_MALLOC_HOOK(totalLen);
    if (buf == NULL) {
        return (ssize_t)LOS_NOK;
    }

    totalBytesRead = read(fd, buf, bufLen);
    if ((size_t)totalBytesRead < totalLen) {
        totalLen = (size_t)totalBytesRead;
    }
    curBuf = buf;
    for (i = 0; i < iovcnt; ++i) {
        readBuf = (char *)iov[i].iov_base;
        bytesToRead = iov[i].iov_len;

        size_t lenToRead = totalLen < bytesToRead ? totalLen : bytesToRead;
        ret = memcpy_s(readBuf, bytesToRead, curBuf, lenToRead);
        if (ret != EOK) {
            LOSCFG_FS_FREE_HOOK(buf);
            return (ssize_t)LOS_NOK;
        }
        if (totalLen < (size_t)bytesToRead) {
            break;
        }
        curBuf += bytesToRead;
        totalLen -= bytesToRead;
    }
    LOSCFG_FS_FREE_HOOK(buf);
    return totalBytesRead;
}

ssize_t writev(int fd, const struct iovec *iovBuf, int iovcnt)
{
    int i;
    errno_t ret;
    char *buf = NULL;
    char *curBuf = NULL;
    char *writeBuf = NULL;
    size_t bufLen = 0;
    size_t bytesToWrite;
    ssize_t totalBytesWritten;
    size_t totalLen;
    const struct iovec *iov = iovBuf;

    if ((iov == NULL) || (iovcnt <= 0) || (iovcnt > IOV_MAX_CNT)) {
        return (ssize_t)LOS_NOK;
    }

    for (i = 0; i < iovcnt; ++i) {
        if ((SSIZE_MAX - bufLen) < iov[i].iov_len) {
            VFS_ERRNO_SET(EINVAL);
            return (ssize_t)LOS_NOK;
        }
        bufLen += iov[i].iov_len;
    }
    if (bufLen == 0) {
        return (ssize_t)LOS_NOK;
    }
    totalLen = bufLen * sizeof(char);
    buf = (char *)LOSCFG_FS_MALLOC_HOOK(totalLen);
    if (buf == NULL) {
        return (ssize_t)LOS_NOK;
    }
    curBuf = buf;
    for (i = 0; i < iovcnt; ++i) {
        writeBuf = (char *)iov[i].iov_base;
        bytesToWrite = iov[i].iov_len;
        if (((ssize_t)totalLen <= 0) || ((ssize_t)bytesToWrite <= 0)) {
            continue;
        }
        ret = memcpy_s(curBuf, totalLen, writeBuf, bytesToWrite);
        if (ret != EOK) {
            LOSCFG_FS_FREE_HOOK(buf);
            return (ssize_t)LOS_NOK;
        }
        curBuf += bytesToWrite;
        totalLen -= bytesToWrite;
    }

    totalBytesWritten = write(fd, buf, bufLen);
    LOSCFG_FS_FREE_HOOK(buf);

    return totalBytesWritten;
}

int remove(const char *filename)
{
    int ret = unlink(filename);
    if (ret == -EISDIR) {
        ret = rmdir(filename);
    }

    return ret;
}

int access(const char *path, int amode)
{
    int result;
    mode_t mode;
    struct stat buf;

    result = stat(path, &buf);
    if (result != 0) {
        return (int)LOS_NOK;
    }

    mode = buf.st_mode;
    if ((unsigned int)amode & R_OK) {
        if ((mode & (S_IROTH | S_IRGRP | S_IRUSR)) == 0) {
            VFS_ERRNO_SET(EACCES);
            return (int)LOS_NOK;
        }
    }
    if ((unsigned int)amode & W_OK) {
        if ((mode & (S_IWOTH | S_IWGRP | S_IWUSR)) == 0) {
            VFS_ERRNO_SET(EACCES);
            return (int)LOS_NOK;
        }
    }
    if ((unsigned int)amode & X_OK) {
        if ((mode & (S_IXOTH | S_IXGRP | S_IXUSR)) == 0) {
            VFS_ERRNO_SET(EACCES);
            return (int)LOS_NOK;
        }
    }
    return 0;
}

int ftruncate(int fd, off_t length)
{
    int ret = (int)LOS_NOK;
    struct File *file = NULL;

    if (length <= 0) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    file = VfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if (file->fMp->mWriteEnable == FALSE) {
        VFS_ERRNO_SET(EACCES);
        VfsDetachFile(file);
        return ret;
    }

    if ((file->fFlags & O_ACCMODE) == O_RDONLY) {
        VFS_ERRNO_SET(EACCES);
    } else if ((file->fFops != NULL) && (file->fFops->truncate != NULL)) {
        ret = file->fFops->truncate(file, length);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    /* else ret will be -1 */
    VfsDetachFile(file);
    return ret;
}

ssize_t pread(int fd, void *buff, size_t bytes, off_t off)
{
    ssize_t ret = (ssize_t)LOS_NOK;
    off_t savepos, pos;

    if (fd < 0 || fd >= CONFIG_NFILE_DESCRIPTORS) {
        return MapToPosixRet((int)ret);
    }

    if (buff == NULL) {
        VFS_ERRNO_SET(EFAULT);
        return MapToPosixRet((int)ret);
    }

    if (bytes == 0) {
        return 0;
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet((int)ret);
    }

    savepos = lseek(fd, 0, SEEK_CUR);
    if (savepos == (off_t)-1) {
        LOS_FsUnlock();
        return MapToPosixRet((int)ret);
    }

    pos = lseek(fd, off, SEEK_SET);
    if (pos == (off_t)-1) {
        LOS_FsUnlock();
        return MapToPosixRet((int)ret);
    }

    ret = read(fd, buff, bytes);
    pos = lseek(fd, savepos, SEEK_SET);
    if ((pos == (off_t)-1) && (ret >= 0)) {
        LOS_FsUnlock();
        return MapToPosixRet((int)LOS_NOK);
    }

    LOS_FsUnlock();

    return MapToPosixRet((int)ret);
}

ssize_t pwrite(int fd, const void *buff, size_t bytes, off_t off)
{
    ssize_t ret = (ssize_t)LOS_NOK;
    off_t savepos, pos;

    if ((fd < 0) || (fd >= CONFIG_NFILE_DESCRIPTORS)) {
        return MapToPosixRet((int)ret);
    }

    if (buff == NULL) {
        VFS_ERRNO_SET(EFAULT);
        return MapToPosixRet((int)ret);
    }

    if (bytes == 0) {
        return 0;
    }

    if (LOS_FsLock() != LOS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return MapToPosixRet((int)ret);
    }

    savepos = lseek(fd, 0, SEEK_CUR);
    if (savepos == (off_t)-1) {
        LOS_FsUnlock();
        return MapToPosixRet((int)ret);
    }

    pos = lseek(fd, off, SEEK_SET);
    if (pos == (off_t)-1) {
        LOS_FsUnlock();
        return MapToPosixRet((int)ret);
    }

    ret = write(fd, buff, bytes);
    pos = lseek(fd, savepos, SEEK_SET);
    if ((pos == (off_t)-1) && (ret >= 0)) {
        LOS_FsUnlock();
        return MapToPosixRet((int)LOS_NOK);
    }

    LOS_FsUnlock();

    return MapToPosixRet((int)ret);
}
