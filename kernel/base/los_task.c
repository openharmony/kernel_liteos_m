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

#include "los_task_pri.h"
#include "los_init.h"
#include "los_task_base.h"
#include "securec.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_interrupt.h"
#include "los_memory_pri.h"
#include "los_mpu.h"
#include "los_sched.h"
#include "los_sched_pri.h"
#include "los_mux.h"
#include "los_sem.h"
#include "los_timer.h"
#if (LOSCFG_BASE_CORE_CPUP == 1)
#include "los_cpup_pri.h"
#endif
#if (LOSCFG_KERNEL_PM == 1)
#include "los_pm.h"
#endif

/**
 * @ingroup los_task
 * @brief check task id's validation
 */
#define OS_TASK_ID_CHECK(taskId)              (OS_TSK_GET_INDEX(taskId) < g_taskMaxNum)

/**
 * @ingroup los_task
 * @brief check task id's invalidation
 */
#define OS_CHECK_TSK_PID_NOIDLE(taskId)       (OS_TSK_GET_INDEX(taskId) >= g_taskMaxNum)

/**
 * @ingroup los_task
 * @brief the offset of task stack's top for skipping the magic word
 */
#define OS_TASK_STACK_TOP_OFFSET                4

#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 1)
/**
 * @ingroup los_task
 * @brief the size of task stack's protection area
 */
#define OS_TASK_STACK_PROTECT_SIZE              32
#endif

LITE_OS_SEC_BSS  LosTaskCB                           *g_taskCBArray = NULL;
LITE_OS_SEC_BSS  UINT32                              g_taskMaxNum;
LITE_OS_SEC_BSS  TSKSWITCHHOOK                       g_pfnUsrTskSwitchHook = NULL;
LITE_OS_SEC_BSS  IDLEHANDLERHOOK                     g_idleHandlerHook = NULL;

LITE_OS_SEC_DATA_INIT LOS_DL_LIST                    g_losFreeTask;
LITE_OS_SEC_DATA_INIT LOS_DL_LIST                    g_taskRecycleList;
LITE_OS_SEC_BSS  BOOL                                g_taskScheduled = FALSE;

LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_taskSpin);

STATIC VOID (*PmEnter)(VOID) = NULL;

#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
TaskSwitchInfo g_taskSwitchInfo;
#endif

STATIC_INLINE UINT32 OsCheckTaskIDValid(UINT32 taskId)
{
    UINT32 ret = LOS_OK;
    if (taskId == OsPercpuGet()->idleTaskId) {
        ret = LOS_ERRNO_TSK_OPERATE_IDLE;
#if (LOSCFG_BASE_CORE_SWTMR == 1) && !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
    } else if (taskId == OsPercpuGet()->swtmrTaskId) {
        ret = LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED;
#endif
    } else if (OS_TSK_GET_INDEX(taskId) >= g_taskMaxNum) {
        ret = LOS_ERRNO_TSK_ID_INVALID;
    }
    return ret;
}

STATIC INLINE VOID OsInsertTCBToFreeList(LosTaskCB *taskCB)
{
    UINT32 taskId = taskCB->taskId;
    (VOID)memset_s(taskCB, sizeof(LosTaskCB), 0, sizeof(LosTaskCB));
    taskCB->taskId = taskId;
    taskCB->taskStatus = OS_TASK_STATUS_UNUSED;
    LOS_ListAdd(&g_losFreeTask, &taskCB->pendList);
}

STATIC VOID OsRecycleTaskResources(LosTaskCB *taskCB, UINTPTR *stackPtr)
{
    if ((taskCB->usrStack == 0) && (taskCB->topOfStack != 0)) {
#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 1)
        *stackPtr = taskCB->topOfStack - OS_TASK_STACK_PROTECT_SIZE;
#else
        *stackPtr = taskCB->topOfStack;
#endif
        taskCB->topOfStack = 0;
    }
    if (OS_TASK_IS_DETACHED(taskCB)) {
        OsInsertTCBToFreeList(taskCB);
    }
}

STATIC VOID OsRecycleFinishedTask(VOID)
{
    LosTaskCB *taskCB = NULL;
    UINT32 intSave;
    UINTPTR stackPtr;

    SCHEDULER_LOCK(intSave);
    while (!LOS_ListEmpty(&g_taskRecycleList)) {
        taskCB = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&g_taskRecycleList));
        LOS_ListDelete(LOS_DL_LIST_FIRST(&g_taskRecycleList));
        stackPtr = 0;
        OsRecycleTaskResources(taskCB, &stackPtr);
        SCHEDULER_UNLOCK(intSave);

        (VOID)LOS_MemFree(OS_TASK_STACK_ADDR, (VOID *)stackPtr);
        SCHEDULER_LOCK(intSave);
    }
    SCHEDULER_UNLOCK(intSave);
}

UINT32 OsPmEnterHandlerSet(VOID (*func)(VOID))
{
    if (func == NULL) {
        return LOS_NOK;
    }

    PmEnter = func;
    return LOS_OK;
}

