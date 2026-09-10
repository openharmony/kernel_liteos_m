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

#ifndef _LOS_SCHED_PRI_H
#define _LOS_SCHED_PRI_H

#include "los_sched.h"
#include "los_task_pri.h"
#include "los_interrupt.h"
#include "los_context.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define OS_SCHED_MINI_PERIOD       (g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND_MINI)
#define OS_SCHED_MAX_RESPONSE_TIME OS_SORT_LINK_UINT64_MAX

extern UINT32 g_taskScheduled;
typedef BOOL (*SchedScan)(VOID);

typedef enum {
    INT_NO_RESCH = 0,
    INT_PEND_RESCH,
    INT_SUSPEND_DELETE_RESCH,
} SchedFlag;

#ifndef LOSCFG_SCHED_LATENCY
extern VOID OsSchedProcSchedFlag(VOID);

STATIC INLINE VOID OsSetSchedFlag(UINT32 schedFlag)
{
    OsPercpuGet()->schedFlag = schedFlag;
}
#else
STATIC INLINE VOID OsSchedProcSchedFlag(VOID)
{
}
STATIC INLINE VOID OsSetSchedFlag(UINT32 schedFlag)
{
    (VOID)schedFlag;
}
#endif

/* Check if preemptible with counter flag */
STATIC INLINE BOOL OsPreemptable(VOID)
{
    /*
     * Unlike OsPreemptableInSched, the int may be not disabled when OsPreemptable
     * is called, needs manually disable interrupt, to prevent current task from
     * being migrated to another core, and get the wrong preemptible status.
     */
    UINT32 intSave = LOS_IntLock();
    BOOL preemptible = (OsPercpuGet()->taskLockCnt == 0);
    if (!preemptible) {
        OsSetSchedFlag(INT_PEND_RESCH);
    }

    LOS_IntRestore(intSave);
    return preemptible;
}

STATIC INLINE BOOL OsPreemptableInSched(VOID)
{
    BOOL preemptible = FALSE;

    preemptible = (OsPercpuGet()->taskLockCnt == 0);
    if (!preemptible) {
        OsSetSchedFlag(INT_PEND_RESCH);
    }

    return preemptible;
}

STATIC INLINE UINT64 OsGetCurrSchedTimeCycle(VOID)
{
    return LOS_SysCycleGet();
}

STATIC INLINE BOOL OsCheckKernelRunning(VOID)
{
    return (g_taskScheduled && LOS_CHECK_SCHEDULE);
}

VOID OsSchedResetSchedResponseTime(UINT64 responseTime);

VOID OsSchedSetIdleTaskSchedParam(LosTaskCB *idleTask);

UINT32 OsSchedSwtmrScanRegister(SchedScan func);

VOID OsSchedUpdateExpireTime(VOID);

UINT64 OsSchedGetNextExpireTime(UINT64 startTime);

VOID OsSchedExpireTimeUpdate(LosTaskCB *newTask, LosTaskCB *runTask);

VOID SchedTimeBaseInit(VOID);

VOID OsSchedFreezeTask(LosTaskCB *taskCB);

VOID OsSchedUnfreezeTask(LosTaskCB *taskCB);

BOOL OsSchedTimeSliceReset(LosTaskCB *taskCB);

STATIC INLINE VOID OsTimeSliceUpdate(LosTaskCB *taskCB, UINT64 currTime)
{
    LOS_ASSERT(currTime >= taskCB->startTime);

    INT32 incTime = currTime - taskCB->startTime;
    if (taskCB->taskId != OsPercpuGet()->idleTaskId) {
        taskCB->timeSlice -= incTime;
    }
    taskCB->startTime = currTime;
}

VOID OsSchedTaskDeQueue(LosTaskCB *taskCB);

VOID OsSchedTaskEnQueue(LosTaskCB *taskCB);

VOID OsSchedWait(LosTaskCB *runTask, LOS_DL_LIST *list, UINT32 timeout);

VOID OsSchedWake(LosTaskCB *resumedTask);

BOOL OsSchedPrioModify(LosTaskCB *taskCB, UINT16 priority);

VOID OsSchedDelay(LosTaskCB *runTask, UINT32 tick);

VOID OsSchedYield(VOID);

VOID OsSchedTaskExit(LosTaskCB *taskCB);

VOID OsSchedSuspend(LosTaskCB *taskCB);

BOOL OsSchedResume(LosTaskCB *taskCB);

VOID OsSchedTick(VOID);

UINT32 OsSchedInit(VOID);

LosTaskCB *OsSchedStart(VOID);

BOOL OsSchedTaskSwitch(VOID);

LosTaskCB *OsGetTopTask(VOID);

VOID OsSchedTimeConvertFreq(UINT32 oldFreq);

VOID OsSchedPreempt(VOID);

VOID OsSchedResched(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_SCHED_PRI_H */
