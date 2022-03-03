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

#include "los_box.h"
#include "los_task.h"
#include "los_context.h"
#include "los_arch_context.h"
#include "los_debug.h"

static UserTaskCB g_UserTaskCBArray[LOSCFG_BASE_CORE_TSK_LIMIT] = { 0 };
static LosBoxCB g_boxCB[1];

VOID OsUserTaskInit(UINT32 taskID, UINTPTR entry, UINTPTR userArea, UINTPTR userSp)
{
    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
    taskCB->taskStatus |= OS_TASK_FLAG_USER_TASK;
    HalUserTaskStackInit(taskCB->stackPointer, entry, userSp);

    g_UserTaskCBArray[taskID].userArea = userArea;
    g_UserTaskCBArray[taskID].userSp = userSp;
    g_UserTaskCBArray[taskID].boxID = g_UserTaskCBArray[g_losTask.runTask->taskID].boxID;
}

VOID OsUserTaskDelete(UINT32 taskID)
{
    memset_s(&g_UserTaskCBArray[taskID], sizeof(UserTaskCB), 0, sizeof(UserTaskCB));
}

UserTaskCB *OsGetUserTaskCB(UINT32 taskID)
{
    return &g_UserTaskCBArray[taskID];
}

static UINT32 BoxInit(VOID)
{
    UINT32 count = sizeof(g_boxCB) / sizeof(LosBoxCB);
    for (UINT32 i = 0; i < count; i++) {
        LosBoxCB *box = &g_boxCB[i];
        box->boxStackAddr = box->boxStartAddr + box->boxSize - box->boxStackSize;
    }

    return LOS_OK;
}

VOID OsBoxStart(VOID)
{
    UINT32 ret, taskID;
    UINT32 count = sizeof(g_boxCB) / sizeof(LosBoxCB);
    TSK_INIT_PARAM_S taskInitParam = { 0 };

    for (UINT32 i = 0; i < count; i++) {
        LosBoxCB *box = &g_boxCB[i];
        taskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)box->boxStartAddr;
        taskInitParam.uwStackSize = 0x1000;
        taskInitParam.pcName = "BoxMainTask";
        taskInitParam.usTaskPrio = LOSCFG_BOX_PRIO;
        taskInitParam.uwResved = LOS_TASK_ATTR_JOINABLE;
        ret = LOS_TaskCreateOnly(&taskID, &taskInitParam);
        if (ret != LOS_OK) {
            PRINT_ERR("Create box %u main task failed, Error 0x%x\n", i, ret);
            return;
        }

        OsUserTaskInit(taskID, (UINTPTR)_ulibc_start, 0, box->boxStackAddr + box->boxStackSize);
        g_UserTaskCBArray[taskID].boxID = i;

        ret = LOS_TaskResume(taskID);
        if (ret != LOS_OK) {
            PRINT_ERR("Box(%u) resume task %u failed, Error 0x%x\n", i, taskID, ret);
            return;
        }
    }
}

UINT32 LOS_BoxStart(VOID)
{
    UINT32 ret, taskID;
    TSK_INIT_PARAM_S taskInitParam = { 0 };

    ret = BoxInit();
    if (ret != LOS_OK) {
        PRINT_ERR("Box init failed! Error 0x%x\n", ret);
        return ret;
    }

    taskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)OsBoxStart;
    taskInitParam.uwStackSize = 0x1000;
    taskInitParam.pcName = "BoxStart";
    taskInitParam.usTaskPrio = LOSCFG_BOX_START_PRIO;
    taskInitParam.uwResved = 0;
    return LOS_TaskCreate(&taskID, &taskInitParam);
}
