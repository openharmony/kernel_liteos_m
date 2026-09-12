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

#include "los_sched_pri.h"
#include "los_task_base.h"
#include "los_priqueue_pri.h"
#include "los_task_pri.h"
#include "los_tick.h"
#include "los_swtmr.h"
#include "los_swtmr_pri.h"
#include "los_debug.h"
#include "los_hook.h"
#if (LOSCFG_KERNEL_PM == 1)
#include "los_pm.h"
#endif
#if (LOSCFG_BASE_CORE_CPUP == 1)
#include "los_cpup_pri.h"
#endif
#if (LOSCFG_DEBUG_TOOLS == 1)
#include "los_debugtools.h"
#endif
#if (LOSCFG_KERNEL_SIGNAL == 1)
#include "los_signal.h"
#endif

#define OS_TASK_BLOCKED_STATUS (OS_TASK_STATUS_PEND | OS_TASK_STATUS_SUSPEND | \
                                OS_TASK_STATUS_EXIT | OS_TASK_STATUS_UNUSED)

LITE_OS_SEC_BSS STATIC LOS_DL_LIST g_taskSortlink[OS_TSK_SORTLINK_LEN];

/* ===========================================================================
 * Shared Functions (no #ifdef on tick mode)
 * ======================================================================== */

/*
 * Core scheduling decision. Assumes SCHEDULER_LOCK is already held
 * (or called from PendSV where interrupts are disabled by hardware).
 * Picks the next task, updates task states, sets runTask, and triggers
 * ArchTaskSchedule to perform the actual context switch.
 */
VOID OsSchedResched(VOID)
{
    LosTaskCB *runTask = NULL;
    LosTaskCB *newTask = NULL;

    LOS_ASSERT(LOS_SpinHeld(&g_taskSpin));

    if (!OsPreemptableInSched()) {
        return;
    }

	runTask = OsCurrTaskGet();
	LOS_ASSERT(runTask != NULL);

#ifdef LOSCFG_SCHED_LATENCY
    if (OsSignalRestorePending()) {
        ArchTaskSchedule(runTask, runTask);
        return;
    }
#endif

    OsTimeSliceUpdate(runTask, OsGetCurrSchedTimeCycle());

    newTask = OsGetTopTask();
    LOS_ASSERT(newTask != NULL);

    OsSchedTaskDeQueue(newTask);

    if (runTask != newTask) {
        runTask->taskStatus &= ~OS_TASK_STATUS_RUNNING;
        newTask->taskStatus |= OS_TASK_STATUS_RUNNING;
        newTask->startTime = runTask->startTime;

#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
        OsTaskSwitchCheck(runTask, newTask);
#endif
        if (g_pfnUsrTskSwitchHook != NULL) {
            g_pfnUsrTskSwitchHook(runTask->taskId, newTask->taskId);
        }
        OsHookCall(LOS_HOOK_TYPE_TASK_SWITCHEDIN);
#if (LOSCFG_DEBUG_TOOLS == 1)
        OsSchedTraceRecord(newTask, runTask);
#endif
        OsCurrTaskSet(newTask);
    }

    OsSchedExpireTimeUpdate(newTask, runTask);

    if (runTask != newTask) {
        OsSetSchedFlag(INT_NO_RESCH);
        ArchTaskSchedule(newTask, runTask);
    }
}

/*
 * Called from task context (not interrupt). Acquires SCHEDULER_LOCK,
 * runs OsSchedResched, then releases. The actual context switch happens
 * inside ArchTaskSchedule (cpsie i triggers PendSV on Cortex-M).
 */
VOID OsSchedPreempt(VOID)
{
    LosTaskCB *runTask = NULL;
    UINT32 intSave;

    if (!OsPreemptable()) {
        return;
    }

    SCHEDULER_LOCK(intSave);

    /* add run task back to ready queue */
    runTask = OsCurrTaskGet();

    /*
     * OsTimeSliceUpdate here is needed before OsSchedTaskEnQueue to get
     * the correct timeSlice for head/tail decision. OsSchedResched will
     * call it again, but the second call has incTime ~= 0 (harmless).
     */
    OsTimeSliceUpdate(runTask, OsGetCurrSchedTimeCycle());

    /*
     * Skip enqueue if already READY: in latency mode, signal restore path
     * in OsSchedResched returns before OsSchedTaskDeQueue, leaving the
     * task with READY set. Without this check the next OsSchedTaskEnQueue
     * would assert on the double-enqueue.
     */
    if (!(runTask->taskStatus & OS_TASK_STATUS_READY)) {
        OsSchedTaskEnQueue(runTask);
    }

    /* reschedule to new thread */
    OsSchedResched();

    SCHEDULER_UNLOCK(intSave);
}

#ifndef LOSCFG_SCHED_LATENCY
/*
 * Description : Process pending schedFlag tagged by others cores
 */
