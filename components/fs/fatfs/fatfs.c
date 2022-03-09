/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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
#include "ff.h"
#include "fatfs.h"
#include "errno.h"
#include "stdbool.h"
#include "limits.h"
#include "pthread.h"
#include "time.h"
#include "securec.h"
#include "los_compiler.h"
#include "los_debug.h"
#include "cmsis_os2.h"
#include "vfs_files.h"
#include "vfs_operations.h"
#include "vfs_partition.h"
#include "vfs_maps.h"
#include "vfs_mount.h"

/* the max name length of different parts should not bigger than 32 */
#define FS_DRIVE_NAME_MAX_LEN 32

#ifndef FAT_MAX_OPEN_DIRS
#define FAT_MAX_OPEN_DIRS     8
#endif /* FAT_MAX_OPEN_DIRS */

#ifndef FS_LOCK_TIMEOUT_SEC
#define FS_LOCK_TIMEOUT_SEC  15
#endif /* FS_LOCK_TIMEOUT_SEC */

static UINT8 g_workBuffer[FF_MAX_SS];
static pthread_mutex_t g_fatfsMutex = PTHREAD_MUTEX_INITIALIZER;
static char *g_volPath[FF_VOLUMES] = {FF_VOLUME_STRS};

PARTITION VolToPart[] = {
    { 0, 0, 1, 0, 0 },
    { 0, 0, 2, 0, 0 },
    { 0, 0, 3, 0, 0 },
    { 0, 0, 4, 0, 0 }
};

static int FsLock(void)
{
    INT32 ret = 0;
    struct timespec absTimeout = {0};
    if (osKernelGetState() != osKernelRunning) {
        return ret;
    }
    ret = clock_gettime(CLOCK_REALTIME, &absTimeout);
    if (ret != 0) {
        PRINT_ERR("clock gettime err 0x%x!\r\n", errno);
        return errno;
    }
    absTimeout.tv_sec += FS_LOCK_TIMEOUT_SEC;
    ret = pthread_mutex_timedlock(&g_fatfsMutex, &absTimeout);
    return ret;
}

static void FsUnlock(void)
{
    if (osKernelGetState() != osKernelRunning) {
        return;
    }
    (void)pthread_mutex_unlock(&g_fatfsMutex);
}

static int FsChangeDrive(const char *path)
{
    INT32 res;
    errno_t retErr;
    UINT16 pathLen = strlen((char const *)path);
    /* the max name length of different parts is 16 */
    CHAR tmpPath[FS_DRIVE_NAME_MAX_LEN] = { "/" };

    /* make sure the path begin with "/", the path like /xxx/yyy/... */
    if (pathLen >= (FS_DRIVE_NAME_MAX_LEN - 1)) {
        /* 2: except first flag "/" and last end flag */
        pathLen = FS_DRIVE_NAME_MAX_LEN - 2;
    }

    retErr = strncpy_s(tmpPath + 1, (FS_DRIVE_NAME_MAX_LEN - 1), (char const *)path, pathLen);
    if (retErr != EOK) {
        return LOS_NOK;
    }

    res = f_chdrive(tmpPath);
    if (res != FR_OK) {
        return LOS_NOK;
    }

    return LOS_OK;
}

static int Remount(struct MountPoint *mp, unsigned long mountflags)
{
    FATFS *fatfs = (FATFS *)mp->mData;

    /* remount is not allowed when the device is not mounted. */
    if (fatfs->fs_type == 0) {
        errno = EINVAL;
        return LOS_NOK;
    }
    mp->mWriteEnable = (mountflags & MS_RDONLY) ? FALSE : TRUE;

    return LOS_OK;
}

static unsigned int FatFsGetMode(int oflags)
{
    UINT32 fmode = FA_READ;

    if ((UINT32)oflags & O_WRONLY) {
        fmode |= FA_WRITE;
    }

    if (((UINT32)oflags & O_ACCMODE) & O_RDWR) {
        fmode |= FA_WRITE;
    }
    /* Creates a new file if the file is not existing, otherwise, just open it. */
    if ((UINT32)oflags & O_CREAT) {
        fmode |= FA_OPEN_ALWAYS;
        /* Creates a new file. If the file already exists, the function shall fail. */
        if ((UINT32)oflags & O_EXCL) {
            fmode |= FA_CREATE_NEW;
        }
    }
    /* Creates a new file. If the file already exists, its length shall be truncated to 0. */
    if ((UINT32)oflags & O_TRUNC) {
        fmode |= FA_CREATE_ALWAYS;
    }

    return fmode;
}

