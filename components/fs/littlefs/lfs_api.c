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

#define _GNU_SOURCE 1
#include "lfs_api.h"
#include "los_config.h"

lfs_t g_lfs;
FileDirInfo g_lfsDir[LFS_MAX_OPEN_DIRS] = {0};

struct FileOpInfo g_fsOp[LOSCFG_LFS_MAX_MOUNT_SIZE] = {0};
static LittleFsHandleStruct g_handle[LITTLE_FS_MAX_OPEN_FILES] = {0};
struct dirent g_nameValue;
static pthread_mutex_t g_FslocalMutex = PTHREAD_MUTEX_INITIALIZER;
static const char *g_littlefsMntName[LOSCFG_LFS_MAX_MOUNT_SIZE] = {"/a","/b","/c"};

LittleFsHandleStruct *LfsAllocFd(const char *fileName, int *fd)
{
    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LITTLE_FS_MAX_OPEN_FILES; i++) {
        if (g_handle[i].useFlag == 0) {
            *fd = i;
            g_handle[i].useFlag = 1;
            g_handle[i].pathName = strdup(fileName);
            pthread_mutex_unlock(&g_FslocalMutex);
            return &(g_handle[i]);
        }
    }
    pthread_mutex_unlock(&g_FslocalMutex);
    *fd = INVALID_FD;
    return NULL;
}

BOOL CheckFileIsOpen(const char *fileName)
{
    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LITTLE_FS_MAX_OPEN_FILES; i++) {
        if (g_handle[i].useFlag == 1) {
            if (strcmp(g_handle[i].pathName, fileName) == 0) {
                pthread_mutex_unlock(&g_FslocalMutex);
                return TRUE;
            }
        }
    }
    pthread_mutex_unlock(&g_FslocalMutex);
    return FALSE;
}

FileDirInfo *GetFreeDir(const char *dirName)
{
    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LFS_MAX_OPEN_DIRS; i++) {
        if (g_lfsDir[i].useFlag == 0) {
            g_lfsDir[i].useFlag = 1;
            g_lfsDir[i].dirName = strdup(dirName);
            pthread_mutex_unlock(&g_FslocalMutex);
            return &(g_lfsDir[i]);
        }
    }
    pthread_mutex_unlock(&g_FslocalMutex);
    return NULL;
}

void FreeDirInfo(const char *dirName) 
{
    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LFS_MAX_OPEN_DIRS; i++) {
        if (g_lfsDir[i].useFlag == 1 && strcmp(g_lfsDir[i].dirName, dirName) == 0) {
            g_lfsDir[i].useFlag = 0;
            if (g_lfsDir[i].dirName) {
                free(g_lfsDir[i].dirName);
                g_lfsDir[i].dirName = NULL;
            }
            pthread_mutex_unlock(&g_FslocalMutex);
        }
    }
    pthread_mutex_unlock(&g_FslocalMutex);
}

BOOL CheckDirIsOpen(const char *dirName)
{
    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LFS_MAX_OPEN_DIRS; i++) {
        if (g_lfsDir[i].useFlag == 1) {
            if (strcmp(g_lfsDir[i].dirName, dirName) == 0) {
                pthread_mutex_unlock(&g_FslocalMutex);
                return TRUE;
            }
        }
    }
    pthread_mutex_unlock(&g_FslocalMutex);
    return FALSE;
}

BOOL CheckPathIsMounted(const char *pathName, struct FileOpInfo **fileOpInfo)
{
    char tmpName[LITTLEFS_MAX_LFN_LEN] = {0};
    int mountPathNameLen;
    int len = strlen(pathName) + 1;

    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LOSCFG_LFS_MAX_MOUNT_SIZE; i++) {
        if (g_fsOp[i].useFlag == 1) {
            mountPathNameLen = strlen(g_fsOp[i].dirName);
            if (len < mountPathNameLen + 1) {
                pthread_mutex_unlock(&g_FslocalMutex);
                return FALSE;
            }

            (void)strncpy_s(tmpName, LITTLEFS_MAX_LFN_LEN, pathName, mountPathNameLen);
            tmpName[mountPathNameLen] = '\0';

            if (strcmp(tmpName, g_fsOp[i].dirName) == 0) {
                *fileOpInfo = &(g_fsOp[i]);
                pthread_mutex_unlock(&g_FslocalMutex);
                return TRUE;
            }
        }
    }
    pthread_mutex_unlock(&g_FslocalMutex);
    return FALSE;
}

