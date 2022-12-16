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
#include "los_sched.h"
#include "vfs_files.h"
#include "vfs_operations.h"
#include "vfs_partition.h"
#include "vfs_maps.h"
#include "vfs_mount.h"
#include "los_fs.h"

/* the max name length of different parts should not bigger than 32 */
#define FS_DRIVE_NAME_MAX_LEN 32

#ifndef FAT_MAX_OPEN_DIRS
#define FAT_MAX_OPEN_DIRS     8
#endif /* FAT_MAX_OPEN_DIRS */

#ifndef FS_LOCK_TIMEOUT_SEC
#define FS_LOCK_TIMEOUT_SEC  15
#endif /* FS_LOCK_TIMEOUT_SEC */

static UINT8 g_workBuffer[FF_MAX_SS];
static char *g_volPath[FF_VOLUMES] = {FF_VOLUME_STRS};

PARTITION VolToPart[] = {
    { 0, 0, 1, 0, 0 },
    { 0, 0, 2, 0, 0 },
    { 0, 0, 3, 0, 0 },
    { 0, 0, 4, 0, 0 }
};

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
        return (int)LOS_NOK;
    }

    res = f_chdrive(tmpPath);
    if (res != FR_OK) {
        return (int)LOS_NOK;
    }

    return (int)LOS_OK;
}

static int Remount(struct MountPoint *mp, unsigned long mountflags)
{
    FATFS *fatfs = (FATFS *)mp->mData;

    /* remount is not allowed when the device is not mounted. */
    if (fatfs->fs_type == 0) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }
    mp->mWriteEnable = (mountflags & MS_RDONLY) ? FALSE : TRUE;

    return (int)LOS_OK;
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
#define LDPATH_PAD 2  // 2 means: strlen("/") + len of '\0'
    int ret;
    int partId = GetPartIdByPartName(source);
    if ((partId < 0) || (partId >= MAX_PARTITION_NUM)) {
        return NULL;
    }

    char *volPath = g_volPath[partId];
    char *ldPath = (char *)LOSCFG_FS_MALLOC_HOOK(strlen(volPath) + LDPATH_PAD);
    if (ldPath == NULL) {
        return NULL;
    }

    (void)memset_s(ldPath, strlen(volPath) + LDPATH_PAD, 0, strlen(volPath) + LDPATH_PAD);

    /* Convert volPath to ldpath, for example, convert "inner" to "/inner" */
    *ldPath = '/';
    ret = strcpy_s(ldPath + 1, strlen(volPath)+1, volPath);
    if (ret != EOK) {
        LOSCFG_FS_FREE_HOOK(ldPath);
        return NULL;
    }

    return ldPath;
}

void PutLdPath(const char *ldPath)
{
    if (ldPath != NULL) {
        LOSCFG_FS_FREE_HOOK((void *)ldPath);
    }
}

int FatfsMount(struct MountPoint *mp, unsigned long mountflags,
               const void *data)
{
    FRESULT res;
    FATFS *fs = NULL;

    if (mountflags & MS_REMOUNT) {
        return Remount(mp, mountflags);
    }

    char *ldPath = GetLdPath(mp->mDev);
    if (ldPath == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    fs = (FATFS *)LOSCFG_FS_MALLOC_HOOK(sizeof(FATFS));
    if (fs == NULL) {
        errno = ENOMEM;
        PutLdPath(ldPath);
        return (int)LOS_NOK;
    }
    (void)memset_s(fs, sizeof(FATFS), 0, sizeof(FATFS));

    res = f_mount(fs, ldPath, 1);
    if (res != FR_OK) {
        LOSCFG_FS_FREE_HOOK(fs);
        PutLdPath(ldPath);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }
    mp->mData = (void *)fs;

    PutLdPath(ldPath);
    return (int)LOS_OK;
}

int FatfsUmount(struct MountPoint *mp)
{
    int volId;
    FRESULT res;
    char *ldPath;
    FATFS *fatfs = (FATFS *)mp->mData;

    /* The volume is not mounted */
    if (fatfs->fs_type == 0) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }

    volId = GetPartIdByPartName(mp->mDev);
    /* umount is not allowed when a file or directory is opened. */
    if (f_checkopenlock(volId) != FR_OK) {
        errno = EBUSY;
        return (int)LOS_NOK;
    }

    ldPath = GetLdPath(mp->mDev);
    if (ldPath == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    res = f_mount((FATFS *)NULL, ldPath, 0);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        PutLdPath(ldPath);
        return (int)LOS_NOK;
    }

    if (fatfs->win != NULL) {
        ff_memfree(fatfs->win);
    }

    LOSCFG_FS_FREE_HOOK(mp->mData);
    mp->mData = NULL;

    PutLdPath(ldPath);
    return (int)LOS_OK;
}

