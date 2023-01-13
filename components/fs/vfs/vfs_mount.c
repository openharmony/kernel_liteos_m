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
#include "vfs_mount.h"
#include "vfs_files.h"
#include "vfs_maps.h"
#include "vfs_config.h"
#include "stdlib.h"
#include "string.h"
#include "limits.h"
#include "errno.h"
#include "securec.h"
#include "vfs_operations.h"
#include "los_compiler.h"
#include "los_debug.h"
#include "los_fs.h"
#include "los_mux.h"

struct MountPoint *g_mountPoints = NULL;

static void MpDeleteFromList(struct MountPoint *mp)
{
    struct MountPoint *prev = NULL;

    /* delete mp from mount list */
    if (g_mountPoints == mp) {
        g_mountPoints = mp->mNext;
    } else {
        for (prev = g_mountPoints; prev != NULL; prev = prev->mNext) {
            if (prev->mNext != mp) {
                continue;
            }

            prev->mNext = mp->mNext;
            break;
        }
    }
}

#if (LOSCFG_FS_SUPPORT_MOUNT_TARGET_RECURSIVE == 1)
struct MountPoint *VfsMpFind(const char *path, const char **pathInMp)
{
    struct MountPoint *mp = g_mountPoints;
    struct MountPoint *bestMp = NULL;
    int bestMatches = 0;

    if (pathInMp != NULL) {
        *pathInMp = NULL;
    }
    while ((mp != NULL) && (mp->mPath != NULL)) {
        const char *mPath = mp->mPath;
        const char *iPath = path;
        const char *t = NULL;
        int matches = 0;
        do {
            while ((*mPath == '/') && (*(mPath + 1) != '/')) {
                mPath++;
            }
            while ((*iPath == '/') && (*(iPath + 1) != '/')) {
                iPath++;
            }

            t = strchr(mPath, '/');
            if (t == NULL) {
                t = strchr(mPath, '\0');
            }
            if ((t == mPath) || (t == NULL)) {
                break;
            }
            if (strncmp(mPath, iPath, (size_t)(t - mPath)) != 0) {
                goto next; /* this mount point do not match, check next */
            }

            iPath += (t - mPath);
            if ((*iPath != '\0') && (*iPath != '/')) {
                goto next;
            }

            matches += (t - mPath);
            mPath += (t - mPath);
        } while (*mPath != '\0');

        if (matches > bestMatches) {
            bestMatches = matches;
            bestMp = mp;

            while ((*iPath == '/') && (*(iPath + 1) != '/')) {
                iPath++;
            }

            if (pathInMp != NULL) {
                *pathInMp = path;
            }
        }
    next:
        mp = mp->mNext;
    }

    return bestMp;
}
#else
struct MountPoint *VfsMpFind(const char *path, const char **pathInMp)
{
    struct MountPoint *mp = g_mountPoints;
    const char *iPath = path;
    const char *mPath = NULL;
    const char *target = NULL;

    if (pathInMp != NULL) {
        *pathInMp = NULL;
    }
    while (*iPath == '/') {
        ++iPath;
    }

    while ((mp != NULL) && (mp->mPath != NULL)) {
        mPath = mp->mPath;
        target = iPath;

        while (*mPath == '/') {
            ++mPath;
        }

        while ((*mPath != '\0') && (*mPath != '/') &&
               (*target != '\0') && (*target != '/')) {
            if (*mPath != *target) {
                break;
            }
            ++mPath;
            ++target;
        }
        if (((*mPath == '\0') || (*mPath == '/')) &&
            ((*target == '\0') || (*target == '/'))) {
            if (pathInMp != NULL) {
                *pathInMp = path;
            }
            return mp;
        }
        mp = mp->mNext;
    }
    return NULL;
}
#endif

STATIC struct MountPoint *VfsMountPointInit(const char *source, const char *target,
        const char *fsType, unsigned long mountflags)
{
    struct MountPoint *mp = NULL;
    const char *pathInMp = NULL;
    struct FsMap *mFs = NULL;
    size_t ssize = 0;
    size_t tsize;

    /* find mp by target, to see if it was mounted */
    mp = VfsMpFind(target, &pathInMp);
    if (mp != NULL && pathInMp != NULL) {
        errno = EINVAL;
        return NULL;
    }

    /* Find fsMap coresponding to the fsType */
    mFs = VfsFsMapGet(fsType);
    if ((mFs == NULL) || (mFs->fsMops == NULL) || (mFs->fsMops->mount == NULL)) {
        errno = ENODEV;
        return NULL;
    }

    if (source != NULL) {
        ssize = strlen(source) + 1;
    }

    tsize = strlen(target) + 1;

    mp = (struct MountPoint *)LOSCFG_FS_MALLOC_HOOK(sizeof(struct MountPoint) + ssize + tsize);
    if (mp == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    mp->mFs = mFs;
    mp->mDev = NULL;
    mp->mRefs = 0;
    mp->mWriteEnable = (mountflags & MS_RDONLY) ? FALSE : TRUE;
    mp->mFs->fsRefs++;

    if (source != NULL && strcpy_s((char *)mp + sizeof(struct MountPoint), ssize, source) != EOK) {
        LOSCFG_FS_FREE_HOOK(mp);
        errno = ENOMEM;
        return NULL;
    }

    if (strcpy_s((char *)mp + sizeof(struct MountPoint) + ssize, tsize, target) != EOK) {
        LOSCFG_FS_FREE_HOOK(mp);
        errno = ENOMEM;
        return NULL;
    }
    mp->mDev = source ? (char *)mp + sizeof(struct MountPoint) : NULL;
    mp->mPath = (char *)mp + sizeof(struct MountPoint) + ssize;

    return mp;
}

STATIC int VfsRemount(const char *source, const char *target,
                      const char *fsType, unsigned long mountflags,
                      const void *data)
{
    (VOID)source;
    (VOID)fsType;
    struct MountPoint *mp;

    mp = VfsMpFind(target, NULL);
    if (mp == NULL) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }

    LOS_ASSERT(mp->mFs != NULL);
    LOS_ASSERT(mp->mFs->fsMops != NULL);
    LOS_ASSERT(mp->mFs->fsMops->mount != NULL);

    return mp->mFs->fsMops->mount(mp, mountflags, data);
}

