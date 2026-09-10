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

#include "los_sched_pri.h"
#include "los_task_pri.h"
#include "los_sortlink_pri.h"
#include "los_tick.h"
#include "los_sched.h"
#include "los_swtmr_pri.h"
#include "los_debug.h"
#include "los_hook.h"
#if (LOSCFG_KERNEL_PM == 1)
#include "los_pm.h"
#endif
#if (LOSCFG_KERNEL_SIGNAL == 1)
#include "los_signal.h"
#endif

#define OS_TICK_RESPONSE_TIME_MAX  LOSCFG_BASE_CORE_TICK_RESPONSE_MAX
#if (LOSCFG_BASE_CORE_TICK_RESPONSE_MAX == 0)
#error "Must specify the maximum value that tick timer counter supports!"
#endif

STATIC UINT32 g_schedResponseID = 0;
STATIC UINT64 g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
STATIC UINT16 g_tickIntLock = 0;

STATIC UINT32 g_schedTickMinPeriod;
STATIC UINT32 g_tickResponsePrecision;

STATIC INT32 g_schedTimeSlice;
STATIC INT32 g_schedTimeSliceMin;

STATIC SchedScan g_swtmrScan = NULL;

VOID OsSchedResetSchedResponseTime(UINT64 responseTime)
{
    if (responseTime <= g_schedResponseTime) {
        g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
    }
}

/* ===========================================================================
 * Tick Mode Abstraction
 * ---------------------------------------------------------------------------
 * One #ifdef block, two complete implementations.  All tick-specific logic
 * (delta conversion, advance strategy, expire-time computation, freeze value)
 * is isolated here.  Shared functions below call these abstractions and
 * contain zero #ifdef.
 * ======================================================================== */
#ifdef LOSCFG_KERNEL_TICK_PERIODIC

UINT64 OsTickWaitToDelta(UINT32 waitTicks)
{
    return (UINT64)waitTicks;
}

VOID OsSchedSortLinkAdvance(SortLinkAttribute *sortLinkHeader)
{
    (void)sortLinkHeader;
}

VOID OsTickScanAdvance(SortLinkAttribute *sortLinkHeader)
{
    OsSortLinkAdvanceHead(sortLinkHeader, 1);
}

STATIC INLINE UINT64 OsTickFreezeValue(UINT64 remainTime)
{
    return remainTime;
}

STATIC INLINE BOOL OsTickTryUnfreeze(LosTaskCB *taskCB, UINT64 savedTime)
{
    if (savedTime > 0 && savedTime != OS_SORT_LINK_INVALID_TIME) {
        SortLinkAttribute *head = &OsPercpuGet()->taskSortLink;
        OsSchedAddSortLink(head, &taskCB->sortList, (UINT32)savedTime);
        return TRUE;
    }
    return FALSE;
}

STATIC VOID OsSchedSetNextExpireTime(UINT32 responseID, UINT64 taskEndTime)
{
    (void)responseID;
    (void)taskEndTime;
}

VOID OsSchedUpdateExpireTime(VOID)
{
    return;
}

UINT64 OsSchedGetNextExpireTime(UINT64 startTime)
{
    Percpu *percpu = OsPercpuGet();
    UINT64 rollSum = OsGetNextRollSum(&percpu->taskSortLink, &percpu->swtmrSortLink);
    if (rollSum == OS_SORT_LINK_UINT64_MAX) {
        return OS_SCHED_MAX_RESPONSE_TIME - g_tickResponsePrecision;
    }
    return startTime + rollSum;
}

STATIC INLINE UINT32 OsTickRollSumToTicks(UINT64 rollSum)
{
    return (rollSum == OS_SORT_LINK_UINT64_MAX) ? OS_INVALID : (UINT32)rollSum;
}

#else /* LOSCFG_KERNEL_TICKLESS_GLOBAL */

UINT64 OsTickWaitToDelta(UINT32 waitTicks)
{
    return OS_SYS_TICK_TO_CYCLE(waitTicks);
}

VOID OsSchedSortLinkAdvance(SortLinkAttribute *sortLinkHeader)
{
    UINT64 delta = OsGetCurrSchedTimeCycle() - sortLinkHeader->baseTime;
    if (delta > 0) {
        OsSortLinkAdvanceHead(sortLinkHeader, delta);
        sortLinkHeader->baseTime = OsGetCurrSchedTimeCycle();
    }
}

VOID OsTickScanAdvance(SortLinkAttribute *sortLinkHeader)
{
    OsSchedSortLinkAdvance(sortLinkHeader);
}

STATIC INLINE UINT64 OsTickFreezeValue(UINT64 remainTime)
{
    return OsGetCurrSchedTimeCycle() + remainTime;
}

