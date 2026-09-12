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

#ifndef _LOS_RWSEM_PRI_H
#define _LOS_RWSEM_PRI_H

#include "los_rwsem.h"
#include "los_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define RWSEM_COUNT_MAX     0xFFFF
#if (LOSCFG_BASE_IPC_RWSEM_LIMIT < 0) || (LOSCFG_BASE_IPC_RWSEM_LIMIT > RWSEM_COUNT_MAX)
#error "LOSCFG_BASE_IPC_RWSEM_LIMIT config error!"
#endif

#define OS_RWSEM_UNUSED     0
#define OS_RWSEM_USED       1

typedef struct {
    UINT8 rwsemState;
    INT16 rwsemCount;
    UINT32 rwsemId;
    LOS_DL_LIST waitList;
} OsRwsemCB;

extern OsRwsemCB *g_osAllRwsem;

#define GET_RWSEM_LIST(ptr)                     LOS_DL_LIST_ENTRY(ptr, OsRwsemCB, waitList)
#define GET_RWSEM_INDEX(rwsemId)                (rwsemId)
#define GET_RWSEM(rwsemId)                      (g_osAllRwsem + GET_RWSEM_INDEX(rwsemId))

enum RwsemState {
    RWSEM_WRITING = -1,
    RWSEM_INITIAL = 0,
    RWSEM_READING = 1,
};

enum RwsemWaitType {
    RWSEM_READ     = 0,
    RWSEM_WRITE    = 1,
};

extern UINT32 OsRwsemInit(VOID);


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_RWSEM_PRI_H */
