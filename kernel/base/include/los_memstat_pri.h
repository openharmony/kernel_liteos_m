/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_MEMSTAT_PRI_H
#define _LOS_MEMSTAT_PRI_H

#include "los_typedef.h"
#include "los_memory.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* extra 1 blocks is for extra temporary task */
#ifdef LOSCFG_LIB_CONFIGURABLE
#define TASK_NUM        (g_taskLimit + 1)
#else
#define TASK_NUM        (LOSCFG_BASE_CORE_TSK_LIMIT + 1)
#endif

#ifdef LOSCFG_MEM_TASK_STAT
typedef struct {
    UINT32 memUsed;
    UINT32 memPeak;
} TaskMemUsedInfo;

typedef struct {
    UINT32 memTotalUsed;
    UINT32 memTotalPeak;
    TaskMemUsedInfo *taskMemstats;
} Memstat;

extern TaskMemUsedInfo *g_osTaskMemstats;
extern VOID OsMemstatTaskUsedInc(Memstat *stat, UINT32 usedSize, UINT32 taskId);
extern VOID OsMemstatTaskUsedDec(Memstat *stat, UINT32 usedSize, UINT32 taskId);
extern VOID OsMemstatTaskClear(Memstat *stat, UINT32 taskId);
extern UINT32 OsMemstatTaskUsage(const Memstat *stat, UINT32 taskId);
#endif

extern VOID OsMemTaskClear(UINT32 taskId);
extern UINT32 OsMemTaskUsage(UINT32 taskId);

#ifdef LOSCFG_MEM_TASK_STAT
#define OS_MEM_ADD_USED(stat, usedSize, taskId)         OsMemstatTaskUsedInc(stat, usedSize, taskId)
#define OS_MEM_REDUCE_USED(stat, usedSize, taskId)      OsMemstatTaskUsedDec(stat, usedSize, taskId)
#define OS_MEM_CLEAR(taskId)                            OsMemTaskClear(taskId)
#define OS_MEM_USAGE(taskId)                            OsMemTaskUsage(taskId)
#else
#define OS_MEM_ADD_USED(stat, usedSize, taskId)
#define OS_MEM_REDUCE_USED(stat, usedSize, taskId)
#define OS_MEM_CLEAR(taskId)
#define OS_MEM_USAGE(taskId) 0
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_MEMSTAT_PRI_H */
