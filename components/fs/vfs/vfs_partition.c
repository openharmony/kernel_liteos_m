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

static struct DeviceDesc *g_deviceList = NULL;

int GetPartIdByPartName(const char *partName)
{
    if (partName == NULL) {
        return (int)LOS_NOK;
    }

    /* the character next to p is the partId */
    char *p = strrchr(partName, 'p');
    if (p != NULL) {
        return atoi(p + 1);
    }

    return (int)LOS_NOK;
}

int GetDevIdByDevName(const char *dev)
{
    if (dev == NULL) {
        return (int)LOS_NOK;
    }

    /* last character is the deviceId */
    char *p = (char *)dev + strlen(dev) - 1;
    if (p != NULL) {
        return atoi(p);
    }

    return (int)LOS_NOK;
}

struct DeviceDesc *getDeviceList(VOID)
{
    return g_deviceList;
}

static int AddDevice(const char *dev, const char *fsType, int *lengthArray, int *addrArray,
                     int partNum)
{
    struct DeviceDesc *prev = NULL;
    for (prev = g_deviceList; prev != NULL; prev = prev->dNext) {
        if (strcmp(prev->dDev, dev) == 0) {
            errno = -EEXIST;
            return (int)LOS_NOK;
        }
    }

    if (addrArray == NULL) {
        errno = -EFAULT;
        return (int)LOS_NOK;
    }

    prev = (struct DeviceDesc *)malloc(sizeof(struct DeviceDesc));
    if (prev == NULL) {
        errno = -ENOMEM;
        return (int)LOS_NOK;
    }
    prev->dDev = strdup(dev);
    prev->dFsType  = strdup(fsType);
    prev->dAddrArray = (int *)malloc(partNum * sizeof(int));
    if (prev->dDev == NULL || prev->dFsType == NULL || prev->dAddrArray == NULL) {
        goto errout;
    }
    (void)memcpy_s(prev->dAddrArray, partNum * sizeof(int), addrArray, partNum * sizeof(int));

    if (lengthArray != NULL) {
        prev->dLengthArray = (int *)malloc(partNum * sizeof(int));
        if (prev->dLengthArray == NULL) {
            goto errout;
        }
        (void)memcpy_s(prev->dLengthArray, partNum * sizeof(int), lengthArray, partNum * sizeof(int));
    }

    prev->dNext = g_deviceList;
    prev->dPartNum = partNum;
    g_deviceList = prev;
    return LOS_OK;
errout:
    if (prev->dDev != NULL) {
        free((void *)prev->dDev);
    }
    if (prev->dFsType != NULL) {
        free((void *)prev->dFsType);
    }
    if (prev->dAddrArray != NULL) {
        free((void *)prev->dAddrArray);
    }
    if (prev->dLengthArray != NULL) {
        free((void *)prev->dLengthArray);
    }

    free(prev);
    errno = -ENOMEM;
    return (int)LOS_NOK;
}


int LOS_DiskPartition(const char *dev, const char *fsType, int *lengthArray, int *addrArray,
                      int partNum)
{
    int ret = (int)LOS_NOK;
    struct FsMap *fMap = VfsFsMapGet(fsType);
    if ((fMap != NULL) && (fMap->fsMgt != NULL) &&
        (fMap->fsMgt->fdisk != NULL)) {
        ret = fMap->fsMgt->fdisk(dev, lengthArray, partNum);
        return ret;
    }

    ret = AddDevice(dev, fsType, lengthArray, addrArray, partNum);
    return ret;
}

int LOS_PartitionFormat(const char *partName, char *fsType, void *data)
{
    int ret = (int)LOS_NOK;

    /* check if the device is mounted by iterate the mp list
       format is not allowed when the device has been mounted. */
    struct MountPoint *iter = NULL;
    LOS_MP_FOR_EACH_ENTRY(iter) {
        if ((iter->mPath != NULL) && (strcmp(iter->mPath, partName) == 0)) {
            errno = EBUSY;
            return (int)LOS_NOK;
        }
    }

    struct FsMap *fMap = VfsFsMapGet(fsType);
    if ((fMap != NULL) && (fMap->fsMgt != NULL) &&
        (fMap->fsMgt->format != NULL)) {
        ret = fMap->fsMgt->format(partName, data);
    }

    return ret;
}
