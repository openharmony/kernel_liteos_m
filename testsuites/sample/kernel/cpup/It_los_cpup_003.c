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
 * ItLosCpup003 - task sampling wired into scheduler
 *
 * Tests: a task that actually ran must report non-zero cpup via
 *        LOS_TaskCpuUsage.
 * Guards: if unification drops/desyncs the OsTskCycleStart/End hooks
 *         on schedule switch, the busy task's allTime stays 0 and cpup == 0.
 */

#include "It_los_cpup.h"

#if (LOSCFG_BASE_CORE_CPUP == 1)

static VOID ItCpupSampleTaskEntry(VOID)
{
    while (1) {
    }
}

static UINT32 TestCase003(VOID)
{
    UINT32 ret;
    UINT32 taskId;
    UINT32 busyCpup;
    TSK_INIT_PARAM_S param = {0};

    /* Measure this short-lived busy task in its own CPUP sampling window. */
    LOS_CpupReset();

    param.pfnTaskEntry = (TSK_ENTRY_FUNC)ItCpupSampleTaskEntry;
    param.uwStackSize = OS_TSK_TEST_STACK_SIZE;
    param.pcName = "ItCpup003";
    param.usTaskPrio = TASK_PRIO_TEST + 2;
    ret = LOS_TaskCreate(&taskId, &param);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_TaskDelay(120);

    busyCpup = LOS_TaskCpuUsage(taskId);
    ICUNIT_ASSERT_WITHIN_EQUAL(busyCpup, LOS_CPUP_PRECISION - 10, LOS_CPUP_PRECISION, busyCpup);

    ret = LOS_TaskDelete(taskId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_CpupReset();

    return LOS_OK;
}

VOID ItLosCpup003(VOID)
{
    TEST_ADD_CASE("ItLosCpup003", TestCase003, TEST_LOS, TEST_CPUP, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
