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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "It_los_rwsem.h"

static UINT32 g_rwsemHandle014;

static VOID TaskF01(VOID)
{
    UINT32 ret;

    g_testCount++;
    /* Writer blocks here while the reader holds the read lock */
    ret = LOS_RwsemPendWrite(g_rwsemHandle014, LOS_WAIT_FOREVER);
    ICUNIT_TRACK_EQUAL(ret, LOS_OK, ret);
    g_testCount++;

    ret = LOS_RwsemPostWrite(g_rwsemHandle014);
    ICUNIT_TRACK_EQUAL(ret, LOS_OK, ret);

    LOS_TaskDelete(g_testTaskID01);
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    TSK_INIT_PARAM_S task = { 0 };

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    task.pcName = "Rwsem14";
    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.usTaskPrio = TASK_PRIO_TEST - 1; /* higher priority than test task */

    g_testCount = 0;

    ret = LOS_RwsemCreate(&g_rwsemHandle014);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* Acquire a read lock so the writer task will block */
    ret = LOS_RwsemPendRead(g_rwsemHandle014, LOS_WAIT_FOREVER);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_TaskCreate(&g_testTaskID01, &task);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Writer task runs, increments g_testCount, then blocks on PendWrite */
    TEST_DELAY(g_testCount, 1, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

    /* Release the read lock — should wake the writer */
    ret = LOS_RwsemPostRead(g_rwsemHandle014);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Writer acquires write lock, increments count, posts and exits */
    TEST_DELAY(g_testCount, 2, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_testCount, 2, g_testCount, EXIT);

EXIT:
    LOS_TaskDelete(g_testTaskID01);
    LOS_RwsemDelete(g_rwsemHandle014);
    return LOS_OK;
}

/**
 * @ingroup TEST_RWSEM
 * @par TestCase_Number
 * ItLosRwsem010
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test reader-writer blocking: writer blocks while reader holds read lock
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: create a rwsem, acquire a read lock
 * step2: create a higher-priority writer task that calls LOS_RwsemPendWrite
 * step3: writer blocks (g_testCount == 1)
 * step4: call LOS_RwsemPostRead to release the read lock
 * step5: writer is woken, acquires write lock, posts and exits (g_testCount == 2)
 * @par TestCase_Expected_Result
 * 1. writer blocks in step3 (g_testCount == 1)
 * 2. writer completes in step5 (g_testCount == 2)
 * @par TestCase_Level
 * Level 1
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */
VOID ItLosRwsem010(VOID)
{
    TEST_ADD_CASE("ItLosRwsem010", Testcase, TEST_LOS, TEST_RWSEM, TEST_LEVEL1, TEST_FUNCTION);
}