/*****************************************************************************
 Function    : OsIdleTask
 Description : Idle task.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT VOID OsIdleTask(VOID)
{
    while (1) {
        OsRecycleFinishedTask();
        OsIdleHandler();

        if (PmEnter != NULL) {
            PmEnter();
        } else {
            (VOID)ArchEnterSleep();
        }
    }
}

/*****************************************************************************
 Function    : OsConvertTskStatus
 Description : Convert task status to string.
 Input       : taskStatus    --- task status
 Output      : None
 Return      : string
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT8 *OsConvertTskStatus(UINT16 taskStatus)
{
    if (taskStatus & OS_TASK_STATUS_RUNNING) {
        return (UINT8 *)"Running";
    } else if (taskStatus & OS_TASK_STATUS_READY) {
        return (UINT8 *)"Ready";
    } else if (taskStatus & OS_TASK_STATUS_EXIT) {
        return (UINT8 *)"Exit";
    } else if (taskStatus & OS_TASK_STATUS_SUSPEND) {
        return (UINT8 *)"Suspend";
    } else if (taskStatus & OS_TASK_STATUS_DELAY) {
        return (UINT8 *)"Delay";
    } else if (taskStatus & OS_TASK_STATUS_PEND) {
        if (taskStatus & OS_TASK_STATUS_PEND_TIME) {
            return (UINT8 *)"PendTime";
        }
        return (UINT8 *)"Pend";
    }

    return (UINT8 *)"Impossible";
}

UINT32 OsGetTaskWaterLine(UINT32 taskId)
{
    UINT32 *stackPtr = NULL;
    UINT32 peakUsed;

    if (*(UINT32 *)(UINTPTR)OS_TCB_FROM_TID(taskId)->topOfStack == OS_TASK_MAGIC_WORD) {
        stackPtr = (UINT32 *)(UINTPTR)(OS_TCB_FROM_TID(taskId)->topOfStack + OS_TASK_STACK_TOP_OFFSET);
        while ((stackPtr < (UINT32 *)(OS_TCB_FROM_TID(taskId)->stackPointer)) && (*stackPtr == OS_TASK_STACK_INIT)) {
            stackPtr += 1;
        }
        peakUsed = OS_TCB_FROM_TID(taskId)->stackSize -
            ((UINT32)(UINTPTR)stackPtr - OS_TCB_FROM_TID(taskId)->topOfStack);
    } else {
        PRINT_ERR("CURRENT task %s stack overflow!\n", OS_TCB_FROM_TID(taskId)->taskName);
        peakUsed = OS_NULL_INT;
    }
    return peakUsed;
}

#if (LOSCFG_BASE_CORE_CPUP == 1)
STATIC UINT32 GetAllTskCpupInfo(CPUP_INFO_S **cpuLessOneSec,
                                CPUP_INFO_S **cpuTenSec,
                                CPUP_INFO_S **cpuOneSec)
{
    if ((cpuLessOneSec == NULL) || (cpuTenSec == NULL) || (cpuOneSec == NULL)) {
        return OS_ERROR;
    }
    *cpuLessOneSec = (CPUP_INFO_S *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, sizeof(CPUP_INFO_S) * g_taskMaxNum);
    if (*cpuLessOneSec == NULL) {
        PRINT_ERR("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        return OS_ERROR;
    }
    // Ignore the return code when matching CSEC rule 6.6(3).
    (VOID)memset_s((VOID *)(*cpuLessOneSec), sizeof(CPUP_INFO_S) * g_taskMaxNum,
                   (INT32)0, sizeof(CPUP_INFO_S) * g_taskMaxNum);

    *cpuTenSec = (CPUP_INFO_S *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, sizeof(CPUP_INFO_S) * g_taskMaxNum);
    if (*cpuTenSec == NULL) {
        PRINT_ERR("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, *cpuLessOneSec);
        *cpuLessOneSec = NULL;
        return OS_ERROR;
    }
    // Ignore the return code when matching CSEC rule 6.6(3).
    (VOID)memset_s((VOID *)(*cpuTenSec), sizeof(CPUP_INFO_S) * g_taskMaxNum,
                   (INT32)0, sizeof(CPUP_INFO_S) * g_taskMaxNum);

    *cpuOneSec = (CPUP_INFO_S *)LOS_MemAlloc((VOID *)OS_SYS_MEM_ADDR, sizeof(CPUP_INFO_S) * g_taskMaxNum);
    if (*cpuOneSec == NULL) {
        PRINT_ERR("%s[%d] malloc failure!\n", __FUNCTION__, __LINE__);
        (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, *cpuLessOneSec);
        (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, *cpuTenSec);
        return OS_ERROR;
    }
    // Ignore the return code when matching CSEC rule 6.6(3).
    (VOID)memset_s((VOID *)(*cpuOneSec), sizeof(CPUP_INFO_S) * g_taskMaxNum,
                   (INT32)0, sizeof(CPUP_INFO_S) * g_taskMaxNum);

    LOS_TaskLock();
    (VOID)LOS_AllTaskCpuUsage(*cpuLessOneSec, CPUP_ALL_TIME);
    (VOID)LOS_AllTaskCpuUsage(*cpuTenSec, CPUP_IN_10S);
    (VOID)LOS_AllTaskCpuUsage(*cpuOneSec, CPUP_IN_1S);
    LOS_TaskUnlock();

    return LOS_OK;
}
#endif

STATIC VOID PrintTskInfo(const LosTaskCB *taskCB)
{
    UINT32 semID;

    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        PRINTK("%4u%9u%10s%#10x%#10x%#11x%#11x%#10x%#7x",
               taskCB->taskId, taskCB->priority, OsConvertTskStatus(taskCB->taskStatus),
               taskCB->stackSize, 0, 0, 0, 0, 0);
        return;
    }

    semID = (taskCB->taskSem == NULL) ? OS_NULL_SHORT : (((LosSemCB *)taskCB->taskSem)->semID);
    PRINTK("%4u%9u%10s%#10x%#10x%#11x%#11x%#10x%#7x",
           taskCB->taskId, taskCB->priority, OsConvertTskStatus(taskCB->taskStatus),
           taskCB->stackSize, OsGetTaskWaterLine(taskCB->taskId),
           (UINT32)(UINTPTR)taskCB->stackPointer, taskCB->topOfStack, taskCB->eventMask, semID);
    return;
}

STATIC VOID PrintTskInfoHeader(VOID)
{
    PRINTK("\r\n TID  Priority   Status StackSize WaterLine StackPoint TopOfStack EventMask  SemID");
#if (LOSCFG_TASK_MEM_USED == 1)
    PRINTK(" AllocSize");
#endif
#if (LOSCFG_BASE_CORE_CPUP == 1)
    PRINTK("  CPUUSE CPUUSE10s CPUUSE1s ");
#endif /* LOSCFG_BASE_CORE_CPUP */
    PRINTK("  TaskEntry name\n");
    PRINTK(" ---  -------- -------- ");
    PRINTK("--------- --------- ---------- ---------- --------- ------ ");
#if (LOSCFG_TASK_MEM_USED == 1)
    PRINTK("--------- ");
#endif
#if (LOSCFG_BASE_CORE_CPUP == 1)
    PRINTK("------- --------- --------  ");
#endif /* LOSCFG_BASE_CORE_CPUP */
    PRINTK("---------- ----\n");
}

#if (LOSCFG_TASK_MEM_USED == 1)
STATIC UINT32                              g_taskMemUsed[LOSCFG_BASE_CORE_TSK_LIMIT + 1];
#endif

/*****************************************************************************
 Function    : OsGetAllTskInfo
 Description : Get all task info.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 OsGetAllTskInfo(VOID)
{
#if (LOSCFG_KERNEL_PRINTF != 0)
    LosTaskCB    *taskCB = (LosTaskCB *)NULL;
    UINT32       loopNum;
#if (LOSCFG_BASE_CORE_CPUP == 1)
    CPUP_INFO_S *cpuLessOneSec = (CPUP_INFO_S *)NULL;
    CPUP_INFO_S *cpuTenSec = (CPUP_INFO_S *)NULL;
    CPUP_INFO_S *cpuOneSec = (CPUP_INFO_S *)NULL;
#endif

#if (LOSCFG_TASK_MEM_USED == 1)
    (VOID)memset_s(g_taskMemUsed, sizeof(UINT32) * g_taskMaxNum, 0, sizeof(UINT32) * g_taskMaxNum);
    OsTaskMemUsed((VOID *)OS_SYS_MEM_ADDR, g_taskMemUsed, g_taskMaxNum);
#endif

#if (LOSCFG_BASE_CORE_CPUP == 1)
    if (GetAllTskCpupInfo(&cpuLessOneSec, &cpuTenSec, &cpuOneSec) != LOS_OK) {
        return OS_ERROR;
    }
#endif /* LOSCFG_BASE_CORE_CPUP */

    PrintTskInfoHeader();

    for (loopNum = 0; loopNum < g_taskMaxNum; loopNum++) {
        taskCB = (((LosTaskCB *)g_taskCBArray) + loopNum);
        if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
            continue;
        }

        PrintTskInfo(taskCB);
