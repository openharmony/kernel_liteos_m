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

UINT16 g_cmsisTestTimeCount;

/**
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is CmsisTaskFuncTestSuite
 */
LITE_TEST_SUIT(Cmsis, Cmsistimer, CmsisTimerFuncTestSuite);

/**
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL CmsisTimerFuncTestSuiteSetUp(void)
{
    return TRUE;
}

/**
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL CmsisTimerFuncTestSuiteTearDown(void)
{
    return TRUE;
}

static void CmsisTimerFunc001(void const *argument)
{
    (void)argument;
    return;
}

static void CmsisTimerFunc002(void const *argument)
{
    (void)argument;
    g_cmsisTestTimeCount++;
    return;
}

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0100
 * @tc.name      : timer operation for creat with parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerNew001, Function | MediumTest | Level1)
{
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    (void)osTimerDelete(id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0200
 * @tc.name      : timer operation for creat with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerNew002, Function | MediumTest | Level1)
{
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    (void)osTimerDelete(id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0300
 * @tc.name      : timer creat operation with NULL func and parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerNew003, Function | MediumTest | Level1)
{
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew(NULL, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0400
 * @tc.name      : timer creat operation with NULL func and parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerNew004, Function | MediumTest | Level1)
{
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew(NULL, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0500
 * @tc.name      : timer operation for invalid timer type
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerNew005, Function | MediumTest | Level1)
{
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, INVALID_TIMER_TYPE, &value, NULL);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0600
 * @tc.name      : timer operation for start with parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStart001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT4;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0700
 * @tc.name      : timer operation for start with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStart002, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT4;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0800
 * @tc.name      : timer operation for start with callback func and parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStart003, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT4;
    g_cmsisTestTimeCount = 0;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc002, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);

    status = osDelay(DELAY_TICKS_10);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(1, g_cmsisTestTimeCount, g_cmsisTestTimeCount); /* 1, common data for test, no special meaning */
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_0900
 * @tc.name      : timer operation for start with callback func and parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStart004, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT4;
    g_cmsisTestTimeCount = 0;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc002, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);

    status = osDelay(DELAY_TICKS_10);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(TIMER_PERIODIC_COUNT, g_cmsisTestTimeCount, g_cmsisTestTimeCount);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1000
 * @tc.name      : timer start operation with ticks = 0 and parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStart005, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    uwRet = osTimerStart(id, 0);
    ICUNIT_ASSERT_EQUAL(osErrorParameter, uwRet, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1100
 * @tc.name      : timer start operation with ticks = 0 and parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStart006, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    uwRet = osTimerStart(id, 0);
    ICUNIT_ASSERT_EQUAL(osErrorParameter, uwRet, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1200
 * @tc.name      : timer start operation with timer_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStart007, Function | MediumTest | Level1)
{
    UINT32 millisec = MILLISEC_NUM_INT4;
    UINT32 uwRet = osTimerStart(NULL, millisec);
    ICUNIT_ASSERT_EQUAL(osErrorParameter, uwRet, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1300
 * @tc.name      : timer operation for delete with parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerDelete001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    status = osDelay(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    uwRet = osTimerDelete(id);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1400
 * @tc.name      : timer operation for delete with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerDelete002, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    status = osDelay(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    uwRet = osTimerDelete(id);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1500
 * @tc.name      : timer operation for delete after osTimerStart with parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerDelete003, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    osTimerStart(id, millisec);
    status = osDelay(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    uwRet = osTimerDelete(id);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1600
 * @tc.name      : timer operation for delete after osTimerStart with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerDelete004, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    status = osDelay(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    uwRet = osTimerDelete(id);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1700
 * @tc.name      : timer delete operation with timer_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerDelete005, Function | MediumTest | Level1)
{
    UINT32 uwRet = osTimerDelete(NULL);
    ICUNIT_ASSERT_EQUAL(osErrorParameter, uwRet, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1800
 * @tc.name      : timer operation for stop after osTimerStart with parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStop001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    status = osDelay(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    uwRet = osTimerStop(id);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_1900
 * @tc.name      : timer operation for stop after osTimerStart with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStop002, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    status = osDelay(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    uwRet = osTimerStop(id);
    ICUNIT_ASSERT_EQUAL(osOK, uwRet, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2000
 * @tc.name      : timer operation for stop with parameter osTimerOnce
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStop003, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    status = osDelay(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    uwRet = osTimerStop(id);
    ICUNIT_ASSERT_EQUAL(osErrorResource, uwRet, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2100
 * @tc.name      : timer operation for stop with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStop004, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    osStatus_t status;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    status = osDelay(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    uwRet = osTimerStop(id);
    ICUNIT_ASSERT_EQUAL(osErrorResource, uwRet, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2200
 * @tc.name      : timer stop operation with timer_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerStop005, Function | MediumTest | Level1)
{
    UINT32 uwRet = osTimerStop(NULL);
    ICUNIT_ASSERT_EQUAL(osErrorParameter, uwRet, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2300
 * @tc.name      : timer operation for running
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerIsRunning(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2400
 * @tc.name      : timer operation for running after osTimerStart
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning002, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    uwRet = osTimerIsRunning(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 1, uwRet); /* 1, common data for test, no special meaning */
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2500
 * @tc.name      : timer operation for running after osTimerStart and osTimerStop
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning003, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    uwRet = osTimerStop(id);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osTimerIsRunning(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2600
 * @tc.name      : timer operation for running after osTimerStart and osTimerDelete
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning004, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerOnce, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    uwRet = osTimerDelete(id);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    uwRet = osTimerIsRunning(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2700
 * @tc.name      : timer operation for running with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning005, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerIsRunning(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2800
 * @tc.name      : timer operation for running after osTimerStart with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning006, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    uwRet = osTimerIsRunning(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 1, uwRet); /* 1, common data for test, no special meaning */
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_2900
 * @tc.name      : timer operation for running after osTimerStart and osTimerStop with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning007, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    uwRet = osTimerStop(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    uwRet = osTimerIsRunning(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    (void)osTimerDelete(id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_3000
 * @tc.name      : timer operation for running after osTimerStart and osTimerDelete with parameter osTimerPeriodic
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning008, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osTimerId_t id;
    UINT32 value = 0xffff;
    UINT32 millisec = MILLISEC_NUM_INT10;
    id = osTimerNew((osTimerFunc_t)CmsisTimerFunc001, osTimerPeriodic, &value, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    uwRet = osTimerStart(id, millisec);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    uwRet = osTimerDelete(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    uwRet = osTimerIsRunning(id);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_3100
 * @tc.name      : timer running inquiry with timer_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsTimerIsRunning009, Function | MediumTest | Level1)
{
    UINT32 uwRet = osTimerIsRunning(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_3200
 * @tc.name      : os operation for get tick freq
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsKernelGetTickFreq001, Function | MediumTest | Level1)
{
    UINT32 uwRet = osKernelGetTickFreq();
    ICUNIT_ASSERT_EQUAL(uwRet, LOSCFG_BASE_CORE_TICK_PER_SECOND, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_3300
 * @tc.name      : os operation for get sys time freq
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsKernelGetSysTimerFreq001, Function | MediumTest | Level1)
{
    UINT32 uwRet = osKernelGetSysTimerFreq();
    ICUNIT_ASSERT_EQUAL(uwRet, OS_SYS_CLOCK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TIMER_OPERATION_3400
 * @tc.name      : os operation for get sys time count
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTimerFuncTestSuite, testOsKernelGetSysTimerCount001, Function | MediumTest | Level1)
{
    UINT32 uwRet = osKernelGetSysTimerCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(uwRet, 0, UINT_MAX, uwRet);
    return 0;
};

RUN_TEST_SUITE(CmsisTimerFuncTestSuite);

void CmsisTimerFuncTest(void)
{
    RUN_ONE_TESTCASE(testOsTimerNew001);
    RUN_ONE_TESTCASE(testOsTimerNew002);
    RUN_ONE_TESTCASE(testOsTimerNew003);
    RUN_ONE_TESTCASE(testOsTimerNew004);
    RUN_ONE_TESTCASE(testOsTimerNew005);
    RUN_ONE_TESTCASE(testOsTimerStart001);
    RUN_ONE_TESTCASE(testOsTimerStart002);
    RUN_ONE_TESTCASE(testOsTimerStart003);
    RUN_ONE_TESTCASE(testOsTimerStart004);
    RUN_ONE_TESTCASE(testOsTimerStart005);
    RUN_ONE_TESTCASE(testOsTimerStart006);
    RUN_ONE_TESTCASE(testOsTimerStart007);
    RUN_ONE_TESTCASE(testOsTimerDelete001);
    RUN_ONE_TESTCASE(testOsTimerDelete002);
    RUN_ONE_TESTCASE(testOsTimerDelete003);
    RUN_ONE_TESTCASE(testOsTimerDelete004);
    RUN_ONE_TESTCASE(testOsTimerDelete005);
    RUN_ONE_TESTCASE(testOsTimerStop001);
    RUN_ONE_TESTCASE(testOsTimerStop002);
    RUN_ONE_TESTCASE(testOsTimerStop003);
    RUN_ONE_TESTCASE(testOsTimerStop004);
    RUN_ONE_TESTCASE(testOsTimerStop005);
    RUN_ONE_TESTCASE(testOsTimerIsRunning001);
    RUN_ONE_TESTCASE(testOsTimerIsRunning002);
    RUN_ONE_TESTCASE(testOsTimerIsRunning003);
    RUN_ONE_TESTCASE(testOsTimerIsRunning004);
    RUN_ONE_TESTCASE(testOsTimerIsRunning005);
    RUN_ONE_TESTCASE(testOsTimerIsRunning006);
    RUN_ONE_TESTCASE(testOsTimerIsRunning007);
    RUN_ONE_TESTCASE(testOsTimerIsRunning008);
    RUN_ONE_TESTCASE(testOsTimerIsRunning009);
    RUN_ONE_TESTCASE(testOsKernelGetTickFreq001);
    RUN_ONE_TESTCASE(testOsKernelGetSysTimerFreq001);
    RUN_ONE_TESTCASE(testOsKernelGetSysTimerCount001);
}