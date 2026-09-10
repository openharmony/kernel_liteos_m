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
 * "AS IS" IS ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
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

#include "los_cpup_pri.h"
#include "securec.h"
#include "los_memory.h"
#include "los_debug.h"
#include "los_tick.h"
#include "los_percpu_pri.h"

#if (LOSCFG_BASE_CORE_SWTMR == 1)
#include "los_swtmr.h"
#endif

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
#include "los_arch_interrupt.h"
#endif

#if (LOSCFG_BASE_CORE_CPUP == 1)

#define OS_CPUP_TASK_CB_NUM  (LOSCFG_BASE_CORE_TSK_LIMIT + 1)
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
#define OS_CPUP_CB_MAX_NUM   (OS_CPUP_TASK_CB_NUM + LOSCFG_CPUP_IRQ_CB_NUM)
#else
#define OS_CPUP_CB_MAX_NUM   OS_CPUP_TASK_CB_NUM
#endif

#if (OS_CPUP_CB_MAX_NUM > UINT16_MAX)
#error "Too many cpup control block!"
#endif

#if ((LOSCFG_CPUP_SAMPLE_PERIOD * 1000) / LOSCFG_BASE_CORE_TICK_PER_SECOND < 100)
#error "The length of sampling period time must be no less than 100ms. \
Please redefine the item LOSCFG_CPUP_SAMPLE_PERIOD!"
#elif ((LOSCFG_CPUP_SAMPLE_PERIOD * 1000) / LOSCFG_BASE_CORE_TICK_PER_SECOND > 10000)
#error "The length of sampling period time must be no greater than 10s. \
Please redefine the item LOSCFG_CPUP_SAMPLE_PERIOD!"
#endif

#define CPUP_STATUS_STOPED  0
#define CPUP_STATUS_STARTED 1

LITE_OS_SEC_BSS STATIC UINT32 g_cpupSwtmrId;
LITE_OS_SEC_BSS STATIC UINT16 g_cpupStatus = CPUP_STATUS_STOPED;
LITE_OS_SEC_BSS OsCpupCB  *g_cpup = NULL;

LITE_OS_SEC_BSS STATIC UINT64 g_cpuHistoryTime[OS_CPUP_HISTORY_RECORD_NUM];
LITE_OS_SEC_BSS STATIC UINT16 g_hisPos = 0; /* next Sampling point of g_cpuHistoryTime */

LITE_OS_SEC_BSS STATIC UINT64 g_startCycles = 0;

/* spinlock for cpup module, only available on SMP mode */
LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_cpupSpin);
LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_cpupTaskSpin);

STATIC struct {
    UINT64 taskSwitchTime;
    UINT32 curTaskId;
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
#ifdef LOSCFG_KERNEL_SMP
    UINT32 curHwiNum;
#endif
    UINT64 irqTimePerTaskSwitch;
    UINT64 irqStartTime;
#endif
} g_coreCpup[LOSCFG_KERNEL_CORE_NUM];

#ifdef LOSCFG_CPUP_CB_NUM_CONFIGURABLE
/* UINT16 type determines the upper limit of OS_CPUP_CB_MAX_NUM */
STATIC UINT16 g_cpupIrqCBIdx[LOSCFG_PLATFORM_HWI_LIMIT];
#endif

#define HIGH_BITS                         32

#define CPUP_PRE_POS(pos)                 (((pos) == 0) ? (OS_CPUP_HISTORY_RECORD_NUM - 1) : ((pos) - 1))
#define CPUP_POST_POS(pos)                (((pos) == (OS_CPUP_HISTORY_RECORD_NUM - 1)) ? 0 : ((pos) + 1))

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
#ifdef LOSCFG_CPUP_CB_NUM_CONFIGURABLE
LITE_OS_SEC_TEXT_INIT UINT32 OsCpupCBIrqCreate(HWI_HANDLE_T hwiNum)
{
    STATIC UINT16 freeIrqCBIdx = OS_CPUP_TASK_CB_NUM;
    UINT32 ret = LOS_OK;
    UINT32 intSave;

    LOS_SpinLockSave(&g_cpupSpin, &intSave);
    if (g_cpupIrqCBIdx[hwiNum] != 0) {
        goto UNLOCK;
    }
    if (freeIrqCBIdx >= OS_CPUP_CB_MAX_NUM) {
        ret = LOS_ERRNO_CPUP_NO_MEMORY;
        goto UNLOCK;
    }
    g_cpupIrqCBIdx[hwiNum] = freeIrqCBIdx;
    ++freeIrqCBIdx;

UNLOCK:
    LOS_SpinUnlockRestore(&g_cpupSpin, intSave);
    return ret;
}