#if (LOSCFG_TASK_MEM_USED == 1)
        PRINTK("%#10x", g_taskMemUsed[loopNum]);
#endif

#if (LOSCFG_BASE_CORE_CPUP == 1)
        PRINTK("%6u.%-2u%7u.%-2u%6u.%-2u ",
               cpuLessOneSec[taskCB->taskId].uwUsage / LOS_CPUP_PRECISION_MULT,
               cpuLessOneSec[taskCB->taskId].uwUsage % LOS_CPUP_PRECISION_MULT,
               cpuTenSec[taskCB->taskId].uwUsage / LOS_CPUP_PRECISION_MULT,
               cpuTenSec[taskCB->taskId].uwUsage % LOS_CPUP_PRECISION_MULT,
               cpuOneSec[taskCB->taskId].uwUsage / LOS_CPUP_PRECISION_MULT,
               cpuOneSec[taskCB->taskId].uwUsage % LOS_CPUP_PRECISION_MULT);
#endif /* LOSCFG_BASE_CORE_CPUP */
        PRINTK("%#10x %-32s\n", (UINT32)(UINTPTR)taskCB->taskEntry, taskCB->taskName);
    }

#if (LOSCFG_BASE_CORE_CPUP == 1)
    (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, cpuLessOneSec);
    (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, cpuTenSec);
    (VOID)LOS_MemFree((VOID *)OS_SYS_MEM_ADDR, cpuOneSec);
#endif
#endif
    return LOS_OK;
}

/*****************************************************************************
 Function    : OsTaskInit
 Description : Task init function.
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsTaskInit(VOID)
{
    UINT32 size;
    UINT32 index;

    g_taskMaxNum = LOSCFG_BASE_CORE_TSK_LIMIT + 1; /* Reserved 1 for IDLE */
    size = (g_taskMaxNum + 1) * sizeof(LosTaskCB);
    g_taskCBArray = (LosTaskCB *)LOS_MemAlloc(m_aucSysMem0, size);
    if (g_taskCBArray == NULL) {
        return LOS_ERRNO_TSK_NO_MEMORY;
    }

    // Ignore the return code when matching CSEC rule 6.6(1).
    (VOID)memset_s(g_taskCBArray, size, 0, size);
    LOS_ListInit(&g_losFreeTask);
    LOS_ListInit(&g_taskRecycleList);
    for (index = 0; index <= LOSCFG_BASE_CORE_TSK_LIMIT; index++) {
        g_taskCBArray[index].taskStatus = OS_TASK_STATUS_UNUSED;
        g_taskCBArray[index].taskId = index;
        LOS_ListTailInsert(&g_losFreeTask, &g_taskCBArray[index].pendList);
    }

    LosTaskCB *runTask = &g_taskCBArray[g_taskMaxNum];
    OsCurrTaskSet(runTask);
    runTask->taskId = index;
    runTask->taskStatus = (OS_TASK_STATUS_UNUSED | OS_TASK_STATUS_RUNNING);
    runTask->priority = OS_TASK_PRIORITY_LOWEST + 1;

    OsPercpuGet()->idleTaskId = OS_INVALID;
    return OsSchedInit();
}
LOS_SYS_INIT(OsTaskInit, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_0);


/*****************************************************************************
 Function    : OsGetIdleTaskId
 Description : Get the idle task ID of the current CPU.
 Input       : None
 Output      : None
 Return      : idle task ID
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 OsGetIdleTaskId(VOID)
{
    return OsPercpuGet()->idleTaskId;
}


/*****************************************************************************
 Function    : OsIdleTaskCreate
 Description : Create idle task.
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsIdleTaskCreate(VOID)
{
    UINT32 retVal;
    TSK_INIT_PARAM_S taskInitParam;
    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)memset_s((VOID *)(&taskInitParam), sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));
    taskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)OsIdleTask;
    taskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE;
    taskInitParam.pcName = "IdleCore000";
    taskInitParam.usTaskPrio = OS_TASK_PRIORITY_LOWEST;
    Percpu *percpu = OsPercpuGet();
    retVal = LOS_TaskCreateOnly(&percpu->idleTaskId, &taskInitParam);
    if (retVal != LOS_OK) {
        return retVal;
    }

    OsSchedSetIdleTaskSchedParam(OS_TCB_FROM_TID(percpu->idleTaskId));
    return LOS_OK;
}
LOS_SYS_INIT(OsIdleTaskCreate, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_3);

/*****************************************************************************
 Function    : LOS_CurTaskIDGet
 Description : get id of current running task.
 Input       : None
 Output      : None
 Return      : task id
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_CurTaskIDGet(VOID)
{
    LosTaskCB *runTask = OsCurrTaskGet();
    if (runTask == NULL) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }
    return runTask->taskId;
}

/*****************************************************************************
 Function    : LOS_NextTaskIDGet
 Description : get id of next running task.
 Input       : None
 Output      : None
 Return      : task id
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_NextTaskIDGet(VOID)
{
    UINT32 intSave;
    SCHEDULER_LOCK(intSave);
    UINT32 taskId = OsGetTopTask()->taskId;
    SCHEDULER_UNLOCK(intSave);

    return taskId;
}

/*****************************************************************************
 Function    : LOS_CurTaskNameGet
 Description : get name of current running task.
 Input       : None
 Output      : None
 Return      : task name
 *****************************************************************************/
LITE_OS_SEC_TEXT CHAR *LOS_CurTaskNameGet(VOID)
{
    CHAR *taskName = NULL;
    LosTaskCB *runTask = OsCurrTaskGet();

    if (runTask != NULL) {
        taskName = runTask->taskName;
    }

    return taskName;
}

#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 0)
/*****************************************************************************
 Function    : OsHandleRunTaskStackOverflow
 Description : handle stack overflow exception of the run task.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT STATIC VOID OsHandleRunTaskStackOverflow(const LosTaskCB *oldTask)
{
    PRINT_ERR("CURRENT task ID: %s:%d stack overflow!\n",
              oldTask->taskName, oldTask->taskId);
    OsDoExcHook(EXC_STACKOVERFLOW);
}

/*****************************************************************************
 Function    : OsHandleNewTaskStackOverflow
 Description : handle stack overflow exception of the new task.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT STATIC VOID OsHandleNewTaskStackOverflow(const LosTaskCB *newTask)
{
    LosTaskCB *tmp = NULL;

    PRINT_ERR("HIGHEST task ID: %s:%d SP error!\n",
              newTask->taskName, newTask->taskId);
    PRINT_ERR("HIGHEST task StackPointer: 0x%x TopOfStack: 0x%x\n",
              (UINT32)(UINTPTR)(newTask->stackPointer), newTask->topOfStack);

    tmp = OsCurrTaskGet();
    OsCurrTaskSet((LosTaskCB *)newTask);
    OsDoExcHook(EXC_STACKOVERFLOW);
    OsCurrTaskSet(tmp);
}
#else
LITE_OS_SEC_TEXT STATIC VOID OsTaskStackProtect(const LosTaskCB *newTask)
{
    MPU_CFG_PARA mpuAttr = {0};
    STATIC INT32 id = -1;

    if (id == -1) {
        id = ArchMpuUnusedRegionGet();
        if (id < 0) {
            PRINT_ERR("%s %d, get unused id failed!\n", __FUNCTION__, __LINE__);
            return;
        }
    }

    mpuAttr.baseAddr = newTask->topOfStack - OS_TASK_STACK_PROTECT_SIZE;
    mpuAttr.size = OS_TASK_STACK_PROTECT_SIZE;
    mpuAttr.memType = MPU_MEM_ON_CHIP_RAM;
    mpuAttr.executable = MPU_NON_EXECUTABLE;
    mpuAttr.shareability = MPU_NO_SHARE;
    mpuAttr.permission = MPU_RO_BY_PRIVILEGED_ONLY;

    ArchMpuDisable();
    (VOID)ArchMpuDisableRegion(id);
    (VOID)ArchMpuSetRegion(id, &mpuAttr);
    ArchMpuEnable(1);
}
#endif
#endif

/*****************************************************************************
 Function    : OsTaskSwitchCheck
 Description : Check task switch
 Input       : Node
 Output      : None
 Return      : None
 *****************************************************************************/