static int FatfsErrno(int result)
{
    INT32 status = 0;

    if (result < 0) {
        return result;
    }

    /* FatFs errno to Libc errno */
    switch (result) {
        case FR_OK:
            break;

        case FR_NO_FILE:
        case FR_NO_PATH:
        case FR_NO_FILESYSTEM:
            status = ENOENT;
            break;

        case FR_INVALID_NAME:
            status = EINVAL;
            break;

        case FR_EXIST:
        case FR_INVALID_OBJECT:
            status = EEXIST;
            break;

        case FR_DISK_ERR:
        case FR_NOT_READY:
        case FR_INT_ERR:
            status = EIO;
            break;

        case FR_WRITE_PROTECTED:
            status = EROFS;
            break;
        case FR_MKFS_ABORTED:
        case FR_INVALID_PARAMETER:
            status = EINVAL;
            break;

        case FR_NO_SPACE_LEFT:
            status = ENOSPC;
            break;
        case FR_NO_DIRENTRY:
            status = ENFILE;
            break;
        case FR_NO_EMPTY_DIR:
            status = ENOTEMPTY;
            break;
        case FR_IS_DIR:
            status = EISDIR;
            break;
        case FR_NO_DIR:
            status = ENOTDIR;
            break;
        case FR_NO_EPERM:
        case FR_DENIED:
            status = EPERM;
            break;
        case FR_LOCKED:
            status = EBUSY;
            break;
        default:
            status = result;
            break;
    }

    return status;
}

char * GetLdPath(const char *source)
{
    int ret;
    int partId = GetPartIdByPartName(source);
    if ((partId < 0) || (partId >= MAX_PARTITION_NUM)) {
        return NULL;
    }

    char *volPath = g_volPath[partId];
    char *ldPath = (char *)malloc(strlen(volPath) + 1);
    if (ldPath == NULL) {
        return NULL;
    }

    (void)memset_s(ldPath, strlen(volPath) + 1, 0, strlen(volPath) + 1);

    /* Convert volPath to ldpath, for example, convert "inner" to "/inner" */
    *ldPath = '/';
    ret = strcpy_s(ldPath + 1, strlen(volPath), volPath);
    if (ret != EOK) {
        free(ldPath);
        return NULL;
    }

    return ldPath;
}

void PutLdPath(const char *ldPath)
{
    if (ldPath != NULL) {
        free((void *)ldPath);
    }
}

int FatfsMount(struct MountPoint *mp, unsigned long mountflags,
               const void *data)
{
    FRESULT res;
    INT32 ret;
    FATFS *fs = NULL;

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    char *ldPath = GetLdPath(mp->mDev);
    if (ldPath == NULL) {
        errno = EFAULT;
        ret = LOS_NOK;
        goto ERROUT;
    }

    if (mountflags & MS_REMOUNT) {
        ret = Remount(mp, mountflags);
        goto ERROUT;
    }

    fs = (FATFS *)malloc(sizeof(FATFS));
    if (fs == NULL) {
        errno = ENOMEM;
        ret = LOS_NOK;
        goto ERROUT;
    }
    (void)memset_s(fs, sizeof(FATFS), 0, sizeof(FATFS));
    mp->mData = (void *)fs;

    res = f_mount((FATFS *)mp->mData, ldPath, 1);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto ERROUT;
    }

    PutLdPath(ldPath);
    FsUnlock();
    return LOS_OK;

ERROUT:
    free(fs);
    mp->mData = NULL;
    PutLdPath(ldPath);
    FsUnlock();
    return ret;
}

