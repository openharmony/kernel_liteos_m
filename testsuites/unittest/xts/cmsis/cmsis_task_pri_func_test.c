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

osThreadId_t g_priTaskID01;
osPriority_t g_setPriority;
UINT16 g_threadCreateExit2;
UINT16 g_threadCreateExit3;

LITE_TEST_SUIT(Cmsis, Cmsistask, CmsisTaskPriFuncTestSuite);

static BOOL CmsisTaskPriFuncTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL CmsisTaskPriFuncTestSuiteTearDown(void)
{
    return TRUE;
}

static void WaitThreadExit(osThreadId_t id, UINT16 const *exitFlag)
{
    osStatus_t status;
    (void)osThreadSetPriority(id, PRIORITY_COUNT_MIN_1);
    while (*exitFlag != TESTCOUNT_NUM_1) {
        status = osDelay(DELAY_TICKS_5);
        ICUNIT_ASSERT_EQUAL(status, osOK, status);
    }
}
static void CmsisThreadGetPriorityFunc001(void const *argument)
{
    (void)argument;
    osThreadAttr_t attr;
    
    g_priTaskID01 = osThreadGetId();
    attr.priority = osThreadGetPriority(g_priTaskID01);
    ICUNIT_ASSERT_EQUAL(attr.priority, g_setPriority, attr.priority);
    g_threadCreateExit2 = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadSetPriorityFunc001(void const *argument)
{
    (void)argument;
    osThreadAttr_t attr;
    UINT32 uwRet;
    g_priTaskID01 = osThreadGetId();
    uwRet = osThreadSetPriority(g_priTaskID01, g_setPriority);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    attr.priority = osThreadGetPriority(g_priTaskID01);
    ICUNIT_ASSERT_EQUAL(attr.priority, g_setPriority, attr.priority);
    g_threadCreateExit2 = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadSetPriorityFunc002(void const *argument)
{
    (void)argument;
    UINT32 uwRet;
    g_priTaskID01 = osThreadGetId();
    uwRet = osThreadSetPriority(g_priTaskID01, g_setPriority);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    g_threadCreateExit2 = TESTCOUNT_NUM_1;
    osThreadExit();
}

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5120
 * @tc.name      : thread operation for get priority when Priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority001, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_setPriority = osPriorityLow1;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_setPriority;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5160
 * @tc.name      : thread operation for get priority input exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority002, Function | MediumTest | Level1)
{
    UINT32 uwRet = osThreadGetPriority(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, osPriorityError, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5200
 * @tc.name      : thread operation for get current priority
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority003, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    g_priTaskID01 = osThreadGetId();
    uwRet = osThreadGetPriority(g_priTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osPriorityLow1, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5240
 * @tc.name      : thread operation for get priority when Priority = osPriorityLow7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority004, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_setPriority = osPriorityLow7;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_setPriority;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5280
 * @tc.name      : thread operation for get priority when Priority = osPriorityBelowNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority005, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_setPriority = osPriorityBelowNormal;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_setPriority;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5320
 * @tc.name      : thread operation for get priority when Priority = osPriorityBelowNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority006, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_setPriority = osPriorityBelowNormal7;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_setPriority;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5360
 * @tc.name      : thread operation for get priority when Priority = osPriorityNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority007, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_setPriority = osPriorityNormal;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_setPriority;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5400
 * @tc.name      : thread operation for get priority when Priority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority008, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    g_setPriority = osPriorityNormal7;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_setPriority;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetPriorityFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5440
 * @tc.name      : thread operation for get priority when Priority = osPriorityAboveNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority009, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    g_setPriority = osPriorityAboveNormal;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_setPriority;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetPriorityFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5480
 * @tc.name      : thread operation for get priority when Priority = osPriorityAboveNormal1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadGetPriority010, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    g_setPriority = osPriorityAboveNormal1;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_setPriority;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetPriorityFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5520
 * @tc.name      : thread operation for set priority input1 exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority001, Function | MediumTest | Level1)
{
    UINT32 uwRet = osThreadSetPriority(NULL, osPriorityNormal);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5560
 * @tc.name      : thread operation for set priority input2 exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority002, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    g_priTaskID01 =  osThreadGetId();
    uwRet = osThreadSetPriority(g_priTaskID01, osPriorityNone);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5600
 * @tc.name      : set invalid priority when curPriority = osPriorityLow1 and setPriority = osPriorityNone
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority003, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow1;

    g_setPriority = osPriorityNone;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityIdle;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = PRIORITY_COUNT_NOT_MIN;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    
    g_setPriority = osPriorityAboveNormal7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityHigh;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityHigh7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5720
 * @tc.name      : set priority when curPriority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority004, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = PRIORITY_COUNT_MIN_1;
    g_setPriority = PRIORITY_COUNT_MIN_1;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);

    g_setPriority = osPriorityLow7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);

    g_setPriority = osPriorityBelowNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityBelowNormal7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5720
 * @tc.name      : set priority when curPriority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority016, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = PRIORITY_COUNT_MIN_1;

    g_setPriority = osPriorityNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityNormal7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityAboveNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityAboveNormal1;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_6040
 * @tc.name      : set invalid priority when curPriority = osPriorityLow1 and setPriority = osPriorityAboveNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority005, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow1;
    
    g_setPriority = osPriorityRealtime;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityRealtime7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityISR;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityError;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityReserved;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_6360
 * @tc.name      : set invalid priority when curPriority = osPriorityNormal and setPriority = osPriorityNone
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority006, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    g_setPriority = osPriorityNone;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityIdle;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = PRIORITY_COUNT_NOT_MIN;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_6480
 * @tc.name      : set priority when curPriority = osPriorityNormal and setPriority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority007, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    g_setPriority = osPriorityLow1;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityLow7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityBelowNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityBelowNormal7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_6680
 * @tc.name      : set priority when curPriority = osPriorityNormal and setPriority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority008, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    g_setPriority = osPriorityNormal7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityAboveNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityAboveNormal1;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_6800
 * @tc.name      : set invalid priority when curPriority = osPriorityNormal and setPriority = osPriorityAboveNormal2
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority009, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    g_setPriority = osPriorityAboveNormal2;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityHigh;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityHigh7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityRealtime;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityRealtime7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityISR;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_7040
 * @tc.name      : set invalid priority when curPriority = osPriorityNormal and setPriority = osPriorityError
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority010, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal;
    g_setPriority = osPriorityError;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityReserved;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_7120
 * @tc.name      : set invalid priority when curPriority = osPriorityAboveNormal1 and setPriority = osPriorityNone
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority011, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal1;
    g_setPriority = osPriorityNone;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityIdle;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = PRIORITY_COUNT_NOT_MIN;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_7240
 * @tc.name      : set priority when curPriority = osPriorityAboveNormal1 and setPriority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority012, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal1;
    g_setPriority = osPriorityLow1;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityLow7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityBelowNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityBelowNormal7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityNormal7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_7480
 * @tc.name      : set priority when curPriority = osPriorityAboveNormal1 and setPriority = osPriorityAboveNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority013, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal1;
    g_setPriority = osPriorityAboveNormal;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityAboveNormal1;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_7560
 * @tc.name      : set invalid priority when curPriority = PriorityAboveNormal6 and setPriority = PriorityAboveNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority014, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal1;
    g_setPriority = osPriorityAboveNormal7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityHigh;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityHigh7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityRealtime;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityRealtime7;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_7760
 * @tc.name      : set invalid priority when curPriority = osPriorityAboveNormal1 and setPriority = osPriorityISR
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsThreadSetPriority015, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal1;
    g_setPriority = osPriorityISR;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityError;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    g_setPriority = osPriorityReserved;
    g_threadCreateExit2 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSetPriorityFunc002, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit2);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_7880
 * @tc.name      : delay operation for 5 ticks
 * @tc.desc      : [C- SOFTWARE -0200]

 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsDelay, Function | MediumTest | Level1)
{
    UINT32 uwRet = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    uwRet = osDelay(0);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_7960
 * @tc.name      : delay until operation
 * @tc.desc      : [C- SOFTWARE -0200]

 */
LITE_TEST_CASE(CmsisTaskPriFuncTestSuite, testOsDelayUntil, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    UINT32 uwTickCnt;
    UINT32 uwUntilTickCnt;
    uwTickCnt = osKernelGetTickCount();
    uwUntilTickCnt = uwTickCnt + DELAY_TICKS_5;
    uwRet = osDelayUntil(uwUntilTickCnt);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osDelayUntil(DELAY_TICKS_1);
    ICUNIT_ASSERT_EQUAL(uwRet, osError, uwRet);

    uwTickCnt = osKernelGetTickCount();
    uwUntilTickCnt = uwTickCnt - DELAY_TICKS_5;
    uwRet = osDelayUntil(uwUntilTickCnt);
    ICUNIT_ASSERT_EQUAL(uwRet, osError, uwRet);
    return 0;
};

RUN_TEST_SUITE(CmsisTaskPriFuncTestSuite);

void CmsisTaskPriFuncTest(void)
{
    RUN_ONE_TESTCASE(testOsThreadGetPriority001);
    RUN_ONE_TESTCASE(testOsThreadGetPriority002);
    RUN_ONE_TESTCASE(testOsThreadGetPriority003);
    RUN_ONE_TESTCASE(testOsThreadGetPriority004);
    RUN_ONE_TESTCASE(testOsThreadGetPriority005);
    RUN_ONE_TESTCASE(testOsThreadGetPriority006);
    RUN_ONE_TESTCASE(testOsThreadGetPriority007);
    RUN_ONE_TESTCASE(testOsThreadGetPriority008);
    RUN_ONE_TESTCASE(testOsThreadGetPriority009);
    RUN_ONE_TESTCASE(testOsThreadGetPriority010);
    RUN_ONE_TESTCASE(testOsThreadSetPriority001);
    RUN_ONE_TESTCASE(testOsThreadSetPriority002);
    RUN_ONE_TESTCASE(testOsThreadSetPriority003);
    RUN_ONE_TESTCASE(testOsThreadSetPriority004);
    RUN_ONE_TESTCASE(testOsThreadSetPriority005);
    RUN_ONE_TESTCASE(testOsThreadSetPriority006);
    RUN_ONE_TESTCASE(testOsThreadSetPriority007);
    RUN_ONE_TESTCASE(testOsThreadSetPriority008);
    RUN_ONE_TESTCASE(testOsThreadSetPriority009);
    RUN_ONE_TESTCASE(testOsThreadSetPriority010);
    RUN_ONE_TESTCASE(testOsThreadSetPriority011);
    RUN_ONE_TESTCASE(testOsThreadSetPriority012);
    RUN_ONE_TESTCASE(testOsThreadSetPriority013);
    RUN_ONE_TESTCASE(testOsThreadSetPriority014);
    RUN_ONE_TESTCASE(testOsThreadSetPriority015);
    RUN_ONE_TESTCASE(testOsThreadSetPriority016);
    RUN_ONE_TESTCASE(testOsDelay);
    RUN_ONE_TESTCASE(testOsDelayUntil);
}