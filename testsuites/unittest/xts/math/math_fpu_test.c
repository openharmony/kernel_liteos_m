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

#if !defined(LOSCFG_ARCH_FPU_DISABLE)

#define MATH_FPU_EPSILON 1e-6

LITE_TEST_SUIT(MATH, MathFPU, MathFPUTestSuite);

static BOOL MathFPUTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL MathFPUTestSuiteTearDown(void)
{
    return TRUE;
}

static BOOL DoubleEquals(double a, double b)
{
    if (a == INFINITY && b == INFINITY) {
        return TRUE;
    }
    if (a == -INFINITY && b == -INFINITY) {
        return TRUE;
    }
    if (a == INFINITY && b != INFINITY) {
        return FALSE;
    }
    if (a == -INFINITY && b != -INFINITY) {
        return FALSE;
    }
    if (isnan(a) && isnan(b)) {
        return TRUE;
    }
    if (isnan(a) || isnan(b)) {
        return FALSE;
    }
    return fabs(a - b) < MATH_FPU_EPSILON;
}

/**
* @tc.number     SUB_KERNEL_MATH_SOFT_FLOAT_ARITH_0100
* @tc.name       test soft float double arithmetic operations
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathFPUTestSuite, testMathFPUArith001, Function | MediumTest | Level1)
{
    const int testCount = 3; /* 3 common data for test, no special meaning */
    double testValuesA[] = {1.5, 2.5, 3.0}; /* common data for test, no special meaning */
    double testValuesB[] = {2.5, 1.5, 4.0}; /* common data for test, no special meaning */
    double expected[] = {3.5, 3.5, 17.0}; /* 2*a*b - a - b: 2*1.5*2.5-1.5-2.5=3.5, 2*2.5*1.5-2.5-1.5=3.5, 2*3.0*4.0-3.0-4.0=17.0 */
    double ret;
    for (int i = 0; i < testCount; ++i) {
        ret = testValuesA[i] * testValuesB[i] - (testValuesA[i] + testValuesB[i]) + (testValuesA[i] * testValuesB[i]);
        /* the formula above equals: a*b - (a+b) + a*b = 2*a*b - a - b */
        ret = 2 * testValuesA[i] * testValuesB[i] - testValuesA[i] - testValuesB[i];
        ICUNIT_ASSERT_EQUAL(DoubleEquals(ret, expected[i]), TRUE, 0);
    }
    return 0;
}

/**
* @tc.number     SUB_KERNEL_MATH_SOFT_FLOAT_ARITH_0200
* @tc.name       test soft float divide operations
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathFPUTestSuite, testMathFPUArith002, Function | MediumTest | Level1)
{
    const int testCount = 3; /* 3 common data for test, no special meaning */
    double testValuesA[] = {12.0, 9.0, 16.0}; /* common data for test, no special meaning */
    double testValuesB[] = {3.0, 3.0, 4.0}; /* common data for test, no special meaning */
    double expected[] = {4.0, 3.0, 4.0}; /* common data for test, no special meaning */
    double ret;
    for (int i = 0; i < testCount; ++i) {
        ret = testValuesA[i] / testValuesB[i];
        ICUNIT_ASSERT_EQUAL(DoubleEquals(ret, expected[i]), TRUE, 0);
    }
    return 0;
}

/**
* @tc.number     SUB_KERNEL_MATH_SOFT_FLOAT_SQRT_0100
* @tc.name       test soft float sqrt operation
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathFPUTestSuite, testMathFPUSqrt001, Function | MediumTest | Level1)
{
    const int testCount = 3; /* 3 common data for test, no special meaning */
    double testValues[] = {4.0, 9.0, 16.0}; /* common data for test, no special meaning */
    double expected[] = {2.000000, 3.000000, 4.000000}; /* common data for test, no special meaning */
    double ret;
    for (int i = 0; i < testCount; ++i) {
        ret = sqrt(testValues[i]);
        ICUNIT_ASSERT_EQUAL(DoubleEquals(ret, expected[i]), TRUE, 0);
    }
    return 0;
}

