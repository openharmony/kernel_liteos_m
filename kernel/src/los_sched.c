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

#include "los_sched.h"
#include "los_task.h"
#include "los_tick.h"
#include "los_debug.h"
#include "los_hook.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define OS_64BIT_MAX               0xFFFFFFFFFFFFFFFFULL
#define OS_PRIORITY_QUEUE_NUM      32
#define PRIQUEUE_PRIOR0_BIT        0x80000000U
#define OS_SCHED_TIME_SLICES       ((LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT * OS_SYS_NS_PER_US) / OS_NS_PER_CYCLE)
#define OS_TIME_SLICE_MIN          (INT32)((50 * OS_SYS_NS_PER_US) / OS_NS_PER_CYCLE) /* 50us */
#define OS_SCHED_MAX_RESPONSE_TIME (UINT64)(OS_64BIT_MAX - 1U)
#define OS_TICK_RESPONSE_TIME_MAX  LOSCFG_BASE_CORE_TICK_RESPONSE_MAX
#if (LOSCFG_BASE_CORE_TICK_RESPONSE_MAX == 0)
#error "Must specify the maximum value that tick timer counter supports!"
#endif

STATIC SchedScan  g_swtmrScan = NULL;
STATIC SortLinkAttribute *g_taskSortLinkList = NULL;
STATIC LOS_DL_LIST g_priQueueList[OS_PRIORITY_QUEUE_NUM];
STATIC UINT32 g_queueBitmap;

STATIC UINT32 g_schedResponseID = 0;
STATIC UINT64 g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
typedef struct {
    SchedSleepInit           init;
    SchedSleepStart          start;
    SchedSleepStop           stop;
    SchedSleepGetSleepTimeNs getTimeNs;
} SchedSleep;

STATIC BOOL       g_schedSleepFlags = FALSE;
STATIC UINT64     g_schedSleepTime;
STATIC UINT64     g_schedEntrySleepTime;
STATIC SchedSleep g_schedSleepCB;
#endif

#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
STATIC UINT64 g_schedTimerBase;

VOID OsSchedUpdateSchedTimeBase(VOID)
{
    UINT32 period = 0;

    (VOID)HalGetTickCycle(&period);
    g_schedTimerBase += period;
}
#endif

UINT64 OsGetCurrSchedTimeCycle(VOID)
{
    if (!g_taskScheduled) {
        return 0;
    }

#if (LOSCFG_BASE_CORE_TICK_WTIMER == 1)
    return HalGetTickCycle(NULL);
#else
    STATIC UINT64 oldSchedTime = 0;
    UINT32 period = 0;
    UINT32 intSave = LOS_IntLock();
    UINT64 time = HalGetTickCycle(&period);
    UINT64 schedTime = g_schedTimerBase + time;
    if (schedTime < oldSchedTime) {
        /* Turn the timer count */
        g_schedTimerBase += period;
        schedTime = g_schedTimerBase + time;
    }

    LOS_ASSERT(schedTime >= oldSchedTime);

    oldSchedTime = schedTime;
    LOS_IntRestore(intSave);
    return schedTime;
#endif
}

STATIC INLINE VOID OsTimeSliceUpdate(LosTaskCB *taskCB, UINT64 currTime)
{
    LOS_ASSERT(currTime >= taskCB->startTime);

    INT32 incTime = currTime - taskCB->startTime;
    if (taskCB->taskID != g_idleTaskID) {
        taskCB->timeSlice -= incTime;
    }
    taskCB->startTime = currTime;
}

