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
 *    of the following disclaimer in the documentation and/or other materials
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

#include "osTest.h"
#include "It_los_task.h"


/* user-provided static stack, 16-byte aligned per LOSCFG_STACK_POINT_ALIGN_SIZE */
STATIC CHAR g_stack304[OS_TSK_TEST_STACK_SIZE] __attribute__((aligned(16)));

static VOID TaskF01(VOID)
{
    UINT32 ret;

    g_testCount++;

    /* suspend self: CreateStatic already started us (g_testCount==1); suspend so
       main can delete the task without racing the DETACHED auto-reclaim. */
    ret = LOS_TaskSuspend(g_testTaskID01);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    return;
}

/* 用例简要描述: LOS_TaskCreateStatic 静态栈自动启动 */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 status = 0;
    TSK_INIT_PARAM_S task1 = { 0 };
    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    task1.uwStackSize = sizeof(g_stack304);
    task1.pcName = "Tsk304A";
    task1.usTaskPrio = TASK_PRIO_TEST - 1;
    task1.uwResved = LOS_TASK_STATUS_DETACHED;
    task1.stackAddr = (UINTPTR)NULL; /* will be overwritten by CreateStatic */

    g_testCount = 0;

    /* F-Pos: CreateStatic creates AND starts the task (higher prio runs now) */
    ret = LOS_TaskCreateStatic(&g_testTaskID01, &task1, g_stack304);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* unlike CreateOnlyStatic, the task has already run: g_testCount==1 */
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

    /* task is now self-suspended */
    ret = LOS_TaskStatusGet(g_testTaskID01, &status);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(status & OS_TASK_STATUS_SUSPEND, 0, status, EXIT);

    /* F-Inv: NULL param returns LOS_ERRNO_TSK_PTR_NULL */
    ret = LOS_TaskCreateStatic(&g_testTaskID01, NULL, g_stack304);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_TSK_PTR_NULL, ret, EXIT);

EXIT:
    (VOID)LOS_TaskDelete(g_testTaskID01);

    return LOS_OK;
}

VOID ItLosTask304(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosTask304", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}
