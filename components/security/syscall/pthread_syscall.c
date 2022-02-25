/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_task.h"
#include "securec.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_interrupt.h"
#include "los_memory.h"
#include "los_mpu.h"
#include "los_sched.h"
#include "los_mux.h"
#include "los_sem.h"
#include "los_timer.h"
#include "los_arch_context.h"
#include "los_box.h"
#include "los_syscall.h"

int SysUserTaskCreate(unsigned long entry, unsigned long userArea, unsigned long userSp, BOOL joinable)
{
    UINT32 ret, taskID;
    TSK_INIT_PARAM_S taskInitParam = { 0 };
    taskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)entry;
    taskInitParam.uwStackSize = 0x1000;
    taskInitParam.pcName = "UserTask";
    taskInitParam.usTaskPrio = OS_TASK_PRIORITY_LOWEST;
    taskInitParam.stackAddr = userSp;
    if (joinable) {
        taskInitParam.uwResved |= LOS_TASK_ATTR_JOINABLE;
    }
    ret = LOS_TaskCreateOnly(&taskID, &taskInitParam);
    if (ret != LOS_OK) {
        return -EINVAL;
    }

    OsUserTaskInit(taskID, entry, userArea, userSp);
    return taskID;
}

int SysSchedSetScheduler(unsigned int tid, int policy, int priority)
{
    if ((tid <= 0) || (tid > LOSCFG_BASE_CORE_TSK_LIMIT)) {
        return EINVAL;
    }

    if (policy != 0) {
        return EINVAL;
    }

    if ((priority <= 0) || (priority > OS_TASK_PRIORITY_LOWEST)) {
        return EINVAL;
    }

    UINT32 ret = LOS_TaskPriSet((UINT32)tid, (UINT16)priority);
    if (ret != LOS_OK) {
        return EINVAL;
    }

    return 0;
}

int *SysSchedGetArea(unsigned int tid)
{
    unsigned int intSave;
    int *area = NULL;

    if ((tid <= 0) || (tid > LOSCFG_BASE_CORE_TSK_LIMIT)) {
        return NULL;
    }

    intSave = LOS_IntLock();
    area = (int *)OsGetUserTaskCB(tid)->userArea;
    LOS_IntRestore(intSave);
    return area;
}

int SysSetThreadArea(const char *area)
{
    unsigned int intSave;

    intSave = LOS_IntLock();
    LosTaskCB *runTask = g_losTask.runTask;
    OsGetUserTaskCB(runTask->taskID)->userArea = (unsigned long)(uintptr_t)area;
    LOS_IntRestore(intSave);
    return 0;
}

char *SysGetThreadArea(void)
{
    unsigned int intSave;
    char *area = NULL;

    intSave = LOS_IntLock();
    LosTaskCB *runTask = g_losTask.runTask;
    area = (char *)OsGetUserTaskCB(runTask->taskID)->userArea;
    LOS_IntRestore(intSave);
    return area;
}
