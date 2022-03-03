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

#ifndef _LOS_BOX_H
#define _LOS_BOX_H

#include "los_compiler.h"

#define LOSCFG_BOX_START_PRIO       5
#define LOSCFG_BOX_PRIO             10

/*
 * -------------------- boxStartAddr(main)
 * | .text            |
 * | -----------------
 * | .data            |
 * | -----------------  boxHeapAddr
 * | .heap            |
 * | -----------------  boxStackAddr
 * | boxStackSize     |
 * --------------------
 */
typedef struct {
    UINTPTR boxStartAddr;
    UINTPTR boxSize;
    UINTPTR boxHeapAddr;
    UINTPTR boxHeapSize;
    UINTPTR boxStackAddr; /* box main task stack addr */
    UINTPTR boxStackSize;
} LosBoxCB;

typedef struct {
    unsigned long userArea;
    unsigned long userSp;
    unsigned int boxID;
} UserTaskCB;

VOID OsUserTaskInit(UINT32 taskID, UINTPTR entry, UINTPTR userArea, UINTPTR userSp);
VOID OsUserTaskDelete(UINT32 taskID);
UserTaskCB *OsGetUserTaskCB(UINT32 taskID);
UINT32 LOS_BoxStart(VOID);

extern INT32 _ulibc_start(UINTPTR main);

#endif