#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
LITE_OS_SEC_TEXT VOID OsTaskSwitchCheck(const LosTaskCB *oldTask, const LosTaskCB *newTask)
{
#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 0)
    UINT32 endOfStack = newTask->topOfStack + newTask->stackSize;

    if ((*(UINT32 *)(UINTPTR)(oldTask->topOfStack)) != OS_TASK_MAGIC_WORD) {
        OsHandleRunTaskStackOverflow(oldTask);
    }
    if (((UINT32)(UINTPTR)(newTask->stackPointer) <= (newTask->topOfStack)) ||
        ((UINT32)(UINTPTR)(newTask->stackPointer) > endOfStack)) {
        OsHandleNewTaskStackOverflow(newTask);
    }
#else
    OsTaskStackProtect(newTask);
#endif

#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
    g_taskSwitchInfo.pid[g_taskSwitchInfo.idx] = (UINT16)(newTask->taskId);

    errno_t ret = memcpy_s(g_taskSwitchInfo.name[g_taskSwitchInfo.idx], LOS_TASK_NAMELEN,
                           newTask->taskName, LOS_TASK_NAMELEN);
    if (ret != EOK) {
        PRINT_ERR("exc task switch copy file name failed!\n");
    }
    g_taskSwitchInfo.name[g_taskSwitchInfo.idx][LOS_TASK_NAMELEN - 1] = '\0';

    if (++g_taskSwitchInfo.idx == OS_TASK_SWITCH_INFO_COUNT) {
        g_taskSwitchInfo.idx = 0;
        g_taskSwitchInfo.cntInfo.isFull = TRUE;
    }
#endif

    LOSCFG_BASE_CORE_TSK_SWITCH_HOOK();

#if (LOSCFG_BASE_CORE_CPUP == 1)
    OsTaskCycleEndStart(newTask);
#endif
}

LITE_OS_SEC_TEXT_MINOR UINT32 OsTaskMonInit(VOID)
{
#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)memset_s(&g_taskSwitchInfo, sizeof(TaskSwitchInfo), 0, sizeof(TaskSwitchInfo));
    g_taskSwitchInfo.cntInfo.maxCnt = OS_TASK_SWITCH_INFO_COUNT;
#endif
    return LOS_OK;
}
LOS_SYS_INIT(OsTaskMonInit, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_0);
#endif