int FatfsUmount(struct MountPoint *mp)
{
    FRESULT res;
    INT32 ret;
    INT32 volId;
    FATFS *fatfs = (FATFS *)mp->mData;

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    char *ldPath = GetLdPath(mp->mDev);
    if (ldPath == NULL) {
        errno = EFAULT;
        ret = LOS_NOK;
        goto OUT;
    }

    /* The volume is not mounted */
    if (fatfs->fs_type == 0) {
        errno = EINVAL;
        ret = LOS_NOK;
        goto OUT;
    }

    volId = GetPartIdByPartName(mp->mDev);
    /* umount is not allowed when a file or diretory is opened. */
    if (f_checkopenlock(volId) != FR_OK) {
        errno = EBUSY;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_mount((FATFS *)NULL, ldPath, 0);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }

    if (fatfs->win != NULL) {
        ff_memfree(fatfs->win);
    }

    free(mp->mData);
    mp->mData = NULL;

    ret = LOS_OK;

OUT:
    PutLdPath(ldPath);
    FsUnlock();
    return ret;
}

int FatfsUmount2(struct MountPoint *mp, int flag)
{
    INT32 ret;
    UINT32 flags;
    FRESULT res;
    FATFS *fatfs = (FATFS *)mp->mData;

    flags = MNT_FORCE | MNT_DETACH | MNT_EXPIRE | UMOUNT_NOFOLLOW;
    if ((UINT32)flag & ~flags) {
        errno = EINVAL;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    char *ldPath = GetLdPath(mp->mDev);
    if (ldPath == NULL) {
        errno = EFAULT;
        ret = LOS_NOK;
        goto OUT;
    }

    /* The volume is not mounted */
    if (fatfs->fs_type == 0) {
        errno = EINVAL;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_mount((FATFS *)NULL, ldPath, 0);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }

    if (fatfs->win != NULL) {
        ff_memfree(fatfs->win);
    }

    free(mp->mData);
    mp->mData = NULL;

    ret = LOS_OK;

OUT:
    PutLdPath(ldPath);
    FsUnlock();
    return ret;
}

int FatfsOpen(struct File *file, const char *path, int oflag)
{
    FRESULT res;
    UINT32 fmode;
    INT32 ret;
    FIL *fp = NULL;

    if (path == NULL) {
        errno = EFAULT;
        return LOS_NOK;
    }

    fmode = FatFsGetMode(oflag);

    fp = (FIL *)malloc(sizeof(FIL));
    if (fp == NULL) {
        errno = ENOMEM;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        free(fp);
        return LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != LOS_OK) {
        PRINT_ERR("FAT open ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        ret = LOS_NOK;
        free(fp);
        goto OUT;
    }

    res = f_open(fp, path, fmode);
    if (res != FR_OK) {
        PRINT_ERR("FAT open err 0x%x!\r\n", res);
        free(fp);
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }

    file->fData = (void *)fp;

OUT:
    FsUnlock();
    return ret;
}

int FatfsClose(struct File *file)
{
    FRESULT res;
    INT32 ret;
    FIL *fp = (FIL *)file->fData;

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if ((fp == NULL) || (fp->obj.fs == NULL)) {
        FsUnlock();
        errno = ENOENT;
        return LOS_NOK;
    }

    res = f_close(fp);
    if (res != FR_OK) {
        PRINT_ERR("FAT close err 0x%x!\r\n", res);
        FsUnlock();
        errno = FatfsErrno(res);
        return LOS_NOK;
    }

#if !FF_FS_TINY
    if (fp->buf != NULL) {
        (void)ff_memfree(fp->buf);
    }
#endif
    free(file->fData);
    file->fData = NULL;
    FsUnlock();

    return LOS_OK;
}

ssize_t FatfsRead(struct File *file, char *buf, size_t nbyte)
{
    FRESULT res;
    INT32 ret;
    UINT32 lenRead;
    FIL *fp = (FIL *)file->fData;

    if (buf == NULL) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }
    if (fp == NULL) {
        FsUnlock();
        errno = ENOENT;
        return LOS_NOK;
    }

    res = f_read(fp, buf, nbyte, &lenRead);
    if (res != FR_OK) {
        FsUnlock();
        errno = FatfsErrno(res);
        return LOS_NOK;
    }
    FsUnlock();

    return (ssize_t)lenRead;
}

ssize_t FatfsWrite(struct File *file, const char *buf, size_t nbyte)
{
    FRESULT res;
    INT32 ret;
    UINT32 lenWrite;
    static BOOL overFlow = FALSE;
    FIL *fp = (FIL *)file->fData;

    if (buf == NULL) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if ((fp ==NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        goto ERROUT;
    }

    res = f_write(fp, buf, nbyte, &lenWrite);
    if ((res == FR_OK) && (lenWrite == 0) && (nbyte != 0) && (overFlow == FALSE)) {
        overFlow = TRUE;
        PRINT_ERR("FAT write!\r\n");
    }

    if ((res != FR_OK) || (nbyte != lenWrite)) {
        errno = FatfsErrno(res);
        goto ERROUT;
    }

    FsUnlock();
    return (ssize_t)lenWrite;

ERROUT:
    FsUnlock();
    return LOS_NOK;
}

off_t FatfsLseek(struct File *file, off_t offset, int whence)
{
    FRESULT res;
    INT32 ret;
    off_t pos;
    FIL *fp = (FIL *)file->fData;

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if ((fp == NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        goto ERROUT;
    }

    if (whence == SEEK_SET) {
        pos = 0;
    } else if (whence == SEEK_CUR) {
        pos = f_tell(fp);
    } else if (whence == SEEK_END) {
        pos = f_size(fp);
    } else {
        errno = EINVAL;
        goto ERROUT;
    }

    res = f_lseek(fp, offset + pos);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        goto ERROUT;
    }

    pos = f_tell(fp);
    FsUnlock();
    return pos;

ERROUT:
    FsUnlock();
    return LOS_NOK;
}

/* Remove the specified FILE */
int FatfsUnlink(struct MountPoint *mp, const char *path)
{
    FRESULT res;
    INT32 ret;

    if (path == NULL) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if (!mp->mWriteEnable) {
        errno = EACCES;
        ret = LOS_NOK;
        goto OUT;
    }

    ret = FsChangeDrive(path);
    if (ret != LOS_OK) {
        PRINT_ERR("FAT unlink ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_unlink(path);
    if (res != FR_OK) {
        PRINT_ERR("FAT unlink err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }

    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

int FatfsStat(struct MountPoint *mp, const char *path, struct stat *buf)
{
    FRESULT res;
    FILINFO fileInfo = {0};
    INT32 ret;

    if ((path == NULL) || (buf == NULL)) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != LOS_OK) {
        PRINT_ERR("FAT stat ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_stat(path, &fileInfo);
    if (res != FR_OK) {
        PRINT_ERR("FAT stat err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }

    buf->st_size = fileInfo.fsize;
    buf->st_mode = S_IFREG | S_IRUSR | S_IRGRP | S_IROTH |
                   S_IWUSR | S_IWGRP | S_IWOTH |
                   S_IXUSR | S_IXGRP | S_IXOTH;

    if (fileInfo.fattrib & AM_RDO) {
        buf->st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    }

    if (fileInfo.fattrib & AM_DIR) {
        buf->st_mode &= ~S_IFREG;
        buf->st_mode |= S_IFDIR;
    }

    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

/* Synchronize all changes to Flash */
int FatfsSync(struct File *file)
{
    FRESULT res;
    INT32 ret;
    FIL *fp = (FIL *)file->fData;

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if ((fp == NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_sync(fp);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }
    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

int FatfsMkdir(struct MountPoint *mp, const char *path)
{
    FRESULT res;
    INT32 ret;

    if (path == NULL) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if (!mp->mWriteEnable) {
        errno = EACCES;
        ret = LOS_NOK;
        goto OUT;
    }

    ret = FsChangeDrive(path);
    if (ret != LOS_OK) {
        PRINT_ERR("FAT mkdir ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_mkdir(path);
    if (res != FR_OK) {
        PRINT_ERR("FAT mkdir err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }
    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

int FatfsOpendir(struct Dir *dir, const char *dirName)
{
    FRESULT res;
    INT32 ret;
    DIR *dp = NULL;

    if (dirName == NULL) {
        errno = EFAULT;
        goto ERROUT;
    }

    dp = (DIR *)malloc(sizeof(DIR));
    if (dp == NULL) {
        errno = ENOENT;
        goto ERROUT;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        goto ERROUT;
    }

    ret = FsChangeDrive(dirName);
    if (ret != LOS_OK) {
        PRINT_ERR("FAT opendir ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        goto ERROUT;
    }

    res = f_opendir(dp, dirName);
    if (res != FR_OK) {
        PRINT_ERR("FAT opendir err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        goto ERROUT;
    }

    dir->dData = dp;
    dir->dOffset = 0;

    FsUnlock();
    return LOS_OK;

ERROUT:
    if (dp != NULL) {
        free(dp);
    }
    FsUnlock();
    return LOS_NOK;
}

int FatfsReaddir(struct Dir *dir, struct dirent *dent)
{
    FRESULT res;
    INT32 ret;
    FILINFO fileInfo = {0};
    DIR *dp = NULL;

    if ((dir == NULL) || (dir->dData == NULL)) {
        errno = EBADF;
        return LOS_NOK;
    }

    dp = (DIR *)dir->dData;
    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    res = f_readdir(dp, &fileInfo);
    /* if res not ok or fname is NULL , return NULL */
    if ((res != FR_OK) || (fileInfo.fname[0] == 0x0)) {
        PRINT_ERR("FAT readdir err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        FsUnlock();
        return LOS_NOK;
    }

    (void)memcpy_s(dent->d_name, sizeof(dent->d_name),
            fileInfo.fname, sizeof(dent->d_name));
    if (fileInfo.fattrib & AM_DIR) {
        dent->d_type = DT_DIR;
    } else {
        dent->d_type = DT_REG;
    }
    FsUnlock();

    return LOS_OK;
}

int FatfsClosedir(struct Dir *dir)
{
    FRESULT res;
    INT32 ret;
    DIR *dp = NULL;

    if ((dir == NULL) || (dir->dData == NULL)) {
        errno = EBADF;
        return LOS_NOK;
    }

    dp = dir->dData;
    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    res = f_closedir(dp);
    if (res != FR_OK) {
        PRINT_ERR("FAT closedir err 0x%x!\r\n", res);
        FsUnlock();
        errno = FatfsErrno(res);
        return LOS_NOK;
    }

    free(dir->dData);
    dir->dData = NULL;
    FsUnlock();

    return LOS_OK;
}

int FatfsRmdir(struct MountPoint *mp, const char *path)
{
    FRESULT res;
    INT32 ret;

    if ((path == NULL) || (mp == NULL)) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if (!mp->mWriteEnable) {
        errno = EACCES;
        ret = LOS_NOK;
        goto OUT;
    }

    ret = FsChangeDrive(path);
    if (ret != LOS_OK) {
        PRINT_ERR("FAT rmdir ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_rmdir(path);
    if (res != FR_OK) {
        PRINT_ERR("FAT rmdir err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }
    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

int FatfsRename(struct MountPoint *mp, const char *oldName, const char *newName)
{
    FRESULT res;
    INT32 ret;

    if ((oldName == NULL) || (newName == NULL)) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if (!mp->mWriteEnable) {
        errno = EACCES;
        ret = LOS_NOK;
        goto OUT;
    }

    ret = FsChangeDrive(oldName);
    if (ret != LOS_OK) {
        PRINT_ERR("FAT f_getfree ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_rename(oldName, newName);
    if (res != FR_OK) {
        PRINT_ERR("FAT frename err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }
    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

int FatfsStatfs(const char *path, struct statfs *buf)
{
    FATFS *fs = NULL;
    UINT32 freeClust;
    FRESULT res;
    INT32 ret;

    if ((path == NULL) || (buf == NULL)) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != FR_OK) {
        PRINT_ERR("FAT f_getfree ChangeDrive err %d.", ret);
        errno = FatfsErrno(FR_INVALID_PARAMETER);
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_getfree(path, &freeClust, &fs);
    if (res != FR_OK) {
        PRINT_ERR("FAT f_getfree err 0x%x.", res);
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }
    buf->f_bfree  = freeClust;
    buf->f_bavail = freeClust;
    /* Cluster #0 and #1 is for VBR, reserve sectors and fat */
    buf->f_blocks = fs->n_fatent - 2;
#if FF_MAX_SS != FF_MIN_SS
    buf->f_bsize  = fs->ssize * fs->csize;
#else
    buf->f_bsize  = FF_MIN_SS * fs->csize;
#endif

    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

static int DoTruncate(struct File *file, off_t length, UINT32 count)
{
    FRESULT res = FR_OK;
    INT32 ret = LOS_OK;
    DWORD csz;
    FIL *fp = (FIL *)file->fData;

    csz = (DWORD)(fp->obj.fs)->csize * SS(fp->obj.fs); /* Cluster size */
    if (length > csz * count) {
#if FF_USE_EXPAND
        res = f_expand(fp, 0, (FSIZE_t)(length), FALLOC_FL_KEEP_SIZE);
#else
        errno = ENOSYS;
        ret = LOS_NOK;
        return ret;
#endif
    } else if (length < csz * count) {
        res = f_truncate(fp, (FSIZE_t)length);
    }

    if (res != FR_OK) {
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        return ret;
    }

    fp->obj.objsize = length; /* Set file size to length */
    fp->flag |= 0x40; /* Set modified flag */

    return ret;
}

int FatfsTruncate(struct File *file, off_t length)
{
    FRESULT res;
    INT32 ret;
    UINT count;
    DWORD fclust;
    FIL *fp = (FIL *)file->fData;

    if ((length < 0) || (length > UINT_MAX)) {
        errno = EINVAL;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        return LOS_NOK;
    }

    if ((fp == NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_getclustinfo(fp, &fclust, &count);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }

    ret = DoTruncate(file, length, count);
    if (ret != FR_OK) {
        goto OUT;
    }

    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

int FatfsFdisk(const char *dev, int *partTbl, int arrayNum)
{
    int pdrv;
    FRESULT res;
    INT32 ret;

    if ((dev == NULL) || (partTbl == NULL)) {
        errno = EFAULT;
        return LOS_NOK;
    }

    pdrv = GetDevIdByDevName(dev);
    if (pdrv < 0) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        ret = LOS_NOK;
        goto OUT;
    }

    res = f_fdisk(pdrv, (DWORD const *)partTbl, g_workBuffer);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }

    ret = LOS_OK;

OUT:
    FsUnlock();
    return ret;
}

int FatfsFormat(const char *partName, void *privData)
{
    FRESULT res;
    INT32 ret;
    MKFS_PARM opt = {0};
    int option = *(int *)privData;
    char *dev = NULL; /* logical driver */

    if (partName == NULL) {
        errno = EFAULT;
        return LOS_NOK;
    }

    dev = GetLdPath(partName);
    if (dev == NULL) {
        errno = EFAULT;
        return LOS_NOK;
    }

    ret = FsLock();
    if (ret != 0) {
        errno = ret;
        PutLdPath(dev);
        return LOS_NOK;
    }

    opt.fmt = option;
    opt.n_sect = 0; /* use default allocation unit size depends on the volume
                       size. */
    res = f_mkfs(dev, &opt, g_workBuffer, FF_MAX_SS);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        ret = LOS_NOK;
        goto OUT;
    }

    ret = LOS_OK;

OUT:
    PutLdPath(dev);
    FsUnlock();
    return ret;
}

static struct MountOps g_fatfsMnt = {
    .mount = FatfsMount,
    .umount = FatfsUmount,
    .umount2 = FatfsUmount2,
    .statfs = FatfsStatfs,
};

static struct FileOps g_fatfsFops = {
    .open = FatfsOpen,
    .close = FatfsClose,
    .read = FatfsRead,
    .write = FatfsWrite,
    .lseek = FatfsLseek,
    .stat = FatfsStat,
    .truncate = FatfsTruncate,
    .unlink = FatfsUnlink,
    .rename = FatfsRename,
    .ioctl = NULL, /* not support */
    .sync = FatfsSync,
    .opendir = FatfsOpendir,
    .readdir = FatfsReaddir,
    .closedir = FatfsClosedir,
    .mkdir = FatfsMkdir,
    .rmdir = FatfsRmdir,
};

static struct FsManagement g_fatfsMgt = {
    .fdisk = FatfsFdisk,
    .format = FatfsFormat,
};

void FatFsInit(void)
{
    (void)OsFsRegister("vfat", &g_fatfsMnt, &g_fatfsFops, &g_fatfsMgt);
}
