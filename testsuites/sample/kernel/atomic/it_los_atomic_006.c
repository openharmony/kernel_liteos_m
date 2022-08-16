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
    volatile INT64 value = 0;
    BOOL ret;
    UINT64 newVal = 0xff;
    UINT64 oldVal = 1;

    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 1, ret);
    ICUNIT_ASSERT_EQUAL(value, 0, value);

    oldVal = 0;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xff, value);

    newVal = 0xffff;
    oldVal = 1;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 1, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xff, value);

    oldVal = 0xff;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffff, value);

    newVal = 0xffffffff;
    oldVal = 1;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 1, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffff, value);

    oldVal = 0xffff;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffffffff, value);

    newVal = 0xffffffffffff;
    oldVal = 1;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 1, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffffffff, value);

    oldVal = 0xffffffff;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffffffffffff, value);

    newVal = 0xffffffffffffffff;
    oldVal = 1;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 1, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffffffffffff, value);

    oldVal = 0xffffffffffff;
    ret = LOS_AtomicCmpXchg64bits(&value, newVal, oldVal);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(value, 0xffffffffffffffff, value);

    return LOS_OK;
}

/**
 * @ingroup TEST_ATO
 * @par TestCase_Number
 * ItLosAtomic006
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_AtomicCmpXchg64bits
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Invoke the LOS_AtomicCmpXchg64bits interface.
 * @par TestCase_Expected_Result
 * 1.LOS_AtomicCmpXchg64bits return expected result.
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

VOID ItLosAtomic006(VOID)
{
    TEST_ADD_CASE("ItLosAtomic006", TestCase, TEST_LOS, TEST_ATO, TEST_LEVEL0, TEST_FUNCTION);
}
