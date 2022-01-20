/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

#include "It_los_signal.h"
#include "los_task.h"

STATIC volatile INT32 g_sigValue1 = -1;
STATIC volatile INT32 g_sigValue2 = -1;
STATIC UINT32 g_task1End = 0;
STATIC UINT32 g_task2End = 0;

STATIC VOID SigHandler1(INT32 sig)
{
    g_sigValue1 = sig;
}

STATIC VOID SigHandler2(INT32 sig)
{
    g_sigValue2 = sig;
}

STATIC VOID TaskF01(VOID)
{
    LOS_SignalSet(SIGUSR1, SigHandler1);

    while (g_sigValue1 != SIGUSR1) {
        LOS_TaskDelay(1);
    }

    g_task1End = 1;
}

STATIC VOID TaskF02(VOID)
{
    LOS_SignalSet(SIGUSR2, SigHandler2);

    while (g_sigValue2 != SIGUSR2) {
        LOS_TaskDelay(1);
    }

    g_task2End = 1;
}

STATIC UINT32 Testcase(VOID)
{
    TSK_INIT_PARAM_S task = {0};
    UINT32 taskID1, taskID2;
    UINT32 ret;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    task.pcName = "Signal1";
    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.usTaskPrio = TASK_PRIO_TEST - 1;

    ret = LOS_TaskCreate(&taskID1, &task);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF02;
    task.pcName = "Signal2";
    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.usTaskPrio = TASK_PRIO_TEST - 1;

    ret = LOS_TaskCreate(&taskID2, &task);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_SignalSend(taskID1, SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_TaskDelay(10);
    ICUNIT_ASSERT_EQUAL(g_sigValue1, SIGUSR1, g_sigValue1);
    ICUNIT_ASSERT_EQUAL(g_task1End, 1, g_task1End);

    ret = LOS_SignalSend(taskID2, SIGUSR2);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_TaskDelay(10);
    ICUNIT_ASSERT_EQUAL(g_sigValue2, SIGUSR2, g_sigValue2);
    ICUNIT_ASSERT_EQUAL(g_task2End, 1, g_task2End);

    return LOS_OK;
}

VOID ItLosSignal003(VOID)
{
    TEST_ADD_CASE("ItLosSignal003", Testcase, TEST_LOS, TEST_SIGNAL, TEST_LEVEL0, TEST_FUNCTION);
}