struct FileOpInfo *AllocMountRes(const char* target, struct FileOps *fileOps)
{
    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LOSCFG_LFS_MAX_MOUNT_SIZE; i++) {
        if (g_fsOp[i].useFlag == 0 && strcmp(target, g_littlefsMntName[i]) == 0) {
            g_fsOp[i].useFlag = 1;
            g_fsOp[i].fsVops = fileOps;
            g_fsOp[i].dirName == strdup(target);
            pthread_mutex_unlock(&g_FslocalMutex);
            return &(g_fsOp[i]);
        }
    }

    pthread_mutex_unlock(&g_FslocalMutex);
    return NULL;
}

int SetDefaultMountPath(int pathNameIndex, const char* target)
{
    if (pathNameIndex >= LOSCFG_LFS_MAX_MOUNT_SIZE) {
        return VFS_ERROR;
    }
    
    pthread_mutex_lock(&g_FslocalMutex);    
    g_littlefsMntName[pathNameIndex] = strdup(target);
    pthread_mutex_unlock(&g_FslocalMutex);
    return VFS_OK;
}

struct FileOpInfo *GetMountRes(const char *target, int *mountIndex)
{
    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LOSCFG_LFS_MAX_MOUNT_SIZE; i++) {
        if (g_fsOp[i].useFlag == 1) {
            if (g_fsOp[i].dirName && strcmp(target, g_fsOp[i].dirName) == 0) {
                *mountIndex = i;
                pthread_mutex_unlock(&g_FslocalMutex);
                return &(g_fsOp[i]);
            }
        }
    }

    pthread_mutex_unlock(&g_FslocalMutex);
    return NULL;
}

int FreeMountResByIndex(int mountIndex)
{
    if (mountIndex < 0 || mountIndex >= LOSCFG_LFS_MAX_MOUNT_SIZE) {
        return VFS_ERROR;
    }

    pthread_mutex_lock(&g_FslocalMutex);
    if (g_fsOp[mountIndex].useFlag == 1 && g_fsOp[mountIndex].dirName != NULL) {
        g_fsOp[mountIndex].useFlag = 0;
        free(g_fsOp[mountIndex].dirName);
        g_fsOp[mountIndex].dirName = NULL;
    }
    pthread_mutex_unlock(&g_FslocalMutex);

    return VFS_OK;
}

int FreeMountRes(const char *target)
{
    pthread_mutex_lock(&g_FslocalMutex);
    for (int i = 0; i < LOSCFG_LFS_MAX_MOUNT_SIZE; i++) {
        if (g_fsOp[i].useFlag == 1) {
            if (g_fsOp[i].dirName && strcmp(target, g_fsOp[i].dirName) == 0) {
                g_fsOp[i].useFlag = 0;
                free(g_fsOp[i].dirName);
                g_fsOp[i].dirName = NULL;
                pthread_mutex_unlock(&g_FslocalMutex);
                return VFS_OK;
            }
        }
    }

    pthread_mutex_unlock(&g_FslocalMutex);
    return VFS_ERROR;
}

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

    if (oflags & O_RDONLY) {
        lfsOpenFlag |= LFS_O_RDONLY;
    }

    return lfsOpenFlag;
}

