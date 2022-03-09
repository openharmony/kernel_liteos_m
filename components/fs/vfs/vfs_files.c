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
#include "vfs_files.h"
#include "errno.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "los_config.h"
#include "vfs_config.h"
#include "vfs_mount.h"
#include "vfs_operations.h"

static struct File g_files[NR_OPEN_DEFAULT];

int FileToFd(struct File *file)
{
    if (file == NULL) {
        return LOS_NOK;
    }
    return file - g_files + MIN_START_FD;
}

struct File *FdToFile(int fd)
{
    if ((fd < MIN_START_FD) || (fd >= CONFIG_NFILE_DESCRIPTORS)) {
        return NULL;
    }
    return &g_files[fd - MIN_START_FD];
}

struct File *VfsFileGet(void)
{
    int i;
    /* protected by g_fsMutex */
    for (i = 0; i < NR_OPEN_DEFAULT; i++) {
        if (g_files[i].fStatus == FILE_STATUS_NOT_USED) {
            g_files[i].fStatus = FILE_STATUS_INITING;
            return &g_files[i];
        }
    }

    return NULL;
}

struct File *VfsFileGetSpec(int fd)
{
    if ((fd < MIN_START_FD) || (fd >= CONFIG_NFILE_DESCRIPTORS)) {
        return NULL;
    }
    if (g_files[fd - MIN_START_FD].fStatus == FILE_STATUS_NOT_USED) {
        g_files[fd - MIN_START_FD].fStatus = FILE_STATUS_INITING;
        return &g_files[fd - MIN_START_FD];
    }

    return NULL;
}

void VfsFilePut(struct File *file)
{
    if (file == NULL) {
        return;
    }
    file->fFlags = 0;
    file->fFops = NULL;
    file->fData = NULL;
    file->fMp = NULL;
    file->fOffset = 0;
    file->fOwner = -1;
    file->fullPath = NULL;
    file->fStatus = FILE_STATUS_NOT_USED;
}
