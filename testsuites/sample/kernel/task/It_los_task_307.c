/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary, with or without modification,
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


static VOID TaskF01(VOID)
{
    UINT32 ret;

    g_testCount++;

    /* self-suspend: stays alive so main can delete it cleanly (DETACHED) */
    ret = LOS_TaskSuspend(g_testTaskID01);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    return;
}

/* 用例简要描述: LOS_TaskUnlockNoSched Lock×2→UnlockNoSched 不调度→Unlock */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 lockCnt;
    TSK_INIT_PARAM_S task1 = { 0 };
    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    task1.uwStackSize = TASK_STACK_SIZE_TEST;
    task1.pcName = "Tsk307A";
    task1.usTaskPrio = TASK_PRIO_TEST - 1; /* higher prio than main */
    task1.uwResved = LOS_TASK_STATUS_DETACHED;

    g_testCount = 0;

    /* double-lock the scheduler; taskLockCnt becomes 2 */
    LOS_TaskLock();
    LOS_TaskLock();
    ICUNIT_GOTO_EQUAL(OsPercpuGet()->taskLockCnt, 2, OsPercpuGet()->taskLockCnt, EXIT_UNLOCK);

    /* create a higher-prio task while locked: it is enqueued but NOT run */
    ret = LOS_TaskCreate(&g_testTaskID01, &task1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT_UNLOCK);

    /* still locked -> helper has not run, g_testCount stays 0 */
    ICUNIT_GOTO_EQUAL(g_testCount, 0, g_testCount, EXIT_UNLOCK);

    /* F-Pos: UnlockNoSched decrements the count (2->1) WITHOUT scheduling.
       Because count is still non-zero, the pending higher-prio task must NOT
       have run yet -> g_testCount still 0. */
    LOS_TaskUnlockNoSched();
    lockCnt = OsPercpuGet()->taskLockCnt;
    ICUNIT_GOTO_EQUAL(lockCnt, 1, lockCnt, EXIT_UNLOCK);
    ICUNIT_GOTO_EQUAL(g_testCount, 0, g_testCount, EXIT_UNLOCK);

EXIT_UNLOCK:
    /* F-Pos: final paired LOS_TaskUnlock brings count to 0 AND triggers the
       pending schedule -> the higher-prio helper finally runs (g_testCount==1). */
    LOS_TaskUnlock();
    lockCnt = OsPercpuGet()->taskLockCnt;
    ICUNIT_GOTO_EQUAL(lockCnt, 0, lockCnt, EXIT);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

EXIT:
    (VOID)LOS_TaskDelete(g_testTaskID01);

    return LOS_OK;
}

VOID ItLosTask307(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosTask307", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}
