/*
 * Copyright (c) 2021 Nuclei Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_task.h"
#include "los_memory.h"
#include "los_timer.h"
#include "los_sched.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "nuclei_sdk_soc.h"

#define INITIAL_MSTATUS                 ( MSTATUS_MPP | MSTATUS_MPIE | MSTATUS_FS_INITIAL)
#define ALIGN_DOWN(size, align)         ((size) & ~((align) - 1))

LITE_OS_SEC_TEXT_INIT VOID ArchInit(VOID)
{
    UINT32 ret;
    HalHwiInit();

    ret = HalTickStart(OsTickHandler);
    if (ret != LOS_OK) {
        PRINT_ERR("Tick start failed!\n");
        return;
    }
}

LITE_OS_SEC_TEXT_MINOR VOID ArchSysExit(VOID)
{
    ArchIntLock();
    while (1) {
    }
}

LITE_OS_SEC_TEXT_INIT VOID *ArchTskStackInit(UINT32 taskID, UINT32 stackSize, VOID *topStack)
{
    UINT32 index;
    UINT8 *stk = 0;
    TaskContext  *context = NULL;

    /* initialize the task stack, write magic num to stack top */
    *((UINT32 *)(topStack)) = OS_TASK_MAGIC_WORD;

    stk = ((UINT8 *)topStack) + stackSize + sizeof(STACK_TYPE);
    stk = (UINT8 *)ALIGN_DOWN((uintptr_t)stk, REGBYTES);
    context = (TaskContext *)(stk - sizeof(TaskContext));

    for (index = 1; index < sizeof(TaskContext)/ sizeof(STACK_TYPE); index ++) {
        ((STACK_TYPE *)context)[index] = OS_TASK_STACK_INIT;
    }
    context->ra      = (STACK_TYPE)ArchSysExit;
    context->a0      = (STACK_TYPE)taskID;
    context->epc     = (STACK_TYPE)OsTaskEntry;

    context->mstatus = INITIAL_MSTATUS;

    return (VOID *)context;
}

extern LosTask g_losTask;
LITE_OS_SEC_TEXT_INIT UINT32 ArchStartSchedule(VOID)
{
    (VOID)LOS_IntLock();
    OsSchedStart();
    HalStartToRun();
    return LOS_OK; /* never return */
}

VOID ArchTaskSchedule(VOID)
{
    SysTimer_SetSWIRQ();
}

VOID HalTaskSwitch(VOID)
{
    SysTimer_ClearSWIRQ();
    OsSchedTaskSwitch();
    /* Set newTask to runTask */
    g_losTask.runTask = g_losTask.newTask;
}

