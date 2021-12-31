/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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


#include "it_los_atomic.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

static VOID TaskF01(VOID)
{
    INT32 i;
    UINTPTR ret;
    UINTPTR count;
    for (i = 0; i < 100; ++i) { // run 100 times.
        count = g_testAtomicID03;
        ret = LOS_AtomicIncRet(&g_testAtomicID03);
        ICUNIT_ASSERT_EQUAL_VOID(ret, g_testAtomicID03, ret);
        ICUNIT_ASSERT_EQUAL_VOID((count + 1), g_testAtomicID03, (count + 1));
    }
    ICUNIT_ASSERT_EQUAL_VOID(g_testAtomicID03, i, g_testAtomicID03);
    ICUNIT_ASSERT_EQUAL_VOID(ret, g_testAtomicID03, ret);

    LOS_AtomicAdd(&g_testCount, 1);
}

static UINT32 TestCase(VOID)
{
    UINT32 ret;

    g_testAtomicID03 = 0;

    g_testCount = 0;

    TSK_INIT_PARAM_S stTask1 = {0};
    stTask1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    stTask1.pcName       = "Atomic_004";
    stTask1.uwStackSize  = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    stTask1.usTaskPrio   = TASK_PRIO_TEST - 2; // TASK_PRIO_TEST - 2 has higher priority than TASK_PRIO_TEST
    stTask1.uwResved     = LOS_TASK_STATUS_DETACHED;

    ret = LOS_TaskCreate(&g_testTaskID01, &stTask1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_TaskDelay(5); // delay 5 ticks.

    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    return LOS_OK;
}

/**
 * @ingroup TEST_ATO
 * @par TestCase_Number
 * ItLosAtomic004
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_AtomicIncRet 100 times.
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Invoke the LOS_AtomicIncRet interface 100 times in a task.
 * @par TestCase_Expected_Result
 * 1.LOS_AtomicIncRet return expected result.
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

VOID ItLosAtomic004(VOID)
{
    TEST_ADD_CASE("ItLosAtomic004", TestCase, TEST_LOS, TEST_ATO, TEST_LEVEL0, TEST_FUNCTION);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
