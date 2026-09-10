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

        /* 8-bit: WRITE_UINT8 + READ_UINT8 + GET_UINT8 — Layer 3 → Layer 2(_MB) → Layer 1 */
        WRITE_UINT8((UINT8)value, &tw);
        READ_UINT8(tmp.data8, &tw);
        ICUNIT_ASSERT_EQUAL(tmp.data8, (UINT8)value, tmp.data8);
        ICUNIT_ASSERT_EQUAL(GET_UINT8(&tw), (UINT8)value, GET_UINT8(&tw));

        /* 16-bit: make value span beyond 8 bits */
        value = (value + 1) | (value << SHIFT_WIDTH8);
        WRITE_UINT16((UINT16)value, &tw);
        READ_UINT16(tmp.data16, &tw);
        ICUNIT_ASSERT_EQUAL(tmp.data16, (UINT16)value, tmp.data16);
        ICUNIT_ASSERT_EQUAL(GET_UINT16(&tw), (UINT16)value, GET_UINT16(&tw));

        /* 32-bit: make value span beyond 16 bits */
        value = (value + 1) | (value << SHIFT_WIDTH16);
        WRITE_UINT32((UINT32)value, &tw);
        READ_UINT32(tmp.data32, &tw);
        ICUNIT_ASSERT_EQUAL(tmp.data32, (UINT32)value, tmp.data32);
        ICUNIT_ASSERT_EQUAL(GET_UINT32(&tw), (UINT32)value, GET_UINT32(&tw));

        /* 64-bit: make value span beyond 32 bits */
        value = (value + 1) | (value << SHIFT_WIDTH32);
        WRITE_UINT64((UINT64)value, &tw);
        READ_UINT64(tmp.data64, &tw);
        ICUNIT_ASSERT_EQUAL(tmp.data64, (UINT64)value, tmp.data64);
        ICUNIT_ASSERT_EQUAL(GET_UINT64(&tw), (UINT64)value, GET_UINT64(&tw));
    }

    return LOS_OK;
}

/**
 * @ingroup TEST_MISC
 * @par TestCase_Number
 * ItLosIo003
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface WRITE_UINT8/READ_UINT8/GET_UINT8/WRITE_UINT16/READ_UINT16/GET_UINT16/WRITE_UINT32/READ_UINT32/GET_UINT32/WRITE_UINT64/READ_UINT64/GET_UINT64
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Loop TEST_COUNT times, each iteration writes an incrementing value via WRITE_UINT8/16/32/64 into a TestWidth union (addr = &tw).
 * step2: Read back via READ_UINT8/16/32/64 and compare.
 * step3: Re-verify via GET_UINT8/16/32/64 (expression context) and compare. 16/32/64-bit values are shifted to span beyond the previous width boundary.
 * @par TestCase_Expected_Result
 * 1.READ_UINT8/16/32/64 return the values written by WRITE_UINT8/16/32/64 via the full Layer3→Layer2(_MB)→Layer1 path.
 * 2.GET_UINT8/16/32/64 return the same values and are usable as expressions.
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

VOID ItLosIo003(VOID)
{
    TEST_ADD_CASE("ItLosIo003", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL0, TEST_FUNCTION);
}
