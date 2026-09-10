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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials
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

#include "los_priqueue_pri.h"
#include "los_task_pri.h"
#include "los_bitmap.h"

LITE_OS_SEC_BSS LOS_DL_LIST g_priQueueList[OS_PRIORITY_QUEUE_NUM];
LITE_OS_SEC_BSS STATIC UINT32 g_priQueueBitmap;

VOID OsPriQueueInit(VOID)
{
    UINT32 priority;
    for (priority = 0; priority < OS_PRIORITY_QUEUE_NUM; ++priority) {
        LOS_ListInit(&g_priQueueList[priority]);
    }
}

VOID OsPriQueueEnqueue(LOS_DL_LIST *priqueueItem, UINT32 priority, PriQueueHeadTail mode)
{
    /*
     * Task control blocks are initialized as zero. And when task is deleted,
     * and at the same time would be deleted from priority queue or
     * other lists, task pend node will restored as zero.
     */
    LOS_ASSERT(priqueueItem->pstNext == NULL);

    if (LOS_ListEmpty(&g_priQueueList[priority])) {
        g_priQueueBitmap |= PRIQUEUE_PRIOR0_BIT >> priority;
    }

    if (mode == PRI_QUEUE_HEAD) {
        LOS_ListHeadInsert(&g_priQueueList[priority], priqueueItem);
    } else {
        LOS_ListTailInsert(&g_priQueueList[priority], priqueueItem);
    }
}

VOID OsPriQueueDequeue(LOS_DL_LIST *priqueueItem)
{
    LosTaskCB *runTask = LOS_DL_LIST_ENTRY(priqueueItem, LosTaskCB, pendList);

    LOS_ListDelete(priqueueItem);

    if (LOS_ListEmpty(&g_priQueueList[runTask->priority])) {
        g_priQueueBitmap &= ~(PRIQUEUE_PRIOR0_BIT >> runTask->priority);
    }
}

LOS_DL_LIST *OsPriQueueTop(VOID)
{
    UINT32 priority;

    if (g_priQueueBitmap != 0) {
        priority = CLZ(g_priQueueBitmap);
        return LOS_DL_LIST_FIRST(&g_priQueueList[priority]);
    }

    return NULL;
}

BOOL OsPriQueueIsEmpty(UINT32 priority)
{
    return ((g_priQueueBitmap & (PRIQUEUE_PRIOR0_BIT >> priority)) == 0);
}

LosTaskCB *OsGetTopTask(VOID)
{
    UINT32 priority;
    UINT32 bitmap;
    LosTaskCB *newTask = NULL;

    bitmap = g_priQueueBitmap;
    if (g_priQueueBitmap) {
        priority = CLZ(bitmap);
        newTask = LOS_DL_LIST_ENTRY(LOS_DL_LIST_FIRST(&g_priQueueList[priority]), LosTaskCB, pendList);
    } else {
        newTask = OS_TCB_FROM_TID(OsPercpuGet()->idleTaskId);
    }

    return newTask;
}