/*****************************************************************************
 Function    : LOS_TaskSwitchHookReg
 Description : Register a user-defined task switching hook. The hook will be
               called from OsSchedTaskSwitch on every task context switch,
               after task status fields are updated and before the actual
               register-level context switch. Runs in PendSV interrupt context
               (interrupts disabled, MSP stack) - hook must be interrupt-safe.
 Input       : hook - the hook function to register, NULL to deregister
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID LOS_TaskSwitchHookReg(TSKSWITCHHOOK hook)
{
    g_pfnUsrTskSwitchHook = hook;
}

LITE_OS_SEC_TEXT_MINOR VOID LOS_IdleHandlerHookReg(IDLEHANDLERHOOK hook)
{
    g_idleHandlerHook = hook;
}

VOID OsIdleHandler(VOID)
{
    IDLEHANDLERHOOK idleHandlerHook = g_idleHandlerHook;
    if (idleHandlerHook != NULL) {
        idleHandlerHook();
    }
}

/*****************************************************************************
 Function    : OsTaskEntry
 Description : All task entry
 Input       : taskId     --- The ID of the task to be run
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT VOID OsTaskEntry(UINT32 taskId)
{
    UINT32 retVal;
    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskId);

#ifndef LOSCFG_TASK_JOINABLE
    (VOID)taskCB->taskEntry(taskCB->args);
#else
#ifdef  LOSCFG_KERNEL_TASK_ENTRY_VOID_PTR
    taskCB->taskRetval = (UINTPTR)taskCB->taskEntry(taskCB->args);
#else
    taskCB->taskRetval = (UINTPTR)taskCB->taskEntry(taskCB->arg);
#endif
#endif
    retVal = LOS_TaskDelete(taskCB->taskId);
    if (retVal != LOS_OK) {
        PRINT_ERR("Delete Task[TID: %d] Failed!\n", taskCB->taskId);
    }
}

LITE_OS_SEC_TEXT_INIT STATIC_INLINE UINT32 OsTaskInitParamCheck(TSK_INIT_PARAM_S *taskInitParam)
{
    if (taskInitParam == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    if (taskInitParam->pcName == NULL) {
        return LOS_ERRNO_TSK_NAME_EMPTY;
    }

    if (taskInitParam->pfnTaskEntry == NULL) {
        return LOS_ERRNO_TSK_ENTRY_NULL;
    }

    if ((taskInitParam->usTaskPrio) > OS_TASK_PRIORITY_LOWEST) {
        return LOS_ERRNO_TSK_PRIOR_ERROR;
    }

    if (((taskInitParam->usTaskPrio) == OS_TASK_PRIORITY_LOWEST)
        && (taskInitParam->pfnTaskEntry != OS_IDLE_TASK_ENTRY)) {
        return LOS_ERRNO_TSK_PRIOR_ERROR;
    }

    if (taskInitParam->uwStackSize > LOSCFG_SYS_HEAP_SIZE) {
        return LOS_ERRNO_TSK_STKSZ_TOO_LARGE;
    }

    if (taskInitParam->uwStackSize == 0) {
        taskInitParam->uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    }

    if (taskInitParam->uwStackSize < LOSCFG_BASE_CORE_TSK_MIN_STACK_SIZE) {
        return LOS_ERRNO_TSK_STKSZ_TOO_SMALL;
    }
    return LOS_OK;
}

STATIC UINT32 OsNewTaskInit(LosTaskCB *taskCB, TSK_INIT_PARAM_S *taskInitParam)
{
#ifdef LOSCFG_KERNEL_TASK_ENTRY_VOID_PTR
    taskCB->args            = taskInitParam->pArgs;
#else
    taskCB->arg             = taskInitParam->uwArg;
#endif
    taskCB->stackSize       = taskInitParam->uwStackSize;
    taskCB->taskSem         = NULL;
    taskCB->taskMux         = NULL;
    taskCB->taskStatus      = OS_TASK_STATUS_SUSPEND;
    taskCB->usrStack        = 0;
    taskCB->priority        = taskInitParam->usTaskPrio;
    taskCB->timeSlice       = 0;
    taskCB->waitTimes       = 0;
    taskCB->taskEntry       = taskInitParam->pfnTaskEntry;
    taskCB->event.uwEventID = OS_NULL_INT;
    taskCB->eventMask       = 0;
    taskCB->taskName        = taskInitParam->pcName;
    taskCB->msg             = NULL;
#if (LOSCFG_KERNEL_SIGNAL == 1)
    taskCB->sig             = NULL;
#endif

    SET_SORTLIST_VALUE(&taskCB->sortList, OS_SORT_LINK_INVALID_TIME);
    LOS_EventInit(&(taskCB->event));

#ifdef LOSCFG_TASK_JOINABLE
    if (taskInitParam->uwResved & LOS_TASK_ATTR_JOINABLE) {
        taskCB->taskFlags = OS_TASK_FLAG_JOINABLE;
        LOS_ListInit(&taskCB->joinList);
    } else {
        taskCB->taskFlags = OS_TASK_FLAG_DETACHED;
    }
#else
    taskCB->taskFlags = OS_TASK_FLAG_DETACHED;
#endif

    if (taskInitParam->stackAddr == (UINTPTR)NULL) {
        taskCB->stackSize = ALIGN(taskInitParam->uwStackSize, OS_TASK_STACK_ADDR_ALIGN);
#if (LOSCFG_EXC_HARDWARE_STACK_PROTECTION == 1)
        UINT32 stackSize = taskCB->stackSize + OS_TASK_STACK_PROTECT_SIZE;
        UINTPTR stackPtr = (UINTPTR)LOS_MemAllocAlign(OS_TASK_STACK_ADDR, stackSize, OS_TASK_STACK_PROTECT_SIZE);
        taskCB->topOfStack = stackPtr + OS_TASK_STACK_PROTECT_SIZE;
#else
        taskCB->topOfStack = (UINTPTR)LOS_MemAllocAlign(OS_TASK_STACK_ADDR, taskCB->stackSize,
                                                        LOSCFG_STACK_POINT_ALIGN_SIZE);
#endif
        if (taskCB->topOfStack == (UINTPTR)NULL) {
            return LOS_ERRNO_TSK_NO_MEMORY;
        }
    } else {
        taskCB->usrStack = 1;
        taskCB->topOfStack = LOS_Align(taskInitParam->stackAddr, LOSCFG_STACK_POINT_ALIGN_SIZE);
        taskCB->stackSize = taskInitParam->uwStackSize - (taskCB->topOfStack - taskInitParam->stackAddr);
        taskCB->stackSize = TRUNCATE(taskCB->stackSize, OS_TASK_STACK_ADDR_ALIGN);
    }

    /* initialize the task stack, write magic num to stack top */
    (VOID)memset_s((VOID *)taskCB->topOfStack, taskCB->stackSize,
                   (INT32)(OS_TASK_STACK_INIT & 0xFF), taskCB->stackSize);

    *((UINT32 *)taskCB->topOfStack) = OS_TASK_MAGIC_WORD;
    taskCB->stackPointer = ArchTaskStackInit(taskCB->taskId, taskCB->stackSize, (VOID *)taskCB->topOfStack);
    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_TaskCreateOnly
 Description : Create a task and suspend
 Input       : taskInitParam --- Task init parameters
 Output      : taskId        --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreateOnly(UINT32 *taskId, TSK_INIT_PARAM_S *taskInitParam)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT32 retVal;

    if (taskId == NULL) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    retVal = OsTaskInitParamCheck(taskInitParam);
    if (retVal != LOS_OK) {
        return retVal;
    }

    OsRecycleFinishedTask();

    SCHEDULER_LOCK(intSave);
    if (LOS_ListEmpty(&g_losFreeTask)) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_TCB_UNAVAILABLE;
    }

    taskCB = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&g_losFreeTask));
    LOS_ListDelete(LOS_DL_LIST_FIRST(&g_losFreeTask));
    SCHEDULER_UNLOCK(intSave);

    retVal = OsNewTaskInit(taskCB, taskInitParam);
    if (retVal != LOS_OK) {
        SCHEDULER_LOCK(intSave);
        OsInsertTCBToFreeList(taskCB);
        SCHEDULER_UNLOCK(intSave);
        return retVal;
    }

    LOSCFG_TASK_CREATE_EXTENSION_HOOK(taskCB);

#if (LOSCFG_BASE_CORE_CPUP == 1)
    SCHEDULER_LOCK(intSave);
    OsCpupCBTaskCreate(taskCB->taskId, taskCB->taskStatus);
    SCHEDULER_UNLOCK(intSave);
#endif
    *taskId = taskCB->taskId;
    OsHookCall(LOS_HOOK_TYPE_TASK_CREATE, taskCB);
    return retVal;
}

