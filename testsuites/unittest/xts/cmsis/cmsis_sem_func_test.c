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

#include "xts_cmsis.h"

osSemaphoreId_t  g_cmsisSemSemaph;

LITE_TEST_SUIT(Cmsis, CmsisSem, CmsisSemFuncTestSuite);

static BOOL CmsisSemFuncTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL CmsisSemFuncTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0100
 * @tc.name      : semaphore operation for creat when Semaphhore count = 1 and 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreNew001, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    (void)osSemaphoreDelete(g_cmsisSemSemaph);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0200
 * @tc.name      : semaphore operation for creat when Semaphhore count = 10 and 1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreNew002, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT10, SEMAPHHORE_COUNT_INT1, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    (void)osSemaphoreDelete(g_cmsisSemSemaph);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0300
 * @tc.name      : semaphore operation for creat when Semaphhore count = 0 and 10
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreNew003, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT0, SEMAPHHORE_COUNT_INT10, NULL);
    ICUNIT_ASSERT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    (void)osSemaphoreDelete(g_cmsisSemSemaph);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0400
 * @tc.name      : semaphore operation for creat when Semaphhore count = 0 and 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreNew004, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT0, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    (void)osSemaphoreDelete(g_cmsisSemSemaph);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0500
 * @tc.name      : semaphore operation for creat when Semaphhore count = 1 and 1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreNew005, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT1, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    (void)osSemaphoreDelete(g_cmsisSemSemaph);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0600
 * @tc.name      : semaphore operation for creat when Semaphhore count = 10 and 10
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreNew006, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT10, SEMAPHHORE_COUNT_INT10, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    (void)osSemaphoreDelete(g_cmsisSemSemaph);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0700
 * @tc.name      : semaphore operation for creat when Semaphhore count = 0xFE and 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreNew007, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_HEX_MAX, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    (void)osSemaphoreDelete(g_cmsisSemSemaph);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0800
 * @tc.name      : semaphore operation for creat when Semaphhore count = 0 and 0xFE
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreNew008, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT0, SEMAPHHORE_COUNT_HEX_MAX, NULL);
    ICUNIT_ASSERT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    (void)osSemaphoreDelete(g_cmsisSemSemaph);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_0900
 * @tc.name      : semaphore operation for delete
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreDelete001, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT10, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1000
 * @tc.name      : semaphore delete operation twice
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreDelete002, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT10, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);

    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1100
 * @tc.name      : semaphore delete operation with semaphore_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreDelete003, Function | MediumTest | Level1)
{
    osStatus_t ret = osSemaphoreDelete(NULL);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1200
 * @tc.name      : semaphore operation for acquire when Semaphhore count = 1 and 1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreAcquire001, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT1, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    ret = osSemaphoreAcquire(g_cmsisSemSemaph, 0);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1300
 * @tc.name      : semaphore operation for acquire when Semaphhore count = 1 and 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreAcquire002, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    ret = osSemaphoreAcquire(g_cmsisSemSemaph, 0);
    ICUNIT_ASSERT_EQUAL(ret, osErrorResource, ret);

    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1400
 * @tc.name      : semaphore operation for acquire when Semaphhore count = 0 and 1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreAcquire003, Function | MediumTest | Level1)
{
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT0, SEMAPHHORE_COUNT_INT1, NULL);
    ICUNIT_ASSERT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1500
 * @tc.name      : semaphore acquire operation with semaphore_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreAcquire004, Function | MediumTest | Level1)
{
    osStatus_t ret = osSemaphoreAcquire(NULL, 0);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1600
 * @tc.name      : semaphore operation for release
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreRelease001, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);

    ret = osSemaphoreAcquire(g_cmsisSemSemaph, 0);
    ICUNIT_ASSERT_EQUAL(ret, osErrorResource, ret);

    ret = osSemaphoreRelease(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1700
 * @tc.name      : semaphore release operation twice
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreRelease002, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);

    ret = osSemaphoreRelease(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ret = osSemaphoreRelease(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osErrorResource, ret);

    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1800
 * @tc.name      : semaphore operation for release after semaphore acquire
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreRelease003, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT1, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);

    ret = osSemaphoreRelease(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osErrorResource, ret);

    ret = osSemaphoreAcquire(g_cmsisSemSemaph, 0);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osSemaphoreRelease(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ret = osSemaphoreRelease(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osErrorResource, ret);

    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_1900
 * @tc.name      : semaphore release operation with semaphore_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreRelease004, Function | MediumTest | Level1)
{
    osStatus_t ret = osSemaphoreRelease(NULL);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_2000
 * @tc.name      : semaphore operation for get count when Semaphhore count = 1 or 0xFE
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreGetCount001, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT1, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    ret = osSemaphoreGetCount(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, 1, ret); /* 1, common data for test, no special meaning */
    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_HEX_MAX, SEMAPHHORE_COUNT_HEX_MAX, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    ret = osSemaphoreGetCount(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, SEMAPHHORE_COUNT_HEX_MAX, ret);
    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_2100
 * @tc.name      : semaphore operation for get count when Semaphhore count = 1 or 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreGetCount002, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT1, SEMAPHHORE_COUNT_INT0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    ret = osSemaphoreGetCount(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_INT0, SEMAPHHORE_COUNT_INT1, NULL);
    ICUNIT_ASSERT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_2200
 * @tc.name      : semaphore operation for get count
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreGetCount003, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisSemSemaph = osSemaphoreNew(SEMAPHHORE_COUNT_HEX_MAX, SEMAPHHORE_COUNT_HEX_MAX, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisSemSemaph, NULL, g_cmsisSemSemaph);
    ret = osSemaphoreAcquire(g_cmsisSemSemaph, osWaitForever);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osSemaphoreGetCount(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, SEMAPHHORE_COUNT_HEX_MAX - 1, ret); /* 1, common data for test, no special meaning */
    ret = osSemaphoreDelete(g_cmsisSemSemaph);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_SEM_OPERATION_2300
 * @tc.name      : semaphore get count operation with semaphore_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisSemFuncTestSuite, testOsSemaphoreGetCount004, Function | MediumTest | Level1)
{
    osStatus_t ret = osSemaphoreGetCount(NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
};

RUN_TEST_SUITE(CmsisSemFuncTestSuite);

void CmsisSemFuncTest(void)
{
    RUN_ONE_TESTCASE(testOsSemaphoreNew001);
    RUN_ONE_TESTCASE(testOsSemaphoreNew002);
    RUN_ONE_TESTCASE(testOsSemaphoreNew003);
    RUN_ONE_TESTCASE(testOsSemaphoreNew004);
    RUN_ONE_TESTCASE(testOsSemaphoreNew005);
    RUN_ONE_TESTCASE(testOsSemaphoreNew006);
    RUN_ONE_TESTCASE(testOsSemaphoreNew007);
    RUN_ONE_TESTCASE(testOsSemaphoreNew008);
    RUN_ONE_TESTCASE(testOsSemaphoreDelete001);
    RUN_ONE_TESTCASE(testOsSemaphoreDelete002);
    RUN_ONE_TESTCASE(testOsSemaphoreDelete003);
    RUN_ONE_TESTCASE(testOsSemaphoreAcquire001);
    RUN_ONE_TESTCASE(testOsSemaphoreAcquire002);
    RUN_ONE_TESTCASE(testOsSemaphoreAcquire003);
    RUN_ONE_TESTCASE(testOsSemaphoreAcquire004);
    RUN_ONE_TESTCASE(testOsSemaphoreRelease001);
    RUN_ONE_TESTCASE(testOsSemaphoreRelease002);
    RUN_ONE_TESTCASE(testOsSemaphoreRelease003);
    RUN_ONE_TESTCASE(testOsSemaphoreRelease004);
    RUN_ONE_TESTCASE(testOsSemaphoreGetCount001);
    RUN_ONE_TESTCASE(testOsSemaphoreGetCount002);
    RUN_ONE_TESTCASE(testOsSemaphoreGetCount003);
    RUN_ONE_TESTCASE(testOsSemaphoreGetCount004);
}
