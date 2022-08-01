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
#include "lfs_adapter.h"
#include "los_config.h"
#include "vfs_files.h"
#include "vfs_operations.h"
#include "vfs_partition.h"
#include "vfs_maps.h"
#include "vfs_mount.h"
#include "securec.h"

struct dirent g_nameValue;
static pthread_mutex_t g_FslocalMutex = PTHREAD_MUTEX_INITIALIZER;

static int ConvertFlagToLfsOpenFlag (int oflags)
{
    int lfsOpenFlag = 0;

    if (oflags & O_CREAT) {
        lfsOpenFlag |= LFS_O_CREAT;
    }

    if (oflags & O_EXCL) {
        lfsOpenFlag |= LFS_O_EXCL;
    }

    if (oflags & O_TRUNC) {
        lfsOpenFlag |= LFS_O_TRUNC;
    }

    if (oflags & O_APPEND) {
        lfsOpenFlag |= LFS_O_APPEND;
    }

    if (oflags & O_RDWR) {
        lfsOpenFlag |= LFS_O_RDWR;
    }

    if (oflags & O_WRONLY) {
        lfsOpenFlag |= LFS_O_WRONLY;
    }

    if (oflags == O_RDONLY) {
        lfsOpenFlag |= LFS_O_RDONLY;
    }

    return lfsOpenFlag;
}

static int LittlefsErrno(int result)
{
    return (result < 0) ? -result : result;
}