VOID OsSchedProcSchedFlag(VOID)
{
    Percpu *percpu = OsPercpuGet();

    if (OsPreemptable()) {
        UINT32 flag = percpu->schedFlag;
        if (flag == INT_PEND_RESCH) {
            OsSetSchedFlag(INT_NO_RESCH);
            OsSchedPreempt();
        } else if (flag == INT_SUSPEND_DELETE_RESCH) {
            OsSetSchedFlag(INT_NO_RESCH);
            OsSchedResched();
        }
    }
}
#endif

/*
 * Description : pend a task in list
 * Input       : runTask    --- Task to be suspended
 *               list       --- wait task list
 *               timeOut    --- Expiry time
 */
VOID OsSchedWait(LosTaskCB *runTask, LOS_DL_LIST *list, UINT32 timeout)
{
    runTask->taskStatus |= OS_TASK_STATUS_PEND;
    LOS_ListTailInsert(list, &runTask->pendList);

    if (timeout != LOS_WAIT_FOREVER) {
        runTask->taskStatus |= OS_TASK_STATUS_PEND_TIME;
        runTask->waitTimes = timeout;
        SortLinkAttribute *head = &OsPercpuGet()->taskSortLink;
        OsSchedAddSortLink(head, &runTask->sortList, timeout);
    }

    OsSchedResched();
}

/*
 * Description : Wakes the task from the pending list and adds the task to the priority queue.
 * Input       : resumedTask --- resumed task
 */
VOID OsSchedWake(LosTaskCB *resumedTask)
{
    LOS_ListDelete(&resumedTask->pendList);

    resumedTask->taskStatus &= ~OS_TASK_STATUS_PEND;
    if (resumedTask->taskStatus & OS_TASK_STATUS_PEND_TIME) {
        SortLinkAttribute *head = &OsPercpuGet()->taskSortLink;
        OsSchedDeleteSortLink(head, &resumedTask->sortList);
        OsSchedResetSchedResponseTime(0);
        resumedTask->taskStatus &= ~OS_TASK_STATUS_PEND_TIME;
    }

    if (!(resumedTask->taskStatus & OS_TASK_STATUS_SUSPEND)) {
        OsSchedTaskEnQueue(resumedTask);
    }
}

BOOL OsSchedPrioModify(LosTaskCB *taskCB, UINT16 priority)
{
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) {
        OsSchedTaskDeQueue(taskCB);
        taskCB->priority = priority;
        OsSchedTaskEnQueue(taskCB);
        return TRUE;
    }

    taskCB->priority = priority;
    OsHookCall(LOS_HOOK_TYPE_TASK_PRIMODIFY, taskCB, taskCB->priority);
    if (taskCB->taskStatus & OS_TASK_STATUS_RUNNING) {
        return TRUE;
    }

    return FALSE;
}

VOID OsSchedSuspend(LosTaskCB *taskCB)
{
    BOOL isPmMode = FALSE;
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) {
        OsSchedTaskDeQueue(taskCB);
    }

#if (LOSCFG_KERNEL_PM == 1)
    isPmMode = OsIsPmMode();
#endif
    if ((taskCB->taskStatus & (OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY)) && isPmMode) {
        OsSchedFreezeTask(taskCB);
    }

    taskCB->taskStatus |= OS_TASK_STATUS_SUSPEND;
    OsHookCall(LOS_HOOK_TYPE_MOVEDTASKTOSUSPENDEDLIST, taskCB);
}

VOID OsSchedAddSortLink(SortLinkAttribute *sortLinkHeader, SortLinkList *sortList, UINT32 waitTicks)
{
    OsSchedSortLinkAdvance(sortLinkHeader);
    sortList->idxRollNum = OsTickWaitToDelta(waitTicks);
    OsAdd2SortLink(sortLinkHeader, sortList);
}

VOID OsSchedDeleteSortLink(SortLinkAttribute *sortLinkHeader, SortLinkList *sortList)
{
    OsSchedSortLinkAdvance(sortLinkHeader);
    OsDeleteSortLink(sortLinkHeader, sortList);
}

VOID OsSchedTaskEnQueue(LosTaskCB *taskCB)
{
    LOS_ASSERT(!(taskCB->taskStatus & OS_TASK_STATUS_READY));

    if (taskCB->taskId != OsPercpuGet()->idleTaskId) {
        if (OsSchedTimeSliceReset(taskCB)) {
            OsPriQueueEnqueue(&taskCB->pendList, taskCB->priority, PRI_QUEUE_HEAD);
        } else {
            OsPriQueueEnqueue(&taskCB->pendList, taskCB->priority, PRI_QUEUE_TAIL);
        }
        OsHookCall(LOS_HOOK_TYPE_MOVEDTASKTOREADYSTATE, taskCB);
    }

    taskCB->taskStatus &= ~(OS_TASK_STATUS_PEND | OS_TASK_STATUS_SUSPEND |
                            OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME);

    taskCB->taskStatus |= OS_TASK_STATUS_READY;
}

VOID OsSchedTaskDeQueue(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) {
        if (taskCB->taskId != OsPercpuGet()->idleTaskId) {
            OsPriQueueDequeue(&taskCB->pendList);
        }

        taskCB->taskStatus &= ~OS_TASK_STATUS_READY;
    }
}

