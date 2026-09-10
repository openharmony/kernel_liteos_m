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
 * ItLosCpup006 - cpup distinguishes load levels
 *
 * Tests: a CPU-heavy task (tight loop) reports usage more than 2x a
 *        CPU-light task (delay-bound) of higher priority. The busy task
 *        burns cycles between the light task's wakeups, so busyAll >> lightAll.
 * Guards: if unification makes cpup stop differentiating tasks (e.g. every
 *         task attributed the same allTime), busy <= light*2.
 */

#include "It_los_cpup.h"

#if (LOSCFG_BASE_CORE_CPUP == 1)

static VOID ItCpupBusyEntry(VOID)
{
    while (1) {
    }
}

static VOID ItCpupLightEntry(VOID)
{
    while (1) {
        LOS_TaskDelay(1);
    }
}

static UINT32 TestCase006(VOID)
{
    UINT32 ret;
    UINT32 busyID;
    UINT32 lightID;
    UINT32 busyCpup;
    UINT32 lightCpup;
    TSK_INIT_PARAM_S param = {0};

    param.uwStackSize = OS_TSK_TEST_STACK_SIZE;

    param.pfnTaskEntry = (TSK_ENTRY_FUNC)ItCpupBusyEntry;
    param.pcName = "ItCpup006B";
    param.usTaskPrio = TASK_PRIO_TEST + 2;
    ret = LOS_TaskCreate(&busyID, &param);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    param.pfnTaskEntry = (TSK_ENTRY_FUNC)ItCpupLightEntry;
    param.pcName = "ItCpup006L";
    param.usTaskPrio = TASK_PRIO_TEST;
    ret = LOS_TaskCreate(&lightID, &param);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_TaskDelay(120);

    busyCpup = LOS_TaskCpuUsage(busyID);
    lightCpup = LOS_TaskCpuUsage(lightID);
    ICUNIT_ASSERT_WITHIN_EQUAL(busyCpup, 1, LOS_CPUP_PRECISION, busyCpup);

    if (busyCpup <= lightCpup * 2) {
        ICunitSaveErr(__LINE__, (iiUINT32)busyCpup);
        return 1;
    }

    ret = LOS_TaskDelete(busyID);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_TaskDelete(lightID);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

VOID ItLosCpup006(VOID)
{
    TEST_ADD_CASE("ItLosCpup006", TestCase006, TEST_LOS, TEST_CPUP, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
