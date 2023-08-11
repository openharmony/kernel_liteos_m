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
}