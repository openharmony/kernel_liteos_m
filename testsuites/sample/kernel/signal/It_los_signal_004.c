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

STATIC UINT32 g_taskErrorFlag = 0;
STATIC UINT32 g_taskEnd = 0;


STATIC VOID TaskF01(VOID)
{
    UINT32 ret;
    sigset_t set = LOS_SIGNAL_MASK(SIGUSR1);
    siginfo_t info = {0};

    LOS_SignalSet(SIGUSR1, SIG_DFL);

    ret = LOS_SignalWait(&set, &info, 10);
    if (ret != LOS_ERRNO_SIGNAL_TIMEOUT) {
        g_taskErrorFlag = ret;
        return;
    }

    ret = LOS_SignalWait(&set, &info, LOS_WAIT_FOREVER);
    if (ret != SIGUSR1) {
        g_taskErrorFlag = ret;
        return;
    }

    g_taskEnd = 1;
}

STATIC UINT32 Testcase(VOID)
{
    TSK_INIT_PARAM_S task = {0};
    UINT32 taskID;
    UINT32 ret;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    task.pcName = "Signal";
    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.usTaskPrio = TASK_PRIO_TEST - 1;

    ret = LOS_TaskCreate(&taskID, &task);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_TaskDelay(20);
    ret = LOS_SignalSend(taskID, SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ICUNIT_ASSERT_EQUAL(g_taskErrorFlag, 0, g_taskErrorFlag);
    ICUNIT_ASSERT_EQUAL(g_taskEnd, 1, g_taskEnd);

    return LOS_OK;
}

VOID ItLosSignal004(VOID)
{
    TEST_ADD_CASE("ItLosSignal004", Testcase, TEST_LOS, TEST_SIGNAL, TEST_LEVEL0, TEST_FUNCTION);
}

