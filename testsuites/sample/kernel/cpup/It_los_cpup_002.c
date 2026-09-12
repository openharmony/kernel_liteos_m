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
 * ItLosCpup002 - LOS_SysCpuUsage formula
 *
 * Tests: LOS_SysCpuUsage() == LOS_CPUP_PRECISION - LOS_TaskCpuUsage(idle).
 *        The system usage plus the idle usage must sum to the full permillage.
 * Guards: if unification rewrites SysCpuUsage to drop the idle subtraction
 *         (different time base, wrong operand), the sum drifts off 1000.
 */

#include "It_los_cpup.h"

#if (LOSCFG_BASE_CORE_CPUP == 1)

static UINT32 TestCase002(VOID)
{
    UINT32 sysCpup;
    UINT32 idleCpup;
    UINT32 sum;
    UINT32 idleTaskId = OsGetIdleTaskId();

    ICUNIT_ASSERT_WITHIN_EQUAL(idleTaskId, 0, g_taskMaxNum - 1, idleTaskId);

    sysCpup = LOS_SysCpuUsage();
    ICUNIT_ASSERT_WITHIN_EQUAL(sysCpup, 0, LOS_CPUP_PRECISION, sysCpup);

    idleCpup = LOS_TaskCpuUsage(idleTaskId);
    ICUNIT_ASSERT_WITHIN_EQUAL(idleCpup, 0, LOS_CPUP_PRECISION, idleCpup);

    sum = sysCpup + idleCpup;
    ICUNIT_ASSERT_WITHIN_EQUAL(sum, LOS_CPUP_PRECISION - 10, LOS_CPUP_PRECISION + 10, sum);

    return LOS_OK;
}

VOID ItLosCpup002(VOID)
{
    TEST_ADD_CASE("ItLosCpup002", TestCase002, TEST_LOS, TEST_CPUP, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