LITE_OS_SEC_TEXT_INIT UINT32 OsCpupIrqCBIdGet(HWI_HANDLE_T hwiNum, BOOL isTaskExcluded)
{
    UINT16 index = g_cpupIrqCBIdx[hwiNum];
    if (index == 0) {
        return UINT32_MAX;
    }

    if (isTaskExcluded == TRUE) {
        index -= OS_CPUP_TASK_CB_NUM;
    }

    return (UINT32)index;
}
#else
LITE_OS_SEC_TEXT_INIT UINT32 OsCpupCBIrqCreate(HWI_HANDLE_T hwiNum)
{
    (VOID)hwiNum;
    return LOS_OK;
}

LITE_OS_SEC_TEXT_INIT UINT32 OsCpupIrqCBIdGet(HWI_HANDLE_T hwiNum, BOOL isTaskExcluded)
{
    UINT32 index = hwiNum;
    if (isTaskExcluded == FALSE) {
        index += OS_CPUP_TASK_CB_NUM;
    }

    return index;
}
#endif
#endif /* LOSCFG_CPUP_INCLUDE_IRQ */

LITE_OS_SEC_TEXT_INIT OsCpupCB *OsCpupCBGet(UINT32 index)
{
    return &g_cpup[index];
}

#if (LOSCFG_BASE_CORE_SWTMR == 1)
LITE_OS_SEC_TEXT_INIT VOID OsCpupGuard(VOID)
{
    UINT16 prevPos = g_hisPos;
    UINT16 loop;
    UINT16 runTaskId;
    UINT64 curCycle;
    UINT32 intSave;
#if defined(LOSCFG_CPUP_INCLUDE_IRQ) && defined(LOSCFG_KERNEL_SMP)
    UINT32 curHwiIdx;
#endif

    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return;
    }
    LOS_SpinLockSave(&g_cpupTaskSpin, &intSave);
    for (loop = 0; loop < OS_CPUP_CB_MAX_NUM; loop++) {
        g_cpup[loop].curTime = g_cpup[loop].allTime;
        g_cpup[loop].historyTime[prevPos] = (UINT32)g_cpup[loop].curTime;
    }

    for (loop = 0; loop < LOSCFG_KERNEL_CORE_NUM; loop++) {
        runTaskId = (UINT16)g_coreCpup[loop].curTaskId;
        /* reacquire the cycle to prevent flip */
        curCycle = OsCpupGetCycle();
        g_cpup[runTaskId].curTime += curCycle - g_coreCpup[loop].taskSwitchTime;
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
        g_cpup[runTaskId].curTime -= g_coreCpup[loop].irqTimePerTaskSwitch;
#endif
        g_cpup[runTaskId].historyTime[prevPos] = (UINT32)g_cpup[runTaskId].curTime;
#if defined(LOSCFG_CPUP_INCLUDE_IRQ) && defined(LOSCFG_KERNEL_SMP)
        if (g_coreCpup[loop].curHwiNum != OS_HWI_INVALID_IRQ) {
            curHwiIdx = OsCpupIrqCBIdGet(g_coreCpup[loop].curHwiNum, FALSE);
            g_cpup[runTaskId].curTime -= curCycle - g_coreCpup[loop].irqStartTime;
            g_cpup[curHwiIdx].curTime += curCycle - g_coreCpup[loop].irqStartTime;
            g_cpup[curHwiIdx].historyTime[prevPos] = g_cpup[curHwiIdx].curTime;
        }
#endif
    }

    g_cpuHistoryTime[prevPos] = OsCpupGetCycle();
    g_hisPos = CPUP_POST_POS(g_hisPos);
    LOS_SpinUnlockRestore(&g_cpupTaskSpin, intSave);
}

