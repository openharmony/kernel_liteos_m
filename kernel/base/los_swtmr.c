/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_swtmr_pri.h"
#include "los_init.h"
#include "securec.h"
#include "los_interrupt.h"
#include "los_task_pri.h"
#include "los_memory.h"
#include "los_queue.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_sched.h"
#include "los_sched_pri.h"

#ifdef LOSCFG_DEBUG_RESOURCE_INFO
#include "los_resource.h"
#endif

#ifdef LOSCFG_EXC_INTERACTION
#include "los_exc.h"
#endif

#ifdef LOSCFG_TRUSTZONE
#include "los_task.h"
#endif


#if (LOSCFG_BASE_CORE_SWTMR == 1)

#ifdef LOSCFG_DEBUG_RESOURCE_INFO
LITE_OS_SEC_BSS UINT8 g_swtmrUsed;
LITE_OS_SEC_BSS UINT8 g_swtmrPeak;
#endif

#if !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
LITE_OS_SEC_BSS UINT32            g_swtmrHandlerQueue;           /* Software Timer timeout queue ID */
#endif
LITE_OS_SEC_BSS SWTMR_CTRL_S      *g_swtmrCBArray = NULL;        /* first address in Timer memory space */
LITE_OS_SEC_BSS LOS_DL_LIST        g_swtmrFreeList;               /* Free list of Software Timer */

/* spinlock for swtmr module */
LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_swtmrSpin);

#if !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
#ifdef LOSCFG_TASK_STACK_STATIC_ALLOCATION
LITE_OS_SEC_BSS UINT8 *g_osSwtmrTaskStack[LOSCFG_KERNEL_CORE_NUM];
#endif
#endif

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
typedef struct SwtmrAlignDataStr {
    UINT32 times : 24;
    UINT32 : 5;
    UINT32 canMultiple : 1;
    UINT32 canAlign : 1;
    UINT32 isAligned : 1;
} SwtmrAlignData;
LITE_OS_SEC_BSS SwtmrAlignData      g_swtmrAlignID[LOSCFG_BASE_CORE_SWTMR_LIMIT] = {0};   /* store swtmr align */
#endif

LITE_OS_SEC_BSS STATIC LOS_DL_LIST g_swtmrSortlink[OS_TSK_SORTLINK_LEN];

#define SWTMR_MAX_RUNNING_TICKS 2
#define OS_SWTMR_MAX_TIMERID    ((0xFFFFFFFF / LOSCFG_BASE_CORE_SWTMR_LIMIT) * LOSCFG_BASE_CORE_SWTMR_LIMIT)

STATIC VOID OsSwtmrDelete(SWTMR_CTRL_S *swtmr);
STATIC UINT32 OsSwtmrDeleteOption(UINT32 swtmrId, BOOL isSync);

STATIC INLINE VOID OsSwtmrCheckSelfDelete(SWTMR_CTRL_S *swtmr)
{
    if ((swtmr->ucState == OS_SWTMR_STATUS_DELETING) && (swtmr->inProcess == 0)) {
        OsSwtmrDelete(swtmr);
    }
}

#if !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
/*****************************************************************************
Function    : OsSwtmrTask
Description : Swtmr task main loop, handle time-out timer.
Input       : None
Output      : None
Return      : None
*****************************************************************************/
LITE_OS_SEC_TEXT VOID OsSwtmrTask(VOID)
{
    SwtmrHandlerItem swtmrHandle;
#if defined(LOSCFG_TRUSTZONE) && defined(LOSCFG_SWTMR_ACCESS_SECURE)
    UINT32 ret = LOS_TaskAllocSecureContext(OsPercpuGet()->swtmrTaskId, LOSCFG_TSK_SWTMR_SECURE_STACK_SIZE);
    if (ret != LOS_OK) {
        PRINT_ERR("OsSwtmrTask alloc secure stack failed!\n");
        return;
    }
#endif
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 intSave;
    UINT32 readSize;
    UINT32 ret;
    UINT64 tick;

    for (;;) {
        readSize = sizeof(SwtmrHandlerItem);
        ret = LOS_QueueReadCopy(g_swtmrHandlerQueue, &swtmrHandle, &readSize, LOS_WAIT_FOREVER);
        if ((ret == LOS_OK) && (readSize == sizeof(SwtmrHandlerItem))) {
            if ((swtmrHandle.handler == NULL) || (swtmrHandle.swtmrID >= OS_SWTMR_MAX_TIMERID)) {
                continue;
            }

            SWTMR_LOCK(intSave);
            swtmr = g_swtmrCBArray + swtmrHandle.swtmrID % LOSCFG_BASE_CORE_SWTMR_LIMIT;
            if (swtmr->usTimerID != swtmrHandle.swtmrID) {
                SWTMR_UNLOCK(intSave);
                continue;
            }
            if (swtmr->ucMode == LOS_SWTMR_MODE_ONCE) {
                swtmr->ucState = OS_SWTMR_STATUS_DELETING;
            }
            SWTMR_UNLOCK(intSave);

            tick = LOS_TickCountGet();
            swtmrHandle.handler(swtmrHandle.arg);
            tick = LOS_TickCountGet() - tick;
            if (tick >= SWTMR_MAX_RUNNING_TICKS) {
                PRINT_WARN("timer_handler(%p) cost too many ms(%d)\n",
                           swtmrHandle.handler,
                           (UINT32)((tick * OS_SYS_MS_PER_SECOND) / LOSCFG_BASE_CORE_TICK_PER_SECOND));
            }

            SWTMR_LOCK(intSave);
            swtmr->inProcess--;
            OsSwtmrCheckSelfDelete(swtmr);
            SWTMR_UNLOCK(intSave);
        }
    }
}

