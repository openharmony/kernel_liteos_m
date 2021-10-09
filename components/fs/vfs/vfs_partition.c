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
#include "vfs_partition.h"
#include "vfs_operations.h"
#include "los_fs.h"
#include "los_list.h"
#include "vfs_maps.h"
#include "vfs_mount.h"
#include "securec.h"
#include "stdlib.h"
#include "string.h"

int GetPartIdByPartName(const char *partName)
{
    if (partName == NULL) {
        return LOS_NOK;
    }

    /* the character next to p is the partId */
    char *p = strrchr(partName, 'p');
    if (p + 1 != NULL) {
        return atoi(p + 1);
    }

    return LOS_NOK;
}

int GetDevIdByDevName(const char *dev)
{
    if (dev == NULL) {
        return LOS_NOK;
    }

    /* last character is the deviceId */
    char *p = (char *)dev + strlen(dev) - 1;
    if (p != NULL) {
        return atoi(p);
    }

    return LOS_NOK;
}

int LOS_DiskPartition(const char *dev, const char *fsType, int *lengthArray,
                      int partnum)
{
    int ret = LOS_NOK;
    struct FsMap *fMap = VfsFsMapGet(fsType);
    if ((fMap != NULL) && (fMap->fsMgt != NULL) &&
        (fMap->fsMgt->fdisk != NULL)) {
        ret = fMap->fsMgt->fdisk(dev, lengthArray, partnum);
    }

    return ret;
}

int LOS_PartitionFormat(const char *partName, char *fsType, void *data)
{
    int ret = LOS_NOK;

    /* check if the device is mounted by iterate the mp list
       format is not allowed when the device has been mounted. */
    struct MountPoint *iter = NULL;
    LOS_MP_FOR_EACH_ENTRY(iter) {
        if ((iter->mFs != NULL) && (iter->mFs->fsType != NULL) &&
            strcmp(iter->mFs->fsType, fsType) == 0) {
            errno = EBUSY;
            return LOS_NOK;
        }
    }

    struct FsMap *fMap = VfsFsMapGet(fsType);
    if ((fMap != NULL) && (fMap->fsMgt != NULL) &&
        (fMap->fsMgt->format != NULL)) {
        ret = fMap->fsMgt->format(partName, data);
    }
    return ret;
}
