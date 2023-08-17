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

UINT32 g_threadCount;
UINT16 g_cmsisTestTaskCount;
UINT16 g_getStackSizeExit;
UINT16 g_threadCreateExit;
UINT16 g_threadCreateExit1;
UINT16 g_getNameExit;
UINT16 g_getStackSpaceExit;
osThreadId_t g_puwTaskID01;
osThreadId_t g_puwTaskID02;
osPriority_t g_threadPriority;

LITE_TEST_SUIT(Cmsis, Cmsistask, CmsisTaskFuncTestSuite);

static BOOL CmsisTaskFuncTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL CmsisTaskFuncTestSuiteTearDown(void)
{
    return TRUE;
}

static void CmsisThreadCreatFunc(void const *argument)
{
    (void)argument;
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadCreat002Func001(void const *argument)
{
    (void)argument;
    g_cmsisTestTaskCount++;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_3, g_cmsisTestTaskCount);
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadCreat002Func002(void const *argument)
{
    (void)argument;
    g_cmsisTestTaskCount++;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    osThreadExit();
}

static void CmsisThreadCreat003Func001(void const *argument)
{
    (void)argument;
    osStatus_t status;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, 0, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_3, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadCreat003Func002(void const *argument)
{
    (void)argument;
    osStatus_t status;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_4, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    osThreadExit();
}

static void CmsisThreadCreat004Func002(void const *argument)
{
    (void)argument;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    osThreadExit();
}

static void CmsisThreadCreat004Func001(void const *argument)
{
    (void)argument;
    osThreadId_t osId;
    osThreadAttr_t osAttr;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityAboveNormal1;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, 0, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreat004Func002, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL_VOID(osId, NULL, osId);
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void KeepRunByTick(UINT32 tick)
{
    UINT32 tickA = osKernelGetTickCount();
    UINT32 runned = 0;
    UINT32 tickB = 0;
    while (runned < tick) {
        tickB = osKernelGetTickCount();
        if (tickB >= tickA) {
            runned = tickB - tickA;
        } else {
            runned = tickB + (MAX_UINT32 - tickA);
        }
    }
    return;
}

static void WaitThreadExit(osThreadId_t id, UINT16 const *exitFlag)
{
    osStatus_t status;
    (void)osThreadSetPriority(id, PRIORITY_COUNT_MIN_1);
    while (*exitFlag != TESTCOUNT_NUM_1) {
        status = osDelay(DELAY_TICKS_10);
        ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    }
}

static void CmsisThreadCreat005Func001(void const *argument)
{
    (void)argument;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);
    while (g_cmsisTestTaskCount < TESTCOUNT_NUM_2) {
        KeepRunByTick(DELAY_TICKS_10);
    }
    g_cmsisTestTaskCount++;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_3, g_cmsisTestTaskCount);
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadGetIDFunc001(void const *argument)
{
    (void)argument;
    g_puwTaskID01 = osThreadGetId();
    ICUNIT_ASSERT_NOT_EQUAL_VOID(g_puwTaskID01, NULL, g_puwTaskID01);
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadGetNameFunc001(void const *argument)
{
    (void)argument;
    osThreadAttr_t attr;
    g_puwTaskID01 = osThreadGetId();
    attr.name = osThreadGetName(g_puwTaskID01);
    ICUNIT_ASSERT_STRING_EQUAL_VOID(attr.name, "testThreadGetName", attr.name);
    g_getNameExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadGetStateFunc001(void const *argument)
{
    (void)argument;
    osThreadState_t state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(state, osThreadBlocked, state);

    g_puwTaskID02 = osThreadGetId();
    state = osThreadGetState(g_puwTaskID02);
    ICUNIT_ASSERT_EQUAL_VOID(state, osThreadRunning, state);
    g_threadCreateExit1 = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadGetStateFunc002(void const *argument)
{
    (void)argument;
    osThreadState_t state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(state, osThreadReady, state);

    g_puwTaskID02 = osThreadGetId();
    state = osThreadGetState(g_puwTaskID02);
    ICUNIT_ASSERT_EQUAL_VOID(state, osThreadRunning, state);
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadSuspendFunc001(void const *argument)
{
    (void)argument;
    osStatus_t status;
    g_puwTaskID01 = osThreadGetId();
    status = osThreadSuspend(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadGetStackSizeFunc001(void const *argument)
{
    (void)argument;
    osThreadAttr_t attr;
    g_puwTaskID01 = osThreadGetId();
    attr.stack_size = osThreadGetStackSize(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(attr.stack_size, TEST_TASK_STACK_SIZE, attr.stack_size);
    g_getStackSizeExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadGetStackSpaceFunc001(void const *argument)
{
    (void)argument;
    UINT32 uwCount;
    g_puwTaskID01 =  osThreadGetId();
    uwCount = osThreadGetStackSpace(g_puwTaskID01);
    ICUNIT_ASSERT_WITHIN_EQUAL_VOID(uwCount, 0, INT_MAX, uwCount);
    g_getStackSpaceExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadYieldFunc002(void const *argument)
{
    (void)argument;
    osThreadState_t state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(state, osThreadReady, state);
    g_cmsisTestTaskCount++;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    g_threadCreateExit = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadYieldFunc001(void const *argument)
{
    (void)argument;
    osStatus_t status;
    osThreadId_t osId;
    osThreadState_t state;
    osThreadAttr_t osAttr;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = g_threadPriority;
    g_threadCreateExit = 0;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);
    g_puwTaskID01 = osThreadGetId();
    osId = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc002, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL_VOID(osId, NULL, osId);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(state, osThreadRunning, state);
    status = osThreadYield();
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    g_threadCreateExit1 = TESTCOUNT_NUM_1;
    WaitThreadExit(osId, &g_threadCreateExit);
    osThreadExit();
}

static void CmsisThreadYieldFunc003(void const *argument)
{
    (void)argument;
    osStatus_t status;
    osThreadId_t osId;
    osThreadState_t state;
    osThreadAttr_t osAttr;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityNormal7;
    g_puwTaskID01 =  osThreadGetId();
    g_threadCreateExit = 0;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL_VOID(osId, NULL, osId);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(state, osThreadRunning, state);
    status = osThreadYield();
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    WaitThreadExit(osId, &g_threadCreateExit);
    g_threadCreateExit1 = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadResumeFunc002(void const *argument)
{
    (void)argument;
    osStatus_t status;
    g_cmsisTestTaskCount++;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    status = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    osThreadExit();
}

static void CmsisThreadResumeFunc001(void const *argument)
{
    (void)argument;
    osStatus_t status;
    osThreadId_t osId;
    osThreadAttr_t osAttr;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityAboveNormal;
    g_puwTaskID01 = osThreadGetId();
    osId = osThreadNew((osThreadFunc_t)CmsisThreadResumeFunc002, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL_VOID(osId, NULL, osId);
    status = osThreadSuspend(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    g_cmsisTestTaskCount++;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_3, g_cmsisTestTaskCount);
    g_threadCreateExit1 = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadTerminateFunc001(void const *argument)
{
    (void)argument;
    osStatus_t status;
    g_puwTaskID01 = osThreadGetId();
    status = osThreadTerminate(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);
    g_cmsisTestTaskCount++;
    ICUNIT_ASSERT_EQUAL_VOID(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);
    osThreadExit();
}

static void CmsisThreadGetCountFunc002(void const *argument)
{
    (void)argument;
    UINT32 uwRet = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL_VOID(uwRet, 0, INT_MAX, uwRet);
    osThreadExit();
}

static void CmsisThreadGetCountFunc001(void const *argument)
{
    (void)argument;
    osThreadId_t osId;
    osThreadAttr_t osAttr;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityAboveNormal;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc002, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL_VOID(osId, NULL, osId);
    g_threadCreateExit1 = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisThreadGetCountFunc003(void const *argument)
{
    (void)argument;
    UINT32 uwRet = osThreadGetCount();
    ICUNIT_ASSERT_EQUAL_VOID(uwRet, g_threadCount + 1, uwRet);
    g_threadCreateExit1 = TESTCOUNT_NUM_1;
    osThreadExit();
}

static void CmsisOSKernelLockFunc002(void const *arg)
{
    (void)arg;
    g_cmsisTestTaskCount++;
    osThreadExit();
}

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_0040
 * @tc.name      : thread operation for creat fail with invalid parameter
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew001, Function | MediumTest | Level1)
{
    osThreadId_t osId;
    osThreadAttr_t osAttr;
    osStatus_t status;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityNormal;
    g_threadCreateExit = 0;
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(osId, NULL, osId);
    osId = osThreadNew(NULL, NULL, NULL);
    ICUNIT_ASSERT_EQUAL(osId, NULL, osId);
    osId = osThreadNew(NULL, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(osId, NULL, osId);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    WaitThreadExit(osId, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_0080
 * @tc.name      : thread operation for creat success
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew002, Function | MediumTest | Level1)
{
    osThreadId_t osId;
    osThreadAttr_t osAttr;
    osStatus_t status;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityLow1;
    g_cmsisTestTaskCount = 0;
    g_threadCreateExit = 0;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreat002Func001, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(osId, NULL, osId);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, 0, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    osAttr.priority = osPriorityAboveNormal;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreat002Func002, NULL, &osAttr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_3, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(osId, NULL, osId);
    WaitThreadExit(osId, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_0120
 * @tc.name      : thread operation for delay scheduler
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew003, Function | MediumTest | Level1)
{
    osThreadId_t osId;
    osStatus_t status;
    osThreadAttr_t osAttr;
    osThreadAttr_t osAttr1;
    g_cmsisTestTaskCount = 0;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityAboveNormal;
    g_threadCreateExit = 0;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreat003Func001, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(osId, NULL, osId);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);
    g_cmsisTestTaskCount++;
    osAttr1.name = "test1";
    osAttr1.attr_bits = 0U;
    osAttr1.cb_mem = NULL;
    osAttr1.cb_size = 0U;
    osAttr1.stack_mem = NULL;
    osAttr1.stack_size = TEST_TASK_STACK_SIZE;
    osAttr1.priority = osPriorityAboveNormal;
    status = osDelay(DELAY_TICKS_1);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreat003Func002, NULL, &osAttr1);
    ICUNIT_ASSERT_NOT_EQUAL(osId, NULL, osId);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_5, g_cmsisTestTaskCount);
    WaitThreadExit(osId, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_0160
 * @tc.name      : thread operation for nesting schedule
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew004, Function | MediumTest | Level1)
{
    osThreadId_t osId;
    osStatus_t status;
    osThreadAttr_t osAttr;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    g_cmsisTestTaskCount = 0;
    osAttr.priority = osPriorityAboveNormal;
    g_threadCreateExit = 0;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreat004Func001, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(osId, NULL, osId);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_3, g_cmsisTestTaskCount);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    WaitThreadExit(osId, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_0200
 * @tc.name      : thread operation for cycle schdule
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew005, Function | MediumTest | Level1)
{
    osThreadId_t osId;
    osStatus_t status;
    osThreadAttr_t osAttr;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityLow1;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit = 0;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreat005Func001, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(osId, NULL, osId);
    status = osDelay(DELAY_TICKS_1);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    g_cmsisTestTaskCount++;
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    while (g_cmsisTestTaskCount != TESTCOUNT_NUM_3) {
        KeepRunByTick(DELAY_TICKS_10);
    }
    WaitThreadExit(osId, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_0240
 * @tc.name      : thread operation for creat fail when priority = osPriorityNone
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew006, Function | MediumTest | Level1)
{
    osThreadAttr_t osAttr;
    osThreadId_t id;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityNone;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityIdle;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = PRIORITY_COUNT_NOT_MIN;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityAboveNormal2;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityHigh;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityHigh7;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityRealtime;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityRealtime7;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityISR;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityError;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityReserved;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_0360
 * @tc.name      : thread operation for creat success when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew007, Function | MediumTest | Level1)
{
    osThreadAttr_t osAttr;
    osThreadId_t id;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityLow1;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityLow7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityBelowNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityBelowNormal7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityNormal7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityAboveNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    osAttr.priority = osPriorityAboveNormal1;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1000
 * @tc.name      : thread creat operation with func = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew008, Function | MediumTest | Level1)
{
    osThreadAttr_t osAttr;
    osThreadId_t osId;
    osAttr.name = "test";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityNormal;
    osId = osThreadNew(NULL, NULL, &osAttr);
    ICUNIT_ASSERT_EQUAL(osId, NULL, osId);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1040
 * @tc.name      : thread creat operation with attr = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadNew009, Function | MediumTest | Level1)
{
    osThreadId_t osId;
    g_threadCreateExit = 0;
    osId = osThreadNew((osThreadFunc_t)CmsisThreadCreatFunc, NULL, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(osId, NULL, osId);
    WaitThreadExit(osId, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1160
 * @tc.name      : thread operation for suspend when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadSuspend001, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osStatus_t uwRet;
    osThreadAttr_t attr;
    osThreadState_t state;
    g_cmsisTestTaskCount = 0;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow1;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSuspendFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    attr.priority = osPriorityNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSuspendFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    WaitThreadExit(id, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1200
 * @tc.name      : thread operation for get current ID
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetId001, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_puwTaskID01 = osThreadGetId();
    ICUNIT_ASSERT_NOT_EQUAL(g_puwTaskID01, NULL, g_puwTaskID01);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1240
 * @tc.name      : thread operation for get ID when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetId002, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    osStatus_t status;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow1;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetIDFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL((uintptr_t)id, (uintptr_t)g_puwTaskID01, (uintptr_t)id);

    attr.priority = osPriorityLow7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetIDFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL((uintptr_t)id, (uintptr_t)g_puwTaskID01, (uintptr_t)id);

    attr.priority = osPriorityBelowNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetIDFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL((uintptr_t)id, (uintptr_t)g_puwTaskID01, (uintptr_t)id);

    attr.priority = osPriorityBelowNormal7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetIDFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL((uintptr_t)id, (uintptr_t)g_puwTaskID01, (uintptr_t)id);

    attr.priority = osPriorityNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetIDFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL((uintptr_t)id, (uintptr_t)g_puwTaskID01, (uintptr_t)id);
    WaitThreadExit(id, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1440
 * @tc.name      : thread operation for get ID when priority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetId003, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;

    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetIDFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ICUNIT_ASSERT_EQUAL((uintptr_t)id, (uintptr_t)g_puwTaskID01, (uintptr_t)id);

    attr.priority = osPriorityAboveNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetIDFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ICUNIT_ASSERT_EQUAL((uintptr_t)id, (uintptr_t)g_puwTaskID01, (uintptr_t)id);

    attr.priority = osPriorityAboveNormal1;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetIDFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ICUNIT_ASSERT_EQUAL((uintptr_t)id, (uintptr_t)g_puwTaskID01, (uintptr_t)id);
    WaitThreadExit(id, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1560
 * @tc.name      : thread operation for get ID then exit thread
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetId004, Function | MediumTest | Level1)
{
    g_puwTaskID01 = osThreadGetId();
    ICUNIT_ASSERT_NOT_EQUAL(g_puwTaskID01, NULL, g_puwTaskID01);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1640
 * @tc.name      : thread operation for get stack space
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetStackSpace001, Function | MediumTest | Level1)
{
    UINT32 uwCount;
    g_cmsisTestTaskCount = 0;
    g_puwTaskID01 = osThreadGetId();
    uwCount = osThreadGetStackSpace(g_puwTaskID01);
    ICUNIT_ASSERT_WITHIN_EQUAL(uwCount, 0, INT_MAX, uwCount);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1680
 * @tc.name      : thread operation for yield when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield001, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_threadPriority = osPriorityLow1;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_threadPriority;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1720
 * @tc.name      : thread operation for yield when priority = osPriorityLow7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield002, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_threadPriority = osPriorityLow7;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_threadPriority;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1760
 * @tc.name      : thread operation for yield when priority = osPriorityBelowNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield003, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_threadPriority = osPriorityBelowNormal;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_threadPriority;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1800
 * @tc.name      : thread operation for yield when priority = osPriorityBelowNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield004, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_threadPriority = osPriorityBelowNormal7;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_threadPriority;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1840
 * @tc.name      : thread operation for yield when priority = osPriorityNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield005, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osThreadAttr_t attr;
    g_threadPriority = osPriorityNormal;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_threadPriority;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1880
 * @tc.name      : thread operation for yield when priority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield006, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    g_threadPriority = osPriorityNormal7;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_threadPriority;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc001, NULL, &attr);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1920
 * @tc.name      : thread operation for yield when priority = osPriorityAboveNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield007, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    g_threadPriority = osPriorityAboveNormal;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_threadPriority;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc001, NULL, &attr);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_1960
 * @tc.name      : thread operation for yield when priority = osPriorityAboveNormal1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield008, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    g_threadPriority = osPriorityAboveNormal1;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = g_threadPriority;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc001, NULL, &attr);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_2, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2000
 * @tc.name      : thread yield operation for thread with different priority
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadYield009, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadYieldFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2040
 * @tc.name      : thread operation for resume
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadResume001, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    id = osThreadNew((osThreadFunc_t)CmsisThreadResumeFunc001, NULL, &attr);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_3, g_cmsisTestTaskCount);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2120
 * @tc.name      : get thread count with callback function when priority = osPriorityAboveNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetCount001, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2160
 * @tc.name      : thread operation for current count
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetCount002, Function | MediumTest | Level1)
{
    UINT32 uwRet = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(uwRet, 0, INT_MAX, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2200
 * @tc.name      : get thread count when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetCount003, Function | MediumTest | Level1)
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
    g_threadCreateExit1 = 0;
    g_threadCount = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(g_threadCount, 0, INT_MAX, g_threadCount);
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityLow7;
    g_threadCreateExit1 = 0;
    g_threadCount = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(g_threadCount, 0, INT_MAX, g_threadCount);
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityBelowNormal;
    g_threadCreateExit1 = 0;
    g_threadCount = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(g_threadCount, 0, INT_MAX, g_threadCount);
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityBelowNormal7;
    g_threadCreateExit1 = 0;
    g_threadCount = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(g_threadCount, 0, INT_MAX, g_threadCount);
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2360
 * @tc.name      : get thread count when priority = osPriorityNormal
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetCount004, Function | MediumTest | Level1)
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
    g_threadCreateExit1 = 0;
    g_threadCount = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(g_threadCount, 0, INT_MAX, g_threadCount);
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityNormal7;
    g_threadCreateExit1 = 0;
    g_threadCount = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(g_threadCount, 0, INT_MAX, g_threadCount);
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityAboveNormal;
    g_threadCreateExit1 = 0;
    g_threadCount = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(g_threadCount, 0, INT_MAX, g_threadCount);
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityAboveNormal1;
    g_threadCreateExit1 = 0;
    g_threadCount = osThreadGetCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(g_threadCount, 0, INT_MAX, g_threadCount);
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetCountFunc003, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2520
 * @tc.name      : thread operation for get name input exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetName002, Function | MediumTest | Level1)
{
    osThreadAttr_t attr;
    attr.name = osThreadGetName(NULL);
    ICUNIT_ASSERT_EQUAL(attr.name, NULL, attr.name);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2560
 * @tc.name      : thread operation for get name when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetName001, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    osStatus_t status;
    attr.name = "testThreadGetName";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow1;
    g_getNameExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetNameFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityLow7;
    g_getNameExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetNameFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityBelowNormal;
    g_getNameExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetNameFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityBelowNormal7;
    g_getNameExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetNameFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityNormal;
    g_cmsisTestTaskCount = 0;
    g_getNameExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetNameFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    WaitThreadExit(id, &g_getNameExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2720
 * @tc.name      : thread operation for get name when priority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetName003, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    attr.name = "testThreadGetName";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal7;
    g_getNameExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetNameFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityAboveNormal;
    g_getNameExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetNameFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityAboveNormal1;
    g_getNameExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetNameFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_getNameExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2840
 * @tc.name      : thread operation for get name
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetName010, Function | MediumTest | Level1)
{
    osThreadAttr_t attr;
    g_puwTaskID01 = osThreadGetId();
    attr.name = osThreadGetName(g_puwTaskID01);
    ICUNIT_ASSERT_NOT_EQUAL(attr.name, NULL, attr.name);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2880
 * @tc.name      : thread operation for get state input exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetState001, Function | MediumTest | Level1)
{
    osStatus_t uwRet = osThreadGetState(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, osThreadError, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_2920
 * @tc.name      : thread operation for get state when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetState002, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    osStatus_t status;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow1;
    g_threadCreateExit1 = 0;
    g_puwTaskID01 = osThreadGetId();
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    WaitThreadExit(id, &g_threadCreateExit1);

    attr.priority = PRIORITY_COUNT_MIN_1;
    g_threadCreateExit1 = 0;
    g_puwTaskID01 = osThreadGetId();
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    WaitThreadExit(id, &g_threadCreateExit1);

    attr.priority = PRIORITY_COUNT_MIN_2;
    g_threadCreateExit1 = 0;
    g_puwTaskID01 = osThreadGetId();
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = PRIORITY_COUNT_MIN_3;
    g_threadCreateExit1 = 0;
    g_puwTaskID01 = osThreadGetId();
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = PRIORITY_COUNT_MIN_4;
    g_threadCreateExit1 = 0;
    g_puwTaskID01 = osThreadGetId();
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    WaitThreadExit(id, &g_threadCreateExit1);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3120
 * @tc.name      : thread operation for get state when priority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetState003, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    osStatus_t status;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal7;
    g_threadCreateExit = 0;
    g_puwTaskID01 = osThreadGetId();
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStateFunc002, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityAboveNormal1;
    g_threadCreateExit = 0;
    g_puwTaskID01 = osThreadGetId();
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStateFunc002, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    attr.priority = osPriorityAboveNormal;
    g_threadCreateExit = 0;
    g_puwTaskID01 = osThreadGetId();
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStateFunc002, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    WaitThreadExit(id, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3200
 * @tc.name      : thread operation for get current state
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetState004, Function | MediumTest | Level1)
{
    osThreadState_t state;
    g_puwTaskID01 = osThreadGetId();
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadRunning, state);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3240
 * @tc.name      : thread operation for suspend input exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadSuspend002, Function | MediumTest | Level1)
{
    osStatus_t uwRet = osThreadSuspend(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3280
 * @tc.name      : thread operation for suspend when priority = osPriorityLow7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadSuspend003, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t status;
    osStatus_t uwRet;
    osThreadAttr_t attr;
    osThreadState_t state;
    g_cmsisTestTaskCount = 0;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSuspendFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    attr.priority = osPriorityBelowNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSuspendFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    attr.priority = osPriorityBelowNormal7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSuspendFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    WaitThreadExit(id, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3440
 * @tc.name      : thread operation for suspend when priority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadSuspend004, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osStatus_t uwRet;
    osThreadAttr_t attr;
    osThreadState_t state;
    g_cmsisTestTaskCount = 0;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal7;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSuspendFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    attr.priority = osPriorityAboveNormal;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSuspendFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    attr.priority = osPriorityAboveNormal1;
    g_threadCreateExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadSuspendFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    state = osThreadGetState(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    WaitThreadExit(id, &g_threadCreateExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3560
 * @tc.name      : thread operation for get stack size input exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetStackSize001, Function | MediumTest | Level1)
{
    UINT32 uwRet = osThreadGetStackSize(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3600
 * @tc.name      : thread operation for get stack size when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetStackSize002, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow1;
    g_getStackSizeExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSizeFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityLow7;
    g_getStackSizeExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSizeFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityBelowNormal;
    g_getStackSizeExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSizeFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityBelowNormal7;
    g_getStackSizeExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSizeFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityNormal;
    g_getStackSizeExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSizeFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityNormal7;
    g_getStackSizeExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSizeFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityAboveNormal;
    g_getStackSizeExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSizeFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityAboveNormal1;
    g_getStackSizeExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSizeFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_getStackSizeExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3880
 * @tc.name      : thread operation for get stack size
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetStackSize003, Function | MediumTest | Level1)
{
    osThreadAttr_t attr;
    g_puwTaskID01 = osThreadGetId();
    attr.stack_size = osThreadGetStackSize(g_puwTaskID01);
    ICUNIT_ASSERT_WITHIN_EQUAL(attr.stack_size, 0, INT_MAX, attr.stack_size);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3920
 * @tc.name      : thread operation for get stack space input exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetStackSpace002, Function | MediumTest | Level1)
{
    UINT32 uwRet = osThreadGetStackSpace(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_3960
 * @tc.name      : thread operation for get stack space when priority = osPriorityLow1
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetStackSpace003, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    osStatus_t status;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow1;
    g_getStackSpaceExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSpaceFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityLow7;
    g_getStackSpaceExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSpaceFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityBelowNormal;
    g_getStackSpaceExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSpaceFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityBelowNormal7;
    g_getStackSpaceExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSpaceFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityNormal;
    g_getStackSpaceExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSpaceFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_getStackSpaceExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4160
 * @tc.name      : thread operation for get stack space when priority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadGetStackSpace004, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    osStatus_t status;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal7;
    g_getStackSpaceExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSpaceFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityAboveNormal;
    g_getStackSpaceExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSpaceFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);

    attr.priority = osPriorityAboveNormal1;
    g_getStackSpaceExit = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadGetStackSpaceFunc001, NULL, &attr);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    WaitThreadExit(id, &g_getStackSpaceExit);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4280
 * @tc.name      : thread operation for resume input exception with NULL parameter
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadResume002, Function | MediumTest | Level1)
{
    UINT32 uwRet = osThreadResume(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);

    g_puwTaskID01 = osThreadGetId();
    uwRet = osThreadResume(g_puwTaskID01);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorResource, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4360
 * @tc.name      : thread operation for terminate input exception
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadTerminate001, Function | MediumTest | Level1)
{
    UINT32 uwRet = osThreadTerminate(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4400
 * @tc.name      : thread operation for terminate when priority = osPriorityLow7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadTerminate002, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    osStatus_t status;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityLow7;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadTerminateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);

    attr.priority = osPriorityBelowNormal;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadTerminateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);

    attr.priority = osPriorityBelowNormal7;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadTerminateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);

    attr.priority = osPriorityNormal;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadTerminateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4560
 * @tc.name      : thread operation for terminate when priority = osPriorityNormal7
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsThreadTerminate003, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t attr;
    osStatus_t status;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityNormal7;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    id = osThreadNew((osThreadFunc_t)CmsisThreadTerminateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);

    attr.priority = osPriorityAboveNormal;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadTerminateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);

    attr.priority = osPriorityAboveNormal1;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    g_threadCreateExit1 = 0;
    id = osThreadNew((osThreadFunc_t)CmsisThreadTerminateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);

    attr.priority = osPriorityLow1;
    g_cmsisTestTaskCount = 0;
    g_cmsisTestTaskCount++;
    id = osThreadNew((osThreadFunc_t)CmsisThreadTerminateFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4680
 * @tc.name      : kernel operation for get info
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelGetInfo001, Function | MediumTest | Level1)
{
    CHAR infobuf[100]; /* 100, common data for test, no special meaning */
    osVersion_t osv;
    osStatus_t status = osKernelGetInfo(&osv, infobuf, sizeof(infobuf));
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4720
 * @tc.name      : kernel operation for get state
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelGetState001, Function | MediumTest | Level1)
{
    osKernelState_t uwRet = osKernelGetState();
    ICUNIT_ASSERT_EQUAL(uwRet, osKernelRunning, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4760
 * @tc.name      : kernel operation for get state after kernel lock
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelGetState002, Function | MediumTest | Level1)
{
    osKernelLock();
    osKernelState_t uwRet;
    uwRet = osKernelGetState();
    ICUNIT_ASSERT_EQUAL(uwRet, osKernelLocked, uwRet);
    osKernelUnlock();
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4800
 * @tc.name      : kernel lock operation twice
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelLock001, Function | MediumTest | Level1)
{
    UINT32 uwRet = osKernelLock();
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    uwRet = osKernelLock();
    ICUNIT_ASSERT_EQUAL(uwRet, 1, uwRet);
    osKernelUnlock();
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4840
 * @tc.name      : kernel operation for lock
 * @tc.desc      : [C- SOFTWARE -0200]

 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelLock002, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    osThreadId_t id;
    osThreadAttr_t attr;
    g_cmsisTestTaskCount = 0;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;

    uwRet = osKernelLock();
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    id = osThreadNew((osThreadFunc_t)CmsisOSKernelLockFunc002, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, 0, g_cmsisTestTaskCount);
    uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 1, uwRet);
    ICUNIT_ASSERT_EQUAL(g_cmsisTestTaskCount, TESTCOUNT_NUM_1, g_cmsisTestTaskCount);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4880
 * @tc.name      : kernel operation for unlock
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelUnLock001, Function | MediumTest | Level1)
{
    UINT32 uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4920
 * @tc.name      : kernel operation for unlock after kernel lock
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelUnLock002, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    (void)osKernelLock();
    uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 1, uwRet); /* 1, common data for test, no special meaning */
    uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_4960
 * @tc.name      : kernel operation for unlock after kernel lock twice
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelUnLock003, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    (void)osKernelLock();
    (void)osKernelLock();
    uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 1, uwRet); /* 1, common data for test, no special meaning */
    uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5000
 * @tc.name      : kernel operation for restore lock after kernel lock
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelRestoreLock001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    (void)osKernelLock();
    uwRet = osKernelRestoreLock(0);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5040
 * @tc.name      : kernel operation for restore lock after kernel lock twice
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelRestoreLock002, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    (void)osKernelLock();
    (void)osKernelLock();
    uwRet = osKernelRestoreLock(0);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_TASK_OPERATION_5080
 * @tc.name      : kernel operation for restore lock
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisTaskFuncTestSuite, testOsKernelRestoreLock003, Function | MediumTest | Level1)
{
    UINT32 uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    uwRet = osKernelRestoreLock(1); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(uwRet, 1, uwRet); /* 1, common data for test, no special meaning */
    uwRet = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uwRet, 1, uwRet); /* 1, common data for test, no special meaning */
    return 0;
};

RUN_TEST_SUITE(CmsisTaskFuncTestSuite);

void CmsisTaskFuncTest(void)
{
    RUN_ONE_TESTCASE(testOsThreadNew001);
    RUN_ONE_TESTCASE(testOsThreadNew002);
    RUN_ONE_TESTCASE(testOsThreadNew003);
    RUN_ONE_TESTCASE(testOsThreadNew004);
    RUN_ONE_TESTCASE(testOsThreadNew005);
    RUN_ONE_TESTCASE(testOsThreadNew006);
    RUN_ONE_TESTCASE(testOsThreadNew007);
    RUN_ONE_TESTCASE(testOsThreadNew008);
    RUN_ONE_TESTCASE(testOsThreadNew009);
    RUN_ONE_TESTCASE(testOsThreadGetName001);
    RUN_ONE_TESTCASE(testOsThreadGetName002);
    RUN_ONE_TESTCASE(testOsThreadGetName003);
    RUN_ONE_TESTCASE(testOsThreadGetState001);
    RUN_ONE_TESTCASE(testOsThreadGetState002);
    RUN_ONE_TESTCASE(testOsThreadGetState003);
    RUN_ONE_TESTCASE(testOsThreadGetState004);
    RUN_ONE_TESTCASE(testOsThreadSuspend001);
    RUN_ONE_TESTCASE(testOsThreadSuspend002);
    RUN_ONE_TESTCASE(testOsThreadSuspend003);
    RUN_ONE_TESTCASE(testOsThreadSuspend004);
    RUN_ONE_TESTCASE(testOsThreadGetId001);
    RUN_ONE_TESTCASE(testOsThreadGetId002);
    RUN_ONE_TESTCASE(testOsThreadGetId003);
    RUN_ONE_TESTCASE(testOsThreadGetId004);
    RUN_ONE_TESTCASE(testOsThreadGetStackSize001);
    RUN_ONE_TESTCASE(testOsThreadGetStackSize002);
    RUN_ONE_TESTCASE(testOsThreadGetStackSize003);
    RUN_ONE_TESTCASE(testOsThreadGetStackSpace001);
    RUN_ONE_TESTCASE(testOsThreadGetStackSpace002);
    RUN_ONE_TESTCASE(testOsThreadGetStackSpace003);
    RUN_ONE_TESTCASE(testOsThreadGetStackSpace004);
    RUN_ONE_TESTCASE(testOsThreadYield001);
    RUN_ONE_TESTCASE(testOsThreadYield002);
    RUN_ONE_TESTCASE(testOsThreadYield003);
    RUN_ONE_TESTCASE(testOsThreadYield004);
    RUN_ONE_TESTCASE(testOsThreadYield005);
    RUN_ONE_TESTCASE(testOsThreadYield006);
    RUN_ONE_TESTCASE(testOsThreadYield007);
    RUN_ONE_TESTCASE(testOsThreadYield008);
    RUN_ONE_TESTCASE(testOsThreadYield009);
    RUN_ONE_TESTCASE(testOsThreadResume001);
    RUN_ONE_TESTCASE(testOsThreadResume002);
    CmsisTaskFuncTest1();
}

void CmsisTaskFuncTest1(void)
{
    RUN_ONE_TESTCASE(testOsThreadTerminate001);
    RUN_ONE_TESTCASE(testOsThreadTerminate002);
    RUN_ONE_TESTCASE(testOsThreadTerminate003);
    RUN_ONE_TESTCASE(testOsThreadGetCount001);
    RUN_ONE_TESTCASE(testOsThreadGetCount002);
    RUN_ONE_TESTCASE(testOsThreadGetCount003);
    RUN_ONE_TESTCASE(testOsThreadGetCount004);

    RUN_ONE_TESTCASE(testOsKernelGetInfo001);
    RUN_ONE_TESTCASE(testOsKernelGetState001);
    RUN_ONE_TESTCASE(testOsKernelGetState002);
    RUN_ONE_TESTCASE(testOsKernelLock001);
    RUN_ONE_TESTCASE(testOsKernelLock002);
    RUN_ONE_TESTCASE(testOsKernelUnLock001);
    RUN_ONE_TESTCASE(testOsKernelUnLock002);
    RUN_ONE_TESTCASE(testOsKernelUnLock003);
    RUN_ONE_TESTCASE(testOsKernelRestoreLock001);
    RUN_ONE_TESTCASE(testOsKernelRestoreLock002);
    RUN_ONE_TESTCASE(testOsKernelRestoreLock003);
}