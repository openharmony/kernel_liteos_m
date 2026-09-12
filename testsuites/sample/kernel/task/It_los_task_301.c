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
 * "AS IS" AND ANY EXPRESS OR IMPLIED, WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
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


static VOID TaskF01(VOID)
{
    UINT32 ret;

    g_testCount++;

    /* suspend self so the task stays alive (DETACHED would auto-reclaim on return,
       making LOS_TaskNameGet return NULL). Main resumes us for cleanup. */
    ret = LOS_TaskSuspend(g_testTaskID01);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    return;
}

/* 用例简要描述: LOS_TaskNameGet F-Pos/F-Inv */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    CHAR *name = NULL;
    TSK_INIT_PARAM_S task1 = { 0 };
    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    task1.uwStackSize = TASK_STACK_SIZE_TEST;
    task1.pcName = "Tsk301A";
    task1.usTaskPrio = TASK_PRIO_TEST - 1;
    task1.uwResved = LOS_TASK_STATUS_DETACHED;

    g_testCount = 0;

    ret = LOS_TaskCreate(&g_testTaskID01, &task1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Pos: helper runs immediately (higher prio) then suspends itself: g_testCount=1 */
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

    /* F-Pos: get name of a valid, suspended (alive) task */
    name = LOS_TaskNameGet(g_testTaskID01);
    ICUNIT_GOTO_NOT_EQUAL(name, NULL, name, EXIT);
    ICUNIT_GOTO_EQUAL(strcmp(name, "Tsk301A"), 0, 0, EXIT);

    /* F-Inv: invalid task ID returns NULL */
    name = LOS_TaskNameGet((UINT32)-1);
    ICUNIT_GOTO_EQUAL(name, NULL, name, EXIT);

    /* cleanup: resume the suspended helper so it can exit (DETACHED -> auto-reclaim) */
    ret = LOS_TaskResume(g_testTaskID01);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    (VOID)LOS_TaskDelete(g_testTaskID01);
    /* DETACHED 子任务 resume 后自删除,栈回收延迟到 idle 任务调度——
       立即 return 会让 ResLeak 采到未回收栈(+4108B);delay 让 idle 先跑 */
    (VOID)LOS_TaskDelay(2); /* 2, delay ticks */

    return LOS_OK;
}

VOID ItLosTask301(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosTask301", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}
