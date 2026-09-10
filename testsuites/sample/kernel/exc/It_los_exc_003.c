/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of the conditions and the following disclaimer in the documentation and/or other materials
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

#include "it_los_exc.h"

#if (LOSCFG_BACKTRACE_TYPE != 0)
static UINT32 g_excSemId;
static volatile UINT32 g_excBlockedFlag;

static VOID TaskF012(VOID)
{
    g_excBlockedFlag = 1;
    (VOID)LOS_SemPend(g_excSemId, LOS_WAIT_FOREVER);
    g_excBlockedFlag = 2;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 semId = 0;
    UINT32 taskCreated = 0;
    TSK_INIT_PARAM_S taskParam = {0};

    g_excBlockedFlag = 0;
    ret = LOS_SemCreate(0, &semId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    g_excSemId = semId;

    taskParam.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF012;
    taskParam.uwStackSize  = TASK_STACK_SIZE_TEST;
    taskParam.pcName       = "ExcBt012";
    taskParam.usTaskPrio   = TASK_PRIO_TEST - 1;
    taskParam.uwResved     = LOS_TASK_STATUS_DETACHED;

    ret = LOS_TaskCreate(&g_testTaskID01, &taskParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    taskCreated = 1;

    ICUNIT_GOTO_EQUAL(g_excBlockedFlag, 1, g_excBlockedFlag, EXIT);

    LOS_TaskBackTrace(g_testTaskID01);

    ICUNIT_GOTO_EQUAL(g_excBlockedFlag, 1, g_excBlockedFlag, EXIT);

    ret = LOS_SemPost(semId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ICUNIT_GOTO_EQUAL(g_excBlockedFlag, 2, g_excBlockedFlag, EXIT);

    ret = LOS_SemDelete(semId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    LOS_TaskResRecycle();
    return LOS_OK;

EXIT:
    if (taskCreated) {
        (VOID)LOS_SemPost(semId);
        (VOID)LOS_TaskDelete(g_testTaskID01);
    }
    (VOID)LOS_SemDelete(semId);
    LOS_TaskResRecycle();
    return LOS_NOK;
}

/**
 * @ingroup TEST_EXC
 * @par TestCase_Number
 * ItLosExc003
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_TaskBackTrace
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: create sem(count=0) + high-prio task "ExcBt012" that blocks on the sem
 * step2: call LOS_TaskBackTrace(blockedTaskID)
 * step3: LOS_SemPost(semId) and verify the task resumed
 * @par TestCase_Expected_Result
 * 1. backtrace call returns cleanly (no fault)
 * 2. blocked task still blocked after backtrace (g_excBlockedFlag stays 1)
 * 3. LOS_SemPost resumes the task (g_excBlockedFlag becomes 2)
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */
VOID ItLosExc003(VOID)
{
    TEST_ADD_CASE("ItLosExc003", Testcase, TEST_LOS, TEST_EXC, TEST_LEVEL0, TEST_FUNCTION);
}
#endif /* LOSCFG_BACKTRACE_TYPE != 0 */
