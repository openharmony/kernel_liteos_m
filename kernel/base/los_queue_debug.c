/* ----------------------------------------------------------------------------
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 * Description: Queue Debug
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
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
 * --------------------------------------------------------------------------- */

#include "los_queue_debug_pri.h"
#include "los_typedef.h"
#include "los_task_pri.h"
#include "securec.h"
#include "los_misc_pri.h"
#include "los_memory.h"
#include "los_debug.h"
#ifdef LOSCFG_SHELL
#include "shcmd.h"
#endif

#ifdef LOSCFG_DEBUG_QUEUE
typedef struct {
    TSK_ENTRY_FUNC creator; /* The task entry who created this queue */
    UINT64 lastAccessTime;  /* The last access time */
} QueueDebugCB;

STATIC QueueDebugCB *g_queueDebugArray = NULL;

STATIC BOOL QueueCompareValue(const SortParam *sortParam, UINT32 left, UINT32 right)
{
    return (*((UINT64 *)(VOID *)SORT_ELEM_ADDR(sortParam, left)) >
            *((UINT64 *)(VOID *)SORT_ELEM_ADDR(sortParam, right)));
}

UINT32 OsQueueDbgInit(VOID)
{
    UINT32 size = OS_ALL_IPC_QUEUE_LIMIT * sizeof(QueueDebugCB);

    /* system resident memory, don't free */
    g_queueDebugArray = (QueueDebugCB *)LOS_MemAllocAlign((VOID *)OS_SYS_MEM_ADDR, size, sizeof(UINT64));
    if (g_queueDebugArray == NULL) {
        PRINT_ERR("%s: malloc failed!\n", __FUNCTION__);
        return LOS_NOK;
    }
    (VOID)memset(g_queueDebugArray, 0, size);
    return LOS_OK;
}

VOID OsQueueDbgTimeUpdate(UINT32 queueId)
{
    QueueDebugCB *queueDebug = &g_queueDebugArray[queueId];
    queueDebug->lastAccessTime = LOS_TickCountGet();
}

VOID OsQueueDbgUpdate(UINT32 queueId, TSK_ENTRY_FUNC entry)
{
    QueueDebugCB *queueDebug = &g_queueDebugArray[queueId];
    queueDebug->creator = entry;
    queueDebug->lastAccessTime = LOS_TickCountGet();
}

STATIC INLINE VOID OsQueueInfoOutPut(const LosQueueCB *node)
{
    PRINTK("Queue ID <0x%x> may leak, queue len is 0x%x, "
           "readable cnt:0x%x, writable cnt:0x%x, ",
           node->queueID,
           node->queueLen,
           node->readWriteableCnt[OS_QUEUE_READ],
           node->readWriteableCnt[OS_QUEUE_WRITE]);
}

STATIC INLINE VOID OsQueueOpsOutput(const QueueDebugCB *node)
{
    PRINTK("TaskEntry of creator:%p, Latest operation time: 0x%llx\n",
           node->creator, node->lastAccessTime);
}

STATIC VOID SortQueueIndexArray(UINT32 *indexArray, UINT32 count)
{
    LosQueueCB queueNode = {0};
    QueueDebugCB queueDebugNode = {0};
    UINT32 index, intSave;
    SortParam queueSortParam;
    queueSortParam.buf = (CHAR *)g_queueDebugArray;
    queueSortParam.ctrlBlockSize = sizeof(QueueDebugCB);
    queueSortParam.ctrlBlockCnt = OS_ALL_IPC_QUEUE_LIMIT;
    queueSortParam.sortElemOff = LOS_OFF_SET_OF(QueueDebugCB, lastAccessTime);

    if (count > 0) {
        SCHEDULER_LOCK(intSave);
        OsArraySort(indexArray, 0, count - 1, &queueSortParam, QueueCompareValue);
        SCHEDULER_UNLOCK(intSave);
        for (index = 0; index < count; index++) {
            SCHEDULER_LOCK(intSave);
            (VOID)memcpy(&queueNode, GET_QUEUE_HANDLE(indexArray[index]), sizeof(LosQueueCB));
            (VOID)memcpy(&queueDebugNode, &g_queueDebugArray[indexArray[index]], sizeof(QueueDebugCB));
            SCHEDULER_UNLOCK(intSave);
            if (queueNode.queueState == OS_QUEUE_UNUSED) {
                continue;
            }
            OsQueueInfoOutPut(&queueNode);
            OsQueueOpsOutput(&queueDebugNode);
        }
    }
    (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, indexArray);
}

VOID OsQueueCheck(VOID)
{
    LosQueueCB *queueCB = NULL;
    BOOL isPending = FALSE;
    LosQueueCB queueNode = {0};
    QueueDebugCB queueDebugNode = {0};
    UINT32 index, intSave;
    UINT32 count = 0;

    UINT32 *indexArray = (UINT32 *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, OS_ALL_IPC_QUEUE_LIMIT * sizeof(UINT32));

    for (index = 0; index < OS_ALL_IPC_QUEUE_LIMIT; index++) {
        queueCB = GET_QUEUE_HANDLE(index);
        SCHEDULER_LOCK(intSave);
        (VOID)memcpy(&queueNode, queueCB, sizeof(LosQueueCB));
        (VOID)memcpy(&queueDebugNode, &g_queueDebugArray[index], sizeof(QueueDebugCB));

        if (LOS_ListEmpty(&queueCB->readWriteList[OS_QUEUE_READ]) &&
            LOS_ListEmpty(&queueCB->readWriteList[OS_QUEUE_WRITE]) &&
            LOS_ListEmpty(&queueCB->memList)) {
            isPending = FALSE;
        } else {
            isPending = TRUE;
        }
        SCHEDULER_UNLOCK(intSave);
        if ((queueNode.queueState == OS_QUEUE_UNUSED) ||
            ((queueNode.queueState == OS_QUEUE_INUSED) && (queueDebugNode.creator == NULL))) {
            continue;
        }
        if ((queueNode.queueState == OS_QUEUE_INUSED) &&
            (queueNode.queueLen == queueNode.readWriteableCnt[OS_QUEUE_WRITE]) &&
            (isPending == FALSE)) {
            PRINTK("Queue ID <0x%x> may leak, No task uses it, "
                   "QueueLen is 0x%x, ",
                   queueNode.queueID,
                   queueNode.queueLen);
            OsQueueOpsOutput(&queueDebugNode);
        } else {
            if (indexArray != NULL) {
                *(indexArray + count) = index;
                count++;
            } else {
                OsQueueInfoOutPut(&queueNode);
                OsQueueOpsOutput(&queueDebugNode);
            }
        }
    }

    if (indexArray != NULL) {
        SortQueueIndexArray(indexArray, count);
    }

    return;
}

#if defined(LOSCFG_SHELL) && defined(LOSCFG_DEBUG_QUEUE)
LITE_OS_SEC_TEXT_MINOR UINT32 OsShellCmdQueueInfoGet(UINT32 argc, const CHAR **argv)
{
    if (argc > 0) {
        PRINTK("\nUsage: queue\n");
        return OS_ERROR;
    }
    PRINTK("used queues information: \n");
    OsQueueCheck();
    return LOS_OK;
}
#endif
#endif /* LOSCFG_DEBUG_QUEUE */
