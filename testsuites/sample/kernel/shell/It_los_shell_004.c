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
 *    of the conditions and the following disclaimer in the documentation and/or other materials
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

#include "It_los_shell.h"

#ifdef LOSCFG_SHELL_LK

#define SHELL_LK_CONCURRENT_TASKS  4
#define SHELL_LK_CALLS_PER_TASK    50

static volatile UINT32 g_totalCalls;
static UINT32 g_taskIds[SHELL_LK_CONCURRENT_TASKS];
static UINT32 g_semId;

static VOID LkConcurrentTask(VOID)
{
    UINT32 i;
    for (i = 0; i < SHELL_LK_CALLS_PER_TASK; i++) {
        LOS_LkPrint(TRACE_INFO, __func__, __LINE__, "concurrent %u", i);
    }
    LOS_SemPost(g_semId);
}

static VOID ShellTestConcurrentHook(INT32 level, const CHAR *func, INT32 line, const CHAR *fmt, va_list ap)
{
    UINT32 intSave;
    (VOID)level;
    (VOID)func;
    (VOID)line;
    (VOID)fmt;
    (VOID)ap;
    intSave = LOS_IntLock();
    g_totalCalls++;
    (VOID)LOS_IntRestore(intSave);
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 i;
    UINT32 expected;
    TSK_INIT_PARAM_S task = { 0 };

    g_totalCalls = 0;
    ret = LOS_SemCreate(0, &g_semId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_LkRegHook(ShellTestConcurrentHook);

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)LkConcurrentTask;
    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.pcName = "LkCc";
    task.usTaskPrio = TASK_PRIO_TEST - 1;
    task.uwResved = LOS_TASK_STATUS_DETACHED;
    for (i = 0; i < SHELL_LK_CONCURRENT_TASKS; i++) {
        ret = LOS_TaskCreate(&g_taskIds[i], &task);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    }

    for (i = 0; i < SHELL_LK_CONCURRENT_TASKS; i++) {
        ret = LOS_SemPend(g_semId, LOS_WAIT_FOREVER);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    }

    expected = SHELL_LK_CONCURRENT_TASKS * SHELL_LK_CALLS_PER_TASK;
    ICUNIT_GOTO_EQUAL(g_totalCalls, expected, g_totalCalls, EXIT);

EXIT:
    OsLkLoggerInit("");
    LOS_SemDelete(g_semId);
    (VOID)LOS_TaskDelay(1);

    return LOS_OK;
}

/**
 * @ingroup TEST_SHELL
 * @par TestCase_Number
 * ItLosShell004
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test LOS_LkPrint concurrent reentrancy from multiple tasks
 * @par TestCase_Pretreatment_Condition
 * LOSCFG_SHELL_LK enabled
 * @par TestCase_Test_Steps
 * step1: create a semaphore and register a counting hook
 * step2: spawn 4 tasks, each calling LOS_LkPrint 50 times then LOS_SemPost
 * step3: wait for all 4 tasks via LOS_SemPend (precise synchronization)
 * step4: verify total hook invocations == 4*50=200
 * step5: cleanup (runs on pass and fail paths): restore the default logger,
 *        delete the semaphore, delay 1 tick so the idle task recycles the
 *        finished task stacks
 * @par TestCase_Expected_Result
 * 1. all tasks created successfully
 * 2. dispatcher reached the hook exactly 200 times
 * 3. no crash under concurrent LOS_LkPrint dispatch
 * 4. no leaked resources: semaphore deleted and task stacks recycled
 *    before the case returns (clean ResLeak report)
 * @par TestCase_Level
 * Level 2
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * Uses LOS_Sem for precise task completion synchronization (no blind delay loop).
 * The hook counter is protected by LOS_IntLock: g_totalCalls++ is not atomic
 * and liteos_m provides no atomic-op API on RISC-V.
 * Does NOT test concurrent LOS_LkRegHook (g_osLkHook is unlocked, known API limitation)
 */
VOID ItLosShell004(VOID)
{
    TEST_ADD_CASE("ItLosShell004", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL2, TEST_FUNCTION);
}

#endif /* LOSCFG_SHELL_LK */
