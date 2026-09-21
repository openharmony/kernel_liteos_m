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

LITE_OS_SEC_BSS LosSwtmrCB *g_osSwtmrCBArray = NULL;        /* First address in Timer memory space */
#ifndef LOSCFG_BASE_CORE_SWTMR_IN_ISR
LITE_OS_SEC_BSS UINT8 *g_osSwtmrTaskStack[LOSCFG_KERNEL_CORE_NUM];
#endif
LITE_OS_SEC_BSS LOS_DL_LIST     g_swtmrFreeList;            /* Free list of Software Timer */

/* spinlock for swtmr module, only available on SMP mode */
LITE_OS_SEC_BSS  SPIN_LOCK_INIT(g_swtmrSpin);
#define SWTMR_LOCK(state)       LOS_SpinLockSave(&g_swtmrSpin, &(state))
#define SWTMR_UNLOCK(state)     LOS_SpinUnlockRestore(&g_swtmrSpin, (state))

LITE_OS_SEC_BSS STATIC LOS_DL_LIST g_swtmrSortlink[OS_TSK_SORTLINK_LEN];

#define SWTMR_MAX_RUNNING_TICKS 2
#define OS_SWTMR_MAX_TIMERID    ((0xFFFFFFFF / LOSCFG_BASE_CORE_SWTMR_LIMIT) * LOSCFG_BASE_CORE_SWTMR_LIMIT)

STATIC VOID OsSwtmrDelete(SWTMR_CTRL_S *swtmr);
STATIC UINT32 OsSwtmrDeleteOption(UINT32 swtmrId, BOOL isSync);

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
STATIC INLINE UINT32 OsSwtmrCalcAlignWait(SWTMR_CTRL_S *swtmr, const SWTMR_CTRL_S *alignSwtmr)
{
    SortLinkAttribute *sortLinkHead = &OsPercpuGet()->swtmrSortLink;
    UINT64 remainTime = OsSortLinkGetTargetExpireTime(sortLinkHead, &alignSwtmr->sortList);
    if (remainTime == 0) {
        return swtmr->interval;
    }

#ifdef LOSCFG_KERNEL_TICKLESS_GLOBAL
    remainTime = OS_SYS_CYCLE_TO_TICK(remainTime);
#endif

    UINT32 waitTime = (UINT32)remainTime;
    if (alignSwtmr->interval < swtmr->interval) {
        UINT32 used = alignSwtmr->interval - waitTime;
        return swtmr->interval - used;
    } else if (alignSwtmr->interval > swtmr->interval) {
        if (waitTime % swtmr->interval == 0) {
            return swtmr->interval;
        }
        return waitTime % swtmr->interval;
    }
    return waitTime;
}

STATIC UINT32 OsSwtmrFindAlignTime(SWTMR_CTRL_S *swtmr)
{
    SWTMR_CTRL_S *minInLarge = (SWTMR_CTRL_S *)NULL;
    SWTMR_CTRL_S *maxInLittle = (SWTMR_CTRL_S *)NULL;
    UINT32 minInLargeVal = OS_NULL_INT;
    UINT32 maxInLittleVal = OS_NULL_INT;
    SortLinkAttribute *sortLinkHead = &OsPercpuGet()->swtmrSortLink;

    OsSchedSortLinkAdvance(sortLinkHead);

    for (UINT32 bucket = 0; bucket < OS_TSK_SORTLINK_LEN; bucket++) {
        LOS_DL_LIST *listHead = sortLinkHead->sortLink + bucket;
        if (LOS_ListEmpty(listHead)) {
            continue;
        }

        LOS_DL_LIST *listObject = listHead->pstNext;
        do {
            SortLinkList *sortList = LOS_DL_LIST_ENTRY(listObject, SortLinkList, sortLinkNode);
            SWTMR_CTRL_S *swtmrListNode = LOS_DL_LIST_ENTRY(sortList, SWTMR_CTRL_S, sortList);

            if ((swtmrListNode->isAligned == 0) || (swtmrListNode->alignEnable == 0)) {
                goto CONTINUE_NEXT_NODE;
            }

            if (swtmrListNode->interval == swtmr->interval) {
                return OsSwtmrCalcAlignWait(swtmr, swtmrListNode);
            }

            if ((swtmr->canMultiple != 1) || (swtmrListNode->times == 0)) {
                goto CONTINUE_NEXT_NODE;
            }

            if (swtmr->times == 0) {
                goto RETURN_PERIOD;
            }

            if ((swtmrListNode->times >= swtmr->times) && ((swtmrListNode->times % swtmr->times) == 0)) {
                if (minInLargeVal > (swtmrListNode->times / swtmr->times)) {
                    minInLargeVal = swtmrListNode->times / swtmr->times;
                    minInLarge = swtmrListNode;
                }
            } else if ((swtmrListNode->times < swtmr->times) && ((swtmr->times % swtmrListNode->times) == 0)) {
                if (maxInLittleVal > (swtmr->times / swtmrListNode->times)) {
                    maxInLittleVal = swtmr->times / swtmrListNode->times;
                    maxInLittle = swtmrListNode;
                }
            }

CONTINUE_NEXT_NODE:
            listObject = listObject->pstNext;
        } while (listObject != listHead);
    }

    if (minInLarge != NULL) {
        return OsSwtmrCalcAlignWait(swtmr, minInLarge);
    } else if (maxInLittle != NULL) {
        return OsSwtmrCalcAlignWait(swtmr, maxInLittle);
    }

RETURN_PERIOD:
    return swtmr->interval;
}
#endif

