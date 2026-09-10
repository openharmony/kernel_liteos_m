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

#include "los_memstat_pri.h"
#include "los_memory_pri.h"
#include "los_task_pri.h"

#define MIN_TASK_ID(x, y)               ((x) > (y) ? (y) : (x))
#define MAX_MEM_USE(x, y)               ((x) > (y) ? (x) : (y))

LITE_OS_SEC_TEXT_MINOR VOID OsMemstatTaskUsedInc(Memstat *stat, UINT32 usedSize, UINT32 taskId)
{
#ifdef LOSCFG_MEM_TASK_STAT
    UINT32 record = MIN_TASK_ID(taskId, TASK_NUM - 1);
    TaskMemUsedInfo *taskMemstats = stat->taskMemstats;

    taskMemstats[record].memUsed += usedSize;
    taskMemstats[record].memPeak = MAX_MEM_USE(taskMemstats[record].memPeak, taskMemstats[record].memUsed);
#else
    (VOID)taskId;
#endif

    stat->memTotalUsed += usedSize;
    stat->memTotalPeak = MAX_MEM_USE(stat->memTotalPeak, stat->memTotalUsed);
}

LITE_OS_SEC_TEXT_MINOR VOID OsMemstatTaskUsedDec(Memstat *stat, UINT32 usedSize, UINT32 taskId)
{
#ifdef LOSCFG_MEM_TASK_STAT
    UINT32 record = MIN_TASK_ID(taskId, TASK_NUM - 1);
    TaskMemUsedInfo *taskMemstats = stat->taskMemstats;

    if (taskMemstats[record].memUsed != 0) {
        if (taskMemstats[record].memUsed < usedSize) {
            PRINT_INFO("mem used of current task '%s':0x%x, decrease size:0x%x\n",
                OsCurrTaskGet()->taskName, taskMemstats[record].memUsed, usedSize);
            return;
        } else {
            taskMemstats[record].memUsed -= usedSize;
        }
    }
#else
    (VOID)taskId;
#endif

    stat->memTotalUsed -= usedSize;
}

#ifdef LOSCFG_MEM_TASK_STAT
LITE_OS_SEC_TEXT_MINOR VOID OsMemstatTaskClear(Memstat *stat, UINT32 taskId)
{
    UINT32 record = MIN_TASK_ID(taskId, TASK_NUM - 1);
    TaskMemUsedInfo *taskMemstats = stat->taskMemstats;

    if (taskMemstats[record].memUsed != 0) {
        PRINT_INFO("mem used of task '%s' is:0x%x, not zero when task being deleted\n",
                   OsCurrTaskGet()->taskName, taskMemstats[record].memUsed);
    }

    taskMemstats[record].memUsed = 0;
    taskMemstats[record].memPeak = 0;
}

LITE_OS_SEC_TEXT_MINOR UINT32 OsMemstatTaskUsage(const Memstat *stat, UINT32 taskId)
{
    UINT32 record = MIN_TASK_ID(taskId, TASK_NUM - 1);
    const TaskMemUsedInfo *taskMemstats = stat->taskMemstats;

    return taskMemstats[record].memUsed;
}

UINT32 OsMemTaskUsage(UINT32 taskId)
{
    LosMemPoolInfo *pool = NULL;
    Memstat *stat = NULL;

#if (LOSCFG_MEM_MUL_POOL != 1)
    /* If Multi-pool is not enabled, then trace SYSTEM MEM only */
    pool = (LosMemPoolInfo *)OS_SYS_MEM_ADDR;
    stat = &(pool->stat);
    return OsMemstatTaskUsage(stat, taskId);
#else
    UINT32 inUse = 0;
    pool = (LosMemPoolInfo *)OsMemMulPoolHeadGet();
    while (pool != NULL) {
        stat = &(pool->stat);
        inUse += OsMemstatTaskUsage(stat, taskId);
        pool = pool->nextPool;
    }
    return inUse;
#endif
}

VOID OsMemTaskClear(UINT32 taskId)
{
    LosMemPoolInfo *pool = NULL;
    Memstat *stat = NULL;

#ifndef LOSCFG_MEM_MUL_POOL
    pool = (LosMemPoolInfo *)OS_SYS_MEM_ADDR;
    stat = &(pool->stat);
    OsMemstatTaskClear(stat, taskId);
#else
    pool = (LosMemPoolInfo *)OsMemMulPoolHeadGet();
    while (pool != NULL) {
        stat = &(pool->stat);
        OsMemstatTaskClear(stat, taskId);
        pool = pool->nextPool;
    }
#endif
}
#endif /* LOSCFG_MEM_TASK_STAT */