/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    the following disclaimer in the documentation and/or materials provided with the
 *    distribution.
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
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "it_los_misc.h"
#include "los_tick.h"

#define CYCLE_TEST_COUNT    5

static UINT32 TestCase(VOID)
{
    UINT32 i;
    UINT64 cycle1, cycle2;
    UINT32 hi, lo;
    UINT64 combined;

    for (i = 0; i < CYCLE_TEST_COUNT; i++) {
        cycle1 = LOS_SysCycleGet();
        LOS_GetCpuCycle(&hi, &lo);
        cycle2 = LOS_SysCycleGet();

        combined = ((UINT64)hi << 32) | lo;

        ICUNIT_ASSERT_NOT_EQUAL(cycle1, 0, cycle1);
        ICUNIT_ASSERT_NOT_EQUAL(combined, 0, combined);

        /* LOS_GetCpuCycle was called between two LOS_SysCycleGet calls,
         * so combined should fall within [cycle1, cycle2] */
        ICUNIT_ASSERT_WITHIN_EQUAL(combined, cycle1, cycle2, cycle1);
    }

    return LOS_OK;
}

VOID ItLosBase002(VOID)
{
    TEST_ADD_CASE("ItLosBase002", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL0, TEST_FUNCTION);
}