static int LittlefsErrno(int result)
{
    int status = 0;

    if (result < 0) {
        return result;
    }

    switch (result) {
        case LFS_ERR_OK:
            break;
        case LFS_ERR_NOTDIR:
            status = ENOTDIR;
            break;
        case LFS_ERR_NOENT:
            status = ENFILE;
            break;
        case LFS_ERR_EXIST:
            status = EEXIST;
            break;
        case LFS_ERR_ISDIR:
            status = EISDIR;
            break;
        case LFS_ERR_NOTEMPTY:
            status = ENOTEMPTY;
            break;
        case LFS_ERR_INVAL:
            status = EINVAL;
            break;
        case LFS_ERR_NOSPC:
            status = ENOSPC;
            break;
        case LFS_ERR_IO:
            status = EIO;
            break;
        default:
            status = result;
            break;
    }

    return status;
}

const struct MountOps g_lfsMnt = {
    .Mount = LfsMount,
    .Umount = LfsUmount,
};

const struct FileOps g_lfsFops = {
    .Mkdir = LfsMkdir,
    .Unlink = LfsUnlink,
    .Rmdir = LfsRmdir,
    .Opendir = LfsOpendir,
    .Readdir = LfsReaddir,
    .Closedir = LfsClosedir,
    .Open = LfsOpen,
    .Close = LfsClose,
    .Write = LfsWrite,
    .Read = LfsRead,
    .Seek = LfsSeek,
    .Rename = LfsRename,
    .Getattr = LfsStat,
    .Fsync = LfsFsync,
};

int LfsMount(const char *source, const char *target, const char *fileSystemType, unsigned long mountflags,
    const void *data)
{
    int ret;
    struct FileOpInfo *fileOpInfo = NULL;

    if (target == NULL || fileSystemType == NULL || data == NULL) {
        errno = EFAULT;
        ret = VFS_ERROR;
        goto errout;
    }

    if (strcmp(fileSystemType, "littlefs") != 0) {
        errno = ENODEV;
        ret = VFS_ERROR;
        goto errout;
    }

    if (CheckPathIsMounted(target, &fileOpInfo)) {
        errno = EBUSY;
        ret = VFS_OK;
        goto errout;
    }

    // select free mount resource
    fileOpInfo = AllocMountRes(target, &g_lfsFops);
    if (fileOpInfo == NULL) {
        errno = ENODEV;
        ret = VFS_ERROR;
        goto errout;
    }
    
    ret = lfs_mount(&(fileOpInfo->lfsInfo), (struct lfs_config*)data);
    if (ret != 0) {
        ret = lfs_format(&(fileOpInfo->lfsInfo), (struct lfs_config*)data);
        if (ret == 0) {
            ret = lfs_mount(&(fileOpInfo->lfsInfo), (struct lfs_config*)data);
        }
    }

    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return ret;
errout:
    return ret;
}