/**
* @tc.number     SUB_KERNEL_MATH_SOFT_FLOAT_POW_0100
* @tc.name       test soft float pow operation
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathFPUTestSuite, testMathFPUPow001, Function | MediumTest | Level1)
{
    const int testCount = 3; /* 3 common data for test, no special meaning */
    double testValuesX[] = {2.0, 2.0, 10.0}; /* common data for test, no special meaning */
    double testValuesY[] = {3.0, 10.0, 2.0}; /* common data for test, no special meaning */
    double expected[] = {8.000000, 1024.000000, 100.000000}; /* common data for test, no special meaning */
    double ret;
    for (int i = 0; i < testCount; ++i) {
        ret = pow(testValuesX[i], testValuesY[i]);
        ICUNIT_ASSERT_EQUAL(DoubleEquals(ret, expected[i]), TRUE, 0);
    }
    return 0;
}

/**
* @tc.number     SUB_KERNEL_MATH_SOFT_FLOAT_LOG_0100
* @tc.name       test soft float log operation
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathFPUTestSuite, testMathFPULog001, Function | MediumTest | Level1)
{
    const int testCount = 2; /* 2 common data for test, no special meaning */
    double testValues[] = {2.718281828459045, 1.0}; /* common data for test, no special meaning */
    double expected[] = {1.000000, 0.000000}; /* common data for test, no special meaning */
    double ret;
    for (int i = 0; i < testCount; ++i) {
        ret = log(testValues[i]);
        ICUNIT_ASSERT_EQUAL(DoubleEquals(ret, expected[i]), TRUE, 0);
    }
    return 0;
}

/**
* @tc.number     SUB_KERNEL_MATH_SOFT_FLOAT_ROUND_0100
* @tc.name       test soft float round operation
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathFPUTestSuite, testMathFPURound001, Function | MediumTest | Level1)
{
    const int testCount = 3; /* 3 common data for test, no special meaning */
    double testValues[] = {2.5, -3.8, 0.125}; /* common data for test, no special meaning */
    double expected[] = {3.000000, -4.000000, 0.000000}; /* common data for test, no special meaning */
    double ret;
    for (int i = 0; i < testCount; ++i) {
        ret = round(testValues[i]);
        ICUNIT_ASSERT_EQUAL(DoubleEquals(ret, expected[i]), TRUE, 0);
    }
    return 0;
}

/**
* @tc.number     SUB_KERNEL_MATH_SOFT_FLOAT_FABS_0100
* @tc.name       test soft float fabs operation
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathFPUTestSuite, testMathFPUFabs001, Function | MediumTest | Level1)
{
    const int testCount = 3; /* 3 common data for test, no special meaning */
    double testValues[] = {-3.0, 0.0, 3.5}; /* common data for test, no special meaning */
    double expected[] = {3.000000, 0.000000, 3.500000}; /* common data for test, no special meaning */
    double ret;
    for (int i = 0; i < testCount; ++i) {
        ret = fabs(testValues[i]);
        ICUNIT_ASSERT_EQUAL(DoubleEquals(ret, expected[i]), TRUE, 0);
    }
    return 0;
}

/**
* @tc.number     SUB_KERNEL_MATH_SOFT_FLOAT_CMP_0100
* @tc.name       test soft float comparison operations
* @tc.desc       [C- SOFTWARE -0100]
**/
LITE_TEST_CASE(MathFPUTestSuite, testMathFPUCmp001, Function | MediumTest | Level1)
{
    float a = 1.0;
    float b = 1.000001; /* 1.000001 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL((a < b), TRUE, 0);
    ICUNIT_ASSERT_EQUAL((b > a), TRUE, 0);
    ICUNIT_ASSERT_EQUAL((a == b), FALSE, 0);

    float inf = INFINITY;
    ICUNIT_ASSERT_EQUAL((inf > 1.0), TRUE, 0);
    ICUNIT_ASSERT_EQUAL((-inf < 1.0), TRUE, 0);

    float nan = NAN;
    ICUNIT_ASSERT_EQUAL((nan == nan), FALSE, 0);
    ICUNIT_ASSERT_EQUAL((nan != nan), TRUE, 0);
    return 0;
}

RUN_TEST_SUITE(MathFPUTestSuite);

void MathFPUTest(void)
{
    RUN_ONE_TESTCASE(testMathFPUArith001);
    RUN_ONE_TESTCASE(testMathFPUArith002);
    RUN_ONE_TESTCASE(testMathFPUSqrt001);
    RUN_ONE_TESTCASE(testMathFPUPow001);
    RUN_ONE_TESTCASE(testMathFPULog001);
    RUN_ONE_TESTCASE(testMathFPURound001);
    RUN_ONE_TESTCASE(testMathFPUFabs001);
    RUN_ONE_TESTCASE(testMathFPUCmp001);
}

#endif /* !LOSCFG_ARCH_FPU_DISABLE */
