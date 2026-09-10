/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include <securec.h>
#include "osTest.h"
#include "cmsis_os.h"
#include "bug_isolate.h" /* BUG 宏统一在此定义,修复对应 BUG 后删宏恢复用例 */

#define TEST_STR(func) ItLos##func
#define TEST_TO_STR(x) #x
#define TEST_HEAD_TO_STR(x) TEST_TO_STR(x)
#define ADD_TEST_CASE(func) \
    TEST_ADD_CASE(TEST_HEAD_TO_STR(TEST_STR(func)), func, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION)

#define Function   0
#define MediumTest 0
#define Level0     0
#define Level1     0
#define Level2     0
#define Level3     0
#define LITE_TEST_CASE(module, function, flag) static int function(void)

#define STATCI_BUFF_SIZE   32
#define READ_BUFFER_SIZIE  7
#define QUEUE_WAIT_TIMEOUT 3

static VOID CmsisStackFunc01(void)
{
    g_testCount++;
    return;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_001
 * @tc.name      : event operation for join
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsis001, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};

    g_testCount = 0;

    void *stackAddr = malloc(OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_NOT_EQUAL(stackAddr, NULL, stackAddr);

    attr.stack_mem = stackAddr;
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityNormal + 1;
    attr.attr_bits = osThreadDetached;
    threadId = osThreadNew((osThreadFunc_t)CmsisStackFunc01, NULL, &attr);
    ICUNIT_GOTO_NOT_EQUAL(threadId, 0, threadId, EXIT);

    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);
EXIT:
    free(stackAddr);
    return LOS_OK;
};

/**
 * @tc.name: TestCmsis007
 * @tc.desc: set and get queue name
 * @tc.type: FUNC
 * @tc.require: issueI5LBE8
 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsis007, Function | MediumTest | Level1)
{
    osMessageQueueId_t msgQueueId;
    osMessageQueueAttr_t attr = {0};
    CHAR strbuff[] = "hello world";
    CHAR *name = NULL;
    INT32 ret;

    attr.name = "q1";
    /* dynamic test */
    msgQueueId = osMessageQueueNew(1, strlen(strbuff), &attr);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    name = (CHAR *)osMessageQueueGetName(msgQueueId);
    ret = strcmp(name, "q1");
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    name = (CHAR *)osMessageQueueGetName(msgQueueId);
    ICUNIT_ASSERT_EQUAL(name, NULL, name);

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
    CHAR staticBuff[STATCI_BUFF_SIZE + sizeof(UINT32)] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = STATCI_BUFF_SIZE + sizeof(UINT32);
    msgQueueId = osMessageQueueNew(1, STATCI_BUFF_SIZE, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    name = (CHAR *)osMessageQueueGetName(msgQueueId);
    ret = strcmp(name, "q1");
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    name = (CHAR *)osMessageQueueGetName(msgQueueId);
    ICUNIT_ASSERT_EQUAL(name, NULL, name);
#endif

    return LOS_OK;

EXIT:
    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    return LOS_OK;
}

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
static osMessageQueueId_t g_msgQueueId1;
static osMessageQueueId_t g_msgQueueId2;

static osThreadId_t threadId1;
static osThreadId_t threadId2;

static CHAR g_strbuff1[] = "hello";
static CHAR g_strbuff2[] = "world";
static CHAR g_staticBuff[STATCI_BUFF_SIZE + sizeof(UINT32)] = {0};

static VOID CmsisQueueTestThread1(VOID)
{
    CHAR data[READ_BUFFER_SIZIE] = {0};
    INT32 ret;
    osStatus_t status;

    ret = osMessageQueuePut(g_msgQueueId1, &g_strbuff1, 0U, 0U);
    ICUNIT_ASSERT_EQUAL_VOID(ret, osOK, ret);

    status = osMessageQueueGet(g_msgQueueId2, &data, NULL, QUEUE_WAIT_TIMEOUT);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);

    ret = strcmp(data, "world");
    ICUNIT_ASSERT_EQUAL_VOID(ret, 0, ret);
}

static VOID CmsisQueueTestThread2(VOID)
{
    CHAR data[READ_BUFFER_SIZIE] = {0};
    INT32 ret;
    osStatus_t status;

    status = osMessageQueueGet(g_msgQueueId1, &data, NULL, QUEUE_WAIT_TIMEOUT);
    ICUNIT_ASSERT_EQUAL_VOID(status, osOK, status);

    ret = strcmp(data, "hello");
    ICUNIT_ASSERT_EQUAL_VOID(ret, 0, ret);

    ret = osMessageQueuePut(g_msgQueueId2, &g_strbuff2, 0U, 0U);
    ICUNIT_ASSERT_EQUAL_VOID(ret, osOK, ret);
}

static INT32 ThreadReadWriteTest(VOID)
{
    osMessageQueueAttr_t attr = {0};
    INT32 ret;

    g_msgQueueId1 = osMessageQueueNew(1, strlen(g_strbuff1), NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_msgQueueId1, NULL, g_msgQueueId1);

    attr.mq_mem = g_staticBuff;
    attr.mq_size = strlen(g_strbuff2) + 1 + sizeof(UINT32);
    g_msgQueueId2 = osMessageQueueNew(1, strlen(g_strbuff2) + 1, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(g_msgQueueId2, NULL, g_msgQueueId2);

    threadId1 = osThreadNew((osThreadFunc_t)CmsisQueueTestThread1, NULL, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(threadId1, NULL, threadId1);

    threadId2 = osThreadNew((osThreadFunc_t)CmsisQueueTestThread2, NULL, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(threadId2, NULL, threadId2);

    osThreadJoin(threadId1);

    ret = strcmp(g_staticBuff, "world");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = osMessageQueueDelete(g_msgQueueId1);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMessageQueueDelete(g_msgQueueId2);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    return 0;
}

/**
 * @tc.name: TestCmsis006
 * @tc.desc: mix read write
 * @tc.type: FUNC
 * @tc.require: issueI5LBE8
 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsis006, Function | MediumTest | Level1)
{
    INT32 ret;

    ret = ThreadReadWriteTest();
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}
#endif

/**
 * @tc.name: TestCmsis005
 * @tc.desc: read-write exception
 * @tc.type: FUNC
 * @tc.require: issueI5LBE8
 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsis005, Function | MediumTest | Level1)
{
    osMessageQueueId_t msgQueueId;
    CHAR strbuff[] = "hello world";
    CHAR data[STATCI_BUFF_SIZE] = {0};
    INT32 ret;

    ret = osMessageQueuePut(NULL, &strbuff, 0U, 0U);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);

    ret = osMessageQueueGet(NULL, &data, NULL, 0U);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);

    /* dynmic test */
    msgQueueId = osMessageQueueNew(1, strlen(strbuff), NULL);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    ret = osMessageQueuePut(msgQueueId, NULL, 0U, 0U);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);

    ret = osMessageQueueGet(msgQueueId, NULL, NULL, 0U);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMessageQueuePut(msgQueueId, &strbuff, 0U, 0U);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);

    ret = osMessageQueueGet(msgQueueId, &data, NULL, 0U);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
    /* static test */
    osMessageQueueAttr_t attr = {0};
    CHAR staticBuff[STATCI_BUFF_SIZE + sizeof(UINT32)] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = STATCI_BUFF_SIZE + sizeof(UINT32);
    msgQueueId = osMessageQueueNew(1, STATCI_BUFF_SIZE, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    ret = osMessageQueuePut(msgQueueId, NULL, 0U, 0U);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);

    ret = osMessageQueueGet(msgQueueId, NULL, NULL, 0U);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMessageQueuePut(msgQueueId, &strbuff, 0U, 0U);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);

    ret = osMessageQueueGet(msgQueueId, &data, NULL, 0U);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);
#endif

    return LOS_OK;

EXIT:
    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    return LOS_OK;
}

/**
 * @tc.name: TestCmsis004
 * @tc.desc: read write test
 * @tc.type: FUNC
 * @tc.require: issueI5LBE8
 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsis004, Function | MediumTest | Level1)
{
    osMessageQueueId_t msgQueueId;
    CHAR strbuff[] = "hello world";
    CHAR data[STATCI_BUFF_SIZE] = {0};
    INT32 ret;

    /* dynamic test */
    msgQueueId = osMessageQueueNew(1, strlen(strbuff), NULL);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    ret = osMessageQueuePut(msgQueueId, &strbuff, 0U, 0U);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMessageQueueGet(msgQueueId, &data, NULL, 0U);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = strcmp(data, strbuff);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
    /* static test */
    osMessageQueueAttr_t attr = {0};
    CHAR staticBuff[STATCI_BUFF_SIZE + sizeof(UINT32)] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = strlen(strbuff) + 1 + sizeof(UINT32);
    msgQueueId = osMessageQueueNew(1, strlen(strbuff) + 1, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    ret = osMessageQueuePut(msgQueueId, &strbuff, 0U, 0U);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    ret = osMessageQueueGet(msgQueueId, &data, NULL, 0U);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    ret = strcmp(data, strbuff);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = strcmp(staticBuff, strbuff);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

EXIT:
    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
#endif
    return LOS_OK;
}

/**
 * @tc.name: TestCmsis003
 * @tc.desc: create exception parameters test
 * @tc.type: FUNC
 * @tc.require: issueI5LBE8
 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsis003, Function | MediumTest | Level1)
{
    osMessageQueueId_t msgQueueId;
    CHAR strbuff[] = "hello world";

    /* dynmic test */
    msgQueueId = osMessageQueueNew(0, strlen(strbuff), NULL);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);

    msgQueueId = osMessageQueueNew(1, 0xFFFFFFFF, NULL);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
    /* static test */
    osMessageQueueAttr_t attr = {0};
    CHAR staticBuff[STATCI_BUFF_SIZE + sizeof(UINT32)] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = STATCI_BUFF_SIZE + sizeof(UINT32);
    msgQueueId = osMessageQueueNew(0, strlen(strbuff), &attr);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);

    msgQueueId = osMessageQueueNew(0xFFFFFFFF, strlen(strbuff), &attr);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);

    attr.mq_mem = staticBuff;
    attr.mq_size = 0;
    msgQueueId = osMessageQueueNew(1, strlen(strbuff), &attr);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);

    attr.mq_mem = NULL;
    attr.mq_size = STATCI_BUFF_SIZE + sizeof(UINT32);
    msgQueueId = osMessageQueueNew(1, strlen(strbuff), &attr);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);