/*****************************************************************************
Function    : OsSwtmrTaskCreate
Description : Create Software Timer
Input       : None
Output      : None
Return      : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsSwtmrTaskCreate(VOID)
{
    UINT32 ret;
    TSK_INIT_PARAM_S swtmrTask;

    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)memset_s(&swtmrTask, sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));

    swtmrTask.pfnTaskEntry    = (TSK_ENTRY_FUNC)OsSwtmrTask;
    swtmrTask.uwStackSize     = LOSCFG_BASE_CORE_TSK_SWTMR_STACK_SIZE;
    swtmrTask.pcName          = "Swt_Task";
    swtmrTask.usTaskPrio      = 0;
#ifdef LOSCFG_KERNEL_SMP
    swtmrTask.usCpuAffiMask = CPUID_TO_AFFI_MASK(ArchCurrCpuid());
#endif
    Percpu *percpu = OsPercpuGet();
#ifdef LOSCFG_TASK_STACK_STATIC_ALLOCATION
    ret = LOS_TaskCreateStatic(&percpu->swtmrTaskId, &swtmrTask, g_osSwtmrTaskStack[ArchCurrCpuid()]);
#else
    ret = LOS_TaskCreate(&percpu->swtmrTaskId, &swtmrTask);
#endif
    if (ret == LOS_OK) {
        OS_TCB_FROM_TID(percpu->swtmrTaskId)->taskFlags |= OS_TASK_FLAG_SYSTEM;
    }
    return ret;
}
#endif

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
STATIC UINT64 OsSwtmrCalcStartTime(UINT64 currTime, SWTMR_CTRL_S *swtmr, const SWTMR_CTRL_S *alignSwtmr)
{
    (void)currTime;
    UINT64 usedTime, startTime;
    UINT64 alignEnd = OS_SYS_TICK_TO_CYCLE(alignSwtmr->uwInterval);
    UINT64 swtmrTime = OS_SYS_TICK_TO_CYCLE(swtmr->uwInterval);
    UINT64 remainTime = OsSortLinkGetTargetExpireTime(&OsPercpuGet()->swtmrSortLink, &alignSwtmr->stSortList);
    if (remainTime == 0) {
        startTime = GET_SORTLIST_VALUE(&alignSwtmr->stSortList);
    } else {
        usedTime = alignEnd - remainTime;
        startTime = alignSwtmr->startTime + (usedTime / swtmrTime) * swtmrTime;
    }

    return startTime;
}

UINT64 OsSwtmrFindAlignPos(UINT64 currTime, SWTMR_CTRL_S *swtmr)
{
    SWTMR_CTRL_S *minInLarge = (SWTMR_CTRL_S *)NULL;
    SWTMR_CTRL_S *maxInLittle = (SWTMR_CTRL_S *)NULL;
    UINT32 minInLargeVal = OS_NULL_INT;
    UINT32 maxInLittleVal = OS_NULL_INT;
    SwtmrAlignData swtmrAlgInfo = g_swtmrAlignID[swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT];
    SortLinkAttribute *sortLinkHead = &OsPercpuGet()->swtmrSortLink;

    for (UINT32 bucket = 0; bucket < OS_TSK_SORTLINK_LEN; bucket++) {
        LOS_DL_LIST *listHead = sortLinkHead->sortLink + bucket;
        if (LOS_ListEmpty(listHead)) {
            continue;
        }

        LOS_DL_LIST *listObject = listHead->pstNext;
        do {
            SortLinkList *sortList = LOS_DL_LIST_ENTRY(listObject, SortLinkList, sortLinkNode);
            SWTMR_CTRL_S *swtmrListNode = LOS_DL_LIST_ENTRY(sortList, SWTMR_CTRL_S, stSortList);
            SwtmrAlignData alignListNode = g_swtmrAlignID[swtmrListNode->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT];

            /* swtmr not start */
            if ((alignListNode.isAligned == 0) || (alignListNode.canAlign == 0)) {
                goto CONTINUE_NEXT_NODE;
            }

            /* find same interval timer, directly return */
            if (swtmrListNode->uwInterval == swtmr->uwInterval) {
                return OsSwtmrCalcStartTime(currTime, swtmr, swtmrListNode);
            }

            if ((swtmrAlgInfo.canMultiple != 1) || (alignListNode.times == 0)) {
                goto CONTINUE_NEXT_NODE;
            }

            if (swtmrAlgInfo.times == 0) {
                goto RETURN_PERIOD;
            }

            if ((alignListNode.times >= swtmrAlgInfo.times) && ((alignListNode.times % swtmrAlgInfo.times) == 0)) {
                if (minInLargeVal > (alignListNode.times / swtmrAlgInfo.times)) {
                    minInLargeVal = alignListNode.times / swtmrAlgInfo.times;
                    minInLarge = swtmrListNode;
                }
            } else if ((alignListNode.times < swtmrAlgInfo.times) && ((swtmrAlgInfo.times % alignListNode.times) == 0)) {
                if (maxInLittleVal > (swtmrAlgInfo.times / alignListNode.times)) {
                    maxInLittleVal = swtmrAlgInfo.times / alignListNode.times;
                    maxInLittle = swtmrListNode;
                }
            }

