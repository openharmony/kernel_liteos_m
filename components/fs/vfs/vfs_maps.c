/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
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

#include "vfs_maps.h"
#include "vfs_operations.h"
#include <stdlib.h>
#include "securec.h"
#include "los_debug.h"
#include "los_compiler.h"

struct FsMap *g_fsMap = NULL;

struct FsMap *VfsFsMapGet(const char *fsType)
{
    struct FsMap *curr = g_fsMap;

    (void)VfsLock();
    while (curr != NULL) {
        if ((curr->fsType != NULL) && (fsType != NULL) &&
            (strcmp(curr->fsType, fsType) == 0)) {
            (void)VfsUnlock();
            return curr;
        }
        curr = curr->next;
    }

    VfsUnlock();
    return NULL;
}

int OsFsRegister(const char *fsType, struct MountOps *fsMops,
                 struct FileOps *fsFops, struct FsManagement *fsMgt)
{
    if ((fsMops == NULL) || (fsFops == NULL)) {
        return (int)LOS_NOK;
    }

    struct FsMap *newfs = (struct FsMap *)malloc(sizeof(struct FsMap));
    if (newfs == NULL) {
        PRINT_ERR("Fs register malloc failed, fsType %s.\n", fsType);
        return (int)LOS_NOK;
    }
    (void)memset_s(newfs, sizeof(struct FsMap), 0, sizeof(struct FsMap));

    newfs->fsType = strdup(fsType);
    if (newfs->fsType == NULL) {
        free(newfs);
        return (int)LOS_NOK;
    }

    newfs->fsMops = fsMops;
    newfs->fsFops = fsFops;
    newfs->fsMgt = fsMgt;
    newfs->fsRefs = 0;

    (void)VfsLock();
    newfs->next = g_fsMap;
    g_fsMap = newfs;

    VfsUnlock();
    return LOS_OK;
}
