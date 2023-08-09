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

osEventFlagsId_t g_eventId;
UINT16 g_cmsisTestEventCount;

LITE_TEST_SUIT(Cmsis, Cmsisevent, CmsisEventFuncTestSuite);

static BOOL CmsisEventFuncTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL CmsisEventFuncTestSuiteTearDown(void)
{
    return TRUE;
}

static void CmsisEventFlagsWaitFunc001(void const *argument)
{
    (void)argument;
    g_cmsisTestEventCount++;
    UINT32 ret = osEventFlagsWait(g_eventId, EVENT_MASK_HEX_11, (osFlagsWaitAll | osFlagsNoClear), osWaitForever);
    ICUNIT_ASSERT_EQUAL_VOID(ret, EVENT_MASK_HEX_11, ret);
    g_cmsisTestEventCount++;
    osThreadExit();
}

static void CmsisEventFlagsSetFunc002(void const *argument)
{
    (void)argument;
    UINT32 ret;
    g_cmsisTestEventCount++;
    ret = osEventFlagsWait(g_eventId, EVENT_MASK_HEX_11, (osFlagsWaitAll | osFlagsNoClear), TIMEOUT_NUM_3);
    ICUNIT_ASSERT_EQUAL_VOID(ret, osErrorTimeout, ret);
    g_cmsisTestEventCount++;

    ret = osEventFlagsWait(g_eventId, EVENT_MASK_HEX_4, (osFlagsWaitAll | osFlagsNoClear), osWaitForever);
    ICUNIT_ASSERT_EQUAL_VOID(ret, TESTCOUNT_NUM_4, ret);
    g_cmsisTestEventCount++;
    osThreadExit();
}