/*****************************************************************************
 Function    : LOS_TaskCreateOnlyStatic
 Description : Create a task with user-provided static stack memory (no dynamic
               allocation) in suspended state. Wraps LOS_TaskCreateOnly.
 Input       : taskInitParam --- Task init parameters
               topStack      --- User-provided top of stack address
 Output      : taskId        --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreateOnlyStatic(UINT32 *taskId, TSK_INIT_PARAM_S *taskInitParam,
                                                      VOID *topStack)
{
    if (taskInitParam == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }
    taskInitParam->stackAddr = (UINTPTR)topStack;
    return LOS_TaskCreateOnly(taskId, taskInitParam);
}

/*****************************************************************************
 Function    : LOS_TaskCreate
 Description : Create a task
 Input       : taskInitParam --- Task init parameters
 Output      : taskId        --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreate(UINT32 *taskId, TSK_INIT_PARAM_S *taskInitParam)
{
    UINT32 retVal;
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;

    retVal = LOS_TaskCreateOnly(taskId, taskInitParam);
    if (retVal != LOS_OK) {
        return retVal;
    }
    taskCB = OS_TCB_FROM_TID(*taskId);

    SCHEDULER_LOCK(intSave);

    OsSchedTaskEnQueue(taskCB);
    SCHEDULER_UNLOCK(intSave);

    if (g_taskScheduled) {
        LOS_Schedule();
    }

    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_TaskCreateStatic
 Description : Create a task with user-provided static stack memory (no dynamic allocation)
 Input       : taskInitParam --- Task init parameters
               topStack      --- User-provided top of stack address
 Output      : taskId        --- Save task ID
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskCreateStatic(UINT32 *taskId, TSK_INIT_PARAM_S *taskInitParam, VOID *topStack)
{
    if (taskInitParam == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }
    taskInitParam->stackAddr = (UINTPTR)topStack;
    return LOS_TaskCreate(taskId, taskInitParam);
}

/*****************************************************************************
 Function    : LOS_TaskResume
 Description : Resume suspend task
 Input       : taskId --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskResume(UINT32 taskId)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 tempStatus;
    UINT32 retErr = OS_ERROR;
    BOOL needSched = FALSE;

    if (!OS_TASK_ID_CHECK(taskId)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    SCHEDULER_LOCK(intSave);
    tempStatus = taskCB->taskStatus;

    if (tempStatus & OS_TASK_STATUS_UNUSED) {
        retErr = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    } else if (!(tempStatus & OS_TASK_STATUS_SUSPEND)) {
        retErr = LOS_ERRNO_TSK_NOT_SUSPENDED;
        OS_GOTO_ERREND();
    }

    needSched = OsSchedResume(taskCB);
    if (needSched && g_taskScheduled) {
        SCHEDULER_UNLOCK(intSave);
        LOS_Schedule();
        return LOS_OK;
    }

    SCHEDULER_UNLOCK(intSave);
    return LOS_OK;

LOS_ERREND:
    SCHEDULER_UNLOCK(intSave);
    return retErr;
}

#if defined(LOSCFG_SCHED_LATENCY)
STATIC VOID OsTaskReSched(VOID)
{
    OsSchedResched();
}
#else
STATIC VOID OsTaskReSched(VOID)
{
    if (OS_INT_ACTIVE) {
        OsSetSchedFlag(INT_SUSPEND_DELETE_RESCH);
        return;
    }
    OsSchedResched();
}
#endif

/*****************************************************************************
 Function    : LOS_TaskSuspend
 Description : Suspend task
 Input       : taskId --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskSuspend(UINT32 taskId)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 tempStatus;
    UINT32 retErr;

    retErr = OsCheckTaskIDValid(taskId);
    if (retErr != LOS_OK) {
        return retErr;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    SCHEDULER_LOCK(intSave);
    tempStatus = taskCB->taskStatus;

    if (tempStatus & OS_TASK_STATUS_UNUSED) {
        retErr = LOS_ERRNO_TSK_NOT_CREATED;
        OS_GOTO_ERREND();
    }

    if (taskCB->taskFlags & OS_TASK_FLAG_SYSTEM) {
        retErr = LOS_ERRNO_TSK_OPERATE_SYSTEM_TASK;
        OS_GOTO_ERREND();
    }

    if (tempStatus & OS_TASK_STATUS_SUSPEND) {
        retErr = LOS_ERRNO_TSK_ALREADY_SUSPENDED;
        OS_GOTO_ERREND();
    }

    if ((tempStatus & OS_TASK_STATUS_RUNNING) && (OsPercpuGet()->taskLockCnt != 0)) {
        retErr = LOS_ERRNO_TSK_SUSPEND_LOCKED;
        OS_GOTO_ERREND();
    }

    OsSchedSuspend(taskCB);

    if (taskId == OsCurrTaskGet()->taskId) {
        OsTaskReSched();
        SCHEDULER_UNLOCK(intSave);
        return LOS_OK;
    }

    SCHEDULER_UNLOCK(intSave);
    return LOS_OK;

LOS_ERREND:
    SCHEDULER_UNLOCK(intSave);
    return retErr;
}

#ifdef LOSCFG_TASK_JOINABLE
STATIC VOID OsTaskJoinPostUnsafe(LosTaskCB *taskCB)
{
    LosTaskCB *resumedTask = NULL;

    if (OS_TASK_IS_JOINABLE(taskCB)) {
        if (!LOS_ListEmpty(&taskCB->joinList)) {
            resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(taskCB->joinList)));
            OsSchedWake(resumedTask);
        }
    }
}

STATIC UINT32 OsTaskJoinPendUnsafe(LosTaskCB *taskCB)
{
    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        return LOS_OK;
    } else if (OS_TASK_IS_JOINABLE(taskCB) && LOS_ListEmpty(&taskCB->joinList)) {
        OsSchedWait(OsCurrTaskGet(), &taskCB->joinList, LOS_WAIT_FOREVER);
        return LOS_OK;
    }

    return LOS_NOK;
}

STATIC UINT32 OsTaskSetDetachUnsafe(LosTaskCB *taskCB)
{
    if (OS_TASK_IS_JOINABLE(taskCB)) {
        if (LOS_ListEmpty(&(taskCB->joinList))) {
            LOS_ListDelete(&(taskCB->joinList));
            taskCB->taskFlags |= OS_TASK_FLAG_DETACHED;
            return LOS_OK;
        }
        /* This error code has a special purpose and is not allowed to appear again on the interface */
        return LOS_ERRNO_TSK_NOT_JOIN;
    }

    return LOS_NOK;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskJoin(UINT32 taskId, UINTPTR *retval)
{
    LosTaskCB *taskCB = NULL;
    UINTPTR stackPtr = 0;
    UINT32 intSave;
    UINT32 ret;

    ret = OsCheckTaskIDValid(taskId);
    if (ret != LOS_OK) {
        return ret;
    }

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_TSK_NOT_ALLOW_IN_INT;
    }

    if (OsPercpuGet()->taskLockCnt != 0) {
        return LOS_ERRNO_TSK_SCHED_LOCKED;
    }

    if (taskId == LOS_CurTaskIDGet()) {
        return LOS_ERRNO_TSK_NOT_JOIN_SELF;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    ret = OsTaskJoinPendUnsafe(taskCB);
    SCHEDULER_UNLOCK(intSave);

    if (ret == LOS_OK) {
        if (retval != NULL) {
            *retval = taskCB->taskRetval;
        }

        SCHEDULER_LOCK(intSave);
        taskCB->taskFlags |= OS_TASK_FLAG_DETACHED;
        OsRecycleTaskResources(taskCB, &stackPtr);
        SCHEDULER_UNLOCK(intSave);
        (VOID)LOS_MemFree(OS_TASK_STACK_ADDR, (VOID *)stackPtr);
        return LOS_OK;
    }

    return ret;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskDetach(UINT32 taskId)
{
    UINT32 intSave;
    UINT32 ret;
    LosTaskCB *taskCB = NULL;

    ret = OsCheckTaskIDValid(taskId);
    if (ret != LOS_OK) {
        return ret;
    }

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_TSK_NOT_ALLOW_IN_INT;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_TaskJoin(taskId, NULL);
    }

    ret = OsTaskSetDetachUnsafe(taskCB);
    SCHEDULER_UNLOCK(intSave);
    return ret;
}
#endif /* LOSCFG_TASK_JOINABLE */