/*
 * Description: Start Software Timer
 * Input      : swtmr --- Need to start software timer
 */
LITE_OS_SEC_TEXT VOID OsSwtmrStart(LosSwtmrCB *swtmr)
{
    swtmr->state = OS_SWTMR_STATUS_TICKING;

    UINT32 waitTicks = swtmr->interval;
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    if ((swtmr->alignEnable == 1) && (swtmr->isAligned == 0)) {
        swtmr->isAligned = 1;
        waitTicks = OsSwtmrFindAlignTime(swtmr);
    }
#endif
    SortLinkAttribute *swtmrSortLink = &OsPercpuGet()->swtmrSortLink;
    OsSchedAddSortLink(swtmrSortLink, &swtmr->sortList, waitTicks);
    OsSchedUpdateExpireTime();

#ifdef LOSCFG_KERNEL_SMP
    swtmr->cpuid = ArchCurrCpuid();
#endif
}

STATIC INLINE VOID OsSwtmrDelete(LosSwtmrCB *swtmr)
{
#ifdef LOSCFG_BASE_CORE_SYS_RES_CHECK
    if (swtmr->timerId < (OS_SWTMR_MAX_TIMERID - LOSCFG_BASE_CORE_SWTMR_LIMIT)) {
        swtmr->timerId += LOSCFG_BASE_CORE_SWTMR_LIMIT;
    } else {
        swtmr->timerId %= LOSCFG_BASE_CORE_SWTMR_LIMIT;
    }
#endif
    /* insert to free list */
    LOS_ListTailInsert(&g_swtmrFreeList, &swtmr->sortList.sortLinkNode);
    swtmr->state = OS_SWTMR_STATUS_UNUSED;

#ifdef LOSCFG_DEBUG_RESOURCE_INFO
    UINT32 intSave;
    SWTMR_LOCK(intSave);
    g_swtmrUsed -= 1;
    SWTMR_UNLOCK(intSave);
#endif

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    swtmr->alignEnable = FALSE;
    swtmr->isAligned = FALSE;
    swtmr->canMultiple = 0;
    swtmr->times = 0;
#endif
}

STATIC INLINE VOID OsSwtmrUpdate(LosSwtmrCB *swtmr)
{
    if (swtmr->mode == LOS_SWTMR_MODE_ONCE) {
        swtmr->state = OS_SWTMR_STATUS_DELETING;
    } else if (swtmr->mode == LOS_SWTMR_MODE_NO_SELFDELETE) {
        swtmr->state = OS_SWTMR_STATUS_CREATED;
    } else {
        swtmr->overrun++;
        OsSwtmrStart(swtmr);
    }
}

STATIC INLINE VOID OsSwtmrCheckSelfDelete(LosSwtmrCB *swtmr)
{
    if ((swtmr->state == OS_SWTMR_STATUS_DELETING) && (swtmr->inProcess == 0)) {
        OsSwtmrDelete(swtmr);
    }
}

