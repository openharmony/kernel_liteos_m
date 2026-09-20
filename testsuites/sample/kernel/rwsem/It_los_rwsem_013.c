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

static UINT32 g_rwsemHandle013;

static VOID TaskF01(VOID)
{
    UINT32 ret;

    g_testCount++;
    /* Reader blocks here while the writer holds the write lock */
    ret = LOS_RwsemPendRead(g_rwsemHandle013, LOS_WAIT_FOREVER);
    ICUNIT_TRACK_EQUAL(ret, LOS_OK, ret);
    g_testCount++;

    LOS_TaskDelete(g_testTaskID01);
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    TSK_INIT_PARAM_S task = { 0 };

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    task.pcName = "Rwsem17";
    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.usTaskPrio = TASK_PRIO_TEST - 1; /* higher priority than test task */

    g_testCount = 0;

    ret = LOS_RwsemCreate(&g_rwsemHandle013);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* Acquire a write lock so the reader task will block on PendRead */
    ret = LOS_RwsemPendWrite(g_rwsemHandle013, LOS_WAIT_FOREVER);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_TaskCreate(&g_testTaskID01, &task);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Reader task runs, increments g_testCount, then blocks on PendRead */
    TEST_DELAY(g_testCount, 1, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

    /* Delete while a task is waiting on the rwsem → PENDED */
    ret = LOS_RwsemDelete(g_rwsemHandle013);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_PENDED, ret, EXIT);

    /* Release the write lock — should wake the reader */
    ret = LOS_RwsemPostWrite(g_rwsemHandle013);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Reader acquires read lock and exits */
    TEST_DELAY(g_testCount, 2, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_testCount, 2, g_testCount, EXIT);

    /* Now delete should succeed — waitList is empty */
    ret = LOS_RwsemDelete(g_rwsemHandle013);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    return LOS_OK;
EXIT:
    LOS_TaskDelete(g_testTaskID01);
    LOS_RwsemPostWrite(g_rwsemHandle013);
    LOS_RwsemDelete(g_rwsemHandle013);
    return LOS_OK;
}

/**
 * @ingroup TEST_RWSEM
 * @par TestCase_Number
 * ItLosRwsem013
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_RwsemDelete when the wait queue is not empty (PENDED)
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: create a rwsem, acquire a write lock
 * step2: create a higher-priority reader task that blocks on PendRead
 * step3: call LOS_RwsemDelete while reader is waiting
 * step4: call LOS_RwsemPostWrite to wake the reader
 * step5: after reader exits, call LOS_RwsemDelete again
 * @par TestCase_Expected_Result
 * 1. step3 returns LOS_ERRNO_RWSEM_PENDED
 * 2. step5 returns LOS_OK
 * @par TestCase_Level
 * Level 1
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */
VOID ItLosRwsem013(VOID)
{
    TEST_ADD_CASE("ItLosRwsem013", Testcase, TEST_LOS, TEST_RWSEM, TEST_LEVEL1, TEST_FUNCTION);
}
