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

static UINT32 TestCase(VOID)
{
    const UINT32 boundaries[] = {4, 8, 16, 32};
    UINT32 count, b;
    UINT32 addr, aligned, truncated, mask;

    for (count = 0; count < TEST_COUNT; count++) {
        addr = count;
        for (b = 0; b < sizeof(boundaries) / sizeof(boundaries[0]); b++) {
            mask = boundaries[b] - 1;

            aligned = LOS_Align(addr, boundaries[b]);
            ICUNIT_ASSERT_EQUAL(ALIGN(addr, boundaries[b]), aligned, 0);
            ICUNIT_ASSERT_EQUAL((aligned & mask), 0, aligned);
            ICUNIT_ASSERT_EQUAL(aligned >= addr, 1, aligned);

            truncated = TRUNCATE(addr, boundaries[b]);
            ICUNIT_ASSERT_EQUAL((truncated & mask), 0, truncated);
            ICUNIT_ASSERT_EQUAL(truncated <= addr, 1, truncated);
        }
    }

    return LOS_OK;
}

/**
 * @ingroup TEST_MISC
 * @par TestCase_Number
 * ItLosCompiler001
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_Align/ALIGN/TRUNCATE
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Loop TEST_COUNT times, each iteration tests LOS_Align/ALIGN/TRUNCATE with boundaries 4/8/16/32 on incrementing addresses.
 * step2: Verify ALIGN result equals LOS_Align, aligned result is boundary-aligned and >= addr, truncated result is boundary-aligned and <= addr.
 * @par TestCase_Expected_Result
 * 1.LOS_Align and ALIGN return the same aligned value (round up, boundary-aligned, >= addr).
 * 2.TRUNCATE returns the truncated value (round down, boundary-aligned, <= addr).
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

VOID ItLosCompiler001(VOID)
{
    TEST_ADD_CASE("ItLosCompiler001", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL0, TEST_FUNCTION);
}
