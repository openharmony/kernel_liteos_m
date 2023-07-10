/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "xts_math.h"

LITE_TEST_SUIT(MATH, MathApi, MathApiTestSuite);

static BOOL MathApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL MathApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
* @tc.number     SUB_KERNEL_MATH_MATH_SIGNBIT_0100
* @tc.name       test signbit api
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathApiTestSuite, testsignbit, Function | MediumTest | Level1)
{
    const int testCount = 3; /* 3 common data for test, no special meaning */
    float testValues[] = {3.000001, -3.000001, 0.0}; /* 3.000001, -3.000001 common data for test, no special meaning */
    float expected[] = {0, 1, 0}; /* 1 common data for test, no special meaning */
    float ret;
    for (int i = 0; i < testCount; ++i) {
        ret = signbit(testValues[i]);
        ICUNIT_ASSERT_EQUAL(ret, expected[i], ret);
    }
    return 0;
}

RUN_TEST_SUITE(MathApiTestSuite);

void MathApiTest(void)
{
    RUN_ONE_TESTCASE(testsignbit);
}