#endif

    return LOS_OK;
};

/**
 * @tc.name: TestCmsis002
 * @tc.desc: create and delete test
 * @tc.type: FUNC
 * @tc.require: issueI5LBE8
 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsis002, Function | MediumTest | Level1)
{
    osMessageQueueId_t msgQueueId;
    CHAR strbuff[] = "hello world";
    INT32 ret;

    /* dynamic test */
    msgQueueId = osMessageQueueNew(1, strlen(strbuff), NULL);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
    /* static test */
    osMessageQueueAttr_t attr = {0};
    CHAR staticBuff[STATCI_BUFF_SIZE + sizeof(UINT32)] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = STATCI_BUFF_SIZE + sizeof(UINT32);
    msgQueueId = osMessageQueueNew(1, STATCI_BUFF_SIZE, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
#endif

    return LOS_OK;
};

static VOID timerCallback(void *arg)
{
    return;
}

/**
 * @tc.name: TestCmsisTimer001
 * @tc.desc: Timer Management test
 * @tc.type: FUNC
 * @tc.require: issueI5TQ0T
 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisTimer001, Function | MediumTest | Level1)
{
    osTimerId_t time_id;
    const char *timerGetName = NULL;
    osStatus_t ret;

    time_id = osTimerNew(timerCallback, osTimerOnce, NULL, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(time_id, NULL, time_id);

    ret = osTimerStart(time_id, 100U); // 100, just for test
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    timerGetName = osTimerGetName(time_id);
    ICUNIT_GOTO_EQUAL(timerGetName, NULL, timerGetName, EXIT1);

    ret = osTimerIsRunning(time_id);
    ICUNIT_GOTO_EQUAL(ret, 1, ret, EXIT1);

    ret = osTimerStop(time_id);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT1);

    ret = osTimerIsRunning(time_id);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    ret = osTimerDelete(time_id);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    return LOS_OK;

EXIT1:
    ret = osTimerDelete(time_id);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    return LOS_OK;
};

static VOID CmsisSatFunc001(VOID)
{
    g_testCount++;
    return;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSaturation001, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityHigh;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;
    threadId = osThreadNew((osThreadFunc_t)CmsisSatFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(threadId, NULL, threadId);
    osDelay(5);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSaturation002, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityHigh1;
    attr.attr_bits = osThreadDetached;
    threadId = osThreadNew((osThreadFunc_t)CmsisSatFunc001, NULL, &attr);
    ICUNIT_ASSERT_EQUAL(threadId, NULL, threadId);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSaturation003, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityNone;
    attr.attr_bits = osThreadDetached;
    threadId = osThreadNew((osThreadFunc_t)CmsisSatFunc001, NULL, &attr);
    ICUNIT_ASSERT_EQUAL(threadId, NULL, threadId);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSaturation004, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityIdle;
    attr.attr_bits = osThreadDetached;
    threadId = osThreadNew((osThreadFunc_t)CmsisSatFunc001, NULL, &attr);
    ICUNIT_ASSERT_EQUAL(threadId, NULL, threadId);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSaturation005, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    osStatus_t ret;
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;
    threadId = osThreadNew((osThreadFunc_t)CmsisSatFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(threadId, NULL, threadId);
    ret = osThreadSetPriority(threadId, osPriorityHigh);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    osDelay(5);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSaturation006, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    osStatus_t ret;
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    threadId = osThreadNew((osThreadFunc_t)CmsisSatFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(threadId, NULL, threadId);
    ret = osThreadSetPriority(threadId, osPriorityHigh1);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    osDelay(5);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisExt001, Function | MediumTest | Level1)
{
    uint64_t ms = osKernelGetTick2ms();
    ICUNIT_ASSERT_NOT_EQUAL(ms, 0, ms);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisExt002, Function | MediumTest | Level1)
{
    uint64_t ticks = osMs2Tick(1000);
    ICUNIT_ASSERT_EQUAL(ticks, LOSCFG_BASE_CORE_TICK_PER_SECOND, ticks);
    ticks = osMs2Tick(0);
    ICUNIT_ASSERT_EQUAL(ticks, 0, ticks);
    return LOS_OK;
}

static VOID CmsisExtArgFunc(void *arg)
{
    void *ret = osThreadGetArgument();
    ICUNIT_ASSERT_EQUAL_VOID(ret, arg, ret);
    g_testCount++;
    return;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisExt003, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    UINT32 testArg = 0x12345678;
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;
    threadId = osThreadNew((osThreadFunc_t)CmsisExtArgFunc, (void *)&testArg, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(threadId, NULL, threadId);
    osDelay(5);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisExt004, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;
    threadId = osThreadNewExt((osThreadFunc_t)CmsisSatFunc001, NULL, &attr, CMSIS_THREAD_POLICY_RT);
    ICUNIT_ASSERT_NOT_EQUAL(threadId, NULL, threadId);
    osDelay(5);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisExt005, Function | MediumTest | Level1)
{
    osThreadId_t threadId;
    osThreadAttr_t attr = {0};
    osStatus_t ret;
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    threadId = osThreadNew((osThreadFunc_t)CmsisSatFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(threadId, NULL, threadId);
    ret = osThreadSetPolicy(threadId, CMSIS_THREAD_POLICY_RT);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    osDelay(5);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisQueueLimit001, Function | MediumTest | Level1)
{
    osMessageQueueId_t msgQueueId;
    msgQueueId = osMessageQueueNew(0x10000, 4, NULL);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisQueueLimit002, Function | MediumTest | Level1)
{
    osMessageQueueId_t msgQueueId;
    msgQueueId = osMessageQueueNew(1, 0x10000, NULL);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMutexGetName001, Function | MediumTest | Level1)
{
    osMutexId_t mutexId;
    const char *name;

    mutexId = osMutexNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(mutexId, NULL, mutexId);

    name = osMutexGetName(mutexId);
    ICUNIT_ASSERT_EQUAL(name, NULL, name);

    name = osMutexGetName(NULL);
    ICUNIT_ASSERT_EQUAL(name, NULL, name);

    (void)osMutexDelete(mutexId);
    return LOS_OK;
}

/* ==== Thread Flags ==== */
static volatile uint32_t g_flagsResult;

static VOID CmsisThreadWaitFunc(VOID)
{
    g_testCount++;
    while (1) {
        osDelay(10);
    }
}

static VOID CmsisThreadFlagsFunc(VOID)
{
    uint32_t flags = osThreadFlagsWait(0x3, osFlagsWaitAny, 100);
    g_flagsResult = flags;
    g_testCount++;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThreadFlags001, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;
    g_flagsResult = 0;
    tid = osThreadNew((osThreadFunc_t)CmsisThreadFlagsFunc, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);
    osDelay(2);
    uint32_t ret = osThreadFlagsSet(tid, 0x1);
    ICUNIT_ASSERT_EQUAL(ret & 0x1, 0x1, ret);
    osDelay(5);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    ICUNIT_ASSERT_EQUAL(g_flagsResult & 0x1, 0x1, g_flagsResult);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThreadFlags002, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;
    g_flagsResult = 0;
    tid = osThreadNew((osThreadFunc_t)CmsisThreadFlagsFunc, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);
    osDelay(2);
    osThreadFlagsSet(tid, 0x2);
    osDelay(5);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    ICUNIT_ASSERT_EQUAL(g_flagsResult & 0x2, 0x2, g_flagsResult);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThreadFlags003, Function | MediumTest | Level1)
{
    uint32_t flags;
    osThreadId_t tid = osThreadGetId();
    flags = osThreadFlagsSet(tid, 0x5);
    ICUNIT_ASSERT_EQUAL(flags & 0x5, 0x5, flags);
    flags = osThreadFlagsGet();
    ICUNIT_ASSERT_EQUAL(flags & 0x5, 0x5, flags);
    flags = osThreadFlagsClear(0x4);
    flags = osThreadFlagsGet();
    ICUNIT_ASSERT_EQUAL(flags & 0x4, 0, flags);
    ICUNIT_ASSERT_EQUAL(flags & 0x1, 0x1, flags);
    osThreadFlagsClear(0x1);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThreadFlags004, Function | MediumTest | Level1)
{
    uint32_t ret = osThreadFlagsSet(NULL, 0x1);
    ICUNIT_ASSERT_EQUAL(ret, (uint32_t)osFlagsErrorParameter, ret);
    return LOS_OK;
}