int LfsUmount(const char *target)
{
    int ret;
    int mountIndex = -1;
    struct FileOpInfo *fileOpInfo = NULL;

    if (target == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    fileOpInfo = GetMountRes(target, &mountIndex);
    if (fileOpInfo == NULL) {
        errno = ENOENT;
        return VFS_ERROR;
    }

    ret = lfs_unmount(&(fileOpInfo->lfsInfo));
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    (void)FreeMountResByIndex(mountIndex);
    return ret;
}

int LfsUnlink(const char *fileName)
{
    int ret;
    struct FileOpInfo *fileOpInfo = NULL;

    if (fileName == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (CheckPathIsMounted(fileName, &fileOpInfo) == FALSE || fileOpInfo == NULL) {
        errno = EACCES;
        return VFS_ERROR;
    }

    ret = lfs_remove(&(fileOpInfo->lfsInfo), fileName);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return ret;
}

int LfsMkdir(const char *dirName, mode_t mode)
{
    int ret;
    struct FileOpInfo *fileOpInfo = NULL;

    if (dirName == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (CheckPathIsMounted(dirName, &fileOpInfo) == FALSE || fileOpInfo == NULL) {
        errno = EACCES;
        return VFS_ERROR;
    }

    ret = lfs_mkdir(&(fileOpInfo->lfsInfo), dirName);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return ret;
}

int LfsRmdir(const char *dirName)
{
    int ret;

    struct FileOpInfo *fileOpInfo = NULL;

    if (dirName == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (CheckPathIsMounted(dirName, &fileOpInfo) == FALSE || fileOpInfo == NULL) {
        errno = EACCES;
        return VFS_ERROR;
    }

    ret = lfs_remove(&(fileOpInfo->lfsInfo), dirName);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return ret;
}

DIR *LfsOpendir(const char *dirName)
{
    int ret;
    struct FileOpInfo *fileOpInfo = NULL;

    if (dirName == NULL) {
        errno = EFAULT;
        goto errout;
    }

    if (CheckPathIsMounted(dirName, &fileOpInfo) == FALSE || fileOpInfo == NULL) {
        errno = EACCES;
        goto errout;
    }

    if (CheckDirIsOpen(dirName)) {
        errno = EBUSY;
        goto errout;
    }

    FileDirInfo *dirInfo = GetFreeDir(dirName);
    if (dirInfo == NULL) {
        errno = ENFILE;
        goto errout;
    }

    ret = lfs_dir_open(&(fileOpInfo->lfsInfo), (lfs_dir_t *)(&(dirInfo->dir)), dirName);

    if (ret != 0) {
        errno = LittlefsErrno(ret);
        goto errout;
    }

    dirInfo->lfsHandle = &(fileOpInfo->lfsInfo);

    return (DIR *)dirInfo;

errout:
    return NULL;
}

struct dirent *LfsReaddir(DIR *dir)
{
    int ret;
    struct lfs_info lfsInfo;

    FileDirInfo *dirInfo = (FileDirInfo *)dir;

    if (dirInfo == NULL || dirInfo->lfsHandle == NULL) {
        errno = EFAULT;
        return NULL;
    }

    ret = lfs_dir_read(dirInfo->lfsHandle, (lfs_dir_t *)(&(dirInfo->dir)), &lfsInfo);
    if (ret == 0) {
        pthread_mutex_lock(&g_FslocalMutex);
        (void)strncpy_s(g_nameValue.d_name, sizeof(g_nameValue.d_name), lfsInfo.name, strlen(lfsInfo.name) + 1);
        if (lfsInfo.type == LFS_TYPE_DIR) {
            g_nameValue.d_type = DT_DIR;
        } else if (lfsInfo.type == LFS_TYPE_REG) {
            g_nameValue.d_type = DT_REG;
        }

        g_nameValue.d_reclen = lfsInfo.size;
        pthread_mutex_unlock(&g_FslocalMutex);

        return &g_nameValue;
    }

    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return NULL;
}

int LfsClosedir(const DIR *dir)
{
    int ret;
    FileDirInfo *dirInfo = (FileDirInfo *)dir;

    if (dirInfo == NULL || dirInfo->lfsHandle == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    ret = lfs_dir_close(dirInfo->lfsHandle, (lfs_dir_t *)(&(dirInfo->dir)));

    FreeDirInfo(dirInfo->dirName);

    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return ret;
}

int LfsOpen(const char *pathName, int openFlag, int mode)
{
    int fd = INVALID_FD;
    int err = INVALID_FD;

    struct FileOpInfo *fileOpInfo = NULL;

    if (pathName == NULL) {
        errno = EFAULT;
        goto errout;
    }

    if (CheckPathIsMounted(pathName, &fileOpInfo) == FALSE || fileOpInfo == NULL) {
        errno = EACCES;
        goto errout;
    }
    // if file is already open, return invalid fd
    if (CheckFileIsOpen(pathName)) {
        errno = EBUSY;
        goto errout;
    }

    LittleFsHandleStruct *fsHandle = LfsAllocFd(pathName, &fd);
    if (fd == INVALID_FD) {
        errno = ENFILE;
        goto errout;
    }

    int lfsOpenFlag = ConvertFlagToLfsOpenFlag(openFlag);
    err = lfs_file_open(&(fileOpInfo->lfsInfo), &(fsHandle->file), pathName, lfsOpenFlag);
    if (err != 0) {
        errno = LittlefsErrno(err);
        goto errout;
    }

    g_handle[fd].lfsHandle = &(fileOpInfo->lfsInfo);
    return fd;
errout:
    return err;
}

int LfsRead(int fd, void *buf, unsigned int len)
{
    int ret;
    if (fd >= LITTLE_FS_MAX_OPEN_FILES || fd < 0 || buf == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (g_handle[fd].lfsHandle == NULL) {
        errno = EBADF;
        return VFS_ERROR;
    }

    ret = lfs_file_read(g_handle[fd].lfsHandle, &(g_handle[fd].file), buf, len);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }
    return ret;
}

int LfsWrite(int fd, const void *buf, unsigned int len)
{
    int ret;
    if (fd >= LITTLE_FS_MAX_OPEN_FILES || fd < 0 || buf == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (g_handle[fd].lfsHandle == NULL) {
        errno = EBADF;
        return VFS_ERROR;
    }

    ret = lfs_file_write(g_handle[fd].lfsHandle, &(g_handle[fd].file), buf, len);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }
    return ret;
}

int LfsSeek(int fd, off_t offset, int whence)
{
    int ret;
    if (fd >= LITTLE_FS_MAX_OPEN_FILES || fd < 0) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (g_handle[fd].lfsHandle == NULL) {
        errno = EBADF;
        return VFS_ERROR;
    }

    ret = lfs_file_seek(g_handle[fd].lfsHandle, &(g_handle[fd].file), offset, whence);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return ret;
}

int LfsClose(int fd)
{
    int ret = VFS_ERROR;

    if (fd >= LITTLE_FS_MAX_OPEN_FILES || fd < 0) {
        errno = EFAULT;
        return ret;
    }

    if (g_handle[fd].lfsHandle == NULL) {
        errno = EBADF;
        return VFS_ERROR;
    }

    pthread_mutex_lock(&g_FslocalMutex);
    ret = lfs_file_close(g_handle[fd].lfsHandle, &(g_handle[fd].file));
    g_handle[fd].useFlag = 0;
    if (g_handle[fd].pathName != NULL) {
        free(g_handle[fd].pathName);
        g_handle[fd].pathName = NULL;
    }

    if (g_handle[fd].lfsHandle != NULL) {
        g_handle[fd].lfsHandle = NULL;
    }
    pthread_mutex_unlock(&g_FslocalMutex);

    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return ret;    
}

int LfsRename(const char *oldName, const char *newName)
{
    int ret;
    struct FileOpInfo *fileOpInfo = NULL;

    if (oldName == NULL || newName == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (CheckPathIsMounted(oldName, &fileOpInfo) == FALSE || fileOpInfo == NULL) {
        errno = EACCES;
        return VFS_ERROR;
    }

    ret = lfs_rename(&(fileOpInfo->lfsInfo), oldName, newName);
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }

    return ret;
}

int LfsStat(const char *path, struct stat *buf)
{
    int ret;
    struct lfs_info info;
    struct FileOpInfo *fileOpInfo = NULL;

    if (path == NULL || buf == NULL) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (CheckPathIsMounted(path, &fileOpInfo) == FALSE || fileOpInfo == NULL) {
        errno = EACCES;
        return VFS_ERROR;
    }

    ret = lfs_stat(&(fileOpInfo->lfsInfo), path, &info);
    if (ret == 0) {
        buf->st_size = info.size;
    } else {
        errno = LittlefsErrno(ret);
    }

    return ret;    
}

int LfsFsync(int fd)
{
    int ret;

    if (fd >= LITTLE_FS_MAX_OPEN_FILES || fd < 0) {
        errno = EFAULT;
        return VFS_ERROR;
    }

    if (g_handle[fd].lfsHandle == NULL) {
        errno = EACCES;
        return VFS_ERROR;
    }

    ret = lfs_file_sync(g_handle[fd].lfsHandle, &(g_handle[fd].file));
    if (ret != 0) {
        errno = LittlefsErrno(ret);
    }
    return ret;
}