LITE_OS_SEC_TEXT_INIT VOID OsCpupGuardCreator(VOID)
{
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    (VOID)LOS_SwtmrCreate(LOSCFG_CPUP_SAMPLE_PERIOD, LOS_SWTMR_MODE_PERIOD,
                          (SWTMR_PROC_FUNC)OsCpupGuard, &g_cpupSwtmrId, 0,
                          OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE);
#else
    (VOID)LOS_SwtmrCreate(LOSCFG_CPUP_SAMPLE_PERIOD, LOS_SWTMR_MODE_PERIOD,
                          (SWTMR_PROC_FUNC)OsCpupGuard, &g_cpupSwtmrId, 0);
#endif

    (VOID)LOS_SwtmrStart(g_cpupSwtmrId);
}

LITE_OS_SEC_TEXT_INIT VOID OsCpupGuardInit(VOID)
{
    TSK_INIT_PARAM_S taskInitParam = {0};
    UINT32 tempId;

    taskInitParam.pfnTaskEntry  = (TSK_ENTRY_FUNC)OsCpupGuardCreator;
    taskInitParam.uwStackSize   = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    taskInitParam.pcName        = "CpupGuardCreator";
    taskInitParam.usTaskPrio    = LOS_TASK_PRIORITY_HIGHEST;
#ifdef LOSCFG_KERNEL_SMP
    taskInitParam.usCpuAffiMask = CPUID_TO_AFFI_MASK(ArchCurrCpuid());
#endif
    (VOID)LOS_TaskCreate(&tempId, &taskInitParam);
}
#endif

/*
 * Description: initialization of CPUP
 * Return     : LOS_OK or Error Information
 */
LITE_OS_SEC_TEXT_INIT UINT32 OsCpupInit(VOID)
{
    UINT32 size;

    /* every task has only one record, and it won't operated at the same time */
    size = OS_CPUP_CB_MAX_NUM * sizeof(OsCpupCB);
    g_cpup = (OsCpupCB *)LOS_MemAlloc(m_aucSysMem0, size);
    if (g_cpup == NULL) {
        return LOS_ERRNO_CPUP_NO_MEMORY;
    }

#if (LOSCFG_BASE_CORE_SWTMR == 1)
    OsCpupGuardInit();
#endif

    (VOID)memset_s(g_cpup, size, 0, size);

    g_cpupStatus = CPUP_STATUS_STARTED;

    return LOS_OK;
}
LOS_SYS_INIT(OsCpupInit, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_1);

LITE_OS_SEC_TEXT_INIT VOID OsCpupCBTaskCreate(UINT32 taskId, UINT16 taskStatus)
{
    OsCpupCB *cpup = OsCpupCBGet(taskId);

    (VOID)memset_s((VOID *)cpup, sizeof(OsCpupCB), 0, sizeof(OsCpupCB));
    cpup->status = taskStatus;
}

LITE_OS_SEC_TEXT_INIT VOID OsCpupCBTaskDelete(UINT32 taskId)
{
    OsCpupCBGet(taskId)->status = OS_TASK_STATUS_UNUSED;
}

STATIC LITE_OS_SEC_TEXT_INIT VOID OsCpupCbReset(VOID)
{
    UINT16 loop;
    UINT32 cpupIndex;
    UINT32 intSave;

    if (g_cpup == NULL) {
        return;
    }

    SCHEDULER_LOCK(intSave);
    for (loop = 0; loop < OS_CPUP_HISTORY_RECORD_NUM; loop++) {
        g_cpuHistoryTime[loop] = 0;
    }

    for (cpupIndex = 0; cpupIndex < OS_CPUP_CB_MAX_NUM; cpupIndex++) {
        g_cpup[cpupIndex].allTime = 0;
        g_cpup[cpupIndex].curTime = 0;
        for (loop = 0; loop < OS_CPUP_HISTORY_RECORD_NUM; loop++) {
            g_cpup[cpupIndex].historyTime[loop] = 0;
        }
    }

    for (loop = 0; loop < LOSCFG_KERNEL_CORE_NUM; loop++) {
        g_coreCpup[loop].taskSwitchTime = 0;
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
        g_coreCpup[loop].irqTimePerTaskSwitch = 0;
#endif
    }
    SCHEDULER_UNLOCK(intSave);

    OsCpupSetCycle(0);
}