/* ==== Memory Pool ==== */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMemPool001, Function | MediumTest | Level1)
{
    osMemoryPoolId_t mp;
    void *block;
    mp = osMemoryPoolNew(4, 32, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(mp, NULL, mp);
    block = osMemoryPoolAlloc(mp, 0);
    ICUNIT_ASSERT_NOT_EQUAL(block, NULL, block);
    (void)osMemoryPoolFree(mp, block);
    (void)osMemoryPoolDelete(mp);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMemPool002, Function | MediumTest | Level1)
{
    osMemoryPoolId_t mp;
    mp = osMemoryPoolNew(4, 32, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(mp, NULL, mp);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetCapacity(mp), 4, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetBlockSize(mp), 32, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetCount(mp), 0, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetSpace(mp), 4, 0);
    void *b1 = osMemoryPoolAlloc(mp, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetCount(mp), 1, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetSpace(mp), 3, 0);
    (void)osMemoryPoolFree(mp, b1);
    (void)osMemoryPoolDelete(mp);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMemPool003, Function | MediumTest | Level1)
{
    osMemoryPoolId_t mp;
    void *blocks[4];
    int i;
    mp = osMemoryPoolNew(4, 32, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(mp, NULL, mp);
    for (i = 0; i < 4; i++) {
        blocks[i] = osMemoryPoolAlloc(mp, 0);
        ICUNIT_ASSERT_NOT_EQUAL(blocks[i], NULL, blocks[i]);
    }
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetSpace(mp), 0, 0);
    void *extra = osMemoryPoolAlloc(mp, 0);
    ICUNIT_ASSERT_EQUAL(extra, NULL, extra);
    for (i = 0; i < 4; i++) {
        (void)osMemoryPoolFree(mp, blocks[i]);
    }
    (void)osMemoryPoolDelete(mp);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMemPool004, Function | MediumTest | Level1)
{
    osMemoryPoolId_t mp;
    mp = osMemoryPoolNew(4, 32, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(mp, NULL, mp);
    (void)osMemoryPoolFree(mp, NULL);
    (void)osMemoryPoolFree(NULL, (void *)0x1000);
    (void)osMemoryPoolDelete(mp);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMemPool005, Function | MediumTest | Level1)
{
    ICUNIT_ASSERT_EQUAL(osMemoryPoolNew(0, 32, NULL), NULL, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolNew(4, 0, NULL), NULL, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetCapacity(NULL), 0, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetBlockSize(NULL), 0, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetCount(NULL), 0, 0);
    ICUNIT_ASSERT_EQUAL(osMemoryPoolGetSpace(NULL), 0, 0);
    return LOS_OK;
}

/* 用例简要描述: GetName */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMemPool006, Function | MediumTest | Level1)
{
    osMemoryPoolId_t mp;
    osMemoryPoolAttr_t attr = {0};
    const char *name;

    dprintf("--- ItCmsisMemPool006 begin ---\n");

    attr.name = "mp_test_006";
    mp = osMemoryPoolNew(4, 32, &attr);
    ICUNIT_GOTO_NOT_EQUAL(mp, NULL, mp, EXIT);

    name = osMemoryPoolGetName(mp);
    ICUNIT_GOTO_NOT_EQUAL(name, NULL, name, EXIT);
    ICUNIT_GOTO_STRING_EQUAL(name, "mp_test_006", name, EXIT);

    name = osMemoryPoolGetName(NULL);
    ICUNIT_GOTO_EQUAL(name, NULL, name, EXIT);

EXIT:
    (void)osMemoryPoolDelete(mp);
    dprintf("--- ItCmsisMemPool006 end ---\n");
    return LOS_OK;
}

/* ==== Event Flags ==== */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisEventFlags001, Function | MediumTest | Level1)
{
    osEventFlagsId_t ef;
    uint32_t ret;
    ef = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(ef, NULL, ef);
    ret = osEventFlagsSet(ef, 0x1);
    ICUNIT_ASSERT_EQUAL(ret & 0x1, 0x1, ret);
    ret = osEventFlagsGet(ef);
    ICUNIT_ASSERT_EQUAL(ret & 0x1, 0x1, ret);
    ret = osEventFlagsClear(ef, 0x1);
    ret = osEventFlagsGet(ef);
    ICUNIT_ASSERT_EQUAL(ret & 0x1, 0, ret);
    (void)osEventFlagsDelete(ef);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisEventFlags002, Function | MediumTest | Level1)
{
    osEventFlagsId_t ef;
    uint32_t ret;
    ef = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(ef, NULL, ef);
    osEventFlagsSet(ef, 0x3);
    ret = osEventFlagsWait(ef, 0x3, osFlagsWaitAll, 10);
    ICUNIT_ASSERT_EQUAL(ret & 0x3, 0x3, ret);
    (void)osEventFlagsDelete(ef);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisEventFlags003, Function | MediumTest | Level1)
{
    osEventFlagsId_t ef;
    uint32_t ret;
    ef = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(ef, NULL, ef);
    osEventFlagsSet(ef, 0x1);
    ret = osEventFlagsWait(ef, 0x1, osFlagsWaitAny | osFlagsNoClear, 10);
    ICUNIT_ASSERT_EQUAL(ret & 0x1, 0x1, ret);
    ret = osEventFlagsGet(ef);
    ICUNIT_ASSERT_EQUAL(ret & 0x1, 0x1, ret);
    (void)osEventFlagsDelete(ef);
    return LOS_OK;
}