CONTINUE_NEXT_NODE:
            listObject = listObject->pstNext;
        } while (listObject != listHead);
    }

    if (minInLarge != NULL) {
        return OsSwtmrCalcStartTime(currTime, swtmr, minInLarge);
    } else if (maxInLittle != NULL) {
        return OsSwtmrCalcStartTime(currTime, swtmr, maxInLittle);
    }

RETURN_PERIOD:
    return currTime;
}
#endif

/*****************************************************************************
Function    : OsSwtmrStart
Description : Start Software Timer
Input       : currTime ------- Current system time
Input       : swtmr ---------- Need to start Software Timer
Output      : None
Return      : None
*****************************************************************************/
LITE_OS_SEC_TEXT VOID OsSwtmrStart(UINT64 currTime, SWTMR_CTRL_S *swtmr)
{
    swtmr->ucState = OS_SWTMR_STATUS_TICKING;

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    if ((g_swtmrAlignID[swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT].canAlign == 1) &&
        (g_swtmrAlignID[swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT].isAligned == 0)) {
        g_swtmrAlignID[swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT].isAligned = 1;
        swtmr->startTime = OsSwtmrFindAlignPos(currTime, swtmr);
    }
#endif
    SortLinkAttribute *swtmrSortLink = &OsPercpuGet()->swtmrSortLink;
    OsSchedAddSortLink(swtmrSortLink, &swtmr->stSortList, swtmr->uwInterval);
    OsSchedUpdateExpireTime();

#ifdef LOSCFG_KERNEL_SMP
    swtmr->cpuid = ArchCurrCpuid();
#endif
}

/*****************************************************************************
Function    : OsSwtmrDelete
Description : Delete Software Timer
Input       : swtmr --- Need to delete Software Timer, When using, Ensure that it can't be NULL.
Output      : None
Return      : None
*****************************************************************************/
STATIC VOID OsSwtmrDelete(SWTMR_CTRL_S *swtmr)
{
    if (swtmr->usTimerID < (OS_SWTMR_MAX_TIMERID - LOSCFG_BASE_CORE_SWTMR_LIMIT)) {
        swtmr->usTimerID += LOSCFG_BASE_CORE_SWTMR_LIMIT;
    } else {
        swtmr->usTimerID %= LOSCFG_BASE_CORE_SWTMR_LIMIT;
    }

    /* insert to free list */
    LOS_ListTailInsert(&g_swtmrFreeList, &swtmr->stSortList.sortLinkNode);
    swtmr->ucState = OS_SWTMR_STATUS_UNUSED;

#ifdef LOSCFG_DEBUG_RESOURCE_INFO
    g_swtmrUsed -= 1;
#endif

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    (VOID)memset_s((VOID *)&g_swtmrAlignID[swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT],
                   sizeof(SwtmrAlignData), 0, sizeof(SwtmrAlignData));
#endif
}


LITE_OS_SEC_TEXT VOID OsSwtmrStop(SWTMR_CTRL_S *swtmr)
{
    SortLinkAttribute *swtmrSortLink = &OsPercpuGet()->swtmrSortLink;
    OsSchedDeleteSortLink(swtmrSortLink, &swtmr->stSortList);
    swtmr->ucState = OS_SWTMR_STATUS_CREATED;

    swtmr->ucOverrun = 0;
    OsSchedResetSchedResponseTime(0);
    OsSchedUpdateExpireTime();
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    g_swtmrAlignID[swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT].isAligned = 0;
#endif
}