VOID OsCpupStart(VOID)
{
    UINT32 intSave;
    if (g_cpupStatus == CPUP_STATUS_STARTED) {
        return;
    }
    OsCpupCbReset();
    intSave = LOS_IntLock();
    (VOID)LOS_SwtmrStart(g_cpupSwtmrId);
    LOS_IntRestore(intSave);
    g_cpupStatus = CPUP_STATUS_STARTED;
}

VOID OsCpupStop(VOID)
{
    UINT32 intSave;
    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return;
    }
    intSave = LOS_IntLock();
    (VOID)LOS_SwtmrStop(g_cpupSwtmrId);
    LOS_IntRestore(intSave);
    g_cpupStatus = CPUP_STATUS_STOPED;
}

#ifdef LOSCFG_CPUP_START_STOP
VOID LOS_CpupStart(VOID)
{
    OsCpupStart();
}

VOID LOS_CpupStop(VOID)
{
    OsCpupStop();
}
#endif

LITE_OS_SEC_TEXT_INIT VOID LOS_CpupReset(VOID)
{
    OsCpupStop();
    OsCpupStart();
}

LITE_OS_SEC_TEXT_MINOR VOID OsCpupSetCycle(UINT64 startCycles)
{
    UINT32 intSave;

    LOS_SpinLockSave(&g_cpupSpin, &intSave);
    g_startCycles = startCycles;
    LOS_SpinUnlockRestore(&g_cpupSpin, intSave);
}

/*
 * Description: get current cycles count
 * Return     : current cycles count
 */
LITE_OS_SEC_TEXT_MINOR UINT64 OsCpupGetCycle(VOID)
{
    UINT32 intSave;
    UINT64 cycles;

    LOS_SpinLockSave(&g_cpupSpin, &intSave);

    cycles = LOS_SysCycleGet();
    if (g_startCycles == 0) {
        g_startCycles = cycles;
    }

    /*
     * The cycles should keep growing, if the checking failed,
     * it mean LOS_SysCycleGet has the problem which should be fixed.
     */
    LOS_ASSERT(cycles >= g_startCycles);

    LOS_SpinUnlockRestore(&g_cpupSpin, intSave);

    return (cycles - g_startCycles);
}

LITE_OS_SEC_TEXT_INIT UINT16 OsCpupHisPosGet(VOID)
{
    return g_hisPos;
}

/*
 * Description: the time when first task is scheduled to one core is recorded,
 *              ensure that the start time of the task is earlier than the
 *              interrupt time (so that irq time subtracted from task time
 *              will not underflow).
 */
LITE_OS_SEC_TEXT_MINOR VOID OsCpupStartToRun(UINT32 taskId)
{
    UINT32 cpuId;

    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return;
    }

    cpuId = ArchCurrCpuid();
    g_coreCpup[cpuId].curTaskId = taskId;
    g_coreCpup[cpuId].taskSwitchTime = OsCpupGetCycle();
}

/*
 * Description: start task to get cycles count in current task ending
 */
LITE_OS_SEC_TEXT_MINOR VOID OsTaskCycleEndStart(const LosTaskCB *newTask)
{
    UINT64 cpuCycle;
    LosTaskCB *runTask = NULL;
    OsCpupCB *cpup = NULL;
    UINT32 cpuId = ArchCurrCpuid();

    if (newTask == NULL) {
        return;
    }

    LOS_SpinLock(&g_cpupTaskSpin);
    if (g_cpupStatus == CPUP_STATUS_STARTED) {
        runTask = OsCurrTaskGet();
        cpuCycle = OsCpupGetCycle();

        cpup = &g_cpup[runTask->taskId];
        cpup->allTime += cpuCycle - g_coreCpup[cpuId].taskSwitchTime;
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
        cpup->allTime -= g_coreCpup[cpuId].irqTimePerTaskSwitch;
        g_coreCpup[cpuId].irqTimePerTaskSwitch = 0;
#endif

        g_coreCpup[cpuId].taskSwitchTime = cpuCycle;
    }
    g_coreCpup[cpuId].curTaskId = newTask->taskId;
    LOS_SpinUnlock(&g_cpupTaskSpin);
}