STATIC INLINE BOOL OsTickTryUnfreeze(LosTaskCB *taskCB, UINT64 savedTime)
{
    UINT64 currTime = OsGetCurrSchedTimeCycle();
    if (savedTime > currTime) {
        UINT32 remainTick = ((savedTime - currTime) + OS_CYCLE_PER_TICK - 1) / OS_CYCLE_PER_TICK;
        SortLinkAttribute *head = &OsPercpuGet()->taskSortLink;
        OsSchedAddSortLink(head, &taskCB->sortList, remainTick);
        return TRUE;
    }
    return FALSE;
}

STATIC VOID OsSchedSetNextExpireTime(UINT32 responseID, UINT64 taskEndTime)
{
    UINT64 nextResponseTime;
    BOOL isTimeSlice = FALSE;
    Percpu *percpu = OsPercpuGet();

    UINT64 currTime = OsGetCurrSchedTimeCycle();
    UINT64 nextRollSum = OsGetNextRollSum(&percpu->taskSortLink, &percpu->swtmrSortLink);
    UINT64 nextExpireTime;
    if (nextRollSum == OS_SORT_LINK_UINT64_MAX) {
        nextExpireTime = OS_SCHED_MAX_RESPONSE_TIME - g_tickResponsePrecision;
    } else {
        nextExpireTime = percpu->taskSortLink.baseTime + nextRollSum;
    }
    /* The response time of the task time slice is aligned to the next response time in the delay queue */
    if ((nextExpireTime > taskEndTime) && ((nextExpireTime - taskEndTime) > g_schedTickMinPeriod)) {
        nextExpireTime = taskEndTime;
        isTimeSlice = TRUE;
    }

    if ((g_schedResponseTime <= nextExpireTime) ||
        ((g_schedResponseTime - nextExpireTime) < g_tickResponsePrecision)) {
        return;
    }

    if (isTimeSlice) {
        /* The expiration time of the current system is the thread's slice expiration time */
        g_schedResponseID = responseID;
    } else {
        g_schedResponseID = OS_INVALID;
    }

    nextResponseTime = nextExpireTime - currTime;
    if ((nextResponseTime < g_tickResponsePrecision) || (nextExpireTime < currTime)) {
        nextResponseTime = g_tickResponsePrecision;
    }
    g_schedResponseTime = currTime + OsTickTimerReload(nextResponseTime);
}

VOID OsSchedUpdateExpireTime(VOID)
{
    UINT64 endTime;
    BOOL isPmMode = FALSE;
    LosTaskCB *runTask = OsCurrTaskGet();

    if (!g_taskScheduled || g_tickIntLock) {
        return;
    }

#if (LOSCFG_KERNEL_PM == 1)
    isPmMode = OsIsPmMode();
#endif
    if ((runTask->taskId != OsPercpuGet()->idleTaskId) && !isPmMode) {
        INT32 timeSlice = (runTask->timeSlice <= g_schedTimeSliceMin) ? g_schedTimeSlice : runTask->timeSlice;
        endTime = runTask->startTime + timeSlice;
    } else {
        endTime = OS_SCHED_MAX_RESPONSE_TIME - g_tickResponsePrecision;
    }
    OsSchedSetNextExpireTime(runTask->taskId, endTime);
}

UINT64 OsSchedGetNextExpireTime(UINT64 startTime)
{
    (void)startTime;
    Percpu *percpu = OsPercpuGet();
    UINT64 rollSum = OsGetNextRollSum(&percpu->taskSortLink, &percpu->swtmrSortLink);
    if (rollSum == OS_SORT_LINK_UINT64_MAX) {
        return OS_SCHED_MAX_RESPONSE_TIME - g_tickResponsePrecision;
    }
    return percpu->taskSortLink.baseTime + rollSum;
}

STATIC INLINE UINT32 OsTickRollSumToTicks(UINT64 rollSum)
{
    return (rollSum == OS_SORT_LINK_UINT64_MAX) ? OS_INVALID : (UINT32)OS_SYS_CYCLE_TO_TICK(rollSum);
}

#endif /* Tick Mode Abstraction */

/*
 * Expire-time bookkeeping after a scheduling decision: computes the next
 * task's slice end (or the idle fallback), invalidates a response armed
 * for runTask, and arms the next timer.
 */
VOID OsSchedExpireTimeUpdate(LosTaskCB *newTask, LosTaskCB *runTask)
{
    UINT64 endTime;

    if (newTask->taskId != OsPercpuGet()->idleTaskId) {
        endTime = newTask->startTime + newTask->timeSlice;
    } else {
        endTime = OS_SCHED_MAX_RESPONSE_TIME - g_tickResponsePrecision;
    }

    if (g_schedResponseID == runTask->taskId) {
        g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
    }
    OsSchedSetNextExpireTime(newTask->taskId, endTime);
}