STATIC VOID OsSwtmrTimeoutHandle(UINT64 currTime, SWTMR_CTRL_S *swtmr, UINT32 *intSave)
{
#if defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
    /* In ISR mode, the timeout handler is executed directly in the tick
     * interrupt context instead of being queued to the software timer task.
     * Refer to hs-fbb: update the timer state first, then run the handler
     * with the swtmr spinlock released, and finally handle deferred delete. */
    SWTMR_PROC_FUNC handler = swtmr->pfnHandler;
    UINT32 arg = swtmr->uwArg;

    if (swtmr->ucMode == LOS_SWTMR_MODE_ONCE) {
        swtmr->ucState = OS_SWTMR_STATUS_DELETING;
    } else if (swtmr->ucMode == LOS_SWTMR_MODE_NO_SELFDELETE) {
        swtmr->ucState = OS_SWTMR_STATUS_CREATED;
    } else {
        swtmr->ucOverrun++;
        OsSwtmrStart(currTime, swtmr);
    }

    swtmr->inProcess++;
    SWTMR_UNLOCK(*intSave);
    if (handler != NULL) {
        handler(arg);
    }
    SWTMR_LOCK(*intSave);
    swtmr->inProcess--;
    OsSwtmrCheckSelfDelete(swtmr);
#else
    SwtmrHandlerItem swtmrHandler;

    (VOID)intSave;
    swtmr->inProcess++;
    swtmrHandler.handler = swtmr->pfnHandler;
    swtmrHandler.arg = swtmr->uwArg;
    swtmrHandler.swtmrID = swtmr->usTimerID;

    (VOID)LOS_QueueWriteCopy(g_swtmrHandlerQueue, &swtmrHandler, sizeof(SwtmrHandlerItem), LOS_NO_WAIT);
    if (swtmr->ucMode == LOS_SWTMR_MODE_PERIOD) {
        swtmr->ucOverrun++;
        OsSwtmrStart(currTime, swtmr);
    } else if (swtmr->ucMode == LOS_SWTMR_MODE_NO_SELFDELETE) {
        swtmr->ucState = OS_SWTMR_STATUS_CREATED;
    }
#endif
}

LITE_OS_SEC_TEXT BOOL OsSwtmrScan(VOID)
{
    BOOL needSchedule = FALSE;
    SortLinkAttribute *sortLinkHead = &OsPercpuGet()->swtmrSortLink;
    LOS_DL_LIST *listObject = NULL;
    UINT32 intSave;

    SWTMR_LOCK(intSave);
    OsTickScanAdvance(sortLinkHead);
    SORTLINK_LISTOBJ_GET(listObject, sortLinkHead);
    if (LOS_ListEmpty(listObject)) {
        SWTMR_UNLOCK(intSave);
        return needSchedule;
    }

    SortLinkList *sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    while (ROLLNUM(sortList->idxRollNum) == 0) {
        SWTMR_CTRL_S *swtmr = LOS_DL_LIST_ENTRY(sortList, SWTMR_CTRL_S, stSortList);
        swtmr->startTime = GET_SORTLIST_VALUE(sortList);

        OsDeleteNodeSortLink(sortList);
        OsHookCall(LOS_HOOK_TYPE_SWTMR_EXPIRED, swtmr);
        OsSwtmrTimeoutHandle(OsGetCurrSchedTimeCycle(), swtmr, &intSave);

        needSchedule = TRUE;
        if (LOS_ListEmpty(listObject)) {
            break;
        }

        sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    }
    SWTMR_UNLOCK(intSave);

    return needSchedule;
}

LITE_OS_SEC_TEXT VOID OsSwtmrResponseTimeReset(UINT64 startTime)
{
    SortLinkAttribute *sortLinkHead = &OsPercpuGet()->swtmrSortLink;

    for (UINT32 bucket = 0; bucket < OS_TSK_SORTLINK_LEN; bucket++) {
        LOS_DL_LIST *listHead = sortLinkHead->sortLink + bucket;
        LOS_DL_LIST *listNext = listHead->pstNext;

        while (listNext != listHead) {
            SortLinkList *sortList = LOS_DL_LIST_ENTRY(listNext, SortLinkList, sortLinkNode);
            SWTMR_CTRL_S *swtmr = LOS_DL_LIST_ENTRY(sortList, SWTMR_CTRL_S, stSortList);
            OsDeleteNodeSortLink(sortList);
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
            g_swtmrAlignID[swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT].isAligned = 0;
#endif
            swtmr->startTime = startTime;
            OsSwtmrStart(startTime, swtmr);
            listNext = listNext->pstNext;
        }
    }
}

STATIC INLINE UINT32 OsSwtmrRollToTick(UINT64 roll)
{
#ifdef LOSCFG_KERNEL_TICKLESS_GLOBAL
    roll = OS_SYS_CYCLE_TO_TICK(roll);
#endif
    if (roll > OS_NULL_INT) {
        return OS_NULL_INT;
    }
    return (UINT32)roll;
}

/*****************************************************************************
Function    : OsSwtmrGetNextTimeout
Description : Get next timeout
Input       : None
Output      : None
Return      : Count of the Timer list
*****************************************************************************/
LITE_OS_SEC_TEXT UINT32 OsSwtmrGetNextTimeout(VOID)
{
    UINT32 intSave;
    SWTMR_LOCK(intSave);
    UINT64 time = OsSortLinkGetNextExpireTime(&OsPercpuGet()->swtmrSortLink);
    SWTMR_UNLOCK(intSave);
    return OsSwtmrRollToTick(time);
}

