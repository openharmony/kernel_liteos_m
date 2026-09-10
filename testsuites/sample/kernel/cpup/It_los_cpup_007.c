/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
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

/*
 * ItLosCpup007 - cpup tracks task lifecycle
 *
 * Tests: on LOS_TaskCreate the task appears active (usStatus != 0) in
 *        LOS_AllTaskCpuUsage; on LOS_TaskDelete it disappears (usStatus == 0).
 * Guards: if unification drops the g_cpup[taskId].status sync on create
 *         (los_task.c) or the memset on delete, status stays wrong.
 */

#include "It_los_cpup.h"

#if (LOSCFG_BASE_CORE_CPUP == 1)

static VOID ItCpupLifeTaskEntry(VOID)
{
    while (1) {
        LOS_TaskDelay(1);
    }
}

static UINT32 TestCase007(VOID)
{
    UINT32 ret;
    UINT32 taskId;
    UINT32 sz;
    TSK_INIT_PARAM_S param = {0};
    CPUP_INFO_S *cpupInfo;

    param.pfnTaskEntry = (TSK_ENTRY_FUNC)ItCpupLifeTaskEntry;
    param.uwStackSize = OS_TSK_TEST_STACK_SIZE;
    param.pcName = "ItCpup007";
    param.usTaskPrio = TASK_PRIO_TEST + 2;

    ret = LOS_TaskCreate(&taskId, &param);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_TaskDelay(20);

    sz = g_taskMaxNum * sizeof(CPUP_INFO_S);
    cpupInfo = (CPUP_INFO_S *)LOS_MemAlloc((void *)LOSCFG_SYS_HEAP_ADDR, sz);
    ICUNIT_ASSERT_NOT_EQUAL(cpupInfo, NULL, 0);

    (VOID)memset_s(cpupInfo, sz, 0, sz);
    ret = LOS_AllTaskCpuUsage(cpupInfo, CPUP_IN_10S);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ICUNIT_ASSERT_NOT_EQUAL(cpupInfo[taskId].usStatus, 0, cpupInfo[taskId].usStatus);

    ret = LOS_TaskDelete(taskId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_TaskDelay(1);

    (VOID)memset_s(cpupInfo, sz, 0, sz);
    ret = LOS_AllTaskCpuUsage(cpupInfo, CPUP_IN_10S);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ICUNIT_ASSERT_EQUAL(cpupInfo[taskId].usStatus, 0, cpupInfo[taskId].usStatus);

    (VOID)LOS_MemFree((void *)LOSCFG_SYS_HEAP_ADDR, cpupInfo);
    return LOS_OK;
}

VOID ItLosCpup007(VOID)
{
    TEST_ADD_CASE("ItLosCpup007", TestCase007, TEST_LOS, TEST_CPUP, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
