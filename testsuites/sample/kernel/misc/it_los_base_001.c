/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of the following disclaimer in the documentation and/or materials provided with the
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
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "it_los_misc.h"

#define DELAY_TICKS         10
#define DELAY_TEST_COUNT    10

static UINT32 TestCase(VOID)
{
    UINT64 tickBefore;
    UINT64 tickAfter;
    UINT32 delta;
    UINT32 i;

    for (i = 0; i < DELAY_TEST_COUNT; i++) {
        tickBefore = LOS_TickCountGet();
        LOS_Msleep(LOS_Tick2MS(DELAY_TICKS));
        tickAfter = LOS_TickCountGet();
        delta = (UINT32)(tickAfter - tickBefore);
        PRINTK("%2d delta:%d\n", i, delta);
        ICUNIT_ASSERT_WITHIN_EQUAL(delta, DELAY_TICKS - 1, DELAY_TICKS + 2, delta);
    }

    return LOS_OK;
}

/**
 * @ingroup TEST_MISC
 * @par TestCase_Number
 * ItLosBase001
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_Msleep
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Loop DELAY_TEST_COUNT times, each iteration records tick count before and after LOS_Msleep.
 * step2: Verify the tick delta is within [DELAY_TICKS-1, DELAY_TICKS+2] (tolerate scheduling jitter on both QEMU and real hardware).
 * @par TestCase_Expected_Result
 * 1.Tick count delta after each LOS_Msleep call falls within [DELAY_TICKS-1, DELAY_TICKS+2].
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

VOID ItLosBase001(VOID)
{
    TEST_ADD_CASE("ItLosBase001", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL0, TEST_FUNCTION);
}