LITE_OS_SEC_TEXT UINT32 OsSwtmrTimeGet(const SWTMR_CTRL_S *swtmr)
{
    UINT64 time = OsSortLinkGetTargetExpireTime(&OsPercpuGet()->swtmrSortLink, &swtmr->stSortList);
    return OsSwtmrRollToTick(time);
}

#ifdef LOSCFG_EXC_INTERACTION
BOOL IsSwtmrTask(UINT32 taskId)
{
    return (taskId == OsPercpuGet()->swtmrTaskId);
}
#endif

/*****************************************************************************
Function    : OsSwtmrInit
Description : Initializes Software Timer
Input       : None
Output      : None
Return      : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsSwtmrInit(VOID)
{
    UINT32 size;
    UINT16 index;
    UINT32 ret;

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    // Ignore the return code when matching CSEC rule 6.6(1).
    (VOID)memset_s((VOID *)g_swtmrAlignID, sizeof(SwtmrAlignData) * LOSCFG_BASE_CORE_SWTMR_LIMIT,
                   0, sizeof(SwtmrAlignData) * LOSCFG_BASE_CORE_SWTMR_LIMIT);
#endif

    size = sizeof(SWTMR_CTRL_S) * LOSCFG_BASE_CORE_SWTMR_LIMIT;
    SWTMR_CTRL_S *swtmr = (SWTMR_CTRL_S *)LOS_MemAlloc(m_aucSysMem0, size);
    if (swtmr == NULL) {
        return LOS_ERRNO_SWTMR_NO_MEMORY;
    }
    // Ignore the return code when matching CSEC rule 6.6(3).
    (VOID)memset_s((VOID *)swtmr, size, 0, size);
    g_swtmrCBArray = swtmr;
    LOS_ListInit(&g_swtmrFreeList);
    for (index = 0; index < LOSCFG_BASE_CORE_SWTMR_LIMIT; index++, swtmr++) {
        swtmr->usTimerID = index;
        LOS_ListTailInsert(&g_swtmrFreeList, &swtmr->stSortList.sortLinkNode);
    }

#if !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
    ret = LOS_QueueCreate((CHAR *)NULL, OS_SWTMR_HANDLE_QUEUE_SIZE,
                          &g_swtmrHandlerQueue, 0, sizeof(SwtmrHandlerItem));
    if (ret != LOS_OK) {
        (VOID)LOS_MemFree(m_aucSysMem0, swtmr);
        return LOS_ERRNO_SWTMR_QUEUE_CREATE_FAILED;
    }

    ret = OsSwtmrTaskCreate();
    if (ret != LOS_OK) {
        (VOID)LOS_MemFree(m_aucSysMem0, swtmr);
        return LOS_ERRNO_SWTMR_TASK_CREATE_FAILED;
    }
#else
    /* No queue or timer task in ISR mode, the timeout handlers are executed
     * directly in the tick interrupt context. Task-id checks in los_task.c
     * are compiled out for ISR mode, so the swtmr task id is never used. */
#endif

    ret = OsSortLinkInit(&OsPercpuGet()->swtmrSortLink, g_swtmrSortlink);
    if (ret != LOS_OK) {
        (VOID)LOS_MemFree(m_aucSysMem0, swtmr);
        return LOS_NOK;
    }

    ret = OsSchedSwtmrScanRegister((SchedScan)OsSwtmrScan);
    if (ret != LOS_OK) {
        (VOID)LOS_MemFree(m_aucSysMem0, swtmr);
        return LOS_NOK;
    }

    return LOS_OK;
}
LOS_SYS_INIT(OsSwtmrInit, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_3);

/*****************************************************************************
Function    : OsSwtmrCreate
Description : Create software timer (internal, 5-param core implementation)
Input       : interval
              mode
              handler
              arg
Output      : swtmrId
Return      : LOS_OK on success or error code on failure
*****************************************************************************/
STATIC LITE_OS_SEC_TEXT_INIT UINT32 OsSwtmrCreate(UINT32 interval,
                                                   UINT8 mode,
                                                   SWTMR_PROC_FUNC handler,
                                                   UINT32 *swtmrId,
                                                   UINT32 arg
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
                                                   , UINT8 rouses, UINT8 sensitive
#endif
                                                   )
{
    SWTMR_CTRL_S *swtmr = NULL;
    SortLinkList *sortList = NULL;
    UINT32 intSave;

    if (interval == 0) {
        return LOS_ERRNO_SWTMR_INTERVAL_NOT_SUITED;
    }

    if ((mode != LOS_SWTMR_MODE_ONCE) &&
        (mode != LOS_SWTMR_MODE_PERIOD) &&
        (mode != LOS_SWTMR_MODE_NO_SELFDELETE)) {
        return LOS_ERRNO_SWTMR_MODE_INVALID;
    }

    if (handler == NULL) {
        return LOS_ERRNO_SWTMR_PTR_NULL;
    }

    if (swtmrId == NULL) {
        return LOS_ERRNO_SWTMR_RET_PTR_NULL;
    }

    SWTMR_LOCK(intSave);
    if (LOS_ListEmpty(&g_swtmrFreeList)) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_MAXSIZE;
    }

    sortList = LOS_DL_LIST_ENTRY(g_swtmrFreeList.pstNext, SortLinkList, sortLinkNode);
    swtmr = LOS_DL_LIST_ENTRY(sortList, SWTMR_CTRL_S, stSortList);
    LOS_ListDelete(LOS_DL_LIST_FIRST(&g_swtmrFreeList));

    swtmr->pfnHandler    = handler;
    swtmr->ucMode        = mode;
    swtmr->ucOverrun     = 0;
    swtmr->uwInterval    = interval;
    swtmr->expiry        = interval;
    swtmr->uwArg         = arg;
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    swtmr->ucRouses      = rouses;
    swtmr->ucSensitive   = sensitive;