#ifndef LOSCFG_BASE_CORE_SWTMR_IN_ISR
LITE_OS_SEC_TEXT VOID OsSwtmrTask(VOID)
{
    LosSwtmrCB *swtmr = NULL;
    UINT32 intSave;

#if defined(LOSCFG_TRUSTZONE) && defined(LOSCFG_SWTMR_ACCESS_SECURE)
    UINT32 ret = LOS_TaskAllocSecureContext(OsPercpuGet()->swtmrTaskId, LOSCFG_TSK_SWTMR_SECURE_STACK_SIZE);
    if (ret != LOS_OK) {
        PRINT_ERR("OsSwtmrTask alloc secure stack failed!\n");
        return;
    }
#endif

    intSave = LOS_IntLock();

    for (;;) {
        OsSysTaskSuspend(OsCurrTaskGet());
        while (OsPercpuGet()->expiryList.free != LOSCFG_BASE_CORE_SWTMR_LIMIT) {
            swtmr = (SWTMR_CTRL_S *)OsPercpuGet()->expiryList.buf[OsPercpuGet()->expiryList.head];
            LOS_IntRestore(intSave);

            if (swtmr->handler != NULL) {
                swtmr->handler(swtmr->arg);
            }

            intSave = LOS_IntLock();
            OsPercpuGet()->expiryList.head++;
            OsPercpuGet()->expiryList.head %= LOSCFG_BASE_CORE_SWTMR_LIMIT;
            OsPercpuGet()->expiryList.free++;
            LOS_SpinLock(&g_swtmrSpin);
            swtmr->inProcess--;
            OsSwtmrCheckSelfDelete(swtmr);
            LOS_SpinUnlock(&g_swtmrSpin);
        }
    }
}

#ifdef LOSCFG_EXC_INTERACTION
BOOL IsSwtmrTask(UINT32 taskId)
{
    UINT32 i;

    for (i = 0; i < LOSCFG_KERNEL_CORE_NUM; i++) {
        if (taskId == g_percpu[i].swtmrTaskId) {
            return TRUE;
        }
    }

    return FALSE;
}
#endif

LITE_OS_SEC_TEXT_INIT STATIC UINT32 OsSwtmrTaskCreate(VOID)
{
    UINT32 ret, swtmrTaskId;
    TSK_INIT_PARAM_S swtmrTask = {0};
	UINT32 cpuid = ArchCurrCpuid();

    swtmrTask.pfnTaskEntry    = (TSK_ENTRY_FUNC)OsSwtmrTask;
    swtmrTask.uwStackSize     = LOSCFG_BASE_CORE_TSK_SWTMR_STACK_SIZE;
    swtmrTask.pcName          = "Swt_Task";
    swtmrTask.usTaskPrio      = LOS_TASK_PRIORITY_HIGHEST;
#ifdef LOSCFG_KERNEL_SMP
    swtmrTask.usCpuAffiMask = CPUID_TO_AFFI_MASK(cpuid);
#endif

#ifdef LOSCFG_TASK_STACK_STATIC_ALLOCATION
    ret = LOS_TaskCreateStatic(&swtmrTaskId, &swtmrTask, g_osSwtmrTaskStack[cpuid]);
#else
    ret = LOS_TaskCreate(&swtmrTaskId, &swtmrTask);
#endif
    if (ret == LOS_OK) {
        g_percpu[cpuid].swtmrTaskId = swtmrTaskId;
        OS_TCB_FROM_TID(swtmrTaskId)->taskFlags |= OS_TASK_FLAG_SYSTEM;
    }

    return ret;
}
#endif

