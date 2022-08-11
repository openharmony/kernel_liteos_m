/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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


#include "it_los_atomic.h"

static UINT32 TestCase(VOID)
{
    volatile INT32 value = 0;
    UINT32 ret;
    UINT32 newVal;

    newVal = 0xff;
    ret = LOS_AtomicXchg32bits(&value, newVal);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xff, value);

    newVal = 0xffff;
    ret = LOS_AtomicXchg32bits(&value, newVal);
    ICUNIT_ASSERT_EQUAL(ret, 0xff, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffff, value);

    newVal = 0xffffff;
    ret = LOS_AtomicXchg32bits(&value, newVal);
    ICUNIT_ASSERT_EQUAL(ret, 0xffff, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffffff, value);

    newVal = 0xffffffff;
    ret = LOS_AtomicXchg32bits(&value, newVal);
    ICUNIT_ASSERT_EQUAL(ret, 0xffffff, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffffffff, value);

    return LOS_OK;
}

/**
 * @ingroup TEST_ATO
 * @par TestCase_Number
 * ItLosAtomic002
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_AtomicXchg32bits
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Invoke the LOS_AtomicXchg32bits interface.
 * @par TestCase_Expected_Result
 * 1.LOS_AtomicXchg32bits return expected result.
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

VOID ItLosAtomic002(VOID)
{
    TEST_ADD_CASE("ItLosAtomic002", TestCase, TEST_LOS, TEST_ATO, TEST_LEVEL0, TEST_FUNCTION);
}