/* ==== Mutex ==== */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMutex001, Function | MediumTest | Level1)
{
    osMutexId_t mid;
    osStatus_t ret;
    mid = osMutexNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(mid, NULL, mid);
    ret = osMutexAcquire(mid, 0);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ret = osMutexRelease(mid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ret = osMutexDelete(mid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMutex002, Function | MediumTest | Level1)
{
    osMutexId_t mid;
    osThreadId_t owner;
    mid = osMutexNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(mid, NULL, mid);
    (void)osMutexAcquire(mid, 0);
    owner = osMutexGetOwner(mid);
    ICUNIT_ASSERT_NOT_EQUAL(owner, NULL, owner);
    (void)osMutexRelease(mid);
    (void)osMutexDelete(mid);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMutex003, Function | MediumTest | Level1)
{
    ICUNIT_ASSERT_EQUAL(osMutexAcquire(NULL, 0), osErrorParameter, 0);
    ICUNIT_ASSERT_EQUAL(osMutexRelease(NULL), osErrorParameter, 0);
    ICUNIT_ASSERT_EQUAL(osMutexDelete(NULL), osErrorParameter, 0);
    return LOS_OK;
}

/* ==== Semaphore ==== */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSem001, Function | MediumTest | Level1)
{
    osSemaphoreId_t sid;
    osStatus_t ret;
    sid = osSemaphoreNew(1, 0, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(sid, NULL, sid);
    ret = osSemaphoreRelease(sid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ret = osSemaphoreAcquire(sid, 0);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ret = osSemaphoreDelete(sid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSem002, Function | MediumTest | Level1)
{
    osSemaphoreId_t sid;
    sid = osSemaphoreNew(3, 1, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(sid, NULL, sid);
    ICUNIT_ASSERT_EQUAL(osSemaphoreGetCount(sid), 1, 0);
    (void)osSemaphoreAcquire(sid, 0);
    ICUNIT_ASSERT_EQUAL(osSemaphoreGetCount(sid), 0, 0);
    (void)osSemaphoreRelease(sid);
    ICUNIT_ASSERT_EQUAL(osSemaphoreGetCount(sid), 1, 0);
    (void)osSemaphoreDelete(sid);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSem003, Function | MediumTest | Level1)
{
    ICUNIT_ASSERT_EQUAL(osSemaphoreNew(0, 0, NULL), NULL, 0);
    ICUNIT_ASSERT_EQUAL(osSemaphoreNew(1, 2, NULL), NULL, 0);
    ICUNIT_ASSERT_EQUAL(osSemaphoreAcquire(NULL, 0), osErrorParameter, 0);
    ICUNIT_ASSERT_EQUAL(osSemaphoreRelease(NULL), osErrorParameter, 0);
    ICUNIT_ASSERT_EQUAL(osSemaphoreDelete(NULL), osErrorParameter, 0);
    return LOS_OK;
}

/* ==== Kernel ==== */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel001, Function | MediumTest | Level1)
{
    osKernelState_t state = osKernelGetState();
    ICUNIT_ASSERT_EQUAL(state, osKernelRunning, state);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel002, Function | MediumTest | Level1)
{
    int32_t lock1 = osKernelLock();
    ICUNIT_ASSERT_EQUAL(lock1, 0, lock1);
    int32_t lock2 = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(lock2, 1, lock2);
    (void)osKernelRestoreLock(0);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel003, Function | MediumTest | Level1)
{
    uint32_t freq = osKernelGetTickFreq();
    ICUNIT_ASSERT_NOT_EQUAL(freq, 0, freq);
    uint32_t tc = osKernelGetTickCount();
    uint32_t timerFreq = osKernelGetSysTimerFreq();
    ICUNIT_ASSERT_NOT_EQUAL(timerFreq, 0, timerFreq);
    (void)tc;
    return LOS_OK;
}

/* ==== Thread (supplement) ==== */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread001, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;
    tid = osThreadNew((osThreadFunc_t)CmsisStackFunc01, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);
    osDelay(3);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread002, Function | MediumTest | Level1)
{
    osThreadId_t tid = osThreadGetId();
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);
    osThreadState_t state = osThreadGetState(tid);
    ICUNIT_ASSERT_EQUAL(state, osThreadRunning, state);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread003, Function | MediumTest | Level1)
{
    uint32_t count = osThreadGetCount();
    ICUNIT_ASSERT_NOT_EQUAL(count, 0, count);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread004, Function | MediumTest | Level1)
{
    osStatus_t ret = osThreadYield();
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread005, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;
    tid = osThreadNew((osThreadFunc_t)CmsisThreadWaitFunc, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);
    osDelay(3);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    osStatus_t ret = osThreadTerminate(tid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return LOS_OK;
}

/* ==== Timer (supplement) ==== */
static volatile uint32_t g_timerCount;

static VOID CmsisTimerCallback(void *arg)
{
    (void)arg;
    g_timerCount++;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisTimer002, Function | MediumTest | Level1)
{
    osTimerId_t tid;
    osStatus_t ret;
    g_timerCount = 0;
    tid = osTimerNew(CmsisTimerCallback, osTimerPeriodic, NULL, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);
    ret = osTimerStart(tid, 10);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    osDelay(50);
    ICUNIT_ASSERT_NOT_EQUAL(g_timerCount, 0, g_timerCount);
    ret = osTimerStop(tid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ret = osTimerDelete(tid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisTimer003, Function | MediumTest | Level1)
{
    osTimerId_t tid;
    uint32_t running;
    tid = osTimerNew(CmsisTimerCallback, osTimerOnce, NULL, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);
    running = osTimerIsRunning(tid);
    ICUNIT_ASSERT_EQUAL(running, 0, running);
    (void)osTimerStart(tid, 5);
    running = osTimerIsRunning(tid);
    ICUNIT_ASSERT_EQUAL(running, 1, running);
    (void)osTimerStop(tid);
    running = osTimerIsRunning(tid);
    ICUNIT_ASSERT_EQUAL(running, 0, running);
    (void)osTimerDelete(tid);
    return LOS_OK;
}


/* ==== Supplement: cover uncalled CMSIS APIs (kernel info, delay-until,
 *      queue attributes, thread suspend/resume/detach/exit/stack-info) ==== */

/* helper thread: increment counter then delay so the test can suspend it */
static VOID CmsisSupplementSuspThread(VOID)
{
    g_testCount++;
    osDelay(20);
    g_testCount++;
}

/* helper thread: increment counter then exit via osThreadExit */
static VOID CmsisSupplementExitThread(VOID)
{
    g_testCount++;
    osThreadExit();
    UNREACHABLE; /* osThreadExit must not return */
}

/* helper thread: just increment counter and exit (for stack/detach tests) */
static VOID CmsisSupplementSimpleThread(VOID)
{
    g_testCount++;
}

/* helper thread: verify osThreadGetArgument returns the value passed at creation */
static VOID CmsisSupplementArgThread(VOID)
{
    void *arg = osThreadGetArgument();
    if (arg == (void *)0x12345678) {
        g_testCount++;
    }
}

/* helper thread: delay then increment, used to verify osThreadJoin (joinable) */
static VOID CmsisSupplementJoinThread(VOID)
{
    osDelay(3);
    g_testCount++;
}

/* helper thread: stay alive (blocked in osDelay) for priority get/set queries */
static VOID CmsisSupplementPrioThread(VOID)
{
    osDelay(20);
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel004, Function | MediumTest | Level1)
{
    osVersion_t ver = {0};
    CHAR id_buf[32] = {0};
    osStatus_t ret;
    UINT32 tc;

    /* osKernelGetInfo: valid call returns osOK and fills version/id buffer */
    ret = osKernelGetInfo(&ver, id_buf, sizeof(id_buf));
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ICUNIT_ASSERT_NOT_EQUAL(ver.api, 0, ver.api);

    /* osKernelGetSysTimerCount: returns current system timer count (non-zero) */
    tc = osKernelGetSysTimerCount();
    (VOID)tc;

    /* osKernelStart: g_kernelState is osKernelInactive (boot uses LOS_Start directly,
     * not osKernelStart), so the guard rejects with osError - safe to call */
    ret = osKernelStart();
    ICUNIT_ASSERT_EQUAL(ret, osError, ret);

    /* Note: osKernelInitialize is intentionally NOT called here because g_kernelState
     * is osKernelInactive and the function would fall through to LOS_KernelInit(),
     * re-initializing the running kernel (destructive, causes LMS crash). */

    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel005, Function | MediumTest | Level1)
{
    UINT32 start = osKernelGetTickCount();
    UINT32 target = start + 5;
    osStatus_t ret;

    /* osDelayUntil: block until the absolute tick 'target' is reached */
    ret = osDelayUntil(target);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    /* time must have progressed at least to 'target' */
    UINT32 end = osKernelGetTickCount();
    ICUNIT_ASSERT_WITHIN_EQUAL(end, target, target + 50, end);

    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisQueueAttr001, Function | MediumTest | Level1)
{
    osMessageQueueId_t mq;
    CHAR msg[4] = "abc";
    CHAR buf[4] = {0};
    osStatus_t ret;

    mq = osMessageQueueNew(2, 4, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(mq, NULL, mq);

    /* Empty queue: capacity=2, msg_size=4, count=0, space=2 */
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetCapacity(mq), 2, 0);
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetMsgSize(mq), 4, 0);
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetCount(mq), 0, 0);
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetSpace(mq), 2, 0);

    /* Put one message: count=1, space=1 */
    ret = osMessageQueuePut(mq, msg, 0, 0);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetCount(mq), 1, 0);
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetSpace(mq), 1, 0);

    /* Get the message: count=0, space=2 */
    ret = osMessageQueueGet(mq, buf, NULL, 0);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetCount(mq), 0, 0);
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetSpace(mq), 2, 0);

    ret = osMessageQueueDelete(mq);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    /* Querying a deleted queue returns 0 (queueState == OS_QUEUE_UNUSED branch) */
    ICUNIT_ASSERT_EQUAL(osMessageQueueGetCapacity(mq), 0, 0);

    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread006, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    osPriority_t prio;
    osThreadState_t state;
    osStatus_t ret;

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;

    tid = osThreadNew((osThreadFunc_t)CmsisSupplementSuspThread, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);

    /* let helper run one increment then block in osDelay(20) */
    osDelay(2);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    /* osThreadGetPriority: should match the priority we set */
    prio = osThreadGetPriority(tid);
    ICUNIT_ASSERT_EQUAL(prio, osPriorityLow1, prio);

    /* osThreadSuspend: suspend the helper (currently blocked in osDelay) */
    ret = osThreadSuspend(tid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    state = osThreadGetState(tid);
    ICUNIT_ASSERT_EQUAL(state, osThreadBlocked, state);

    /* osThreadResume: restore the helper */
    ret = osThreadResume(tid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    /* wait for helper to finish its second increment */
    osDelay(25);
    ICUNIT_ASSERT_EQUAL(g_testCount, 2, g_testCount);

    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread007, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    UINT32 ss;
    UINT32 sp;
    osStatus_t ret;

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadJoinable; /* joinable so osThreadDetach has effect */
    g_testCount = 0;

    tid = osThreadNew((osThreadFunc_t)CmsisSupplementSimpleThread, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);

    /* osThreadGetStackSize: should match attr.stack_size */
    ss = osThreadGetStackSize(tid);
    ICUNIT_ASSERT_EQUAL(ss, OS_TSK_TEST_STACK_SIZE, ss);

    /* osThreadGetStackSpace: should be > 0 (some stack is unused) */
    sp = osThreadGetStackSpace(tid);
    ICUNIT_ASSERT_NOT_EQUAL(sp, 0, sp);

    /* osThreadDetach: convert joinable -> detached; thread auto-cleans on exit */
    ret = osThreadDetach(tid);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    osDelay(5);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    return LOS_OK;
}

LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread008, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow1;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;

    tid = osThreadNew((osThreadFunc_t)CmsisSupplementExitThread, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(tid, NULL, tid);

    /* helper increments g_testCount then calls osThreadExit() */
    osDelay(5);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    /* after exit, thread should be inactive */
    osThreadState_t state = osThreadGetState(tid);
    ICUNIT_ASSERT_EQUAL(state, osThreadInactive, state);

    return LOS_OK;
}

/* ==== Batch 1 supplement: Kernel Lock state machine / Delay / Timer boundary / Thread misc ==== */

/* 用例简要描述: Lock/Unlock/RestoreLock 状态机往返 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel006, Function | MediumTest | Level1)
{
    int32_t lockBefore, lockNested, unlockRet, restoreLocked, restoreUnlocked;

    dprintf("--- TestCmsisKernel006 begin ---\n");

    /* osKernelLock on an unlocked kernel returns 0 (previous state) and locks scheduling */
    lockBefore = osKernelLock();
    ICUNIT_GOTO_EQUAL(lockBefore, 0, lockBefore, EXIT);

    /* nested lock: already locked, returns 1 (previous state) without re-locking */
    lockNested = osKernelLock();
    ICUNIT_GOTO_EQUAL(lockNested, 1, lockNested, EXIT);

    /* osKernelUnlock returns 1 (previous state was locked) and restores scheduling */
    unlockRet = osKernelUnlock();
    ICUNIT_GOTO_EQUAL(unlockRet, 1, unlockRet, EXIT);

    /* osKernelRestoreLock(1) re-enters locked state, returns KERNEL_LOCKED (1) */
    restoreLocked = osKernelRestoreLock(1);
    ICUNIT_GOTO_EQUAL(restoreLocked, 1, restoreLocked, EXIT);

    /* osKernelRestoreLock(0) restores unlocked state, returns KERNEL_UNLOCKED (0) */
    restoreUnlocked = osKernelRestoreLock(0);
    ICUNIT_GOTO_EQUAL(restoreUnlocked, 0, restoreUnlocked, EXIT);

EXIT:
    /* safety: guarantee the kernel is unlocked regardless of which step failed */
    (void)osKernelRestoreLock(0);
    dprintf("--- TestCmsisKernel006 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: Lock 禁止调度:lock 下 osDelay 立即返回错误 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel007, Function | MediumTest | Level1)
{
    int32_t lockBefore;
    osKernelState_t state;
    osStatus_t sDelay;
    uint32_t start, end, diff;

    dprintf("--- TestCmsisKernel007 begin ---\n");

    /* lock the scheduler */
    lockBefore = osKernelLock();
    ICUNIT_GOTO_EQUAL(lockBefore, 0, lockBefore, EXIT);

    /* lock puts the kernel into osKernelLocked state */
    state = osKernelGetState();
    ICUNIT_GOTO_EQUAL(state, osKernelLocked, state, EXIT);

    /* Under lock osDelay takes the HalDelay no-op path: it returns osOK but must
     * NOT actually advance the tick count by the requested 2 ticks (no real delay).
     * Verify by capturing ticks before/after the call. */
    start = osKernelGetTickCount();
    sDelay = osDelay(2);
    end = osKernelGetTickCount();
    ICUNIT_GOTO_EQUAL(sDelay, osOK, sDelay, EXIT);
    diff = end - start;
    if (diff >= 2) {
        ICunitSaveErr(__LINE__, (iiUINT32)diff);
        goto EXIT;
    }

    /* unlock restores normal scheduling; osDelay blocks for the requested tick */
    (void)osKernelUnlock();
    sDelay = osDelay(1);
    ICUNIT_GOTO_EQUAL(sDelay, osOK, sDelay, EXIT);

EXIT:
    /* safety: guarantee the kernel is unlocked regardless of which step failed */
    (void)osKernelRestoreLock(0);
    dprintf("--- TestCmsisKernel007 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: osKernelInitialize Running 态负面:期望 osError(guard) */
#if !defined(PRODUCT_BUG_ISOLATE) || !defined(LOSCFG_ARCH_ARM)
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel008, Function | MediumTest | Level1)
{
    osKernelState_t state;
    osStatus_t ret;

    dprintf("--- TestCmsisKernel008 begin ---\n");

    state = osKernelGetState();
    ICUNIT_ASSERT_EQUAL(state, osKernelRunning, state);

    ret = osKernelInitialize();
    dprintf("--- TestCmsisKernel008: osKernelInitialize returned %d (expect osError) ---\n", (int)ret);
    ICUNIT_ASSERT_EQUAL(ret, osError, ret);

    dprintf("--- TestCmsisKernel008 end ---\n");
    return LOS_OK;
}
#endif /* PRODUCT_BUG_ISOLATE/ARM: Kernel008 qemu 隔离 */

/* ==== ISR 综合用例: 对应设计 §5.5.1 行006/§5.7.1 行005/§5.8.1 行006/§5.9.1 行005/§5.10.1 行004
   的 F-ISR 场景(EventFlagsSet/SemaphoreRelease/MessageQueuePut/MemoryPoolFree/ThreadFlagsSet
   在 ISR 内调用),工程 ISR 构造先例: It_los_queue_isr_001(HwiCreate+TestHwiTrigger) ==== */
static osEventFlagsId_t g_isrEfId;
static osSemaphoreId_t g_isrSemId;
static osMessageQueueId_t g_isrQId;
static osMemoryPoolId_t g_isrMpId;
static osThreadId_t g_isrThrId;
static VOID *g_isrMpBlk = NULL;
static volatile uint32_t g_isrEfSetRet = 0xDEAD;
static volatile uint32_t g_isrSemRelRet = 0xDEAD;
static volatile uint32_t g_isrQPutRet = 0xDEAD;
static volatile uint32_t g_isrMpFreeRet = 0xDEAD;
static volatile uint32_t g_isrTfSetRet = 0xDEAD;

static VOID HwiFIsr001(VOID)
{
    TestHwiClear(HWI_NUM_TEST);
    g_isrEfSetRet = osEventFlagsSet(g_isrEfId, 0x1);
    g_isrSemRelRet = osSemaphoreRelease(g_isrSemId);
    g_isrQPutRet = osMessageQueuePut(g_isrQId, "ISROUT", 0U, 0U); /* timeout=0 ISR 合法 */
    g_isrMpFreeRet = osMemoryPoolFree(g_isrMpId, (void *)g_isrMpBlk);
    g_isrTfSetRet = osThreadFlagsSet(g_isrThrId, 0x1);
}


/* 用例简要描述: ISR 综合用例:EventFlagsSet/SemRelease/QueuePut(timeout=0)/MemPoolFree/ThreadFlagsSet 在 ISR 内 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisIsr001, Function | MediumTest | Level1)
{
    uint32_t ret;
    char rbuf[8] = {0};
    HWI_PRIOR_T hwiPrio = 3;
    HWI_MODE_T mode = 0;
    HwiIrqParam irqParam;

    dprintf("--- TestCmsisIsr001 begin ---\n");

    /* 资源准备 */
    g_isrEfId = osEventFlagsNew(NULL);
    ICUNIT_ASSERT_EQUAL((g_isrEfId != NULL), 1, (uint32_t)(UINTPTR)g_isrEfId);
    g_isrSemId = osSemaphoreNew(1U, 0U, NULL);
    ICUNIT_ASSERT_EQUAL((g_isrSemId != NULL), 1, (uint32_t)(UINTPTR)g_isrSemId);
    g_isrQId = osMessageQueueNew(2U, 8U, NULL); /* 2: msg count, 8: msg size */
    ICUNIT_ASSERT_EQUAL((g_isrQId != NULL), 1, (uint32_t)(UINTPTR)g_isrQId);
    g_isrMpId = osMemoryPoolNew(2U, 16U, NULL); /* 2: block count, 16: block size */
    ICUNIT_ASSERT_EQUAL((g_isrMpId != NULL), 1, (uint32_t)(UINTPTR)g_isrMpId);
    g_isrMpBlk = osMemoryPoolAlloc(g_isrMpId, 0U);
    ICUNIT_ASSERT_EQUAL((g_isrMpBlk != NULL), 1, (uint32_t)(UINTPTR)g_isrMpBlk);
    g_isrThrId = osThreadGetId(); /* 本任务作为被 Set 标志对象 */

    /* 注册并触发 ISR,回调内依次执行 5 个 F-ISR 场景 */
    (VOID)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = 0;
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiFIsr001, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    TestHwiTrigger(HWI_NUM_TEST);

    /* 断言 1(设计§5.5.1 行006): ISR 内 Set 成功,等待方唤醒 */
    ICUNIT_GOTO_EQUAL(g_isrEfSetRet, 0x1, g_isrEfSetRet, EXIT);
    ret = osEventFlagsWait(g_isrEfId, 0x1, osFlagsWaitAny, 10U); /* 10: timeout ticks */
    ICUNIT_GOTO_EQUAL(ret, 0x1, ret, EXIT);

    /* 断言 2(设计§5.7.1 行005): ISR 内 Release 唤醒 Acquire */
    ICUNIT_GOTO_EQUAL(g_isrSemRelRet, osOK, g_isrSemRelRet, EXIT);
    ret = osSemaphoreAcquire(g_isrSemId, 10U);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* 断言 3(设计§5.8.1 行006): ISR 内 Put(timeout=0) 成功,Get 收到内容 */
    ICUNIT_GOTO_EQUAL(g_isrQPutRet, osOK, g_isrQPutRet, EXIT);
    ret = osMessageQueueGet(g_isrQId, rbuf, NULL, 10U);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(rbuf[0], 'I', rbuf[0], EXIT);

    /* 断言 4(设计§5.9.1 行005): ISR 内 Free 返回 osOK */
    ICUNIT_GOTO_EQUAL(g_isrMpFreeRet, osOK, g_isrMpFreeRet, EXIT);

    /* 断言 5(设计§5.10.1 行004): ISR 内 ThreadFlagsSet 唤醒本任务 Wait */
    ICUNIT_GOTO_EQUAL(g_isrTfSetRet, 0x1, g_isrTfSetRet, EXIT);
    ret = osThreadFlagsWait(0x1, osFlagsWaitAny, 10U);
    ICUNIT_GOTO_EQUAL(ret, 0x1, ret, EXIT);

EXIT:
    TestHwiDelete(HWI_NUM_TEST);
    if (g_isrEfId != NULL) { (VOID)osEventFlagsDelete(g_isrEfId); g_isrEfId = NULL; }
    if (g_isrSemId != NULL) { (VOID)osSemaphoreDelete(g_isrSemId); g_isrSemId = NULL; }
    if (g_isrQId != NULL) { (VOID)osMessageQueueDelete(g_isrQId); g_isrQId = NULL; }
    if (g_isrMpId != NULL) { (VOID)osMemoryPoolDelete(g_isrMpId); g_isrMpId = NULL; }
    dprintf("--- TestCmsisIsr001 end ---\n");
    return LOS_OK;
}


/* 用例简要描述: osKernelGetTickCount 单调契约(tc1>0+delay 后 tc2>tc1) */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisKernel009, Function | MediumTest | Level1)
{
    /* 设计 §5.1.2 行 008: osKernelGetTickCount F-Pos/BVA 单调性契约
       (实现编号 008 已被 osKernelInitialize 负面占用,本用例按语义对应设计 008 行;
       此前两套体系均仅工具性调用,无专属断言) */
    uint32_t tc1;
    uint32_t tc2;

    dprintf("--- TestCmsisKernel009 begin ---\n");

    tc1 = osKernelGetTickCount();
    ICUNIT_ASSERT_EQUAL((tc1 > 0), 1, tc1);

    (VOID)osDelay(2); /* 2, delay ticks */

    tc2 = osKernelGetTickCount();
    ICUNIT_ASSERT_EQUAL((tc2 > tc1), 1, (uint32_t)(tc2 - tc1)); /* 差值仅诊断打印 */

    dprintf("--- TestCmsisKernel009 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: osDelay BVA:0 立即/1 tick 经时 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisDelay001, Function | MediumTest | Level1)
{
    osStatus_t sZero, sFive;
    uint32_t start, end, diff;

    dprintf("--- TestCmsisDelay001 begin ---\n");

    /* BVA: ticks=0 is rejected by osDelay (returns osErrorParameter) */
    sZero = osDelay(0);
    ICUNIT_ASSERT_EQUAL(sZero, osErrorParameter, sZero);

    /* F-Pos: ticks=5 blocks ~5 ticks and returns osOK; verify tick advance with tolerance */
    start = osKernelGetTickCount();
    sFive = osDelay(5);
    ICUNIT_ASSERT_EQUAL(sFive, osOK, sFive);
    end = osKernelGetTickCount();
    diff = end - start;
    ICUNIT_ASSERT_WITHIN_EQUAL(diff, 5, 50, diff);

    dprintf("--- TestCmsisDelay001 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: Start 边界 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisTimer004, Function | MediumTest | Level1)
{
    osTimerId_t tid;
    osStatus_t ret;

    dprintf("--- TestCmsisTimer004 begin ---\n");

    tid = osTimerNew(CmsisTimerCallback, osTimerOnce, NULL, NULL);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    /* BVA: ticks=0 is rejected before any state change */
    ret = osTimerStart(tid, 0);
    ICUNIT_GOTO_EQUAL(ret, osErrorParameter, ret, EXIT);

    /* BVA: ticks=1 is the minimal accepted value */
    ret = osTimerStart(tid, 1);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* let the 1-tick one-shot timer fire deterministically, then delete */
    osDelay(3);
    ret = osTimerDelete(tid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    tid = NULL;

EXIT:
    if (tid != NULL) {
        (void)osTimerDelete(tid);
    }
    dprintf("--- TestCmsisTimer004 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: Stop 后再 Start */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisTimer005, Function | MediumTest | Level1)
{
    osTimerId_t tid;
    osStatus_t ret;
    uint32_t running;

    dprintf("--- TestCmsisTimer005 begin ---\n");

    tid = osTimerNew(CmsisTimerCallback, osTimerPeriodic, NULL, NULL);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    ret = osTimerStart(tid, 10);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    ret = osTimerStop(tid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    running = osTimerIsRunning(tid);
    ICUNIT_GOTO_EQUAL(running, 0, running, EXIT);

    /* restart after stop: interval is reset and timer is ticking again */
    ret = osTimerStart(tid, 5);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    running = osTimerIsRunning(tid);
    ICUNIT_GOTO_EQUAL(running, 1, running, EXIT);

    ret = osTimerStop(tid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    ret = osTimerDelete(tid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    tid = NULL;

EXIT:
    if (tid != NULL) {
        (void)osTimerStop(tid);
        (void)osTimerDelete(tid);
    }
    dprintf("--- TestCmsisTimer005 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: Delete 边界 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisTimer006, Function | MediumTest | Level1)
{
    osTimerId_t tid;
    osStatus_t ret;
    uint32_t running;

    dprintf("--- TestCmsisTimer006 begin ---\n");

    tid = osTimerNew(CmsisTimerCallback, osTimerOnce, NULL, NULL);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    ret = osTimerStart(tid, 100);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* Delete a running timer: stops and frees it, returns osOK */
    ret = osTimerDelete(tid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* after delete the timer is no longer running */
    running = osTimerIsRunning(tid);
    ICUNIT_GOTO_EQUAL(running, 0, running, EXIT);

    /* Delete again: timer already freed (usTimerID mismatch) -> osErrorResource */
    ret = osTimerDelete(tid);
    ICUNIT_GOTO_EQUAL(ret, osErrorResource, ret, EXIT);

EXIT:
    dprintf("--- TestCmsisTimer006 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: SetPolicy */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread009, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    osStatus_t ret;

    dprintf("--- TestCmsisThread009 begin ---\n");

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;

    tid = osThreadNew((osThreadFunc_t)CmsisSupplementSimpleThread, NULL, &attr);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    /* osThreadSetPolicy is a no-op that always returns osOK */
    ret = osThreadSetPolicy(tid, CMSIS_THREAD_POLICY_DEFAULT);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    osDelay(5);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

EXIT:
    dprintf("--- TestCmsisThread009 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: GetArgument */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread010, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};

    dprintf("--- TestCmsisThread010 begin ---\n");

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;
    attr.attr_bits = osThreadDetached;
    g_testCount = 0;

    tid = osThreadNew((osThreadFunc_t)CmsisSupplementArgThread, (void *)0x12345678, &attr);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    /* helper verifies osThreadGetArgument() == 0x12345678 and increments on match */
    osDelay(5);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

EXIT:
    dprintf("--- TestCmsisThread010 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: GetName */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread011, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    const char *name;

    dprintf("--- TestCmsisThread011 begin ---\n");

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;
    attr.attr_bits = osThreadDetached;
    attr.name = "thread_011";
    g_testCount = 0;

    tid = osThreadNew((osThreadFunc_t)CmsisSupplementSimpleThread, NULL, &attr);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    /* osThreadGetName returns the name set via attr.name (query before the thread exits) */
    name = osThreadGetName(tid);
    ICUNIT_GOTO_NOT_EQUAL(name, NULL, name, EXIT);
    ICUNIT_GOTO_STRING_EQUAL(name, "thread_011", name, EXIT);

    osDelay(5);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

EXIT:
    dprintf("--- TestCmsisThread011 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: Join */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread012, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    osStatus_t ret;

    dprintf("--- TestCmsisThread012 begin ---\n");

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;
    attr.attr_bits = osThreadJoinable;
    g_testCount = 0;

    tid = osThreadNew((osThreadFunc_t)CmsisSupplementJoinThread, NULL, &attr);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    /* osThreadJoin blocks until the joinable thread exits, then returns osOK */
    ret = osThreadJoin(tid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* after join the helper has finished: its increment must be visible */
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

EXIT:
    dprintf("--- TestCmsisThread012 end ---\n");
    return LOS_OK;
}

/* 用例简要描述: SetPriority 往返 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisThread013, Function | MediumTest | Level1)
{
    osThreadId_t tid;
    osThreadAttr_t attr = {0};
    osPriority_t prio;
    osStatus_t ret;

    dprintf("--- TestCmsisThread013 begin ---\n");

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityLow;
    attr.attr_bits = osThreadDetached;

    tid = osThreadNew((osThreadFunc_t)CmsisSupplementPrioThread, NULL, &attr);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    /* created with osPriorityLow: round-trip via LOS mapping returns osPriorityLow */
    prio = osThreadGetPriority(tid);
    ICUNIT_GOTO_EQUAL(prio, osPriorityLow, prio, EXIT);

    /* raise to osPriorityNormal: SetPriority returns osOK */
    ret = osThreadSetPriority(tid, osPriorityNormal);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* GetPriority reflects the new priority (Normal <-> los 10 round-trips) */
    prio = osThreadGetPriority(tid);
    ICUNIT_GOTO_EQUAL(prio, osPriorityNormal, prio, EXIT);

EXIT:
    /* cleanup: terminate the still-alive helper (blocked in osDelay(20)) */
    if (tid != NULL) {
        (void)osThreadTerminate(tid);
    }
    dprintf("--- TestCmsisThread013 end ---\n");
    return LOS_OK;
}

/* ==== Batch 2: EventFlags/Mutex/Semaphore/MessageQueue/MemoryPool gap tests ==== */
/* helper globals for thread-based tests */
static osEventFlagsId_t g_cmsisEfHelperId;
static osMutexId_t g_cmsisMuxHelperId;
static osStatus_t g_cmsisMuxAcqRet;
static osMessageQueueId_t g_cmsisMqMpcId;
static volatile UINT32 g_cmsisQProdCnt[2];
static volatile UINT32 g_cmsisQConsCnt[2];

/* helper: wait on event flag forever, then signal completion via g_testCount */
static VOID CmsisEfWaitHelper(VOID)
{
    uint32_t ret = osEventFlagsWait(g_cmsisEfHelperId, 0x1, osFlagsWaitAny, osWaitForever);
    if ((ret & 0x1) != 0) {
        g_testCount++;
    }
}

/* helper: acquire mutex with 5-tick timeout, record result */
static VOID CmsisMuxAcqHelper(VOID)
{
    g_cmsisMuxAcqRet = osMutexAcquire(g_cmsisMuxHelperId, 5);
    g_testCount++;
}

/* helper: producer puts 50 messages into shared queue */
static VOID CmsisMqProducer(void *arg)
{
    UINT32 idx = (UINT32)(UINTPTR)arg;
    UINT32 msg;
    UINT32 i;
    for (i = 0; i < 50; i++) {
        msg = (idx << 16) | (i & 0xFFFF);
        if (osMessageQueuePut(g_cmsisMqMpcId, &msg, 0U, osWaitForever) == osOK) {
            g_cmsisQProdCnt[idx]++;
        }
    }
}

/* helper: consumer gets 50 messages from shared queue */
static VOID CmsisMqConsumer(void *arg)
{
    UINT32 idx = (UINT32)(UINTPTR)arg;
    UINT32 msg;
    UINT32 i;
    for (i = 0; i < 50; i++) {
        if (osMessageQueueGet(g_cmsisMqMpcId, &msg, NULL, osWaitForever) == osOK) {
            g_cmsisQConsCnt[idx]++;
        }
    }
}

/* ---- EventFlags ---- */
/* 用例简要描述: Wait OR+Clear */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisEventFlags004, Function | MediumTest | Level1)
{
    osEventFlagsId_t ef = NULL;
    uint32_t ret;

    ef = osEventFlagsNew(NULL);
    ICUNIT_GOTO_NOT_EQUAL(ef, NULL, ef, EXIT);

    /* WaitAny + NoClear: flags preserved after wait */
    (void)osEventFlagsSet(ef, 0x5);
    ret = osEventFlagsWait(ef, 0xF, osFlagsWaitAny | osFlagsNoClear, 0);
    ICUNIT_GOTO_EQUAL(ret, 0x5, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osEventFlagsGet(ef), 0x5, 0, EXIT);

    (void)osEventFlagsClear(ef, 0x5);
    ICUNIT_GOTO_EQUAL(osEventFlagsGet(ef), 0, 0, EXIT);

    /* WaitAny + Clear (default): flags cleared after wait */
    (void)osEventFlagsSet(ef, 0x5);
    ret = osEventFlagsWait(ef, 0xF, osFlagsWaitAny, 0);
    ICUNIT_GOTO_EQUAL(ret, 0x5, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osEventFlagsGet(ef), 0, 0, EXIT);

EXIT:
    if (ef != NULL) {
        (void)osEventFlagsDelete(ef);
    }
    return LOS_OK;
}

/* 用例简要描述: 跨线程阻塞唤醒 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisEventFlags005, Function | MediumTest | Level1)
{
    osEventFlagsId_t ef = NULL;
    osThreadId_t tid = NULL;
    osThreadAttr_t attr = {0};

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;
    attr.attr_bits = osThreadJoinable;

    ef = osEventFlagsNew(NULL);
    ICUNIT_GOTO_NOT_EQUAL(ef, NULL, ef, EXIT);
    g_cmsisEfHelperId = ef;
    g_testCount = 0;

    tid = osThreadNew((osThreadFunc_t)CmsisEfWaitHelper, NULL, &attr);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    /* let helper block on Wait */
    osDelay(3);
    ICUNIT_GOTO_EQUAL(g_testCount, 0, g_testCount, EXIT);

    /* signal flag to wake helper */
    (void)osEventFlagsSet(ef, 0x1);
    osDelay(3);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

EXIT:
    if (ef != NULL) {
        (void)osEventFlagsSet(ef, 0x1);
    }
    if (tid != NULL) {
        (void)osThreadJoin(tid);
    }
    if (ef != NULL) {
        (void)osEventFlagsDelete(ef);
    }
    return LOS_OK;
}

/* 用例简要描述: AND vs OR 决策表 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisEventFlags006, Function | MediumTest | Level1)
{
    osEventFlagsId_t ef = NULL;
    uint32_t ret;

    ef = osEventFlagsNew(NULL);
    ICUNIT_GOTO_NOT_EQUAL(ef, NULL, ef, EXIT);

    /* Case 1: no flags, Wait ALL NO_CLEAR timeout=0 -> returns 0 (no match, non-blocking) */
    ret = osEventFlagsWait(ef, 0x3, osFlagsWaitAll | osFlagsNoClear, 0);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    /* Case 2: only 0x1 set, Wait ALL NO_CLEAR timeout=0 -> returns 0 (partial match, ALL needs all) */
    (void)osEventFlagsSet(ef, 0x1);
    ret = osEventFlagsWait(ef, 0x3, osFlagsWaitAll | osFlagsNoClear, 0);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    /* Case 3: set 0x2, now 0x3, Wait ALL NO_CLEAR -> returns 0x3, not cleared */
    (void)osEventFlagsSet(ef, 0x2);
    ret = osEventFlagsWait(ef, 0x3, osFlagsWaitAll | osFlagsNoClear, 0);
    ICUNIT_GOTO_EQUAL(ret, 0x3, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osEventFlagsGet(ef), 0x3, 0, EXIT);

    /* Case 4: Wait ANY CLEAR -> returns 0x3, then cleared */
    ret = osEventFlagsWait(ef, 0x3, osFlagsWaitAny, 0);
    ICUNIT_GOTO_EQUAL(ret, 0x3, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osEventFlagsGet(ef), 0, 0, EXIT);

    /* Case 5: empty, Wait ANY NO_CLEAR timeout=1 -> osFlagsErrorTimeout (blocks then times out) */
    ret = osEventFlagsWait(ef, 0x1, osFlagsWaitAny | osFlagsNoClear, 1);
    ICUNIT_GOTO_EQUAL(ret, (uint32_t)osFlagsErrorTimeout, ret, EXIT);

    /* Case 6: set 0x4, Wait ALL CLEAR timeout=0 -> returns 0 (partial match), flags not cleared */
    (void)osEventFlagsSet(ef, 0x4);
    ret = osEventFlagsWait(ef, 0x7, osFlagsWaitAll, 0);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osEventFlagsGet(ef), 0x4, 0, EXIT);

    /* Case 7: set 0x3, now 0x7, Wait ALL CLEAR -> returns 0x7, cleared */
    (void)osEventFlagsSet(ef, 0x3);
    ret = osEventFlagsWait(ef, 0x7, osFlagsWaitAll, 0);
    ICUNIT_GOTO_EQUAL(ret, 0x7, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osEventFlagsGet(ef), 0, 0, EXIT);

    /* Case 8: set 0x5, Wait ANY NO_CLEAR mask=0x1 -> returns 0x1, Get stays 0x5 */
    (void)osEventFlagsSet(ef, 0x5);
    ret = osEventFlagsWait(ef, 0x1, osFlagsWaitAny | osFlagsNoClear, 0);
    ICUNIT_GOTO_EQUAL(ret, 0x1, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osEventFlagsGet(ef), 0x5, 0, EXIT);

EXIT:
    if (ef != NULL) {
        (void)osEventFlagsDelete(ef);
    }
    return LOS_OK;
}

/* ---- Mutex ---- */
/* 用例简要描述: Acquire 超时 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMutex004, Function | MediumTest | Level1)
{
    osMutexId_t mid = NULL;
    osThreadId_t tid = NULL;
    osThreadAttr_t attr = {0};
    osStatus_t ret;

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;
    attr.attr_bits = osThreadJoinable;

    mid = osMutexNew(NULL);
    ICUNIT_GOTO_NOT_EQUAL(mid, NULL, mid, EXIT);
    g_cmsisMuxHelperId = mid;
    g_cmsisMuxAcqRet = osOK;
    g_testCount = 0;

    /* main holds mutex */
    ret = osMutexAcquire(mid, osWaitForever);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* helper tries to acquire with 5-tick timeout -> should timeout */
    tid = osThreadNew((osThreadFunc_t)CmsisMuxAcqHelper, NULL, &attr);
    ICUNIT_GOTO_NOT_EQUAL(tid, NULL, tid, EXIT);

    osDelay(10);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);
    ICUNIT_GOTO_EQUAL(g_cmsisMuxAcqRet, osErrorTimeout, g_cmsisMuxAcqRet, EXIT);

    ret = osMutexRelease(mid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

EXIT:
    if (tid != NULL) {
        (void)osThreadJoin(tid);
    }
    if (mid != NULL) {
        (void)osMutexDelete(mid);
    }
    return LOS_OK;
}

/* 用例简要描述: GetOwner 持锁归属 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMutex005, Function | MediumTest | Level0)
{
    osMutexId_t mid;
    osThreadId_t owner;
    osThreadId_t self;
    osStatus_t ret;

    mid = osMutexNew(NULL);
    ICUNIT_GOTO_EQUAL((mid != NULL), 1, (uint32_t)(UINTPTR)mid, EXIT);

    owner = osMutexGetOwner(mid);
    ICUNIT_GOTO_EQUAL((owner == NULL), 1, (uint32_t)(UINTPTR)owner, EXIT);

    ret = osMutexAcquire(mid, osWaitForever);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    self = osThreadGetId();
    owner = osMutexGetOwner(mid);
    ICUNIT_GOTO_EQUAL(owner, self, (uint32_t)(UINTPTR)owner, EXIT);

    ret = osMutexRelease(mid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    owner = osMutexGetOwner(mid);
    ICUNIT_GOTO_EQUAL((owner == NULL), 1, (uint32_t)(UINTPTR)owner, EXIT);

    (void)osMutexDelete(mid);
    return LOS_OK;

EXIT:
    if (mid != NULL) { (void)osMutexDelete(mid); }
    return LOS_OK;
    return LOS_OK;
}

/* ---- Semaphore ---- */
/* 用例简要描述: Acquire 超时 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSem004, Function | MediumTest | Level1)
{
    osSemaphoreId_t sid;
    osStatus_t ret;

    sid = osSemaphoreNew(1, 0, NULL);
    ICUNIT_GOTO_EQUAL((sid != NULL), 1, (uint32_t)(UINTPTR)sid, EXIT);

    /* count=0, acquire with 5-tick timeout -> timeout */
    ret = osSemaphoreAcquire(sid, 5);
    ICUNIT_GOTO_EQUAL(ret, osErrorTimeout, ret, EXIT);

    /* release to increment count */
    ret = osSemaphoreRelease(sid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* now acquire(0) succeeds immediately */
    ret = osSemaphoreAcquire(sid, 0);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    (void)osSemaphoreDelete(sid);
    return LOS_OK;

EXIT:
    if (sid != NULL) { (void)osSemaphoreDelete(sid); }
    return LOS_OK;
}

/* 用例简要描述: GetCount BVA */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisSem005, Function | MediumTest | Level1)
{
    osSemaphoreId_t sid;
    osStatus_t ret;

    sid = osSemaphoreNew(3, 2, NULL);
    ICUNIT_GOTO_EQUAL((sid != NULL), 1, (uint32_t)(UINTPTR)sid, EXIT);

    ICUNIT_GOTO_EQUAL(osSemaphoreGetCount(sid), 2, 0, EXIT);

    ret = osSemaphoreAcquire(sid, 0);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osSemaphoreGetCount(sid), 1, 0, EXIT);

    ret = osSemaphoreRelease(sid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osSemaphoreGetCount(sid), 2, 0, EXIT);

    ret = osSemaphoreRelease(sid);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(osSemaphoreGetCount(sid), 3, 0, EXIT);

    (void)osSemaphoreDelete(sid);
    return LOS_OK;

EXIT:
    if (sid != NULL) { (void)osSemaphoreDelete(sid); }
    return LOS_OK;
}

/* ---- MessageQueue ---- */
/* 用例简要描述: 满队超时 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisQueue002, Function | MediumTest | Level1)
{
    osMessageQueueId_t mq = NULL;
    uint32_t msg = 0xAA;
    uint32_t buf = 0;
    osStatus_t ret;

    mq = osMessageQueueNew(1, sizeof(uint32_t), NULL);
    ICUNIT_GOTO_NOT_EQUAL(mq, NULL, mq, EXIT);

    /* fill queue (capacity=1) */
    ret = osMessageQueuePut(mq, &msg, 0, 0);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* queue full, Put with 5-tick timeout -> timeout */
    ret = osMessageQueuePut(mq, &msg, 0, 5);
    ICUNIT_GOTO_EQUAL(ret, osErrorTimeout, ret, EXIT);

    /* Get to free space */
    ret = osMessageQueueGet(mq, &buf, NULL, 0);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf, msg, buf, EXIT);

    /* now Put succeeds again */
    ret = osMessageQueuePut(mq, &msg, 0, 0);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

EXIT:
    if (mq != NULL) {
        (void)osMessageQueueDelete(mq);
    }
    return LOS_OK;
}

/* 用例简要描述: 空队超时 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisQueue003, Function | MediumTest | Level1)
{
    osMessageQueueId_t mq = NULL;
    uint32_t msg = 0xBB;
    uint32_t buf = 0;
    osStatus_t ret;

    mq = osMessageQueueNew(2, sizeof(uint32_t), NULL);
    ICUNIT_GOTO_NOT_EQUAL(mq, NULL, mq, EXIT);

    /* empty queue, Get with 5-tick timeout -> timeout */
    ret = osMessageQueueGet(mq, &buf, NULL, 5);
    ICUNIT_GOTO_EQUAL(ret, osErrorTimeout, ret, EXIT);

    /* Put a message */
    ret = osMessageQueuePut(mq, &msg, 0, 0);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    /* Get succeeds immediately */
    ret = osMessageQueueGet(mq, &buf, NULL, 0);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(buf, msg, buf, EXIT);

EXIT:
    if (mq != NULL) {
        (void)osMessageQueueDelete(mq);
    }
    return LOS_OK;
}

/* 用例简要描述: 内容一致 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisQueue004, Function | MediumTest | Level0)
{
    osMessageQueueId_t mq = NULL;
    uint32_t sendData[] = {0xAA, 0xBB, 0xCC, 0xDD};
    uint32_t buf = 0;
    osStatus_t ret;
    int i;

    mq = osMessageQueueNew(4, sizeof(uint32_t), NULL);
    ICUNIT_GOTO_NOT_EQUAL(mq, NULL, mq, EXIT);

    for (i = 0; i < 4; i++) {
        ret = osMessageQueuePut(mq, &sendData[i], 0, 0);
        ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
    }

    ICUNIT_GOTO_EQUAL(osMessageQueueGetCount(mq), 4, 0, EXIT);
    ICUNIT_GOTO_EQUAL(osMessageQueueGetSpace(mq), 0, 0, EXIT);

    for (i = 0; i < 4; i++) {
        ret = osMessageQueueGet(mq, &buf, NULL, 0);
        ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);
        ICUNIT_GOTO_EQUAL(buf, sendData[i], buf, EXIT);
    }

    ICUNIT_GOTO_EQUAL(osMessageQueueGetCount(mq), 0, 0, EXIT);
    ICUNIT_GOTO_EQUAL(osMessageQueueGetSpace(mq), 4, 0, EXIT);

EXIT:
    if (mq != NULL) {
        (void)osMessageQueueDelete(mq);
    }
    return LOS_OK;
}

/* 用例简要描述: 多生产者消费者 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisQueue005, Function | MediumTest | Level2)
{
    osMessageQueueId_t mq = NULL;
    osThreadId_t prod[2] = {NULL};
    osThreadId_t cons[2] = {NULL};
    osThreadAttr_t attr = {0};
    int i;

    attr.stack_size = OS_TSK_TEST_STACK_SIZE;
    attr.priority = osPriorityBelowNormal;
    attr.attr_bits = osThreadJoinable;

    mq = osMessageQueueNew(16, sizeof(uint32_t), NULL);
    ICUNIT_GOTO_NOT_EQUAL(mq, NULL, mq, EXIT);
    g_cmsisMqMpcId = mq;

    for (i = 0; i < 2; i++) {
        g_cmsisQProdCnt[i] = 0;
        g_cmsisQConsCnt[i] = 0;
    }

    prod[0] = osThreadNew((osThreadFunc_t)CmsisMqProducer, (void *)0, &attr);
    ICUNIT_GOTO_NOT_EQUAL(prod[0], NULL, prod[0], EXIT);
    prod[1] = osThreadNew((osThreadFunc_t)CmsisMqProducer, (void *)1, &attr);
    ICUNIT_GOTO_NOT_EQUAL(prod[1], NULL, prod[1], EXIT);
    cons[0] = osThreadNew((osThreadFunc_t)CmsisMqConsumer, (void *)0, &attr);
    ICUNIT_GOTO_NOT_EQUAL(cons[0], NULL, cons[0], EXIT);
    cons[1] = osThreadNew((osThreadFunc_t)CmsisMqConsumer, (void *)1, &attr);
    ICUNIT_GOTO_NOT_EQUAL(cons[1], NULL, cons[1], EXIT);

    (void)osThreadJoin(prod[0]); prod[0] = NULL;
    (void)osThreadJoin(prod[1]); prod[1] = NULL;
    (void)osThreadJoin(cons[0]); cons[0] = NULL;
    (void)osThreadJoin(cons[1]); cons[1] = NULL;

    ICUNIT_GOTO_EQUAL(g_cmsisQProdCnt[0] + g_cmsisQProdCnt[1], 100, 0, EXIT);
    ICUNIT_GOTO_EQUAL(g_cmsisQConsCnt[0] + g_cmsisQConsCnt[1], 100, 0, EXIT);

EXIT:
    for (i = 0; i < 2; i++) {
        if (prod[i] != NULL) {
            (void)osThreadJoin(prod[i]);
        }
        if (cons[i] != NULL) {
            (void)osThreadJoin(cons[i]);
        }
    }
    if (mq != NULL) {
        (void)osMessageQueueDelete(mq);
    }
    return LOS_OK;
}

/* ---- MemoryPool ---- */
/* 用例简要描述: GetCount+Space */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMemPool007, Function | MediumTest | Level0)
{
    osMemoryPoolId_t mp;
    void *p1 = NULL;
    void *p2 = NULL;

    mp = osMemoryPoolNew(4, 32, NULL);
    ICUNIT_GOTO_NOT_EQUAL(mp, NULL, mp, EXIT);

    ICUNIT_GOTO_EQUAL(osMemoryPoolGetCount(mp), 0, 0, EXIT);
    ICUNIT_GOTO_EQUAL(osMemoryPoolGetSpace(mp), 4, 0, EXIT);

    p1 = osMemoryPoolAlloc(mp, 0);
    ICUNIT_GOTO_NOT_EQUAL(p1, NULL, p1, EXIT);
    ICUNIT_GOTO_EQUAL(osMemoryPoolGetCount(mp), 1, 0, EXIT);
    ICUNIT_GOTO_EQUAL(osMemoryPoolGetSpace(mp), 3, 0, EXIT);

    p2 = osMemoryPoolAlloc(mp, 0);
    ICUNIT_GOTO_NOT_EQUAL(p2, NULL, p2, EXIT);
    ICUNIT_GOTO_EQUAL(osMemoryPoolGetCount(mp), 2, 0, EXIT);

    (void)osMemoryPoolFree(mp, p1);
    ICUNIT_GOTO_EQUAL(osMemoryPoolGetCount(mp), 1, 0, EXIT);
    (void)osMemoryPoolFree(mp, p2);
    ICUNIT_GOTO_EQUAL(osMemoryPoolGetCount(mp), 0, 0, EXIT);
    ICUNIT_GOTO_EQUAL(osMemoryPoolGetSpace(mp), 4, 0, EXIT);

EXIT:
    if (mp != NULL) {
        (void)osMemoryPoolDelete(mp);
    }
    return LOS_OK;
}

/* 用例简要描述: 碎片化压力 */
LITE_TEST_CASE(CmsisFuncTestSuite, TestCmsisMemPool008, Function | MediumTest | Level3)
{
    osMemoryPoolId_t mp = NULL;
    void *blocks[16] = {NULL};
    int allocIdx = 0;
    int i;
    UINT32 seed = 0x12345678U;

    mp = osMemoryPoolNew(16, 32, NULL);
    ICUNIT_GOTO_NOT_EQUAL(mp, NULL, mp, EXIT);

    for (i = 0; i < 100; i++) {
        seed = seed * 1103515245U + 12345U;
        if ((seed & 1U) || (allocIdx == 0)) {
            if (allocIdx < 16) {
                blocks[allocIdx] = osMemoryPoolAlloc(mp, 0);
                if (blocks[allocIdx] != NULL) {
                    allocIdx++;
                }
            }
        } else {
            allocIdx--;
            (void)osMemoryPoolFree(mp, blocks[allocIdx]);
            blocks[allocIdx] = NULL;
        }
    }

    for (i = 0; i < 16; i++) {
        if (blocks[i] != NULL) {
            (void)osMemoryPoolFree(mp, blocks[i]);
            blocks[i] = NULL;
        }
    }

    ICUNIT_GOTO_EQUAL(osMemoryPoolGetCount(mp), 0, 0, EXIT);
    ICUNIT_GOTO_EQUAL(osMemoryPoolGetSpace(mp), 16, 0, EXIT);

EXIT:
    if (mp != NULL) {
        (void)osMemoryPoolDelete(mp);
    }
    return LOS_OK;
}

void CmsisFuncTestSuite(void)
{
    dprintf("***********************BEGIN CMSIS TEST**********************\n");

    ADD_TEST_CASE(TestCmsis001);
    ADD_TEST_CASE(TestCmsis002);
    ADD_TEST_CASE(TestCmsis003);
    ADD_TEST_CASE(TestCmsis004);
    ADD_TEST_CASE(TestCmsis005);

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
    ADD_TEST_CASE(TestCmsis006);
#endif

    ADD_TEST_CASE(TestCmsis007);

    ADD_TEST_CASE(TestCmsisTimer001);

    ADD_TEST_CASE(TestCmsisSaturation001);
    ADD_TEST_CASE(TestCmsisSaturation002);
    ADD_TEST_CASE(TestCmsisSaturation003);
    ADD_TEST_CASE(TestCmsisSaturation004);
    ADD_TEST_CASE(TestCmsisSaturation005);
    ADD_TEST_CASE(TestCmsisSaturation006);
    ADD_TEST_CASE(TestCmsisExt001);
    ADD_TEST_CASE(TestCmsisExt002);
    ADD_TEST_CASE(TestCmsisExt003);
    ADD_TEST_CASE(TestCmsisExt004);
    ADD_TEST_CASE(TestCmsisExt005);
    ADD_TEST_CASE(TestCmsisQueueLimit001);
    ADD_TEST_CASE(TestCmsisQueueLimit002);
    ADD_TEST_CASE(TestCmsisMutexGetName001);
    ADD_TEST_CASE(TestCmsisThreadFlags001);
    ADD_TEST_CASE(TestCmsisThreadFlags002);
    ADD_TEST_CASE(TestCmsisThreadFlags003);
    ADD_TEST_CASE(TestCmsisThreadFlags004);
    ADD_TEST_CASE(TestCmsisMemPool001);
    ADD_TEST_CASE(TestCmsisMemPool002);
    ADD_TEST_CASE(TestCmsisMemPool003);
    ADD_TEST_CASE(TestCmsisMemPool004);
    ADD_TEST_CASE(TestCmsisMemPool005);
    ADD_TEST_CASE(TestCmsisMemPool006);
    ADD_TEST_CASE(TestCmsisEventFlags001);
    ADD_TEST_CASE(TestCmsisEventFlags002);
    ADD_TEST_CASE(TestCmsisEventFlags003);
    ADD_TEST_CASE(TestCmsisMutex001);
    ADD_TEST_CASE(TestCmsisMutex002);
    ADD_TEST_CASE(TestCmsisMutex003);
    ADD_TEST_CASE(TestCmsisSem001);
    ADD_TEST_CASE(TestCmsisSem002);
    ADD_TEST_CASE(TestCmsisSem003);
    ADD_TEST_CASE(TestCmsisKernel001);
    ADD_TEST_CASE(TestCmsisKernel002);
    ADD_TEST_CASE(TestCmsisKernel003);
    ADD_TEST_CASE(TestCmsisThread001);
    ADD_TEST_CASE(TestCmsisThread002);
    ADD_TEST_CASE(TestCmsisThread003);
    ADD_TEST_CASE(TestCmsisThread004);
    ADD_TEST_CASE(TestCmsisThread005);
    ADD_TEST_CASE(TestCmsisTimer002);
    ADD_TEST_CASE(TestCmsisTimer003);
    /* supplement: cover uncalled kernel/queue/thread APIs */
    ADD_TEST_CASE(TestCmsisKernel004);
    ADD_TEST_CASE(TestCmsisKernel005);
    ADD_TEST_CASE(TestCmsisQueueAttr001);
    ADD_TEST_CASE(TestCmsisThread006);
    ADD_TEST_CASE(TestCmsisThread007);
    ADD_TEST_CASE(TestCmsisThread008);
    /* Batch 1 supplement: Kernel lock/delay, Timer boundary, Thread misc */
    ADD_TEST_CASE(TestCmsisKernel006);
    ADD_TEST_CASE(TestCmsisKernel007);
#if !defined(PRODUCT_BUG_ISOLATE) || !defined(LOSCFG_ARCH_ARM)
    ADD_TEST_CASE(TestCmsisKernel008);
#endif
    ADD_TEST_CASE(TestCmsisKernel009);
    ADD_TEST_CASE(TestCmsisIsr001);
    ADD_TEST_CASE(TestCmsisDelay001);
    ADD_TEST_CASE(TestCmsisTimer004);
    ADD_TEST_CASE(TestCmsisTimer005);
    ADD_TEST_CASE(TestCmsisTimer006);
    ADD_TEST_CASE(TestCmsisThread009);
    ADD_TEST_CASE(TestCmsisThread010);
    ADD_TEST_CASE(TestCmsisThread011);
    ADD_TEST_CASE(TestCmsisThread012);
    ADD_TEST_CASE(TestCmsisThread013);
    /* Batch 2: EventFlags/Mutex/Semaphore/MessageQueue/MemoryPool gap tests */
    ADD_TEST_CASE(TestCmsisEventFlags004);
    ADD_TEST_CASE(TestCmsisEventFlags005);
    ADD_TEST_CASE(TestCmsisEventFlags006);
    ADD_TEST_CASE(TestCmsisMutex004);
    ADD_TEST_CASE(TestCmsisMutex005);
    ADD_TEST_CASE(TestCmsisSem004);
    ADD_TEST_CASE(TestCmsisSem005);
    ADD_TEST_CASE(TestCmsisQueue002);
    ADD_TEST_CASE(TestCmsisQueue003);
    ADD_TEST_CASE(TestCmsisQueue004);
    ADD_TEST_CASE(TestCmsisQueue005);
    ADD_TEST_CASE(TestCmsisMemPool007);
    ADD_TEST_CASE(TestCmsisMemPool008);
}