int LfsMount(struct MountPoint *mp, unsigned long mountflags, const void *data)
{
    int ret;
    lfs_t *mountHdl = NULL;

    if ((mp == NULL) || (mp->mPath == NULL) || (data == NULL)) {
        errno = EFAULT;
        ret = (int)LOS_NOK;
        goto errout;
    }

    mountHdl = (lfs_t *)malloc(sizeof(lfs_t));
    if (mountHdl == NULL) {
        errno = ENODEV;
        ret = (int)LOS_NOK;
        goto errout;
    }
    (void)memset_s(mountHdl, sizeof(lfs_t), 0, sizeof(lfs_t));
    mp->mData = (void *)mountHdl;

    ret = lfs_mount((lfs_t *)mp->mData, (struct lfs_config *)data);
    if (ret != 0) {
        ret = lfs_format((lfs_t *)mp->mData, (struct lfs_config*)data);
        if (ret == 0) {
            ret = lfs_mount((lfs_t *)mp->mData, (struct lfs_config*)data);
        }
    }

    if (ret != 0) {
        free(mountHdl);
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

errout:
    return ret;
}

int LfsUmount(struct MountPoint *mp)
{
    int ret;

    if (mp == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    ret = lfs_unmount((lfs_t *)mp->mData);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

    free(mp->mData);
    mp->mData = NULL;
    return ret;
}

int LfsUnlink(struct MountPoint *mp, const char *fileName)
{
    int ret;

    if ((mp == NULL) || (fileName == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    ret = lfs_remove((lfs_t *)mp->mData, fileName);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

    return ret;
}

int LfsMkdir(struct MountPoint *mp, const char *dirName)
{
    int ret;

    if ((dirName == NULL) || (mp == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    lfs_t *lfs = (lfs_t *)mp->mData;

    ret = lfs_mkdir(lfs, dirName);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

    return ret;
}

int LfsRmdir(struct MountPoint *mp, const char *dirName)
{
    int ret;
    lfs_t *lfs = NULL;

    if (mp == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    lfs = (lfs_t *)mp->mData;

    if (dirName == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    ret = lfs_remove(lfs, dirName);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

    return ret;
}

int LfsOpendir(struct Dir *dir, const char *dirName)
{
    int ret;

    if ((dir == NULL) || (dir->dMp == NULL) || (dir->dMp->mData == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    lfs_t *lfs = (lfs_t *)dir->dMp->mData;
    lfs_dir_t *dirInfo = (lfs_dir_t *)malloc(sizeof(lfs_dir_t));
    if (dirInfo == NULL) {
        errno = ENOMEM;
        return (int)LOS_NOK;
    }

    (void)memset_s(dirInfo, sizeof(lfs_dir_t), 0, sizeof(lfs_dir_t));
    ret = lfs_dir_open(lfs, dirInfo, dirName);
    if (ret != 0) {
        free(dirInfo);
        errno = LittlefsErrno(ret);
        goto errout;
    }

    dir->dData = dirInfo;
    dir->dOffset = 0;

    return LOS_OK;

errout:
    return (int)LOS_NOK;
}

int LfsReaddir(struct Dir *dir, struct dirent *dent)
{
    int ret;
    struct lfs_info lfsInfo;

    if ((dir == NULL) || (dir->dMp == NULL) || (dir->dMp->mData == NULL) ||
        (dent == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (dir->dData == NULL) {
        errno = EBADF;
        return (int)LOS_NOK;
    }

    lfs_t *lfs = (lfs_t *)dir->dMp->mData;
    lfs_dir_t *dirInfo = (lfs_dir_t *)dir->dData;

    ret = lfs_dir_read(lfs, dirInfo, &lfsInfo);
    if (ret == TRUE) {
        pthread_mutex_lock(&g_FslocalMutex);
        (void)strncpy_s(dent->d_name, sizeof(dent->d_name), lfsInfo.name, strlen(lfsInfo.name) + 1);
        if (lfsInfo.type == LFS_TYPE_DIR) {
            dent->d_type = DT_DIR;
        } else if (lfsInfo.type == LFS_TYPE_REG) {
            dent->d_type = DT_REG;
        }

        dent->d_reclen = lfsInfo.size;
        pthread_mutex_unlock(&g_FslocalMutex);

        return LOS_OK;
    }

    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return (int)LOS_NOK;
}

int LfsClosedir(struct Dir *dir)
{
    int ret;

    if ((dir == NULL) || (dir->dMp == NULL) || (dir->dMp->mData == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (dir->dData == NULL) {
        errno = EBADF;
        return (int)LOS_NOK;
    }

    lfs_t *lfs = (lfs_t *)dir->dMp->mData;
    lfs_dir_t *dirInfo = (lfs_dir_t *)dir->dData;

    ret = lfs_dir_close(lfs, dirInfo);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

    free(dirInfo);
    dir->dData = NULL;

    return ret;
}

int LfsOpen(struct File *file, const char *pathName, int openFlag)
{
    int ret;
    lfs_file_t *lfsHandle = NULL;

    if ((pathName == NULL) || (file == NULL) || (file->fMp == NULL) ||
        (file->fMp->mData == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    lfsHandle = (lfs_file_t *)malloc(sizeof(lfs_file_t));
    if (lfsHandle == NULL) {
        errno = ENOMEM;
        return (int)LOS_NOK;
    }

    int lfsOpenFlag = ConvertFlagToLfsOpenFlag(openFlag);
    ret = lfs_file_open((lfs_t *)file->fMp->mData, lfsHandle, pathName, lfsOpenFlag);
    if (ret != 0) {
        free(lfsHandle);
        errno = LittlefsErrno(ret);
        goto errout;
    }

    file->fData = (void *)lfsHandle;
    return ret;

errout:
    return INVALID_FD;
}

int LfsRead(struct File *file, char *buf, size_t len)
{
    int ret;
    struct MountPoint *mp = NULL;
    lfs_file_t *lfsHandle = NULL;

    if (buf == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return (int)LOS_NOK;
    }

    lfsHandle = (lfs_file_t *)file->fData;
    mp = file->fMp;
    if ((mp == NULL) || (mp->mData == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    ret = lfs_file_read((lfs_t *)mp->mData, lfsHandle, buf, len);
    if (ret < 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }
    return ret;
}

int LfsWrite(struct File *file, const char *buf, size_t len)
{
    int ret;
    struct MountPoint *mp = NULL;
    lfs_file_t *lfsHandle = NULL;

    if (buf == NULL) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return (int)LOS_NOK;
    }

    lfsHandle = (lfs_file_t *)file->fData;
    mp = file->fMp;
    if ((mp == NULL) || (mp->mData == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    ret = lfs_file_write((lfs_t *)mp->mData, lfsHandle, buf, len);
    if (ret < 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }
    return ret;
}

off_t LfsSeek(struct File *file, off_t offset, int whence)
{
    off_t ret;
    struct MountPoint *mp = NULL;
    lfs_file_t *lfsHandle = NULL;

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return (off_t)LOS_NOK;
    }

    lfsHandle = (lfs_file_t *)file->fData;
    mp = file->fMp;
    if ((mp == NULL) || (mp->mData == NULL)) {
        errno = EFAULT;
        return (off_t)LOS_NOK;
    }

    ret = (off_t)lfs_file_seek((lfs_t *)mp->mData, lfsHandle, offset, whence);
    if (ret < 0) {
        errno = LittlefsErrno(ret);
        ret = (off_t)LOS_NOK;
    }

    return ret;
}

int LfsClose(struct File *file)
{
    int ret;
    struct MountPoint *mp = NULL;
    lfs_file_t *lfsHandle = NULL;

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return (int)LOS_NOK;
    }

    lfsHandle = (lfs_file_t *)file->fData;
    mp = file->fMp;
    if ((mp == NULL) || (mp->mData == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    pthread_mutex_lock(&g_FslocalMutex);
    ret = lfs_file_close((lfs_t *)mp->mData, lfsHandle);
    pthread_mutex_unlock(&g_FslocalMutex);

    if (ret != 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

    free(file->fData);
    file->fData = NULL;
    return ret;
}

int LfsRename(struct MountPoint *mp, const char *oldName, const char *newName)
{
    int ret;

    if ((mp == NULL) || (oldName == NULL) || (newName == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    ret = lfs_rename((lfs_t *)mp->mData, oldName, newName);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

    return ret;
}

int LfsStat(struct MountPoint *mp, const char *path, struct stat *buf)
{
    int ret;
    struct lfs_info info;

    if ((mp == NULL) || (path == NULL) || (buf == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return (int)LOS_NOK;
    }

    ret = lfs_stat((lfs_t *)mp->mData, path, &info);
    if (ret == 0) {
        buf->st_size = info.size;
        if (info.type == LFS_TYPE_REG) {
            buf->st_mode = S_IFREG;
        } else {
            buf->st_mode = S_IFDIR;
        }
    } else {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }

    return ret;
}

int LfsSync(struct File *file)
{
    int ret;
    struct MountPoint *mp = NULL;

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return (int)LOS_NOK;
    }

    if ((file->fMp == NULL) || (file->fMp->mData == NULL)) {
        errno = EFAULT;
        return (int)LOS_NOK;
    }

    mp = file->fMp;
    ret = lfs_file_sync((lfs_t *)mp->mData, (lfs_file_t *)file->fData);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
        ret = (int)LOS_NOK;
    }
    return ret;
}

static struct MountOps g_lfsMnt = {
    .mount = LfsMount,
    .umount = LfsUmount,
    .umount2 = NULL,
    .statfs = NULL,
};

static struct FileOps g_lfsFops = {
    .open = LfsOpen,
    .close = LfsClose,
    .read = LfsRead,
    .write = LfsWrite,
    .lseek = LfsSeek,
    .stat = LfsStat,
    .truncate = NULL,
    .unlink = LfsUnlink,
    .rename = LfsRename,
    .ioctl = NULL, /* not support */
    .sync = LfsSync,
    .rmdir = LfsRmdir,
    .opendir = LfsOpendir,
    .readdir = LfsReaddir,
    .closedir = LfsClosedir,
    .mkdir = LfsMkdir,
};

static struct FsManagement g_lfsMgt = {
    .fdisk = NULL,
    .format = NULL,
};

void LfsInit(void)
{
    (void)OsFsRegister("littlefs", &g_lfsMnt, &g_lfsFops, &g_lfsMgt);
}
