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
 * ItLosCpup001 - parameter validation of all cpup public APIs
 *
 * Tests: every public cpup API rejects bad parameters with the documented
 *        error code.
 *  - LOS_HistorySysCpuUsage(mode): CPUP_IN_10S/CPUP_IN_1S/CPUP_ALL_TIME
 *        must not leak LOS_ERRNO_CPUP_NO_INIT.
 *  - LOS_TaskCpuUsage(taskId): invalid ID -> LOS_ERRNO_CPUP_TSK_ID_INVALID,
 *        deleted task -> LOS_ERRNO_CPUP_THREAD_NO_CREATED.
 *  - LOS_HistoryTaskCpuUsage(taskId, mode): same as above for each mode.
 *  - LOS_AllTaskCpuUsage(cpupInfo, mode): NULL -> LOS_ERRNO_CPUP_TASK_PTR_NULL.
 *  - LOS_CpupUsageMonitor(type, mode, taskId): invalid type/taskId -> OS_ERROR.
 *  - LOS_GetAllIrqCpuUsage(mode, cpupInfo): NULL -> LOS_ERRNO_CPUP_TASK_PTR_NULL.
 *
 * Guards: if unification merges/touches the error-code branches or NULL
 *         handling, any changed error code fails here.
 */

#include "It_los_cpup.h"

#if (LOSCFG_BASE_CORE_CPUP == 1)

static VOID ItCpupParamTaskEntry(VOID)
{
    while (1) {
        LOS_TaskDelay(1);
    }
}

static UINT32 TestCase001(VOID)
{
    UINT32 ret;
    UINT32 taskId;
    TSK_INIT_PARAM_S param = {0};

    ret = LOS_HistorySysCpuUsage(CPUP_IN_10S);
    ICUNIT_ASSERT_NOT_EQUAL(ret, LOS_ERRNO_CPUP_NO_INIT, ret);
    ret = LOS_HistorySysCpuUsage(CPUP_IN_1S);
    ICUNIT_ASSERT_NOT_EQUAL(ret, LOS_ERRNO_CPUP_NO_INIT, ret);
    ret = LOS_HistorySysCpuUsage(CPUP_ALL_TIME);
    ICUNIT_ASSERT_NOT_EQUAL(ret, LOS_ERRNO_CPUP_NO_INIT, ret);

    ret = LOS_TaskCpuUsage(0xFFFF);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_TSK_ID_INVALID, ret);
    ret = LOS_HistoryTaskCpuUsage(0xFFFF, CPUP_IN_10S);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_TSK_ID_INVALID, ret);

    param.pfnTaskEntry = (TSK_ENTRY_FUNC)ItCpupParamTaskEntry;
    param.uwStackSize = OS_TSK_TEST_STACK_SIZE;
    param.pcName = "ItCpup001";
    param.usTaskPrio = TASK_PRIO_TEST + 2;
    ret = LOS_TaskCreate(&taskId, &param);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_TaskDelay(2);
    ret = LOS_TaskDelete(taskId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_TaskCpuUsage(taskId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_THREAD_NO_CREATED, ret);
    ret = LOS_HistoryTaskCpuUsage(taskId, CPUP_IN_10S);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_THREAD_NO_CREATED, ret);

    ret = LOS_AllTaskCpuUsage(NULL, CPUP_IN_10S);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_TASK_PTR_NULL, ret);

    ret = LOS_CpupUsageMonitor(SYS_CPU_USAGE, CPUP_IN_10S, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_CpupUsageMonitor(TASK_CPU_USAGE, CPUP_IN_10S, 0xFFFF);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERROR, ret);
    ret = LOS_CpupUsageMonitor((CPUP_TYPE_E)0xFF, CPUP_IN_10S, 0);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERROR, ret);

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    ret = LOS_GetAllIrqCpuUsage(CPUP_IN_10S, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_TASK_PTR_NULL, ret);
#endif

    return LOS_OK;
}

VOID ItLosCpup001(VOID)
{
    TEST_ADD_CASE("ItLosCpup001", TestCase001, TEST_LOS, TEST_CPUP, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