#endif
    swtmr->ucState       = OS_SWTMR_STATUS_CREATED;
    swtmr->inProcess     = 0;
    swtmr->pstNext       = (SWTMR_CTRL_S *)NULL;
    SET_SORTLIST_VALUE(&swtmr->stSortList, OS_SORT_LINK_INVALID_TIME);
    *swtmrId = swtmr->usTimerID;
    SWTMR_UNLOCK(intSave);
#ifdef LOSCFG_DEBUG_RESOURCE_INFO
    g_swtmrUsed += 1;
    if (g_swtmrUsed > g_swtmrPeak) {
        g_swtmrPeak = g_swtmrUsed;
    }
#endif
    OsHookCall(LOS_HOOK_TYPE_SWTMR_CREATE, swtmr);
    return LOS_OK;
}

/*****************************************************************************
Function    : LOS_SwtmrCreate
Description : Create software timer (public API, delegates to OsSwtmrCreate)
Input       : interval
              mode
              handler
              arg
              rouses (ALIGN only)
              sensitive (ALIGN only)
Output      : swtmrId
Return      : LOS_OK on success or error code on failure
*****************************************************************************/
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
LITE_OS_SEC_TEXT_INIT UINT32 LOS_SwtmrCreate(UINT32 interval,
                                             UINT8 mode,
                                             SWTMR_PROC_FUNC handler,
                                             UINT32 *swtmrId,
                                             UINT32 arg,
                                             UINT8 rouses,
                                             UINT8 sensitive)
#else
LITE_OS_SEC_TEXT_INIT UINT32 LOS_SwtmrCreate(UINT32 interval,
                                             UINT8 mode,
                                             SWTMR_PROC_FUNC handler,
                                             UINT32 *swtmrId,
                                             UINT32 arg)
#endif
{
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    if ((rouses != OS_SWTMR_ROUSES_IGNORE) && (rouses != OS_SWTMR_ROUSES_ALLOW)) {
        return OS_ERRNO_SWTMR_ROUSES_INVALID;
    }

    if ((sensitive != OS_SWTMR_ALIGN_INSENSITIVE) && (sensitive != OS_SWTMR_ALIGN_SENSITIVE)) {
        return OS_ERRNO_SWTMR_ALIGN_INVALID;
    }

    return OsSwtmrCreate(interval, mode, handler, swtmrId, arg, rouses, sensitive);
#else
    return OsSwtmrCreate(interval, mode, handler, swtmrId, arg);
#endif
}

/*****************************************************************************
Function    : LOS_SwtmrStart
Description : Start software timer
Input       : swtmrId ------- Software timer ID
Output      : None
Return      : LOS_OK on success or error code on failure
*****************************************************************************/
SWTMR_CTRL_S *OsSwtmrIdGet(UINT32 swtmrId)
{
    SWTMR_CTRL_S *swtmr = NULL;
    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return NULL;
    }
    swtmr = g_swtmrCBArray + (swtmrId % LOSCFG_BASE_CORE_SWTMR_LIMIT);
    if (swtmr->usTimerID != swtmrId) {
        return NULL;
    }
    return swtmr;
}

BOOL OsSwtmrIdVerify(UINT32 swtmrId)
{
    return (OsSwtmrIdGet(swtmrId) != NULL);
}