LITE_OS_SEC_TEXT_INIT STATIC_INLINE VOID OsRunningTaskDelete(UINT32 taskId, LosTaskCB *taskCB)
{
    LosTaskCB *runTask;
    LOS_ListTailInsert(&g_taskRecycleList, &taskCB->pendList);
    runTask = &g_taskCBArray[g_taskMaxNum];
    OsCurrTaskSet(runTask);
    runTask->taskId = taskId;
    runTask->taskStatus = taskCB->taskStatus | OS_TASK_STATUS_RUNNING;
    runTask->topOfStack = taskCB->topOfStack;
    runTask->taskName = taskCB->taskName;
}
/*****************************************************************************
 Function    : LOS_TaskDelete
 Description : Delete a task
 Input       : taskId --- Task ID
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskDelete(UINT32 taskId)
{
    UINT32 intSave;
    UINTPTR stackPtr = 0;
    LosTaskCB *taskCB = NULL;

    UINT32 ret = OsCheckTaskIDValid(taskId);
    if (ret != LOS_OK) {
        return ret;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    SCHEDULER_LOCK(intSave);
    if (taskCB->taskFlags & OS_TASK_FLAG_SYSTEM) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_OPERATE_SYSTEM_TASK;
    }

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_ALREADY_EXIT;
    }

    if (taskCB->taskStatus & OS_TASK_FLAG_SIGNAL) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_PROCESS_SIGNAL;
    }

    /* If the task is running and scheduler is locked then you can not delete it */
    Percpu *percpu = OsPercpuGet();
    if (((taskCB->taskStatus) & OS_TASK_STATUS_RUNNING) && (percpu->taskLockCnt != 0)) {
        PRINT_INFO("In case of task lock, task deletion is not recommended\n");
        percpu->taskLockCnt = 0;
    }

    OsHookCall(LOS_HOOK_TYPE_TASK_DELETE, taskCB);
#ifdef LOSCFG_TASK_JOINABLE
    OsTaskJoinPostUnsafe(taskCB);
#endif
    OsSchedTaskExit(taskCB);

    LOS_EventDestroy(&(taskCB->event));
    taskCB->event.uwEventID = OS_NULL_INT;
    taskCB->eventMask = 0;
#if (LOSCFG_BASE_CORE_CPUP == 1)
    OsCpupCBTaskDelete(taskCB->taskId);
#endif

#if (LOSCFG_KERNEL_SIGNAL == 1)
    if (taskCB->sig != NULL) {
        LOS_MemFree(OS_SYS_MEM_ADDR, taskCB->sig);
        taskCB->sig = NULL;
    }
#endif

    LOSCFG_TASK_DELETE_EXTENSION_HOOK(taskCB);

    if (taskCB->taskStatus & OS_TASK_STATUS_RUNNING) {
        if (OS_TASK_IS_DETACHED(taskCB)) {
            taskCB->taskStatus |= OS_TASK_STATUS_UNUSED;
            OsRunningTaskDelete(taskId, taskCB);
        }
        OsTaskReSched();
        SCHEDULER_UNLOCK(intSave);
        return LOS_OK;
    }

#ifdef LOSCFG_TASK_JOINABLE
    taskCB->taskRetval = LOS_CurTaskIDGet();