LITE_OS_SEC_TEXT_INIT UINT32 OsSwtmrInit(VOID)
{
    UINT32 size;
    UINT16 index;
    UINT32 ret;

    size = sizeof(SWTMR_CTRL_S) * LOSCFG_BASE_CORE_SWTMR_LIMIT;
    SWTMR_CTRL_S *swtmr = (SWTMR_CTRL_S *)LOS_MemAlloc(m_aucSysMem0, size);
    if (swtmr == NULL) {
        return LOS_ERRNO_SWTMR_NO_MEMORY;
    }
    // Ignore the return code when matching CSEC rule 6.6(3).
    (VOID)memset_s((VOID *)swtmr, size, 0, size);
    g_osSwtmrCBArray = swtmr;
    LOS_ListInit(&g_swtmrFreeList);
    for (index = 0; index < LOSCFG_BASE_CORE_SWTMR_LIMIT; index++, swtmr++) {
        swtmr->timerId = index;
        LOS_ListTailInsert(&g_swtmrFreeList, &swtmr->sortList.sortLinkNode);
    }

#if !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
    OsPercpuGet()->expiryList.free = LOSCFG_BASE_CORE_SWTMR_LIMIT;

    ret = OsSwtmrTaskCreate();
    if (ret != LOS_OK) {
        (VOID)LOS_MemFree(m_aucSysMem0, swtmr);
        return LOS_ERRNO_SWTMR_TASK_CREATE_FAILED;
    }
#else
    /* In ISR mode, the timeout handlers are executed directly in the tick
     * interrupt context. No timer task or expiry list is needed. */
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
/*
 * Description: Scan expired software timer timers.
 */
LITE_OS_SEC_TEXT BOOL OsSwtmrScan(VOID)
{
    SortLinkList *sortList = NULL;
    LosSwtmrCB *swtmr = NULL;
    LOS_DL_LIST *listObject = NULL;
    SortLinkAttribute *swtmrSortLink = &OsPercpuGet()->swtmrSortLink;
	BOOL needSchedule = FALSE;

    /*
     * it needs to be carefully coped with, since the swtmr is in specific sortlink
     * while other cores still has the chance to process it, like stop the timer.
     */
    LOS_SpinLock(&g_swtmrSpin);
    OsTickScanAdvance(swtmrSortLink);
    SORTLINK_LISTOBJ_GET(listObject, swtmrSortLink);
    if (LOS_ListEmpty(listObject)) {
        LOS_SpinUnlock(&g_swtmrSpin);
        return needSchedule;
    }

    sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);

    while (ROLLNUM(sortList->idxRollNum) == 0) {
        OsDeleteNodeSortLink(sortList);
        swtmr = LOS_DL_LIST_ENTRY(sortList, LosSwtmrCB, sortList);

        OsHookCall(LOS_HOOK_TYPE_SWTMR_EXPIRED, swtmr);
#ifdef LOSCFG_BASE_CORE_SWTMR_IN_ISR
        SWTMR_PROC_FUNC handler = swtmr->handler;
        UINTPTR arg = swtmr->arg;
        OsSwtmrUpdate(swtmr);
        if (handler != NULL) {
            swtmr->inProcess++;
            LOS_SpinUnlock(&g_swtmrSpin);

            handler(arg); /* do swtmr callback */

            LOS_SpinLock(&g_swtmrSpin);
            swtmr->inProcess--;
        }
        OsSwtmrCheckSelfDelete(swtmr);
#else
        if (OsPercpuGet()->expiryList.free != 0) {
            swtmr->inProcess++;
            OsPercpuGet()->expiryList.buf[OsPercpuGet()->expiryList.tail] = (UINTPTR)swtmr;
            OsPercpuGet()->expiryList.tail++;
            OsPercpuGet()->expiryList.tail %= LOSCFG_BASE_CORE_SWTMR_LIMIT;
            OsPercpuGet()->expiryList.free--;
            (VOID)LOS_TaskResume(OsPercpuGet()->swtmrTaskId);
        }
        OsSwtmrUpdate(swtmr);
#endif

        needSchedule = TRUE;
        if (LOS_ListEmpty(listObject)) {
            break;
        }

        sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    }
    LOS_SpinUnlock(&g_swtmrSpin);

    return needSchedule;
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

/*
 * Description: Get next timeout
 * Return     : Count of the Timer list
 */
LITE_OS_SEC_TEXT UINT32 OsSwtmrGetNextTimeout(VOID)
{
    UINT64 time = OsSortLinkGetNextExpireTime(&OsPercpuGet()->swtmrSortLink);
    return OsSwtmrRollToTick(time);
}

/*
 * Description: Stop of Software Timer interface
 * Input      : swtmr --- the software timer control handler
 */
LITE_OS_SEC_TEXT STATIC VOID OsSwtmrStop(LosSwtmrCB *swtmr)
{
    SortLinkAttribute *sortLinkHeader = NULL;

#ifdef LOSCFG_KERNEL_SMP
    /*
     * the timer is running on the specific processor,
     * we need delete the timer from that processor's sortlink.
     */
    sortLinkHeader = &g_percpu[swtmr->cpuid].swtmrSortLink;
#else
    sortLinkHeader = &g_percpu[0].swtmrSortLink;
#endif

    OsSchedDeleteSortLink(sortLinkHeader, &swtmr->sortList);

    swtmr->state = OS_SWTMR_STATUS_CREATED;
    swtmr->overrun = 0;

    OsSchedResetSchedResponseTime(0);
    OsSchedUpdateExpireTime();

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    swtmr->isAligned = FALSE;
#endif
}

/*
 * Description: Get next software timer expiretime
 * Input      : swtmr --- the software timer control handler
 */
LITE_OS_SEC_TEXT STATIC UINT32 OsSwtmrTimeGet(const LosSwtmrCB *swtmr)
{
    SortLinkAttribute *sortLinkHeader = NULL;
	UINT64 time;

#ifdef LOSCFG_KERNEL_SMP
    /*
     * the timer is running on the specific processor,
     * we need search the timer from that processor's sortlink.
     */
    sortLinkHeader = &g_percpu[swtmr->cpuid].swtmrSortLink;
#else
    sortLinkHeader = &g_percpu[0].swtmrSortLink;
#endif
    time = OsSortLinkGetTargetExpireTime(sortLinkHeader, &swtmr->sortList);
    return OsSwtmrRollToTick(time);
}

STATIC LITE_OS_SEC_TEXT_INIT UINT32 OsSwtmrCreate(UINT32 interval,
                                                   UINT8 mode,
                                                   SWTMR_PROC_FUNC handler,
                                                   UINT32 *swtmrId,
                                                   UINTPTR arg
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
                                                   , UINT8 rouses, UINT8 sensitive
#endif
                                                   )
{
    LosSwtmrCB *swtmr = NULL;
    UINT32 intSave;
    SortLinkList *sortList = NULL;

    if (interval == 0) {
        return LOS_ERRNO_SWTMR_INTERVAL_NOT_SUITED;
    }

    if ((mode != LOS_SWTMR_MODE_ONCE) && (mode != LOS_SWTMR_MODE_PERIOD) &&
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
    swtmr = LOS_DL_LIST_ENTRY(sortList, LosSwtmrCB, sortList);
    LOS_ListDelete(LOS_DL_LIST_FIRST(&g_swtmrFreeList));
    SWTMR_UNLOCK(intSave);

    swtmr->handler  = handler;
    swtmr->mode     = mode;
    swtmr->overrun  = 0;
    swtmr->interval = interval;
    swtmr->expiry   = interval;
    swtmr->arg      = arg;
    swtmr->state    = OS_SWTMR_STATUS_CREATED;
    SET_SORTLIST_VALUE(&swtmr->sortList, OS_SORT_LINK_INVALID_TIME);

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    (VOID)rouses;
    if ((sensitive == OS_SWTMR_ALIGN_INSENSITIVE) && (mode == LOS_SWTMR_MODE_PERIOD)) {
        swtmr->alignEnable = TRUE;
        if ((swtmr->interval % LOS_COMMON_DIVISOR) == 0) {
            swtmr->canMultiple = 1;
            swtmr->times = swtmr->interval / LOS_COMMON_DIVISOR;
        }
    }
    swtmr->isAligned = FALSE;
#endif
    *swtmrId = swtmr->timerId;

#ifdef LOSCFG_DEBUG_RESOURCE_INFO
    SWTMR_LOCK(intSave);
    g_swtmrUsed += 1;
    if (g_swtmrUsed > g_swtmrPeak) {
        g_swtmrPeak = g_swtmrUsed;
    }
    SWTMR_UNLOCK(intSave);
#endif

    OsHookCall(LOS_HOOK_TYPE_SWTMR_CREATE, swtmr);

    return LOS_OK;
}

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
LITE_OS_SEC_TEXT_INIT UINT32 LOS_SwtmrCreate(UINT32 interval,
                                             UINT8 mode,
                                             SWTMR_PROC_FUNC handler,
                                             UINT32 *swtmrId,
                                             UINTPTR arg,
                                             UINT8 rouses,
                                             UINT8 sensitive)
#else
LITE_OS_SEC_TEXT_INIT UINT32 LOS_SwtmrCreate(UINT32 interval,
                                             UINT8 mode,
                                             SWTMR_PROC_FUNC handler,
                                             UINT32 *swtmrId,
                                             UINTPTR arg)
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

#ifdef LOSCFG_BASE_CORE_SYS_RES_CHECK
LosSwtmrCB *OsSwtmrIdVerify(UINT32 swtmrId)
{
    LosSwtmrCB *swtmr = OS_SWT_FROM_SWTID(swtmrId);
    if (swtmr->timerId != swtmrId) {
        return NULL;
    }

    return swtmr;
}
#else

LosSwtmrCB *OsSwtmrIdVerify(UINT32 swtmrId)
{
    if (swtmrId >= KERNEL_SWTMR_LIMIT) {
        return NULL;
    }

    return OS_SWT_FROM_SWTID(swtmrId);
}
#endif

STATIC INLINE VOID OsSwtmrSetTimerParms(SWTMR_CTRL_S *swtmr, UINT32 interval, UINT32 expiry)
{
#ifdef LOSCFG_COMPAT_POSIX
    if (expiry > 0) {
        swtmr->expiry = expiry;
        swtmr->interval = interval;
        if (interval == 0) {
            swtmr->mode = LOS_SWTMR_MODE_NO_SELFDELETE;
        } else {
            swtmr->mode = LOS_SWTMR_MODE_OPP;
        }
        return;
    }
#endif
#ifdef LOSCFG_COMPAT_CMSIS
    if (interval > 0) {
        swtmr->interval = interval;
        swtmr->expiry = interval;
    }
#endif
    (VOID)swtmr;
    (VOID)interval;
    (VOID)expiry;
}

LITE_OS_SEC_TEXT UINT32 OsSwtmrStartTimer(UINT32 swtmrId, UINT32 interval, UINT32 expiry)
{
    LosSwtmrCB *swtmr = NULL;
    UINT32 intSave;
    UINT32 ret = LOS_OK;

    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    SWTMR_LOCK(intSave);
    swtmr = OsSwtmrIdVerify(swtmrId);
    if (swtmr == NULL) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_NOT_CREATED;
    }

    switch (swtmr->state) {
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
            OsSwtmrStart(swtmr);
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
    return OsSwtmrStartTimer(swtmrId, 0, 0);
}
LITE_OS_SEC_TEXT UINT32 LOS_SwtmrStop(UINT32 swtmrId)
{
    LosSwtmrCB *swtmr = NULL;
    UINT32 intSave;
    UINT32 ret = LOS_OK;

    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }
    SWTMR_LOCK(intSave);
    swtmr = OsSwtmrIdVerify(swtmrId);
    if (swtmr == NULL) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_NOT_CREATED;
    }

    switch (swtmr->state) {
        case OS_SWTMR_STATUS_UNUSED:
            /* fall-through */
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
    LosSwtmrCB *swtmr = NULL;
    UINT32 intSave;
    UINT32 ret = LOS_OK;

    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }

    if (tick == NULL) {
        return LOS_ERRNO_SWTMR_TICK_PTR_NULL;
    }

    SWTMR_LOCK(intSave);
    swtmr = OsSwtmrIdVerify(swtmrId);
    if (swtmr == NULL) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_NOT_CREATED;
    }

    switch (swtmr->state) {
        case OS_SWTMR_STATUS_UNUSED:
            /* fall-through */
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
STATIC UINT32 OsSwtmrDeleteOption(UINT32 swtmrId, BOOL isSync)
{
    LosSwtmrCB *swtmr = NULL;
    UINT32 intSave;
    UINT32 ret = LOS_OK;

    if (swtmrId >= OS_SWTMR_MAX_TIMERID) {
        return LOS_ERRNO_SWTMR_ID_INVALID;
    }
    SWTMR_LOCK(intSave);
    swtmr = OsSwtmrIdVerify(swtmrId);
    if (swtmr == NULL) {
        SWTMR_UNLOCK(intSave);
        return LOS_ERRNO_SWTMR_NOT_CREATED;
    }

    switch (swtmr->state) {
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
            swtmr->state = OS_SWTMR_STATUS_DELETING;
#ifdef LOSCFG_SWTMR_SYNC_DELETE
            /* fall-through */
        case OS_SWTMR_STATUS_DELETING:
            if (isSync) {
                while (swtmr->inProcess != 0) {
                    SWTMR_UNLOCK(intSave);
                    LOS_TaskDelay(1);
                    SWTMR_LOCK(intSave);
                }
                if (swtmr->timerId != swtmrId) {
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

LITE_OS_SEC_TEXT VOID OsSwtmrResponseTimeReset(VOID)
{
    SortLinkAttribute *sortLinkHead = &OsPercpuGet()->swtmrSortLink;

    for (UINT32 bucket = 0; bucket < OS_TSK_SORTLINK_LEN; bucket++) {
        LOS_DL_LIST *listHead = sortLinkHead->sortLink + bucket;
        LOS_DL_LIST *listNext = listHead->pstNext;

        while (listNext != listHead) {
            SortLinkList *sortList = LOS_DL_LIST_ENTRY(listNext, SortLinkList, sortLinkNode);
            SWTMR_CTRL_S *swtmr = LOS_DL_LIST_ENTRY(sortList, SWTMR_CTRL_S, sortList);
            OsDeleteNodeSortLink(sortList);
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
            swtmr->isAligned = FALSE;
#endif
            OsSwtmrStart(swtmr);
            listNext = listNext->pstNext;
        }
    }
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
