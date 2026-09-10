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
 * ItLosCpup005 - LOS_SysCpuUsage responds to load
 *
 * Tests: after injecting a CPU-heavy task, LOS_SysCpuUsage must rise
 *        (after > before). Idle is starved while the busy task runs, so the
 *        non-idle share strictly grows.
 * Guards: if unification breaks the sampling period/guard so cpup stops
 *         tracking real load, SysCpuUsage stops rising (after <= before).
 */

#include "It_los_cpup.h"

#if (LOSCFG_BASE_CORE_CPUP == 1)

static VOID ItCpupLoadTaskEntry(VOID)
{
    while (1) {
    }
}

static UINT32 TestCase005(VOID)
{
    UINT32 ret;
    UINT32 taskId;
    UINT32 sysCpupBefore;
    UINT32 sysCpupAfter;
    TSK_INIT_PARAM_S param = {0};

    sysCpupBefore = LOS_SysCpuUsage();
    ICUNIT_ASSERT_WITHIN_EQUAL(sysCpupBefore, 0, LOS_CPUP_PRECISION, sysCpupBefore);

    param.pfnTaskEntry = (TSK_ENTRY_FUNC)ItCpupLoadTaskEntry;
    param.uwStackSize = OS_TSK_TEST_STACK_SIZE;
    param.pcName = "ItCpup005";
    param.usTaskPrio = TASK_PRIO_TEST + 2;
    ret = LOS_TaskCreate(&taskId, &param);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_TaskDelay(120);

    sysCpupAfter = LOS_SysCpuUsage();
    ICUNIT_ASSERT_WITHIN_EQUAL(sysCpupAfter, 0, LOS_CPUP_PRECISION, sysCpupAfter);

    if (sysCpupAfter <= sysCpupBefore) {
        ICunitSaveErr(__LINE__, (iiUINT32)sysCpupAfter);
        return 1;
    }

    ret = LOS_TaskDelete(taskId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

VOID ItLosCpup005(VOID)
{
    TEST_ADD_CASE("ItLosCpup005", TestCase005, TEST_LOS, TEST_CPUP, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