LITE_OS_SEC_TEXT_MINOR STATIC VOID OsCpupGetPos(UINT32 mode, UINT16 *curPosPointer, UINT16 *prePosPointer)
{
    UINT16 curPos;
    UINT16 tmpPos;
    UINT16 prePos;

    tmpPos = g_hisPos;
    curPos = CPUP_PRE_POS(tmpPos);

    /*
     * The current position has nothing to do with the CPUP modes,
     * however, the previous position differs.
     */
    switch (mode) {
        case CPUP_IN_1S:
            prePos = CPUP_PRE_POS(curPos);
            break;
        case CPUP_IN_10S:
            prePos = tmpPos;
            break;
        case CPUP_ALL_TIME:
            /* fall-through */
        default:
            prePos = OS_CPUP_HISTORY_RECORD_NUM; /* invalid index for all-time mode */
            break;
    }

    *curPosPointer = curPos;
    *prePosPointer = prePos;

    return;
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE UINT32 OsCpuUsageParaCheck(UINT32 taskId)
{
    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return LOS_ERRNO_CPUP_NO_INIT;
    }

    if (OS_TSK_GET_INDEX(taskId) >= g_taskMaxNum) {
        return LOS_ERRNO_CPUP_TSK_ID_INVALID;
    }

    /* weather the task is created */
    if ((g_cpup[taskId].status & OS_TASK_STATUS_UNUSED) || (g_cpup[taskId].status == 0)) {
        return LOS_ERRNO_CPUP_THREAD_NO_CREATED;
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE UINT64 OsCpupCpuCycleGet(UINT32 mode, UINT16 pos, UINT16 prePos)
{
    if ((mode == CPUP_IN_1S) || (mode == CPUP_IN_10S)) {
        return g_cpuHistoryTime[pos] - g_cpuHistoryTime[prePos];
    } else {
        return g_cpuHistoryTime[pos];
    }
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE UINT64 TaskIntervalCycleGet(UINT32 index, UINT16 pos, UINT16 prePos)
{
    UINT64 sum = 0;
    UINT16 i = prePos;
    UINT16 next;

    do {
        next = CPUP_POST_POS(i);
        /* NOTICE: (UINT32) is used to ensure that higher 32-bit is ignored */
        sum += (UINT64)(UINT32)(g_cpup[index].historyTime[next] + 0x100000000ULL - g_cpup[index].historyTime[i]);
        i = next;
    } while (i != pos);

    return sum;
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE UINT64 TaskAllCycleGet(UINT32 index)
{
    return g_cpup[index].curTime;
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE UINT64 OsCpupTaskCycleGet(UINT32 index, UINT32 mode, UINT16 pos, UINT16 prePos)
{
    if ((mode == CPUP_IN_1S) || (mode == CPUP_IN_10S)) {
        return TaskIntervalCycleGet(index, pos, prePos);
    } else {
        return TaskAllCycleGet(index);
    }
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_SysCpuUsage(VOID)
{
    return LOS_HistorySysCpuUsage(CPUP_ALL_TIME);
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_HistorySysCpuUsage(UINT32 mode)
{
    UINT64 cpuCycleAll;
    UINT64 idleCycleAll = 0;
    UINT32 cpupRet = 0;
    UINT16 pos;
    UINT16 prePos;
    UINT32 intSave;
    UINT32 idleTaskId;
#ifdef LOSCFG_KERNEL_SMP
    UINT32 cpuId = 0;
#endif

    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return LOS_ERRNO_CPUP_NO_INIT;
    }

    /* get end time of current task */
    intSave = LOS_IntLock();

    OsCpupGetPos(mode, &pos, &prePos);
    cpuCycleAll = OsCpupCpuCycleGet(mode, pos, prePos);

#ifdef LOSCFG_KERNEL_SMP
    LOS_SpinLock(&g_cpupTaskSpin);
    /* For SMP system, each idle task needs to be accounted */
    while (cpuId < LOSCFG_KERNEL_CORE_NUM) {
        idleTaskId = g_percpu[cpuId].idleTaskId;
        idleCycleAll += OsCpupTaskCycleGet(idleTaskId, mode, pos, prePos);
        cpuId++;
    }
    cpuCycleAll *= LOSCFG_KERNEL_CORE_NUM;
    LOS_SpinUnlock(&g_cpupTaskSpin);
#else
    idleTaskId = OsGetIdleTaskId();
    idleCycleAll = OsCpupTaskCycleGet(idleTaskId, mode, pos, prePos);
#endif

    if (cpuCycleAll) {
        cpupRet = (LOS_CPUP_PRECISION - (UINT32)((LOS_CPUP_PRECISION * idleCycleAll) / cpuCycleAll));
    }

    LOS_IntRestore(intSave);

    return cpupRet;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskCpuUsage(UINT32 taskId)
{
    return LOS_HistoryTaskCpuUsage(taskId, CPUP_ALL_TIME);
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_HistoryTaskCpuUsage(UINT32 taskId, UINT32 mode)
{
    UINT64 cpuCycleAll;
    UINT64 cpuCycleCurTask;
    UINT16 pos;
    UINT16 prePos;
    UINT32 intSave;
    UINT32 cpupRet = 0;
    UINT32 ret;

    intSave = LOS_IntLock();

    ret = OsCpuUsageParaCheck(taskId);
    if (ret != LOS_OK) {
        LOS_IntRestore(intSave);
        return ret;
    }

    OsCpupGetPos(mode, &pos, &prePos);
    cpuCycleAll = OsCpupCpuCycleGet(mode, pos, prePos);
    cpuCycleCurTask = OsCpupTaskCycleGet(taskId, mode, pos, prePos);
    if (cpuCycleAll) {
        cpupRet = (UINT32)((LOS_CPUP_PRECISION * cpuCycleCurTask) / cpuCycleAll);
    }

    LOS_IntRestore(intSave);

    return cpupRet;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_AllTaskCpuUsage(CPUP_INFO_S *cpupInfo, UINT16 mode)
{
    UINT16 loopNum;
    UINT16 pos;
    UINT16 prePos;
    UINT32 intSave;
    UINT64 cpuCycleAll;
    UINT64 cpuCycleCurTsk;

    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return LOS_ERRNO_CPUP_NO_INIT;
    }
    if (cpupInfo == NULL) {
        return LOS_ERRNO_CPUP_TASK_PTR_NULL;
    }

    intSave = LOS_IntLock();
    OsCpupGetPos(mode, &pos, &prePos);
    cpuCycleAll = OsCpupCpuCycleGet(mode, pos, prePos);

    for (loopNum = 0; loopNum < g_taskMaxNum; loopNum++) {
        if ((g_cpup[loopNum].status & OS_TASK_STATUS_UNUSED) || (g_cpup[loopNum].status == 0)) {
            continue;
        }
        cpuCycleCurTsk = OsCpupTaskCycleGet(loopNum, mode, pos, prePos);
        cpupInfo[loopNum].usStatus = g_cpup[loopNum].status;
        if (cpuCycleAll) {
            cpupInfo[loopNum].uwUsage = (UINT32)((LOS_CPUP_PRECISION * cpuCycleCurTsk) / cpuCycleAll);
        }
    }
    LOS_IntRestore(intSave);

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_CpupUsageMonitor(CPUP_TYPE_E type, CPUP_MODE_E mode, UINT32 taskId)
{
    UINT32 ret;
    LosTaskCB *taskCB = NULL;

    switch (type) {
        case SYS_CPU_USAGE:
            if (mode == CPUP_IN_10S) {
                PRINTK("\nSysCpuUsage in 10s: ");
            } else if (mode == CPUP_IN_1S) {
                PRINTK("\nSysCpuUsage in 1s: ");
            } else {
                PRINTK("\nSysCpuUsage all time: ");
            }
            ret = LOS_HistorySysCpuUsage(mode);
            PRINTK("%d.%d", ret / LOS_CPUP_PRECISION_MULT, ret % LOS_CPUP_PRECISION_MULT);
            break;

        case TASK_CPU_USAGE:
            if (taskId > LOSCFG_BASE_CORE_TSK_LIMIT) {
                PRINT_ERR("\nThe taskid is invalid.\n");
                return OS_ERROR;
            }
            taskCB = OS_TCB_FROM_TID(taskId);
            if ((taskCB->taskStatus & OS_TASK_STATUS_UNUSED)) {
                PRINT_ERR("\nThe taskid is invalid.\n");
                return OS_ERROR;
            }
            if (mode == CPUP_IN_10S) {
                PRINTK("\nCPUusage of taskID %d in 10s: ", taskId);
            } else if (mode == CPUP_IN_1S) {
                PRINTK("\nCPUusage of taskID %d in 1s: ", taskId);
            } else {
                PRINTK("\nCPUusage of taskID %d all time: ", taskId);
            }
            ret = LOS_HistoryTaskCpuUsage(taskId, mode);
            PRINTK("%u.%u", ret / LOS_CPUP_PRECISION_MULT, ret % LOS_CPUP_PRECISION_MULT);
            break;

        default:
            PRINT_ERR("\nThe type is invalid.\n");
            return OS_ERROR;
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_AllCpuUsage(UINT16 maxNum, CPUP_INFO_S *cpupInfo, UINT32 mode, UINT16 flag)
{
#ifndef LOSCFG_CPUP_INCLUDE_IRQ
    (VOID)flag;
#endif
    UINT16 loop;
    UINT16 pos;
    UINT16 prePos;
    UINT32 intSave;
    UINT64 cpuCycleAll;
    UINT64 cpuCycleCurTask;
    UINT16 numTmpMax = maxNum;
    UINT16 numTmpMin = 0;
    UINT16 numMax = OS_CPUP_TASK_CB_NUM;

    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return LOS_ERRNO_CPUP_NO_INIT;
    }

    if (cpupInfo == NULL) {
        return LOS_ERRNO_CPUP_TASK_PTR_NULL;
    }

    if (maxNum == 0) {
        return LOS_ERRNO_CPUP_MAXNUM_INVALID;
    }

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    if (flag == 0) {
        if (numTmpMax > OS_NULL_SHORT - OS_CPUP_TASK_CB_NUM) {
            return LOS_ERRNO_CPUP_MAXNUM_INVALID;
        }
        numTmpMax += OS_CPUP_TASK_CB_NUM;
        numTmpMin += OS_CPUP_TASK_CB_NUM;
        numMax = OS_CPUP_CB_MAX_NUM;
    }
#endif

    if (numTmpMax > numMax) {
        numTmpMax = numMax;
    }

    LOS_SpinLockSave(&g_cpupTaskSpin, &intSave);

    OsCpupGetPos(mode, &pos, &prePos);
    cpuCycleAll = OsCpupCpuCycleGet(mode, pos, prePos);

    for (loop = numTmpMin; loop < numTmpMax; loop++) {
        if ((g_cpup[loop].status & OS_TASK_STATUS_UNUSED) || (g_cpup[loop].status == 0)) {
            continue;
        }
        cpuCycleCurTask = OsCpupTaskCycleGet(loop, mode, pos, prePos);
        cpupInfo[loop - numTmpMin].usStatus = g_cpup[loop].status;

        if (cpuCycleAll) {
            cpupInfo[loop - numTmpMin].uwUsage = (UINT32)((LOS_CPUP_PRECISION * cpuCycleCurTask) / cpuCycleAll);
        }
    }

    LOS_SpinUnlockRestore(&g_cpupTaskSpin, intSave);

    return LOS_OK;
}

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
LITE_OS_SEC_TEXT_MINOR STATIC INLINE VOID OsCpupAddIrqCycles(UINT32 cpuId, UINT32 hwiNum, UINT64 cycles)
{
    UINT32 idx = OsCpupIrqCBIdGet(hwiNum, FALSE);
    if ((idx < OS_CPUP_TASK_CB_NUM) || (idx >= OS_CPUP_CB_MAX_NUM)) {
        return;
    }
    g_cpup[idx].status = OS_TASK_STATUS_RUNNING;
    g_cpup[idx].allTime += cycles; /* may have race conditions in SMP */
    g_coreCpup[cpuId].irqTimePerTaskSwitch += cycles;
}

LITE_OS_SEC_TEXT_MINOR VOID OsCpupIrqStart(UINT32 prevHwi, UINT32 currHwi)
{
#ifndef LOSCFG_KERNEL_SMP
    (VOID)currHwi;
#endif
    UINT64 cycles;
    UINT64 intStartTime = OsCpupGetCycle();
    UINT32 cpuId = ArchCurrCpuid();

    LOS_SpinLock(&g_cpupTaskSpin);
    if (prevHwi != OS_HWI_INVALID_IRQ) {
        cycles = intStartTime - g_coreCpup[cpuId].irqStartTime;
        OsCpupAddIrqCycles(cpuId, prevHwi, cycles);
    }
    g_coreCpup[cpuId].irqStartTime = intStartTime;
#ifdef LOSCFG_KERNEL_SMP
    g_coreCpup[cpuId].curHwiNum = currHwi;
#endif
    LOS_SpinUnlock(&g_cpupTaskSpin);
}

LITE_OS_SEC_TEXT_MINOR VOID OsCpupIrqEnd(UINT32 prevHwi, UINT32 currHwi)
{
#ifndef LOSCFG_KERNEL_SMP
    (VOID)prevHwi;
#endif
    UINT64 cycles;
    UINT64 intTimeEnd = OsCpupGetCycle();
    UINT32 cpuId = ArchCurrCpuid();

    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return;
    }

    LOS_SpinLock(&g_cpupTaskSpin);
    cycles = intTimeEnd - g_coreCpup[cpuId].irqStartTime;
    g_coreCpup[cpuId].irqStartTime = intTimeEnd;
#ifdef LOSCFG_KERNEL_SMP
    g_coreCpup[cpuId].curHwiNum = prevHwi;
#endif
    OsCpupAddIrqCycles(cpuId, currHwi, cycles);
    LOS_SpinUnlock(&g_cpupTaskSpin);
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_GetAllIrqCpuUsage(UINT16 mode, CPUP_INFO_S *cpupInfo)
{
    UINT16 loopNum;
    UINT16 pos;
    UINT16 prePos;
    UINT32 intSave;
    UINT64 cpuCycleAll;
    UINT64 cpuCycleCurIrq;

    if (g_cpupStatus == CPUP_STATUS_STOPED) {
        return LOS_ERRNO_CPUP_NO_INIT;
    }
    if (cpupInfo == NULL) {
        return LOS_ERRNO_CPUP_TASK_PTR_NULL;
    }

    intSave = LOS_IntLock();
    OsCpupGetPos(mode, &pos, &prePos);
    cpuCycleAll = OsCpupCpuCycleGet(mode, pos, prePos);

    for (loopNum = OS_CPUP_TASK_CB_NUM; loopNum < OS_CPUP_CB_MAX_NUM; loopNum++) {
        if ((g_cpup[loopNum].status & OS_TASK_STATUS_UNUSED) || (g_cpup[loopNum].status == 0)) {
            continue;
        }
        cpuCycleCurIrq = OsCpupTaskCycleGet(loopNum, mode, pos, prePos);
        cpupInfo[loopNum - OS_CPUP_TASK_CB_NUM].usStatus = g_cpup[loopNum].status;
        if (cpuCycleAll) {
            cpupInfo[loopNum - OS_CPUP_TASK_CB_NUM].uwUsage =
                (UINT32)((LOS_CPUP_PRECISION * cpuCycleCurIrq) / cpuCycleAll);
        }
    }
    LOS_IntRestore(intSave);

    return LOS_OK;
}
#endif /* LOSCFG_CPUP_INCLUDE_IRQ */

#endif /* LOSCFG_BASE_CORE_CPUP */