static void CmsisEventFlagsClearFunc001(void const *argument)
{
    (void)argument;
    g_cmsisTestEventCount++;
    UINT32 ret = osEventFlagsWait(g_eventId, EVENT_MASK_HEX_1, osFlagsWaitAll, osWaitForever);
    ICUNIT_ASSERT_EQUAL_VOID(ret, EVENT_MASK_HEX_1, ret);
    g_cmsisTestEventCount++;
    osThreadExit();
}

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0100
 * @tc.name      : event operation for creat
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsNew001, Function | MediumTest | Level1)
{
    g_eventId = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_eventId, NULL, g_eventId);
    (void)osEventFlagsDelete(g_eventId);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0200
 * @tc.name      : event operation for delete
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsDelete001, Function | MediumTest | Level1)
{
    UINT32 ret;
    g_eventId = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_eventId, NULL, g_eventId);
    ret = osEventFlagsDelete(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0300
 * @tc.name      : event delete operation with EventFlagsId = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsDelete002, Function | MediumTest | Level1)
{
    UINT32 ret = osEventFlagsDelete(NULL);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0400
 * @tc.name      : event operation for flags set
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsSet001, Function | MediumTest | Level1)
{
    UINT32 ret;
    g_eventId = osEventFlagsNew(NULL);
    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_10);
    ICUNIT_ASSERT_EQUAL(ret, EVENT_MASK_HEX_10, ret);
    ret = osEventFlagsDelete(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0500
 * @tc.name      : event reliability test for flags set
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsSet002, Function | MediumTest | Level1)
{
    UINT32 ret;
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;

    g_cmsisTestEventCount = 0;
    g_eventId = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_eventId, NULL, g_eventId);
    id = osThreadNew((osThreadFunc_t)CmsisEventFlagsSetFunc002, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestEventCount, EVENT_MASK_HEX_1, g_cmsisTestEventCount);
    g_cmsisTestEventCount++;
    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_2);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestEventCount, TESTCOUNT_NUM_3, g_cmsisTestEventCount);

    g_cmsisTestEventCount++;
    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_11);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    ICUNIT_ASSERT_EQUAL(g_cmsisTestEventCount, TESTCOUNT_NUM_4, g_cmsisTestEventCount);
    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_4);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestEventCount, TESTCOUNT_NUM_5, g_cmsisTestEventCount);
    ret = osEventFlagsDelete(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0600
 * @tc.name      : event flags set operation with EventFlagsId = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsSet003, Function | MediumTest | Level1)
{
    UINT32 ret = osEventFlagsSet(NULL, EVENT_MASK_HEX_10);
    ICUNIT_ASSERT_EQUAL(ret, osFlagsErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0700
 * @tc.name      : event operation for wait
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsWait001, Function | MediumTest | Level1)
{
    UINT32 ret;
    osThreadId_t id;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;

    g_cmsisTestEventCount = 0;
    g_eventId = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_eventId, NULL, g_eventId);
    id = osThreadNew((osThreadFunc_t)CmsisEventFlagsWaitFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_10);
    ICUNIT_ASSERT_EQUAL(ret, EVENT_MASK_HEX_10, ret);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestEventCount, EVENT_MASK_HEX_1, g_cmsisTestEventCount);

    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_1);
    ICUNIT_ASSERT_EQUAL(ret, EVENT_MASK_HEX_11, ret);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestEventCount, TESTCOUNT_NUM_2, g_cmsisTestEventCount);

    ret = osEventFlagsDelete(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0800
 * @tc.name      : event operation for invalid option
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsWait002, Function | MediumTest | Level1)
{
    UINT32 ret;
    g_eventId = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_eventId, NULL, g_eventId);

    ret = osEventFlagsWait(g_eventId, EVENT_MASK_HEX_11, INVALID_FLAG_OPTION, osWaitForever);
    ICUNIT_ASSERT_EQUAL(ret, osFlagsErrorParameter, ret);

    ret = osEventFlagsDelete(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_0900
 * @tc.name      : event wait operation with EventFlagsId = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsWait003, Function | MediumTest | Level1)
{
    UINT32 ret = osEventFlagsWait(NULL, EVENT_MASK_HEX_11, (osFlagsWaitAll | osFlagsNoClear), osWaitForever);
    ICUNIT_ASSERT_EQUAL(ret, osFlagsErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_1000
 * @tc.name      : event operation for flags get
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsGet001, Function | MediumTest | Level1)
{
    UINT32 ret;
    g_eventId = osEventFlagsNew(NULL);
    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_10);
    ICUNIT_ASSERT_EQUAL(ret, EVENT_MASK_HEX_10, ret);
    ret = osEventFlagsGet(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, EVENT_MASK_HEX_10, ret);

    ret = osEventFlagsDelete(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_1100
 * @tc.name      : event flags get operation with EventFlagsId = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsGet002, Function | MediumTest | Level1)
{
    UINT32 ret = osEventFlagsGet(NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_1200
 * @tc.name      : event operation for flags clear
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsClear001, Function | MediumTest | Level1)
{
    UINT32 ret;
    osThreadId_t id;

    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;

    g_cmsisTestEventCount = 0;
    g_eventId = osEventFlagsNew(NULL);
    id = osThreadNew((osThreadFunc_t)CmsisEventFlagsClearFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_10);
    ICUNIT_ASSERT_EQUAL(ret, EVENT_MASK_HEX_10, ret);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestEventCount, EVENT_MASK_HEX_1, g_cmsisTestEventCount);

    ret = osEventFlagsClear(g_eventId, 0xffff);
    ICUNIT_ASSERT_EQUAL(ret, EVENT_MASK_HEX_10, ret);
    ret = osEventFlagsGet(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = osEventFlagsSet(g_eventId, EVENT_MASK_HEX_1);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestEventCount, TESTCOUNT_NUM_2, g_cmsisTestEventCount);
    ret = osEventFlagsDelete(g_eventId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_EVENT_OPERATION_1300
 * @tc.name      : event flags clear operation with EventFlagsId = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisEventFuncTestSuite, testOsEventFlagsClear002, Function | MediumTest | Level1)
{
    UINT32 ret = osEventFlagsClear(NULL, 0xffff);
    ICUNIT_ASSERT_EQUAL(ret, osFlagsErrorParameter, ret);
    return 0;
};

RUN_TEST_SUITE(CmsisEventFuncTestSuite);

void CmsisEventFuncTest(void)
{
    RUN_ONE_TESTCASE(testOsEventFlagsNew001);
    RUN_ONE_TESTCASE(testOsEventFlagsDelete001);
    RUN_ONE_TESTCASE(testOsEventFlagsDelete002);
    RUN_ONE_TESTCASE(testOsEventFlagsSet001);
    RUN_ONE_TESTCASE(testOsEventFlagsSet002);
    RUN_ONE_TESTCASE(testOsEventFlagsSet003);
    RUN_ONE_TESTCASE(testOsEventFlagsWait001);
    RUN_ONE_TESTCASE(testOsEventFlagsWait002);
    RUN_ONE_TESTCASE(testOsEventFlagsWait003);
    RUN_ONE_TESTCASE(testOsEventFlagsGet001);
    RUN_ONE_TESTCASE(testOsEventFlagsGet002);
    RUN_ONE_TESTCASE(testOsEventFlagsClear001);
    RUN_ONE_TESTCASE(testOsEventFlagsClear002);
}