/*
 * Returns TRUE when the task still holds time slice (queue at head);
 * FALSE when exhausted — the slice is recharged (queue at tail).
 */
BOOL OsSchedTimeSliceReset(LosTaskCB *taskCB)
{
    if (taskCB->timeSlice > g_schedTimeSliceMin) {
        return TRUE;
    }
    taskCB->timeSlice = g_schedTimeSlice;
    return FALSE;
}

LITE_OS_SEC_TEXT UINT32 LOS_SleepTicksGet(VOID)
{
    UINT32 tskSortLinkTicks, sleepTicks;
    UINT32 intSave = LOS_IntLock();

    LOS_SpinLock(&g_taskSpin);
    tskSortLinkTicks = OsTickRollSumToTicks(OsSortLinkGetNextExpireTime(&OsPercpuGet()->taskSortLink));
    LOS_SpinUnlock(&g_taskSpin);

#if (LOSCFG_BASE_CORE_SWTMR == 1)
    UINT32 swtmrSortLinkTicks;
    LOS_SpinLock(&g_swtmrSpin);
    swtmrSortLinkTicks = OsTickRollSumToTicks(OsSortLinkGetNextExpireTime(&OsPercpuGet()->swtmrSortLink));
    LOS_SpinUnlock(&g_swtmrSpin);
    sleepTicks = (tskSortLinkTicks < swtmrSortLinkTicks) ? tskSortLinkTicks : swtmrSortLinkTicks;
#else
    sleepTicks = tskSortLinkTicks;
#endif

    LOS_IntRestore(intSave);
    return sleepTicks;
}

STATIC INLINE VOID OsSchedWakePendTimeTask(LosTaskCB *taskCB, BOOL *needSchedule)
{
    UINT16 tempStatus = taskCB->taskStatus;
    if (tempStatus & (OS_TASK_STATUS_PEND | OS_TASK_STATUS_DELAY)) {
        taskCB->taskStatus &= ~(OS_TASK_STATUS_PEND | OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY);
        if (tempStatus & OS_TASK_STATUS_PEND) {
            taskCB->taskStatus |= OS_TASK_STATUS_TIMEOUT;
            LOS_ListDelete(&taskCB->pendList);
            taskCB->taskMux = NULL;
            taskCB->taskSem = NULL;
        }

        if (!(tempStatus & OS_TASK_STATUS_SUSPEND)) {
            OsSchedTaskEnQueue(taskCB);
            *needSchedule = TRUE;
        }
    }
}

STATIC INLINE BOOL OsSchedScanTimerList(VOID)
{
    BOOL needSchedule = FALSE;
    SortLinkAttribute *sortLinkHeader = &OsPercpuGet()->taskSortLink;
    LOS_DL_LIST *listObject = NULL;
    /*
     * When task is pended with timeout, the task block is on the timeout sortlink
     * (per cpu) and ipc(mutex,sem and etc.)'s block at the same time, it can be waken
     * up by either timeout or corresponding ipc it's waiting.
     *
     * Now synchronize sortlink procedure is used, therefore the whole task scan needs
     * to be protected, preventing another core from doing sortlink deletion at same time.
     */

    OsTickScanAdvance(sortLinkHeader);
    SORTLINK_LISTOBJ_GET(listObject, sortLinkHeader);
    if (LOS_ListEmpty(listObject)) {
        return needSchedule;
    }

    SortLinkList *sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    while (ROLLNUM(sortList->idxRollNum) == 0) {
        LosTaskCB *taskCB = LOS_DL_LIST_ENTRY(sortList, LosTaskCB, sortList);
        OsDeleteNodeSortLink(&taskCB->sortList);
        OsSchedWakePendTimeTask(taskCB, &needSchedule);
        if (LOS_ListEmpty(listObject)) {
            break;
        }

        sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    }

    return needSchedule;
}

VOID OsSchedFreezeTask(LosTaskCB *taskCB)
{
    SortLinkAttribute *head = &OsPercpuGet()->taskSortLink;
    OsSchedSortLinkAdvance(head);
    UINT64 remainTime = OsSortLinkGetTargetExpireTime(head, &taskCB->sortList);
    OsSchedDeleteSortLink(head, &taskCB->sortList);
    SET_SORTLIST_VALUE(&taskCB->sortList, OsTickFreezeValue(remainTime));
    taskCB->taskStatus |= OS_TASK_FLAG_FREEZE;
}