STATIC int VfsMountPathCheck(const char *target)
{
    /* target must begin with '/', for example /system, /data, etc. */
    if ((target == NULL) || (target[0] != '/')) {
        errno = EINVAL;
        return (int)LOS_NOK;
    }

    if (strlen(target) >= PATH_MAX) {
        errno = ENAMETOOLONG;
        return (int)LOS_NOK;
    }

    return LOS_OK;
}

int mount(const char *source, const char *target,
                const char *fsType, unsigned long mountflags,
                const void *data)
{
    int ret;
    struct MountPoint *mp = NULL;

    if (VfsMountPathCheck(target) != LOS_OK) {
        return (int)LOS_NOK;
    }

    (void)LOS_FsLock();

    if (mountflags & MS_REMOUNT) {
        ret = VfsRemount(source, target, fsType, mountflags, data);
        LOS_FsUnlock();
        return ret;
    }

    mp = VfsMountPointInit(source, target, fsType, mountflags);
    if (mp == NULL) {
        LOS_FsUnlock();
        return (int)LOS_NOK;
    }

    ret = mp->mFs->fsMops->mount(mp, mountflags, data);
    if (ret != 0) {
        /* errno is set */
        PRINT_ERR("mount failed, target %s.\n", target);
        goto errout;
    }

    mp->mNext = g_mountPoints;
    g_mountPoints = mp;
    LOS_FsUnlock();
    return LOS_OK;

errout:
    LOSCFG_FS_FREE_HOOK(mp);
    LOS_FsUnlock();
    return (int)LOS_NOK;
}

int umount(const char *target)
{
    struct MountPoint *mp = NULL;
    int ret = (int)LOS_NOK;

    (void)LOS_FsLock();
    if (VfsMountPathCheck(target) != LOS_OK) {
        goto errout;
    }

    mp = VfsMpFind(target, NULL);
    if ((mp == NULL) || (mp->mRefs != 0)) {
        goto errout;
    }

    if ((mp->mFs == NULL) || (mp->mFs->fsMops == NULL) ||
        (mp->mFs->fsMops->umount == NULL)) {
        goto errout;
    }

    ret = mp->mFs->fsMops->umount(mp);
    if (ret != 0) {
        /* errno is set */
        goto errout;
    }

    /* delete mp from mount list */
    MpDeleteFromList(mp);
    mp->mFs->fsRefs--;
    LOSCFG_FS_FREE_HOOK(mp);

    LOS_FsUnlock();
    return LOS_OK;

errout:
    PRINT_ERR("umount2 failed, target %s.\n", target);
    LOS_FsUnlock();
    return (int)LOS_NOK;
}

static void CloseFdsInMp(const struct MountPoint *mp)
{
    for (int fd = 0; fd < NR_OPEN_DEFAULT; fd++) {
        struct File *f = FdToFile(fd);
        if (f == NULL) {
            continue;
        }
        if ((f->fMp == mp) &&
            (f->fFops != NULL) &&
            (f->fFops->close != NULL)) {
            (void)f->fFops->close(f);
        }
    }
}

int umount2(const char *target, int flag)
{
    struct MountPoint *mp = NULL;
    int ret = (int)LOS_NOK;

    (void)LOS_FsLock();
    if (VfsMountPathCheck(target) != LOS_OK) {
        goto errout;
    }

    mp = VfsMpFind(target, NULL);
    if ((mp == NULL) || (mp->mRefs != 0) ||
        (mp->mFs == NULL) || (mp->mFs->fsMops == NULL) ||
        (mp->mFs->fsMops->umount2 == NULL)) {
        goto errout;
    }

    /* Close all files under the mount point */
    if ((UINT32)flag & MNT_FORCE) {
        CloseFdsInMp(mp);
    }

    ret = mp->mFs->fsMops->umount2(mp, flag);
    if (ret != 0) {
        /* errno is set */
        goto errout;
    }

    /* delete mp from mount list */
    MpDeleteFromList(mp);
    mp->mFs->fsRefs--;
    LOSCFG_FS_FREE_HOOK(mp);

    LOS_FsUnlock();
    return LOS_OK;

errout:
    PRINT_ERR("umount2 failed, target %s.\n", target);
    LOS_FsUnlock();
    return (int)LOS_NOK;
}
