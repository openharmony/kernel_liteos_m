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

#define TEST_STR(func) ItLos##func
#define TEST_TO_STR(x) #x
#define TEST_HEAD_TO_STR(x) TEST_TO_STR(x)
#define ADD_TEST_CASE(func) \
    TEST_ADD_CASE(TEST_HEAD_TO_STR(TEST_STR(func)), func, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION)

#define Function   0
#define MediumTest 0
#define Level1     0
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

#if (LOSCFG_BASE_IPC_QUEUE_STATIC == 1)
    CHAR staticBuff[STATCI_BUFF_SIZE] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = STATCI_BUFF_SIZE;
    msgQueueId = osMessageQueueNew(1, STATCI_BUFF_SIZE, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    name = osMessageQueueGetName(msgQueueId);
    ret = strcmp(name, "q1");
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    name = osMessageQueueGetName(msgQueueId);
    ICUNIT_ASSERT_EQUAL(name, NULL, name);
#endif

    return LOS_OK;

EXIT:
    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    return LOS_OK;
}

#if (LOSCFG_BASE_IPC_QUEUE_STATIC == 1)
static osMessageQueueId_t g_msgQueueId1;
static osMessageQueueId_t g_msgQueueId2;

static osThreadId_t threadId1;
static osThreadId_t threadId2;

static CHAR g_strbuff1[] = "hello";
static CHAR g_strbuff2[] = "world";
static CHAR g_staticBuff[STATCI_BUFF_SIZE] = {0};

static VOID CmsisQueueTestThread1(VOID)
{
    CHAR data[READ_BUFFER_SIZIE] = {0};
    INT32 ret;
    osStatus_t status;

    ret = osMessageQueuePut(g_msgQueueId1, &g_strbuff1, 0U, 0U);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    status = osMessageQueueGet(g_msgQueueId2, &data, NULL, QUEUE_WAIT_TIMEOUT);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    ret = strcmp(data, "world");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
}

static VOID CmsisQueueTestThread2(VOID)
{
    CHAR data[READ_BUFFER_SIZIE] = {0};
    INT32 ret;
    osStatus_t status;

    status = osMessageQueueGet(g_msgQueueId1, &data, NULL, QUEUE_WAIT_TIMEOUT);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);

    ret = strcmp(data, "hello");
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = osMessageQueuePut(g_msgQueueId2, &g_strbuff2, 0U, 0U);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
}

static INT32 ThreadReadWriteTest(VOID)
{
    osMessageQueueAttr_t attr = {0};
    INT32 ret;

    g_msgQueueId1 = osMessageQueueNew(1, strlen(g_strbuff1), NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_msgQueueId1, NULL, g_msgQueueId1);

    attr.mq_mem = g_staticBuff;
    attr.mq_size = strlen(g_strbuff2) + 1;
    g_msgQueueId2 = osMessageQueueNew(1, strlen(g_strbuff2), &attr);
    ICUNIT_ASSERT_NOT_EQUAL(g_msgQueueId2, NULL, g_msgQueueId2);

    threadId1 = osThreadNew(CmsisQueueTestThread1, NULL, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(threadId1, NULL, threadId1);

    threadId2 = osThreadNew(CmsisQueueTestThread2, NULL, NULL);
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

#if (LOSCFG_BASE_IPC_QUEUE_STATIC == 1)
    /* static test */
    osMessageQueueAttr_t attr = {0};
    CHAR staticBuff[STATCI_BUFF_SIZE] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = STATCI_BUFF_SIZE;
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

#if (LOSCFG_BASE_IPC_QUEUE_STATIC == 1)
    /* static test */
    osMessageQueueAttr_t attr = {0};
    CHAR staticBuff[STATCI_BUFF_SIZE] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = strlen(strbuff) + 1;
    msgQueueId = osMessageQueueNew(1, strlen(strbuff), &attr);
    ICUNIT_ASSERT_NOT_EQUAL(msgQueueId, NULL, msgQueueId);

    ret = osMessageQueuePut(msgQueueId, &strbuff, 0U, 0U);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    ret = osMessageQueueGet(msgQueueId, &data, NULL, 0U);
    ICUNIT_GOTO_EQUAL(ret, osOK, ret, EXIT);

    ret = strcmp(data, strbuff);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = strcmp(staticBuff, strbuff);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
#endif
    return LOS_OK;

EXIT:
    ret = osMessageQueueDelete(msgQueueId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

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

#if (LOSCFG_BASE_IPC_QUEUE_STATIC == 1)
    /* static test */
    osMessageQueueAttr_t attr = {0};
    CHAR staticBuff[STATCI_BUFF_SIZE] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = STATCI_BUFF_SIZE;
    msgQueueId = osMessageQueueNew(0, strlen(strbuff), &attr);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);

    msgQueueId = osMessageQueueNew(0xFFFFFFFF, strlen(strbuff), &attr);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);

    attr.mq_mem = staticBuff;
    attr.mq_size = 0;
    msgQueueId = osMessageQueueNew(1, strlen(strbuff), &attr);
    ICUNIT_ASSERT_EQUAL(msgQueueId, NULL, msgQueueId);

    attr.mq_mem = NULL;
    attr.mq_size = STATCI_BUFF_SIZE;
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

#if (LOSCFG_BASE_IPC_QUEUE_STATIC == 1)
    /* static test */
    osMessageQueueAttr_t attr = {0};
    CHAR staticBuff[STATCI_BUFF_SIZE] = {0};
    attr.mq_mem = staticBuff;
    attr.mq_size = STATCI_BUFF_SIZE;
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


void CmsisFuncTestSuite(void)
{
    PRINTF("***********************BEGIN CMSIS TEST**********************\n");

    ADD_TEST_CASE(TestCmsis001);
    ADD_TEST_CASE(TestCmsis002);
    ADD_TEST_CASE(TestCmsis003);
    ADD_TEST_CASE(TestCmsis004);
    ADD_TEST_CASE(TestCmsis005);

#if (LOSCFG_BASE_IPC_QUEUE_STATIC == 1)
    ADD_TEST_CASE(TestCmsis006);
#endif

    ADD_TEST_CASE(TestCmsis007);

    ADD_TEST_CASE(TestCmsisTimer001);
}