#ifdef LOSCFG_COMPAT_POSIX
STATIC INLINE VOID OsSwtmrSetTimerParms(SWTMR_CTRL_S *swtmr, UINT32 interval, UINT32 expiry)
{
    if (expiry > 0) {
        swtmr->expiry = expiry;
        swtmr->uwInterval = interval;
        if (interval == 0) {
            swtmr->ucMode = LOS_SWTMR_MODE_NO_SELFDELETE;
        } else {
            swtmr->ucMode = LOS_SWTMR_MODE_OPP;
        }
        return;
    }
}
#elif defined(LOSCFG_COMPAT_CMSIS) || defined(LOSCFG_COMPAT_FREERTOS)
STATIC INLINE VOID OsSwtmrSetTimerParms(SWTMR_CTRL_S *swtmr, UINT32 interval, UINT32 expiry)
{
    (VOID)expiry;
    if (interval > 0) {
        swtmr->uwInterval = interval;
        swtmr->expiry = interval;
    }
}
#else
STATIC INLINE VOID OsSwtmrSetTimerParms(SWTMR_CTRL_S *swtmr, UINT32 interval, UINT32 expiry)
{
    (VOID)swtmr;
    (VOID)interval;
    (VOID)expiry;
}
#endif

LITE_OS_SEC_TEXT UINT32 OsSwtmrStartTimer(UINT32 swtmrId, UINT32 interval, UINT32 expiry)
{
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 intSave;
    UINT32 ret = LOS_OK;

    SWTMR_LOCK(intSave);
    swtmr = OsSwtmrIdGet(swtmrId);
    if (swtmr == NULL) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    switch (swtmr->ucState) {
        case OS_SWTMR_STATUS_UNUSED:
            /* fall-through */
        case OS_SWTMR_STATUS_DELETING:
            ret = LOS_ERRNO_SWTMR_NOT_CREATED;
            break;
        /*
         * If the state of swtmr is ticking, it should stop the swtmr first,
         * then start the swtmr again.
         */
        case OS_SWTMR_STATUS_TICKING:
            OsSwtmrStop(swtmr);
            /* fall-through */
        case OS_SWTMR_STATUS_CREATED:
            OsSwtmrSetTimerParms(swtmr, interval, expiry);
            swtmr->startTime = OsGetCurrSchedTimeCycle();
            OsSwtmrStart(swtmr->startTime, swtmr);
            break;
        default:
            ret = LOS_ERRNO_SWTMR_STATUS_INVALID;
            break;
    }

    SWTMR_UNLOCK(intSave);
    OsHookCall(LOS_HOOK_TYPE_SWTMR_START, swtmr);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_SwtmrStart(UINT32 swtmrId)
{
    UINT32 intSave;
    UINT32 ret = LOS_OK;

    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    SWTMR_LOCK(intSave);
    SWTMR_CTRL_S *swtmr = g_swtmrCBArray + swtmrId % LOSCFG_BASE_CORE_SWTMR_LIMIT;
    if (swtmr->usTimerID != swtmrId) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_NOT_CREATED;
    }

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    if ((swtmr->ucSensitive == OS_SWTMR_ALIGN_INSENSITIVE) && (swtmr->ucMode == LOS_SWTMR_MODE_PERIOD)) {
        UINT32 swtmrAlignIdIndex = swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT;
        g_swtmrAlignID[swtmrAlignIdIndex].canAlign = 1;
        if ((swtmr->uwInterval % LOS_COMMON_DIVISOR) == 0) {
            g_swtmrAlignID[swtmrAlignIdIndex].canMultiple = 1;
            g_swtmrAlignID[swtmrAlignIdIndex].times = swtmr->uwInterval / LOS_COMMON_DIVISOR;
        }
    }
#endif

    switch (swtmr->ucState) {
        case OS_SWTMR_STATUS_UNUSED:
        case OS_SWTMR_STATUS_DELETING:
            ret = LOS_ERRNO_SWTMR_NOT_CREATED;
            break;
        case OS_SWTMR_STATUS_TICKING:
            OsSwtmrStop(swtmr);
            /* fall through */
        case OS_SWTMR_STATUS_CREATED:
            swtmr->startTime = OsGetCurrSchedTimeCycle();
            OsSwtmrStart(swtmr->startTime, swtmr);
            break;
        default:
            ret = LOS_ERRNO_SWTMR_STATUS_INVALID;
            break;
    }

    SWTMR_UNLOCK(intSave);
    OsHookCall(LOS_HOOK_TYPE_SWTMR_START, swtmr);
    return ret;
}