STATIC INLINE VOID OsSchedSetNextExpireTime(UINT64 startTime, UINT32 responseID, UINT64 taskEndTime)
{
    UINT64 nextExpireTime = OsGetNextExpireTime(startTime);
    UINT64 nextResponseTime;
    BOOL isTimeSlice = FALSE;

    /* The current thread's time slice has been consumed, but the current system lock task cannot
     * trigger the schedule to release the CPU
     */
    if (taskEndTime < nextExpireTime) {
        nextExpireTime = taskEndTime;
        isTimeSlice = TRUE;
    }

    if ((g_schedResponseTime > nextExpireTime) && ((g_schedResponseTime - nextExpireTime) >= OS_CYCLE_PER_TICK)) {
        nextResponseTime = nextExpireTime - startTime;
        if (nextResponseTime > OS_TICK_RESPONSE_TIME_MAX) {
#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
            g_schedSleepTime = nextResponseTime - OS_CYCLE_PER_TICK;
#endif
            nextResponseTime = OS_TICK_RESPONSE_TIME_MAX;
            nextExpireTime = startTime + nextResponseTime;
        } else if (nextResponseTime < OS_CYCLE_PER_TICK) {
#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
            g_schedSleepTime = 0;
#endif
            nextResponseTime = OS_CYCLE_PER_TICK;
            nextExpireTime = startTime + nextResponseTime;
            if (nextExpireTime >= g_schedResponseTime) {
                return;
            }
        }
    } else {
        /* There is no point earlier than the current expiration date */
        return;
    }

    if (isTimeSlice) {
        /* The expiration time of the current system is the thread's slice expiration time */
        g_schedResponseID = responseID;
    } else {
        g_schedResponseID = OS_INVALID;
    }

    g_schedResponseTime = nextExpireTime;
#if (LOSCFG_BASE_CORE_TICK_WTIMER == 0)
    g_schedTimerBase = OsGetCurrSchedTimeCycle();
#endif
    HalSysTickReload(nextResponseTime);
}

VOID OsSchedUpdateExpireTime(UINT64 startTime)
{
    UINT64 endTime;
    LosTaskCB *runTask = g_losTask.runTask;

    if (runTask->taskID != g_idleTaskID) {
        INT32 timeSlice = (runTask->timeSlice <= OS_TIME_SLICE_MIN) ? OS_SCHED_TIME_SLICES : runTask->timeSlice;
        endTime = startTime + timeSlice;
    } else {
        endTime = OS_SCHED_MAX_RESPONSE_TIME;
    }
    OsSchedSetNextExpireTime(startTime, runTask->taskID, endTime);
}

STATIC INLINE VOID OsSchedPriQueueEnHead(LOS_DL_LIST *priqueueItem, UINT32 priority)
{
    /*
     * Task control blocks are inited as zero. And when task is deleted,
     * and at the same time would be deleted from priority queue or
     * other lists, task pend node will restored as zero.
     */
    if (LOS_ListEmpty(&g_priQueueList[priority])) {
        g_queueBitmap |= PRIQUEUE_PRIOR0_BIT >> priority;
    }

    LOS_ListAdd(&g_priQueueList[priority], priqueueItem);
}

STATIC INLINE VOID OsSchedPriQueueEnTail(LOS_DL_LIST *priqueueItem, UINT32 priority)
{
    if (LOS_ListEmpty(&g_priQueueList[priority])) {
        g_queueBitmap |= PRIQUEUE_PRIOR0_BIT >> priority;
    }

    LOS_ListTailInsert(&g_priQueueList[priority], priqueueItem);
}

STATIC INLINE VOID OsSchedPriQueueDelete(LOS_DL_LIST *priqueueItem, UINT32 priority)
{
    LOS_ListDelete(priqueueItem);
    if (LOS_ListEmpty(&g_priQueueList[priority])) {
        g_queueBitmap &= ~(PRIQUEUE_PRIOR0_BIT >> priority);
    }
}