int FatfsUmount2(struct MountPoint *mp, int flag)
{
    UINT32 flags;
    FRESULT res;
    char *ldPath;
    FATFS *fatfs = (FATFS *)mp->mData;

    flags = MNT_FORCE | MNT_DETACH | MNT_EXPIRE | UMOUNT_NOFOLLOW;
    if ((UINT32)flag & ~flags) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }

    /* The volume is not mounted */
    if (fatfs->fs_type == 0) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }

    ldPath = GetLdPath(mp->mDev);
    if (ldPath == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    res = f_mount((FATFS *)NULL, ldPath, 0);
    if (res != FR_OK) {
        PutLdPath(ldPath);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    if (fatfs->win != NULL) {
        ff_memfree(fatfs->win);
    }

    LOSCFG_FS_FREE_HOOK(mp->mData);
    mp->mData = NULL;

    PutLdPath(ldPath);
    return (int)LOS_OK;
}

int FatfsOpen(struct File *file, const char *path, int oflag)
{
    FRESULT res;
    UINT32 fmode;
    FIL *fp = NULL;
    int ret;

    if (path == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    fmode = FatFsGetMode(oflag);

    fp = (FIL *)LOSCFG_FS_MALLOC_HOOK(sizeof(FIL));
    if (fp == NULL) {
        errno = ENOMEM;
        return (int)LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != (int)LOS_OK) {
        PRINT_ERR("FAT open ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        LOSCFG_FS_FREE_HOOK(fp);
        return (int)LOS_NOK;
    }

    res = f_open(fp, path, fmode);
    if (res != FR_OK) {
        PRINT_ERR("FAT open err 0x%x!\r\n", res);
        LOSCFG_FS_FREE_HOOK(fp);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    file->fData = (void *)fp;

    return (int)LOS_OK;
}

int FatfsClose(struct File *file)
{
    FRESULT res;
    FIL *fp = (FIL *)file->fData;

    if ((fp == NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_close(fp);
    if (res != FR_OK) {
        PRINT_ERR("FAT close err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

#if !FF_FS_TINY
    if (fp->buf != NULL) {
        (void)ff_memfree(fp->buf);
    }
#endif
    LOSCFG_FS_FREE_HOOK(file->fData);
    file->fData = NULL;

    return (int)LOS_OK;
}

ssize_t FatfsRead(struct File *file, char *buf, size_t nbyte)
{
    FRESULT res;
    UINT32 lenRead;
    FIL *fp = (FIL *)file->fData;

    if (buf == NULL) {
        errno = EFAULT;
        return (ssize_t)LOS_NOK;
    }

    if (fp == NULL) {
        errno = ENOENT;
        return (ssize_t)LOS_NOK;
    }

    res = f_read(fp, buf, nbyte, &lenRead);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        return (ssize_t)LOS_NOK;
    }

    return (ssize_t)lenRead;
}

ssize_t FatfsWrite(struct File *file, const char *buf, size_t nbyte)
{
    FRESULT res;
    UINT32 lenWrite;
    static BOOL overFlow = FALSE;
    FIL *fp = (FIL *)file->fData;

    if (buf == NULL) {
        errno = EFAULT;
        return (ssize_t)LOS_NOK;
    }

    if ((fp ==NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        return (ssize_t)LOS_NOK;
    }

    res = f_write(fp, buf, nbyte, &lenWrite);
    if ((res == FR_OK) && (lenWrite == 0) && (nbyte != 0) && (overFlow == FALSE)) {
        overFlow = TRUE;
        PRINT_ERR("FAT write err!\r\n");
    }

    if ((res != FR_OK) || (nbyte != lenWrite)) {
        errno = FatfsErrno(res);
        return (ssize_t)LOS_NOK;
    }

    return (ssize_t)lenWrite;
}

off_t FatfsLseek(struct File *file, off_t offset, int whence)
{
    FRESULT res;
    off_t pos;
    FIL *fp = (FIL *)file->fData;

    if ((fp == NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        return (off_t)LOS_NOK;
    }

    if (whence == SEEK_SET) {
        pos = 0;
    } else if (whence == SEEK_CUR) {
        pos = f_tell(fp);
    } else if (whence == SEEK_END) {
        pos = f_size(fp);
    } else {
        errno = EINVAL;
        return (off_t)LOS_NOK;
    }

    res = f_lseek(fp, offset + pos);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        return (off_t)LOS_NOK;
    }

    pos = f_tell(fp);
    return pos;
}

/* Remove the specified FILE */
int FatfsUnlink(struct MountPoint *mp, const char *path)
{
    FRESULT res;
    int ret;

    if (path == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (!mp->mWriteEnable) {
        errno = EACCES;
        return (int)LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != (int)LOS_OK) {
        PRINT_ERR("FAT unlink ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_unlink(path);
    if (res != FR_OK) {
        PRINT_ERR("FAT unlink err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    return (int)LOS_OK;
}

int FatfsStat(struct MountPoint *mp, const char *path, struct stat *buf)
{
    FRESULT res;
    FILINFO fileInfo = {0};
    int ret;

    if ((path == NULL) || (buf == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != (int)LOS_OK) {
        PRINT_ERR("FAT stat ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_stat(path, &fileInfo);
    if (res != FR_OK) {
        PRINT_ERR("FAT stat err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
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

    return (int)LOS_OK;
}

/* Synchronize all changes to Flash */
int FatfsSync(struct File *file)
{
    FRESULT res;
    FIL *fp = (FIL *)file->fData;

    if ((fp == NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_sync(fp);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    return (int)LOS_OK;
}

int FatfsMkdir(struct MountPoint *mp, const char *path)
{
    FRESULT res;
    int ret;

    if (path == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (!mp->mWriteEnable) {
        errno = EACCES;
        return (int)LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != (int)LOS_OK) {
        PRINT_ERR("FAT mkdir ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_mkdir(path);
    if (res != FR_OK) {
        PRINT_ERR("FAT mkdir err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    return (int)LOS_OK;
}

int FatfsOpendir(struct Dir *dir, const char *dirName)
{
    FRESULT res;
    DIR *dp = NULL;
    int ret;

    if (dirName == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    ret = FsChangeDrive(dirName);
    if (ret != (int)LOS_OK) {
        PRINT_ERR("FAT opendir ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    dp = (DIR *)LOSCFG_FS_MALLOC_HOOK(sizeof(DIR));
    if (dp == NULL) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_opendir(dp, dirName);
    if (res != FR_OK) {
        PRINT_ERR("FAT opendir err 0x%x!\r\n", res);
        LOSCFG_FS_FREE_HOOK(dp);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    dir->dData = dp;
    dir->dOffset = 0;

    return (int)LOS_OK;
}

int FatfsReaddir(struct Dir *dir, struct dirent *dent)
{
    FRESULT res;
    FILINFO fileInfo = {0};
    DIR *dp = NULL;

    if ((dir == NULL) || (dir->dData == NULL)) {
        errno = EBADF;
        return (int)LOS_NOK;
    }

    dp = (DIR *)dir->dData;
    res = f_readdir(dp, &fileInfo);
    /* if res not ok or fname is NULL , return NULL */
    if ((res != FR_OK) || (fileInfo.fname[0] == 0x0)) {
        PRINT_ERR("FAT readdir err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    (void)memcpy_s(dent->d_name, sizeof(dent->d_name),
            fileInfo.fname, sizeof(dent->d_name));
    if (fileInfo.fattrib & AM_DIR) {
        dent->d_type = DT_DIR;
    } else {
        dent->d_type = DT_REG;
    }

    return (int)LOS_OK;
}

int FatfsClosedir(struct Dir *dir)
{
    FRESULT res;
    DIR *dp = NULL;

    if ((dir == NULL) || (dir->dData == NULL)) {
        errno = EBADF;
        return (int)LOS_NOK;
    }

    dp = dir->dData;
    res = f_closedir(dp);
    if (res != FR_OK) {
        PRINT_ERR("FAT closedir err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    LOSCFG_FS_FREE_HOOK(dir->dData);
    dir->dData = NULL;

    return (int)LOS_OK;
}

int FatfsRmdir(struct MountPoint *mp, const char *path)
{
    FRESULT res;
    int ret;

    if ((path == NULL) || (mp == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (!mp->mWriteEnable) {
        errno = EACCES;
        return (int)LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != (int)LOS_OK) {
        PRINT_ERR("FAT rmdir ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_rmdir(path);
    if (res != FR_OK) {
        PRINT_ERR("FAT rmdir err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    return (int)LOS_OK;
}

int FatfsRename(struct MountPoint *mp, const char *oldName, const char *newName)
{
    FRESULT res;
    int ret;

    if ((oldName == NULL) || (newName == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (!mp->mWriteEnable) {
        errno = EACCES;
        return (int)LOS_NOK;
    }

    ret = FsChangeDrive(oldName);
    if (ret != (int)LOS_OK) {
        PRINT_ERR("FAT f_getfree ChangeDrive err 0x%x!\r\n", ret);
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_rename(oldName, newName);
    if (res != FR_OK) {
        PRINT_ERR("FAT frename err 0x%x!\r\n", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    return (int)LOS_OK;
}

int FatfsStatfs(const char *path, struct statfs *buf)
{
    FATFS *fs = NULL;
    UINT32 freeClust;
    FRESULT res;
    int ret;

    if ((path == NULL) || (buf == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    ret = FsChangeDrive(path);
    if (ret != FR_OK) {
        PRINT_ERR("FAT f_getfree ChangeDrive err %d.", ret);
        errno = FatfsErrno(FR_INVALID_PARAMETER);
        return (int)LOS_NOK;
    }

    res = f_getfree(path, &freeClust, &fs);
    if (res != FR_OK) {
        PRINT_ERR("FAT f_getfree err 0x%x.", res);
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
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

    return (int)LOS_OK;
}

static int DoTruncate(struct File *file, off_t length, UINT32 count)
{
    FRESULT res = FR_OK;
    DWORD csz;
    FIL *fp = (FIL *)file->fData;

    csz = (DWORD)(fp->obj.fs)->csize * SS(fp->obj.fs); /* Cluster size */
    if (length > csz * count) {
#if FF_USE_EXPAND
        res = f_expand(fp, 0, (FSIZE_t)(length), FALLOC_FL_KEEP_SIZE);
#else
        errno = ENOSYS;
        return (int)LOS_NOK;
#endif
    } else if (length < csz * count) {
        res = f_truncate(fp, (FSIZE_t)length);
    }

    if (res != FR_OK) {
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    fp->obj.objsize = length; /* Set file size to length */
    fp->flag |= 0x40; /* Set modified flag */

    return (int)LOS_OK;
}

int FatfsTruncate(struct File *file, off_t length)
{
    FRESULT res;
    UINT count;
    DWORD fclust;
    FIL *fp = (FIL *)file->fData;

    if ((length < 0) || (length > UINT_MAX)) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }

    if ((fp == NULL) || (fp->obj.fs == NULL)) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    res = f_getclustinfo(fp, &fclust, &count);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    return DoTruncate(file, length, count);
}

int FatfsFdisk(const char *dev, int *partTbl, int arrayNum)
{
    int pdrv;
    FRESULT res;

    if ((dev == NULL) || (partTbl == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    pdrv = GetDevIdByDevName(dev);
    if (pdrv < 0) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    res = f_fdisk(pdrv, (DWORD const *)partTbl, g_workBuffer);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        return (int)LOS_NOK;
    }

    return (int)LOS_OK;
}

int FatfsFormat(const char *partName, void *privData)
{
    FRESULT res;
    MKFS_PARM opt = {0};
    int option = *(int *)privData;
    char *dev = NULL; /* logical driver */

    if (partName == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    dev = GetLdPath(partName);
    if (dev == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    opt.fmt = option;
    opt.n_sect = 0; /* use default allocation unit size depends on the volume
                       size. */
    res = f_mkfs(dev, &opt, g_workBuffer, FF_MAX_SS);
    if (res != FR_OK) {
        errno = FatfsErrno(res);
        PutLdPath(dev);
        return (int)LOS_NOK;
    }

    return (int)LOS_OK;
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