VOID OsSchedUnfreezeTask(LosTaskCB *taskCB)
{
    taskCB->taskStatus &= ~OS_TASK_FLAG_FREEZE;
    UINT64 savedTime = GET_SORTLIST_VALUE(&taskCB->sortList);
    if (OsTickTryUnfreeze(taskCB, savedTime)) {
        return;
    }

    SET_SORTLIST_VALUE(&taskCB->sortList, OS_SORT_LINK_INVALID_TIME);
    if (taskCB->taskStatus & OS_TASK_STATUS_PEND) {
        LOS_ListDelete(&taskCB->pendList);
    }
    taskCB->taskStatus &= ~(OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_PEND);
}

UINT32 OsSchedSwtmrScanRegister(SchedScan func)
{
    if (func == NULL) {
        return LOS_NOK;
    }

    g_swtmrScan = func;
    return LOS_OK;
}

UINT32 OsTaskNextSwitchTimeGet(VOID)
{
    UINT32 intSave;
    SCHEDULER_LOCK(intSave);
    UINT32 ticks = OsSortLinkGetNextExpireTime(&OsPercpuGet()->taskSortLink);
    SCHEDULER_UNLOCK(intSave);
    return ticks;
}

STATIC VOID TaskSchedTimeConvertFreq(UINT32 oldFreq)
{
    for (UINT32 loopNum = 0; loopNum < g_taskMaxNum; loopNum++) {
        LosTaskCB *taskCB = (((LosTaskCB *)g_taskCBArray) + loopNum);
        if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
            continue;
        }
        if (taskCB->timeSlice > 0) {
            taskCB->timeSlice = (INT32)OsTimeConvertFreq((UINT64)taskCB->timeSlice, oldFreq, g_sysClock);
        } else {
            taskCB->timeSlice = 0;
        }

        if (taskCB->taskStatus & OS_TASK_STATUS_RUNNING) {
            taskCB->startTime = OsTimeConvertFreq(taskCB->startTime, oldFreq, g_sysClock);
        }
    }
}

VOID SchedTimeBaseInit(VOID)
{
    g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
    g_schedResponseID = OS_INVALID;

    g_schedTickMinPeriod = g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND_MINI;
    g_tickResponsePrecision =  (g_schedTickMinPeriod * 75) / 100; /* 75 / 100: minimum accuracy */
    g_schedTimeSlice = (INT32)(((UINT64)g_sysClock * LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT) / OS_SYS_US_PER_SECOND);
    g_schedTimeSliceMin = (INT32)(((UINT64)g_sysClock * 50) / OS_SYS_US_PER_SECOND); /* Minimum time slice 50 us */
}

VOID OsSchedTimeConvertFreq(UINT32 oldFreq)
{
    SchedTimeBaseInit();
    TaskSchedTimeConvertFreq(oldFreq);
    OsSortLinkResponseTimeConvertFreq(oldFreq);
    OsSchedUpdateExpireTime();
}

UINT64 LOS_SchedTickTimeoutNsGet(VOID)
{
    UINT32 intSave;
    UINT64 responseTime;
    UINT64 currTime;

    SCHEDULER_LOCK(intSave);
    responseTime = g_schedResponseTime;
    currTime = OsGetCurrSchedTimeCycle();
    SCHEDULER_UNLOCK(intSave);

    if (responseTime > currTime) {
        responseTime = responseTime - currTime;
    } else {
        responseTime = 0; /* Tick interrupt already timeout */
    }

    return OS_SYS_CYCLE_TO_NS(responseTime, g_sysClock);
}

VOID LOS_SchedTickHandler(VOID)
{
    if (!g_taskScheduled) {
        return;
    }

    UINT32 intSave;
    SCHEDULER_LOCK(intSave);
    UINT64 tickStartTime = OsGetCurrSchedTimeCycle();
    LosTaskCB *runTask = OsCurrTaskGet();
    if (g_schedResponseID == OS_INVALID) {
        g_tickIntLock++;
        if (g_swtmrScan != NULL) {
            (VOID)g_swtmrScan();
        }

        (VOID)OsSchedScanTimerList();
        g_tickIntLock--;
    }

    if (OsCurrTaskGet() == runTask) {
        /*
         * With LOSCFG_BASE_CORE_SWTMR_IN_ISR, a swtmr handler runs inline in
         * this tick and may trigger a resched (e.g. via LOS_SemPost), which
         * advances runTask->startTime to a mid-tick timestamp. In that case
         * the elapsed-time accounting for this tick was already done, so only
         * re-baseline startTime instead of charging the stale tickStartTime.
         */
        if (tickStartTime >= runTask->startTime) {
            OsTimeSliceUpdate(runTask, tickStartTime);
        }
        runTask->startTime = OsGetCurrSchedTimeCycle();
    }

    g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
    if (LOS_CHECK_SCHEDULE) {
        LOS_Schedule();
    } else {
        OsSchedUpdateExpireTime();
    }

    SCHEDULER_UNLOCK(intSave);
}