VOID OsSchedTaskExit(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_STATUS_READY) {
        OsSchedTaskDeQueue(taskCB);
    } else if (taskCB->taskStatus & OS_TASK_STATUS_PEND) {
        LOS_ListDelete(&taskCB->pendList);
        taskCB->taskStatus &= ~OS_TASK_STATUS_PEND;
    }

    if (taskCB->taskStatus & (OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME)) {
        SortLinkAttribute *head = &OsPercpuGet()->taskSortLink;
        OsSchedDeleteSortLink(head, &taskCB->sortList);
        OsSchedResetSchedResponseTime(0);
        taskCB->taskStatus &= ~(OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME);
    }
    taskCB->taskStatus |= OS_TASK_STATUS_EXIT;
}

VOID OsSchedYield(VOID)
{
    LosTaskCB *runTask = OsCurrTaskGet();

    runTask->timeSlice = 0;
    OsSchedTaskEnQueue(runTask);
}

VOID OsSchedDelay(LosTaskCB *runTask, UINT32 tick)
{
    runTask->taskStatus |= OS_TASK_STATUS_DELAY;
    runTask->waitTimes = tick;
    SortLinkAttribute *head = &OsPercpuGet()->taskSortLink;
    OsSchedAddSortLink(head, &runTask->sortList, tick);
}

BOOL OsSchedResume(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_FLAG_FREEZE) {
        OsSchedUnfreezeTask(taskCB);
    }

    taskCB->taskStatus &= (~OS_TASK_STATUS_SUSPEND);
    if (!(taskCB->taskStatus & (OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND))) {
        OsSchedTaskEnQueue(taskCB);
        return TRUE;
    }

    return FALSE;
}

VOID OsSchedSetIdleTaskSchedParam(LosTaskCB *idleTask)
{
    OsSchedTaskEnQueue(idleTask);
}

UINT32 OsSchedInit(VOID)
{
    OsPriQueueInit();

    OsSortLinkInit(&OsPercpuGet()->taskSortLink, g_taskSortlink);
    SchedTimeBaseInit();

    return LOS_OK;
}

LosTaskCB *OsSchedStart(VOID)
{
    PRINTK("Entering scheduler\n");

    (VOID)LOS_IntLock();
    LosTaskCB *newTask = OsGetTopTask();

    newTask->taskStatus |= OS_TASK_STATUS_RUNNING;
    OsCurrTaskSet(newTask);

#if (LOSCFG_BASE_CORE_CPUP == 1)
    OsCpupStartToRun(OsCurrTaskGet()->taskId);
#endif

    newTask->startTime = OsGetCurrSchedTimeCycle();
    OsSchedTaskDeQueue(newTask);

    OsTickSysTimerStartTimeSet(newTask->startTime);

#if (LOSCFG_BASE_CORE_SWTMR == 1)
    OsSwtmrResponseTimeReset(newTask->startTime);
#endif

    g_taskScheduled = TRUE;

    OsSchedExpireTimeUpdate(newTask, newTask);

    ArchStartToRun(newTask);
    return newTask;
}

/*
 * Legacy interface — kept for compatibility. Now just calls OsSchedResched.
 * Called from PendSV handler (Cortex-M) when schedFlag indicates a pending
 * reschedule that hasn't been resolved yet.
 */
BOOL OsSchedTaskSwitch(VOID)
{
    LosTaskCB *runTask = OsCurrTaskGet();
    OsTimeSliceUpdate(runTask, OsGetCurrSchedTimeCycle());

    if (runTask->taskStatus & (OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY)) {
        SortLinkAttribute *head = &OsPercpuGet()->taskSortLink;
        OsSchedAddSortLink(head, &runTask->sortList, runTask->waitTimes);
    } else if (!(runTask->taskStatus & OS_TASK_BLOCKED_STATUS)) {
        OsSchedTaskEnQueue(runTask);
    }

    OsSchedResched();
#ifdef LOSCFG_SCHED_LATENCY
    return (OsCurrTaskGet() != g_oldTask);
#else
    return (OsCurrTaskGet() != runTask);
#endif
}

#ifndef LOSCFG_SCHED_LATENCY
/*
 * Just like OsSchedPreempt, except this function will do the OS_INT_ACTIVE
 * check, in case the schedule taken place in the middle of an interrupt.
 */
VOID LOS_Schedule(VOID)
{
    if (OsCheckKernelRunning()) {
        if (OS_INT_ACTIVE) {
            OsSetSchedFlag(INT_PEND_RESCH);
        } else {
            OsSchedPreempt();
        }
    }
}
#else  /* !LOSCFG_SCHED_LATENCY */
/* There is no need to support latency sched interface after interrupt in Cortex-M */
VOID LOS_Schedule(VOID)
{
    if (OsCheckKernelRunning()) {
        OsSchedPreempt();
    }
}
#endif /* LOSCFG_SCHED_LATENCY */