#endif
    OsRecycleTaskResources(taskCB, &stackPtr);
    SCHEDULER_UNLOCK(intSave);
    (VOID)LOS_MemFree(OS_TASK_STACK_ADDR, (VOID *)stackPtr);
    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_TaskDelay
 Description : delay the current task
 Input       : tick    --- time
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_TaskDelay(UINT32 tick)
{
    UINT32 intSave;
    LosTaskCB *runTask;

    /* TODO: Temporary manual interception */
    if (!g_taskScheduled) {
        return LOS_OK;
    }

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_TSK_DELAY_IN_INT;
    }

    if (OsPercpuGet()->taskLockCnt != 0) {
        return LOS_ERRNO_TSK_DELAY_IN_LOCK;
    }

    runTask = OsCurrTaskGet();
    if (runTask->taskFlags & OS_TASK_FLAG_SYSTEM) {
        return LOS_ERRNO_TSK_OPERATE_SYSTEM_TASK;
    }
    OsHookCall(LOS_HOOK_TYPE_TASK_DELAY, tick);
    if (tick == 0) {
        return LOS_TaskYield();
    } else {
        SCHEDULER_LOCK(intSave);
        OsSchedDelay(runTask, tick);
        OsHookCall(LOS_HOOK_TYPE_MOVEDTASKTODELAYEDLIST, runTask);
        OsSchedResched();
        SCHEDULER_UNLOCK(intSave);
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT16 LOS_TaskPriGet(UINT32 taskId)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 priority;

    if (OS_CHECK_TSK_PID_NOIDLE(taskId)) {
        return (UINT16)OS_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskId);

    SCHEDULER_LOCK(intSave);

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return (UINT16)OS_INVALID;
    }

    priority = taskCB->priority;
    SCHEDULER_UNLOCK(intSave);
    return priority;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskPriSet(UINT32 taskId, UINT16 taskPrio)
{
    BOOL isReady = FALSE;
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    UINT16 tempStatus;

    if (taskPrio > OS_TASK_PRIORITY_LOWEST) {
        return LOS_ERRNO_TSK_PRIOR_ERROR;
    }

    Percpu *percpu = OsPercpuGet();
    if (taskId == percpu->idleTaskId) {
        return LOS_ERRNO_TSK_OPERATE_IDLE;
    }

#if (LOSCFG_BASE_CORE_SWTMR == 1) && !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
    if (taskId == percpu->swtmrTaskId) {
        return LOS_ERRNO_TSK_OPERATE_SWTMR;
    }
#endif

    if (OS_CHECK_TSK_PID_NOIDLE(taskId)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    SCHEDULER_LOCK(intSave);
    tempStatus = taskCB->taskStatus;
    if (tempStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }
    if (taskCB->taskFlags & OS_TASK_FLAG_SYSTEM) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_OPERATE_SYSTEM_TASK;
    }

    isReady = OsSchedPrioModify(taskCB, taskPrio);
    SCHEDULER_UNLOCK(intSave);
    /* delete the task and insert with right priority into ready queue */
    if (isReady) {
        LOS_Schedule();
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_CurTaskPriSet(UINT16 taskPrio)
{
    return LOS_TaskPriSet(OsCurrTaskGet()->taskId, taskPrio);
}

/*****************************************************************************
 Function    : LOS_TaskYield
 Description : Adjust the procedure order of specified task
 Input       : None
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskYield(VOID)
{
    UINT32 intSave;

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_TSK_YIELD_IN_INT;
    }

    if (OsPercpuGet()->taskLockCnt != 0) {
        return LOS_ERRNO_TSK_YIELD_IN_LOCK;
    }

    SCHEDULER_LOCK(intSave);
    OsSchedYield();
    OsSchedResched();
    SCHEDULER_UNLOCK(intSave);
    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_TaskLock
 Description : Task lock
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID LOS_TaskLock(VOID)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    OsPercpuGet()->taskLockCnt++;
    LOS_IntRestore(intSave);
}

/*****************************************************************************
 Function    : LOS_TaskUnlock
 Description : Task unlock
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID LOS_TaskUnlock(VOID)
{
    UINT32 intSave;
    Percpu *percpu = OsPercpuGet();

    intSave = LOS_IntLock();
    if (percpu->taskLockCnt > 0) {
        percpu->taskLockCnt--;
        if (percpu->taskLockCnt == 0) {
            LOS_IntRestore(intSave);
            LOS_Schedule();
            return;
        }
    }

    LOS_IntRestore(intSave);
}

/*****************************************************************************
 Function    : LOS_TaskUnlockNoSched
 Description : Decrement task scheduling lock count without triggering schedule.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR VOID LOS_TaskUnlockNoSched(VOID)
{
    UINT32 intSave;
    Percpu *percpu = OsPercpuGet();

    intSave = LOS_IntLock();
    if (percpu->taskLockCnt > 0) {
        percpu->taskLockCnt--;
    }

    LOS_IntRestore(intSave);
}

STATIC INLINE UINT32 OsGetSemID(const LosTaskCB *taskCB)
{
    UINT32 semId = LOSCFG_BASE_IPC_SEM_LIMIT;

    if (taskCB->taskSem != NULL) {
        semId = ((LosSemCB *)taskCB->taskSem)->semID;
    }

    return semId;
}

STATIC INLINE UINT32 OsGetMuxID(const LosTaskCB *taskCB)
{
    UINT32 muxId = LOSCFG_BASE_IPC_MUX_LIMIT;

    if (taskCB->taskMux != NULL) {
        muxId = ((LosMuxCB *)taskCB->taskMux)->muxID;
    }

    return muxId;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskInfoGet(UINT32 taskId, TSK_INFO_S *taskInfo)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;

    if (taskInfo == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    if (OS_CHECK_TSK_PID_NOIDLE(taskId)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    SCHEDULER_LOCK(intSave);

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    taskInfo->uwSP = (UINTPTR)taskCB->stackPointer;
    taskInfo->usTaskStatus = taskCB->taskStatus;
    taskInfo->usTaskPrio = taskCB->priority;
    taskInfo->uwStackSize = taskCB->stackSize;
    taskInfo->uwTopOfStack = taskCB->topOfStack;
    taskInfo->uwEvent = taskCB->event;
    taskInfo->uwEventMask = taskCB->eventMask;
    taskInfo->uwSemID = OsGetSemID(taskCB);
    taskInfo->uwMuxID = OsGetMuxID(taskCB);
    taskInfo->pTaskSem = taskCB->taskSem;
    taskInfo->pTaskMux = taskCB->taskMux;
    taskInfo->uwTaskID = taskId;
    // Ignore the return code when matching CSEC rule 6.6(4).
    (VOID)strncpy_s(taskInfo->acName, LOS_TASK_NAMELEN, taskCB->taskName, LOS_TASK_NAMELEN - 1);
    taskInfo->acName[LOS_TASK_NAMELEN - 1] = '\0';

    taskInfo->uwBottomOfStack = TRUNCATE((taskCB->topOfStack + taskCB->stackSize),
                                         OS_TASK_STACK_ADDR_ALIGN);
    taskInfo->uwCurrUsed = taskInfo->uwBottomOfStack - taskInfo->uwSP;
    taskInfo->uwPeakUsed = OsGetTaskWaterLine(taskId);
    taskInfo->bOvf = (taskInfo->uwPeakUsed == OS_NULL_INT) ? TRUE : FALSE;
    SCHEDULER_UNLOCK(intSave);

    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskStatusGet(UINT32 taskId, UINT32 *taskStatus)
{
    UINT32    intSave;
    LosTaskCB *taskCB = NULL;

    if (taskStatus == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    if (OS_CHECK_TSK_PID_NOIDLE(taskId)) {
        return LOS_ERRNO_TSK_ID_INVALID;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    SCHEDULER_LOCK(intSave);

    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return LOS_ERRNO_TSK_NOT_CREATED;
    }

    *taskStatus = taskCB->taskStatus;

    SCHEDULER_UNLOCK(intSave);

    return LOS_OK;
}

#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskSwitchInfoGet(UINT32 index, UINT32 *taskSwitchInfo)
{
    UINT32 intSave;
    UINT32 curIndex;

    curIndex = index;
    if (curIndex >= OS_TASK_SWITCH_INFO_COUNT) {
        curIndex %= OS_TASK_SWITCH_INFO_COUNT;
    }

    if (taskSwitchInfo == NULL) {
        return LOS_ERRNO_TSK_PTR_NULL;
    }

    SCHEDULER_LOCK(intSave);

    (*taskSwitchInfo) = g_taskSwitchInfo.pid[curIndex];

    if (memcpy_s((VOID *)(taskSwitchInfo + 1), LOS_TASK_NAMELEN,
                 g_taskSwitchInfo.name[curIndex], LOS_TASK_NAMELEN) != EOK) {
        PRINT_ERR("LOS_TaskSwitchInfoGet copy task name failed\n");
    }

    SCHEDULER_UNLOCK(intSave);
    return LOS_OK;
}
#endif

/*****************************************************************************
Function    : LOS_TaskInfoMonitor
Description : Get all task info
Input       : None
Return      : LOS_OK on success ,or OS_ERROR on failure
*****************************************************************************/
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskInfoMonitor(VOID)
{
    UINT32 retVal;

    retVal = OsGetAllTskInfo();

    return retVal;
}

/*****************************************************************************
 Function    : LOS_TaskIsRunning
 Description : Check if LiteOS has been started.
 Input       : VOID
 Output      : VOID
 Return      : TRUE means LiteOS was started, FALSE means not.
 *****************************************************************************/
LITE_OS_SEC_TEXT_MINOR BOOL LOS_TaskIsRunning(VOID)
{
    return g_taskScheduled;
}

/*****************************************************************************
 Function    : LOS_NewTaskIDGet
 Description : get id of current new task.
 Input       : None
 Output      : None
 Return      : task id
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_NewTaskIDGet(VOID)
{
    return LOS_NextTaskIDGet();
}

/*****************************************************************************
 Function    : LOS_TaskNameGet
 Description : get Name of current new task.
 Input       : taskId -----task id
 Output      : None
 Return      : task name
 *****************************************************************************/
LITE_OS_SEC_TEXT CHAR* LOS_TaskNameGet(UINT32 taskId)
{
    UINT32    intSave;
    LosTaskCB *taskCB = NULL;

    if (OS_CHECK_TSK_PID_NOIDLE(taskId)) {
        return NULL;
    }

    taskCB = OS_TCB_FROM_TID(taskId);

    SCHEDULER_LOCK(intSave);
    if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        return NULL;
    }
    SCHEDULER_UNLOCK(intSave);

    return taskCB->taskName;
}

LITE_OS_SEC_TEXT_MINOR VOID LOS_Msleep(UINT32 mSecs)
{
    UINT32 interval;

    if (OS_INT_ACTIVE) {
        return;
    }

    if (mSecs == 0) {
        interval = 0;
    } else {
        interval = LOS_MS2Tick(mSecs);
        if (interval == 0) {
            interval = 1;
        }
    }

    (VOID)LOS_TaskDelay(interval);
}

VOID LOS_TaskResRecycle(VOID)
{
    OsRecycleFinishedTask();
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_TaskCpuAffiSet(UINT32 taskId, UINT16 cpuAffiMask)
{
    (VOID)taskId;
    (VOID)cpuAffiMask;
    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT16 LOS_TaskCpuAffiGet(UINT32 taskId)
{
    (VOID)taskId;
    return 1; /* 1: mask of current cpu */
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_TaskAllocSecureContext(UINT32 taskId, UINT32 size)
{
    (VOID)taskId;
    (VOID)size;
    return LOS_OK;
}
