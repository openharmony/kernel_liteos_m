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
    TestWidth tw;
    TestWidth tmp;
    UINT64 value;
    UINT32 count;

    for (count = 0; count < TEST_COUNT; count++) {
        value = (UINT64)count;

        /* 8-bit: WRITE8_MB(addr, value) + READ8_MB(addr) — with dsb() barrier */
        WRITE8_MB(&tw, (UINT8)value);
        tmp.data8 = READ8_MB(&tw);
        ICUNIT_ASSERT_EQUAL(tmp.data8, (UINT8)value, tmp.data8);

        /* 16-bit: make value span beyond 8 bits */
        value = (value + 1) | (value << SHIFT_WIDTH8);
        WRITE16_MB(&tw, (UINT16)value);
        tmp.data16 = READ16_MB(&tw);
        ICUNIT_ASSERT_EQUAL(tmp.data16, (UINT16)value, tmp.data16);

        /* 32-bit: make value span beyond 16 bits */
        value = (value + 1) | (value << SHIFT_WIDTH16);
        WRITE32_MB(&tw, (UINT32)value);
        tmp.data32 = READ32_MB(&tw);
        ICUNIT_ASSERT_EQUAL(tmp.data32, (UINT32)value, tmp.data32);

        /* 64-bit: make value span beyond 32 bits */
        value = (value + 1) | (value << SHIFT_WIDTH32);
        WRITE64_MB(&tw, (UINT64)value);
        tmp.data64 = READ64_MB(&tw);
        ICUNIT_ASSERT_EQUAL(tmp.data64, (UINT64)value, tmp.data64);
    }

    return LOS_OK;
}

/**
 * @ingroup TEST_MISC
 * @par TestCase_Number
 * ItLosIo002
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface WRITE8_MB/READ8_MB/WRITE16_MB/READ16_MB/WRITE32_MB/READ32_MB/WRITE64_MB/READ64_MB
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Loop TEST_COUNT times, each iteration writes an incrementing value via WRITE8_MB/16/32/64_MB into a TestWidth union (addr = &tw).
 * step2: Read back via READ8_MB/16/32/64_MB and compare. 16/32/64-bit values are shifted to span beyond the previous width boundary.
 * @par TestCase_Expected_Result
 * 1.READ8_MB/16/32/64_MB return the values written by WRITE8_MB/16/32/64_MB without crash (dsb() barrier applied) across all iterations.
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

VOID ItLosIo002(VOID)
{
    TEST_ADD_CASE("ItLosIo002", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL0, TEST_FUNCTION);
}
