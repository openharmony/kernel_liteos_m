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


static VOID TaskF02(VOID)
{
    UINT32 ret;

    g_testCount++;

    /* suspend self: state becomes SUSPEND, control returns to main task */
    ret = LOS_TaskSuspend(g_testTaskID02);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    return;
}

/* 用例简要描述: LOS_TaskStatusGet F-Pos/F-Inv/F-State */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 status = 0;
    TSK_INIT_PARAM_S task2 = { 0 };
    task2.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF02;
    task2.uwStackSize = TASK_STACK_SIZE_TEST;
    task2.pcName = "Tsk302A";
    task2.usTaskPrio = TASK_PRIO_TEST - 1;
    task2.uwResved = LOS_TASK_STATUS_DETACHED;

    g_testCount = 0;

    ret = LOS_TaskCreate(&g_testTaskID02, &task2);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* helper runs immediately (higher prio) and suspends itself: g_testCount=1, then yields */
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

    /* F-State: status of a suspended task must carry the SUSPEND bit */
    ret = LOS_TaskStatusGet(g_testTaskID02, &status);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(status & OS_TASK_STATUS_SUSPEND, 0, status, EXIT);

    /* F-Inv: NULL output pointer -> LOS_ERRNO_TSK_PTR_NULL */
    ret = LOS_TaskStatusGet(g_testTaskID02, NULL);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_TSK_PTR_NULL, ret, EXIT);

    /* F-Inv: invalid task ID -> LOS_ERRNO_TSK_ID_INVALID */
    ret = LOS_TaskStatusGet((UINT32)-1, &status);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_TSK_ID_INVALID, ret, EXIT);

    /* cleanup: resume the suspended helper so it can exit (DETACHED -> auto-reclaim) */
    ret = LOS_TaskResume(g_testTaskID02);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    (VOID)LOS_TaskDelete(g_testTaskID02);
    /* 同 task_301: DETACHED 栈回收需 idle 先调度,防 ResLeak 采到未回收栈 */
    (VOID)LOS_TaskDelay(2); /* 2, delay ticks */

    return LOS_OK;
}

VOID ItLosTask302(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosTask302", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}
