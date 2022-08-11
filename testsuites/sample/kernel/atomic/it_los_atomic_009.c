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

#define DB_ATOMIC_MUTI_TASK_NUM (ATOMIC_MUTI_TASK_NUM * 2) // 2 is coefficients

static VOID TaskF01(VOID)
{
    UINT32 i;
    for (i = 0; i < 10; ++i) { // run 10 times.
        LOS_Atomic64Dec(&g_testAtomicID05);
    }

    ++g_testCount;
}

static VOID TaskF02(VOID)
{
    UINT64 i;
    for (i = 0; i < 10; ++i) { // run 10 times.
        LOS_AtomicCmpXchg64bits(&g_testAtomicID05, g_testAtomicID05, g_testAtomicID05);
    }

    ++g_testCount;
}

static UINT32 TestCase(VOID)
{
    UINT32 ret, i;
    UINT32 taskId[DB_ATOMIC_MUTI_TASK_NUM];
    TSK_INIT_PARAM_S task[DB_ATOMIC_MUTI_TASK_NUM] = {0, };
    CHAR taskName[DB_ATOMIC_MUTI_TASK_NUM][20] = {"", }; // max taskName size is 20.
    CHAR buf[10] = ""; // max buf size is 10.
    Atomic uCount = 0;
    g_testCount = 0;
    g_testAtomicID05 = 0xffffffffff;
    UINT32 uLoop = g_taskMaxNum - TASK_EXISTED_NUM;

    if (uLoop > DB_ATOMIC_MUTI_TASK_NUM) {
        uLoop = DB_ATOMIC_MUTI_TASK_NUM;
    }

    for (i = 0; i < uLoop; i++) {
        memset(buf, 0, 10); // max buf size is 10.
        memset(taskName[i], 0, 20); // max taskName size is 20.

        if (i % 2 == 0) { // 2 is index.
            uCount++;
            task[i].pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
        } else {
            task[i].pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF02;
        }
        task[i].pcName       = taskName[i];
        task[i].uwStackSize  = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
        task[i].usTaskPrio   = TASK_PRIO_TEST - 2; // TASK_PRIO_TEST - 2 has higher priority than TASK_PRIO_TEST
        task[i].uwResved     = LOS_TASK_STATUS_DETACHED;
    }
    for (i = 0; i < uLoop; i++) {
        ret = LOS_TaskCreate(&taskId[i], &task[i]);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    }

    LOS_TaskDelay(20); // delay 20 ticks.
    ICUNIT_GOTO_EQUAL(g_testCount, uLoop, g_testCount, EXIT);
    ICUNIT_GOTO_EQUAL(g_testAtomicID05, (0xffffffffff - 10 * uCount), g_testAtomicID05, EXIT);  // run 10 times.
EXIT:
    for (i = 0; i < uLoop; i++) {
        (VOID)LOS_TaskDelete(taskId[i]);
    }
    return LOS_OK;
}

 /**
 * @ingroup TEST_ATO
 * @par TestCase_Number
 * ItLosAtomic009
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_Atomic64Dec and LOS_AtomicCmpXchg64bits in different task.
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Invoke the LOS_Atomic64Dec interface in TaskF01.
 * step2: Invoke the LOS_AtomicCmpXchg64bits interface in TaskF02.
 * @par TestCase_Expected_Result
 * 1.LOS_Atomic64Dec and LOS_AtomicCmpXchg64bits return expected result.
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

VOID ItLosAtomic009(VOID)
{
    TEST_ADD_CASE("ItLosAtomic009", TestCase, TEST_LOS, TEST_ATO, TEST_LEVEL0, TEST_FUNCTION);
}