/*****************************************************************************
Function    : LOS_SwtmrStop
Description : Stop software timer
Input       : swtmrId ------- Software timer ID
Output      : None
Return      : LOS_OK on success or error code on failure
*****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_SwtmrStop(UINT32 swtmrId)
{
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 intSave;
    UINT16 swtmrCbId;
    UINT32 ret = LOS_OK;

    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }
    SWTMR_LOCK(intSave);
    swtmrCbId = swtmrId % LOSCFG_BASE_CORE_SWTMR_LIMIT;
    swtmr = g_swtmrCBArray + swtmrCbId;
    if (swtmr->usTimerID != swtmrId) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_NOT_CREATED;
    }

    switch (swtmr->ucState) {
        case OS_SWTMR_STATUS_UNUSED:
        case OS_SWTMR_STATUS_DELETING:
            ret = LOS_ERRNO_SWTMR_NOT_CREATED;
            break;
        case OS_SWTMR_STATUS_CREATED:
            ret = LOS_ERRNO_SWTMR_NOT_STARTED;
            break;
        case OS_SWTMR_STATUS_TICKING:
            OsSwtmrStop(swtmr);
            break;
        default:
            ret = LOS_ERRNO_SWTMR_STATUS_INVALID;
            break;
    }

    SWTMR_UNLOCK(intSave);
    OsHookCall(LOS_HOOK_TYPE_SWTMR_STOP, swtmr);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_SwtmrTimeGet(UINT32 swtmrId, UINT32 *tick)
{
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 intSave;
    UINT32 ret = LOS_OK;
    UINT16 swtmrCbId;

    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    if (tick == NULL) {
        return LOS_ERRNO_SWTMR_TICK_PTR_NULL;
    }

    SWTMR_LOCK(intSave);
    swtmrCbId = swtmrId % LOSCFG_BASE_CORE_SWTMR_LIMIT;
    swtmr = g_swtmrCBArray + swtmrCbId;
    if (swtmr->usTimerID != swtmrId) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_NOT_CREATED;
    }
    switch (swtmr->ucState) {
        case OS_SWTMR_STATUS_UNUSED:
        case OS_SWTMR_STATUS_DELETING:
            ret = LOS_ERRNO_SWTMR_NOT_CREATED;
            break;
        case OS_SWTMR_STATUS_CREATED:
            ret = LOS_ERRNO_SWTMR_NOT_STARTED;
            break;
        case OS_SWTMR_STATUS_TICKING:
            *tick = OsSwtmrTimeGet(swtmr);
            break;
        default:
            ret = LOS_ERRNO_SWTMR_STATUS_INVALID;
            break;
    }
    SWTMR_UNLOCK(intSave);
    return ret;
}

/*****************************************************************************
Function    : LOS_SwtmrDelete
Description : Delete software timer
Input       : swtmrId ------- Software timer ID
Output      : None
Return      : LOS_OK on success or error code on failure
*****************************************************************************/
STATIC UINT32 OsSwtmrDeleteOption(UINT32 swtmrId, BOOL isSync)
{
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 intSave;
    UINT32 ret = LOS_OK;
    UINT16 swtmrCbId;

    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }
    SWTMR_LOCK(intSave);
    swtmrCbId = swtmrId % LOSCFG_BASE_CORE_SWTMR_LIMIT;
    swtmr = g_swtmrCBArray + swtmrCbId;
    if (swtmr->usTimerID != swtmrId) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_NOT_CREATED;
    }

    switch (swtmr->ucState) {
        case OS_SWTMR_STATUS_UNUSED:
#ifndef LOSCFG_SWTMR_SYNC_DELETE
            /* fall-through */
        case OS_SWTMR_STATUS_DELETING:
#endif
            ret = LOS_ERRNO_SWTMR_NOT_CREATED;
            break;
        case OS_SWTMR_STATUS_TICKING:
            OsSwtmrStop(swtmr);
            /* fall-through */
        case OS_SWTMR_STATUS_CREATED:
            swtmr->ucState = OS_SWTMR_STATUS_DELETING;
#ifdef LOSCFG_SWTMR_SYNC_DELETE
            /* fall-through */
        case OS_SWTMR_STATUS_DELETING:
            if (isSync) {
                while (swtmr->inProcess != 0) {
                    SWTMR_UNLOCK(intSave);
                    LOS_TaskDelay(1);
                    SWTMR_LOCK(intSave);
                }
                if (swtmr->usTimerID != swtmrId) {
                    goto EXIT;
                }
            } else {
#endif
                (VOID)isSync;
                if (swtmr->inProcess != 0) {
                    goto EXIT;
                }
#ifdef LOSCFG_SWTMR_SYNC_DELETE
            }
#endif
            OsSwtmrDelete(swtmr);
            break;
        default:
            ret = LOS_ERRNO_SWTMR_STATUS_INVALID;
            break;
    }

EXIT:
    SWTMR_UNLOCK(intSave);
    OsHookCall(LOS_HOOK_TYPE_SWTMR_DELETE, swtmr);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_SwtmrDelete(UINT32 swtmrId)
{
    return OsSwtmrDeleteOption(swtmrId, FALSE);
}

#ifdef LOSCFG_SWTMR_SYNC_DELETE
LITE_OS_SEC_TEXT UINT32 LOS_SwtmrSyncDelete(UINT32 swtmrId)
{
#if defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
    if (OS_INT_ACTIVE) {
#else
    if (OS_INT_ACTIVE || (LOS_CurTaskIDGet() == OsPercpuGet()->swtmrTaskId)) {
#endif
        return LOS_ERRNO_SWTMR_INVALID_SYNCDEL;
    }
    return OsSwtmrDeleteOption(swtmrId, TRUE);
}
#endif /* LOSCFG_SWTMR_SYNC_DELETE */

#endif /* (LOSCFG_BASE_CORE_SWTMR == 1) */