STATIC INLINE VOID OsSchedWakePendTimeTask(UINT64 currTime, LosTaskCB *taskCB, BOOL *needSchedule)
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
    LOS_DL_LIST *listObject = &g_taskSortLinkList->sortLink;
    /*
     * When task is pended with timeout, the task block is on the timeout sortlink
     * (per cpu) and ipc(mutex,sem and etc.)'s block at the same time, it can be waken
     * up by either timeout or corresponding ipc it's waiting.
     *
     * Now synchronize sortlink preocedure is used, therefore the whole task scan needs
     * to be protected, preventing another core from doing sortlink deletion at same time.
     */

    if (LOS_ListEmpty(listObject)) {
        return needSchedule;
    }

    SortLinkList *sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    UINT64 currTime = OsGetCurrSchedTimeCycle();
    while (sortList->responseTime <= currTime) {
        LosTaskCB *taskCB = LOS_DL_LIST_ENTRY(sortList, LosTaskCB, sortList);
        OsDeleteNodeSortLink(g_taskSortLinkList, &taskCB->sortList);

        OsSchedWakePendTimeTask(currTime, taskCB, &needSchedule);

        if (LOS_ListEmpty(listObject)) {
            break;
        }

        sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    }

    return needSchedule;
}

VOID OsSchedTaskEnQueue(LosTaskCB *taskCB)
{
    LOS_ASSERT(!(taskCB->taskStatus & OS_TASK_STATUS_READY));

    if (taskCB->taskID != g_idleTaskID) {
        if (taskCB->timeSlice > OS_TIME_SLICE_MIN) {
            OsSchedPriQueueEnHead(&taskCB->pendList, taskCB->priority);
        } else {
            taskCB->timeSlice = OS_SCHED_TIME_SLICES;
            OsSchedPriQueueEnTail(&taskCB->pendList, taskCB->priority);
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
        if (taskCB->taskID != g_idleTaskID) {
            OsSchedPriQueueDelete(&taskCB->pendList, taskCB->priority);
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
        OsDeleteSortLink(&taskCB->sortList, OS_SORT_LINK_TASK);
        taskCB->taskStatus &= ~(OS_TASK_STATUS_DELAY | OS_TASK_STATUS_PEND_TIME);
    }
}

VOID OsSchedYield(VOID)
{
    LosTaskCB *runTask = g_losTask.runTask;

    runTask->timeSlice = 0;
}

VOID OsSchedDelay(LosTaskCB *runTask, UINT32 tick)
{
    runTask->taskStatus |= OS_TASK_STATUS_DELAY;
    runTask->waitTimes = tick;
}

VOID OsSchedTaskWait(LOS_DL_LIST *list, UINT32 ticks)
{
    LosTaskCB *runTask = g_losTask.runTask;

    runTask->taskStatus |= OS_TASK_STATUS_PEND;
    LOS_ListTailInsert(list, &runTask->pendList);

    if (ticks != LOS_WAIT_FOREVER) {
        runTask->taskStatus |= OS_TASK_STATUS_PEND_TIME;
        runTask->waitTimes = ticks;
    }
}

VOID OsSchedTaskWake(LosTaskCB *resumedTask)
{
    LOS_ListDelete(&resumedTask->pendList);
    resumedTask->taskStatus &= ~OS_TASK_STATUS_PEND;

    if (resumedTask->taskStatus & OS_TASK_STATUS_PEND_TIME) {
        OsDeleteSortLink(&resumedTask->sortList, OS_SORT_LINK_TASK);
        resumedTask->taskStatus &= ~OS_TASK_STATUS_PEND_TIME;
    }

    if (!(resumedTask->taskStatus & OS_TASK_STATUS_SUSPEND)) {
        OsSchedTaskEnQueue(resumedTask);
    }
}

BOOL OsSchedModifyTaskSchedParam(LosTaskCB *taskCB, UINT16 priority)
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

VOID OsSchedSetIdleTaskSchedParam(LosTaskCB *idleTask)
{
    OsSchedTaskEnQueue(idleTask);
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
    UINT32 intSave = LOS_IntLock();
    UINT32 ticks = OsSortLinkGetNextExpireTime(g_taskSortLinkList);
    LOS_IntRestore(intSave);
    return ticks;
}

UINT32 OsSchedInit(VOID)
{
    UINT16 pri;
    for (pri = 0; pri < OS_PRIORITY_QUEUE_NUM; pri++) {
        LOS_ListInit(&g_priQueueList[pri]);
    }
    g_queueBitmap = 0;

    g_taskSortLinkList = OsGetSortLinkAttribute(OS_SORT_LINK_TASK);
    if (g_taskSortLinkList == NULL) {
        return LOS_NOK;
    }

    OsSortLinkInit(g_taskSortLinkList);
    g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;

    return LOS_OK;
}

LosTaskCB *OsGetTopTask(VOID)
{
    UINT32 priority;
    LosTaskCB *newTask = NULL;
    if (g_queueBitmap) {
        priority = CLZ(g_queueBitmap);
        newTask = LOS_DL_LIST_ENTRY(((LOS_DL_LIST *)&g_priQueueList[priority])->pstNext, LosTaskCB, pendList);
    } else {
        newTask = OS_TCB_FROM_TID(g_idleTaskID);
    }

    return newTask;
}

VOID OsSchedStart(VOID)
{
    (VOID)LOS_IntLock();
    LosTaskCB *newTask = OsGetTopTask();

    newTask->taskStatus |= OS_TASK_STATUS_RUNNING;
    g_losTask.newTask = newTask;
    g_losTask.runTask = g_losTask.newTask;

    g_taskScheduled = 1;
    newTask->startTime = OsGetCurrSchedTimeCycle();
    OsSchedTaskDeQueue(newTask);

    g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
    g_schedResponseID = OS_INVALID;
    OsSchedSetNextExpireTime(newTask->startTime, newTask->taskID, newTask->startTime + newTask->timeSlice);

    PRINTK("Entering scheduler\n");
}

BOOL OsSchedTaskSwitch(VOID)
{
    UINT64 endTime;
    BOOL isTaskSwitch = FALSE;
    LosTaskCB *runTask = g_losTask.runTask;
    OsTimeSliceUpdate(runTask, OsGetCurrSchedTimeCycle());

    if (runTask->taskStatus & (OS_TASK_STATUS_PEND_TIME | OS_TASK_STATUS_DELAY)) {
        OsAdd2SortLink(&runTask->sortList, runTask->startTime, runTask->waitTimes, OS_SORT_LINK_TASK);
    } else if (!(runTask->taskStatus & (OS_TASK_STATUS_PEND | OS_TASK_STATUS_SUSPEND | OS_TASK_STATUS_UNUSED))) {
        OsSchedTaskEnQueue(runTask);
    }

    LosTaskCB *newTask = OsGetTopTask();
    g_losTask.newTask = newTask;

    if (runTask != newTask) {
#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
        OsTaskSwitchCheck();
#endif
        runTask->taskStatus &= ~OS_TASK_STATUS_RUNNING;
        newTask->taskStatus |= OS_TASK_STATUS_RUNNING;
        newTask->startTime = runTask->startTime;
        isTaskSwitch = TRUE;

        OsHookCall(LOS_HOOK_TYPE_TASK_SWITCHEDIN);
    }

    OsSchedTaskDeQueue(newTask);

    if (newTask->taskID != g_idleTaskID) {
        endTime = newTask->startTime + newTask->timeSlice;
    } else {
        endTime = OS_SCHED_MAX_RESPONSE_TIME;
    }
    OsSchedSetNextExpireTime(newTask->startTime, newTask->taskID, endTime);

    return isTaskSwitch;
}

VOID LOS_SchedTickHandler(VOID)
{
    UINT64 currTime;
    BOOL needSched = FALSE;

    LOS_ASSERT(g_taskScheduled);

    UINT32 intSave = LOS_IntLock();

    if (g_schedResponseID == OS_INVALID) {
        if (g_swtmrScan != NULL) {
            needSched = g_swtmrScan();
        }

        needSched |= OsSchedScanTimerList();
    }

    g_schedResponseTime = OS_SCHED_MAX_RESPONSE_TIME;
    if (needSched && LOS_CHECK_SCHEDULE) {
        HalTaskSchedule();
    } else {
        currTime = OsGetCurrSchedTimeCycle();
        OsTimeSliceUpdate(g_losTask.runTask, currTime);
        OsSchedUpdateExpireTime(currTime);
    }

    LOS_IntRestore(intSave);
}

VOID LOS_Schedule(VOID)
{
    if (g_taskScheduled && LOS_CHECK_SCHEDULE) {
        HalTaskSchedule();
    }
}

#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
VOID OsSchedUpdateSleepTime(VOID)
{
    UINT64 nextResponseTime;
    UINT64 currTime, realSleepTime;
    UINT32 intSave;

    if ((g_schedSleepFlags == FALSE) || (g_schedSleepCB.stop == NULL)) {
        return;
    }

    intSave = LOS_IntLock();
    if (g_schedSleepCB.getTimeNs != NULL) {
        realSleepTime = g_schedSleepCB.getTimeNs();
        realSleepTime = (realSleepTime / OS_SYS_NS_PER_SECOND) * OS_SYS_CLOCK +
                        (realSleepTime % OS_SYS_NS_PER_SECOND) * OS_SYS_CLOCK / OS_SYS_NS_PER_SECOND;
        if (realSleepTime < g_schedSleepTime) {
            nextResponseTime = g_schedSleepTime - realSleepTime;
        } else {
            nextResponseTime = 0;
        }

#if (LOSCFG_BASE_CORE_TICK_WTIMER == 1)
        currTime = HalGetTickCycle(NULL);
#else
        g_schedTimerBase = g_schedEntrySleepTime + realSleepTime;
        currTime = g_schedTimerBase;
#endif
        if (nextResponseTime > OS_TICK_RESPONSE_TIME_MAX) {
            nextResponseTime = OS_TICK_RESPONSE_TIME_MAX;
        } else if (nextResponseTime < OS_CYCLE_PER_TICK) {
            nextResponseTime = OS_CYCLE_PER_TICK;
        }

        g_schedResponseID = OS_INVALID;
        g_schedResponseTime = currTime + nextResponseTime;
        HalSysTickReload(nextResponseTime);
        g_schedSleepTime = 0;
    }
    g_schedSleepFlags = FALSE;
    g_schedSleepCB.stop();
    LOS_IntRestore(intSave);
}

VOID OsSchedToSleep(VOID)
{
    UINT32 intSave;
    UINT64 sleepTime;

    if (g_schedSleepCB.start == NULL) {
        return;
    }

    if (g_schedSleepCB.getTimeNs != NULL) {
        sleepTime = (g_schedSleepTime / OS_SYS_CLOCK) * OS_SYS_NS_PER_SECOND +
                    (g_schedSleepTime % OS_SYS_CLOCK) * OS_SYS_NS_PER_SECOND / OS_SYS_CLOCK;
        if (sleepTime == 0) {
            return;
        }

        intSave = LOS_IntLock();
        HalTickLock();
        g_schedEntrySleepTime = OsGetCurrSchedTimeCycle();
    } else {
        intSave = LOS_IntLock();
    }

    g_schedSleepCB.start(sleepTime);
    g_schedSleepFlags = TRUE;
    LOS_IntRestore(intSave);
}

UINT32 LOS_SchedSleepInit(SchedSleepInit init, SchedSleepStart start,
                          SchedSleepStop stop, SchedSleepGetSleepTimeNs getTime)
{
    UINT32 ret;

    if ((init == NULL) && (start == NULL) && (stop == NULL)) {
        return LOS_NOK;
    }

    g_schedSleepCB.init = init;
    g_schedSleepCB.start = start;
    g_schedSleepCB.stop = stop;
    g_schedSleepCB.getTimeNs = getTime;

    if (g_schedSleepCB.init != NULL) {
        ret = g_schedSleepCB.init();
        if (ret != LOS_OK) {
            return ret;
        }
    }

    return LOS_OK;
}
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
