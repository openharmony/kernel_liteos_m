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

osMessageQueueId_t g_cmsisMqId;
static char *g_cmsisMessageInfo[] = {"msg1", "msg2", "msg3", "msg4", "msg5", "msg6", "msg7", "msg8"};

LITE_TEST_SUIT(Cmsis, Cmsismsg, CmsisMsgFuncTestSuite);

static BOOL CmsisMsgFuncTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL CmsisMsgFuncTestSuiteTearDown(void)
{
    return TRUE;
}

static void CmsisMessageQueueGetFunc001(void const *argument)
{
    (void)argument;
    osStatus_t uwRet;
    UINT8 msgPrio = 0;
    CHAR ucTemp[MSGINFO_LEN] = "";
    UINT32 uwCmp;
    uwRet = osMessageQueueGet(g_cmsisMqId, ucTemp, &msgPrio, TIMEOUT_COUNT);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    uwCmp = memcmp(ucTemp, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_0], MSGINFO_LEN);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwCmp);

    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    osThreadExit();
}

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0100
 * @tc.name      : message queue operation for creat
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueNew001, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);
    (void)osMessageQueueDelete(g_cmsisMqId);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0200
 * @tc.name      : message queue operation for creat when msg_count = 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueNew002, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisMqId = osMessageQueueNew(0, MSG_SIZE, NULL);
    ICUNIT_ASSERT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0300
 * @tc.name      : message queue operation for creat when msg_size = 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueNew003, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, 0, NULL);
    ICUNIT_ASSERT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0400
 * @tc.name      : message queue operation for delete
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueDelete001, Function | MediumTest | Level1)
{
    osStatus_t uwRet;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);
    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0500
 * @tc.name      : message queue delete operation with mq_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueDelete002, Function | MediumTest | Level1)
{
    osStatus_t uwRet = osMessageQueueDelete(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0600
 * @tc.name      : message queue operation for put
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueuePut001, Function | MediumTest | Level1)
{
    osStatus_t uwRet;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);

    uwRet = osMessageQueuePut(g_cmsisMqId, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_0], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0700
 * @tc.name      : message queue put operation with mq_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueuePut002, Function | MediumTest | Level1)
{
    osStatus_t uwRet = osMessageQueuePut(NULL, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_0], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0800
 * @tc.name      : message queue operation for put when msg_ptr = 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueuePut003, Function | MediumTest | Level1)
{
    osStatus_t uwRet;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);

    uwRet = osMessageQueuePut(g_cmsisMqId, NULL, 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);

    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_0900
 * @tc.name      : message queue operation for get
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGet001, Function | MediumTest | Level1)
{
    osStatus_t uwRet;
    UINT32 uId;
    osThreadId_t id;
    osThreadAttr_t attr;
    attr.name = "test";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = TEST_TASK_STACK_SIZE;
    attr.priority = osPriorityAboveNormal;
    uId = osKernelLock();
    ICUNIT_ASSERT_EQUAL(uId, 0, uId); /* 1, common data for test, no special meaning */
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);

    id = osThreadNew((osThreadFunc_t)CmsisMessageQueueGetFunc001, NULL, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    uwRet = osMessageQueuePut(g_cmsisMqId, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_0], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    uId = osKernelUnlock();
    ICUNIT_ASSERT_EQUAL(uId, 1, uId); /* 1, common data for test, no special meaning */
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1000
 * @tc.name      : message queue get operation with mq_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGet002, Function | MediumTest | Level1)
{
    osStatus_t uwRet;
    UINT8 msgPrio = 0;
    CHAR ucTemp[MSGINFO_LEN] = "";
    uwRet = osMessageQueueGet(NULL, ucTemp, &msgPrio, TIMEOUT_COUNT);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1100
 * @tc.name      : message queue operation for get when msg_ptr = 0
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGet003, Function | MediumTest | Level1)
{
    osStatus_t uwRet;
    UINT8 msgPrio = 0;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);

    uwRet = osMessageQueueGet(g_cmsisMqId, NULL, &msgPrio, TIMEOUT_COUNT);
    ICUNIT_ASSERT_EQUAL(uwRet, osErrorParameter, uwRet);

    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1200
 * @tc.name      : message queue operation for get msg size
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGetMsgSize001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);

    uwRet = osMessageQueueGetMsgSize(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, MSG_SIZE, uwRet);

    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1300
 * @tc.name      : message queue get msg size with mq_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGetMsgSize002, Function | MediumTest | Level1)
{
    UINT32 uwRet = osMessageQueueGetMsgSize(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1400
 * @tc.name      : message queue operation for get capacity
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGetCapacity001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);
    uwRet = osMessageQueueGetCapacity(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, MSGQUEUE_COUNT, uwRet);

    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1500
 * @tc.name      : message queue get capacity with mq_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGetCapacity002, Function | MediumTest | Level1)
{
    UINT32 uwRet = osMessageQueueGetCapacity(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1600
 * @tc.name      : message queue operation for get count
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGetCount001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);

    uwRet = osMessageQueuePut(g_cmsisMqId, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_0], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osMessageQueuePut(g_cmsisMqId, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_1], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osMessageQueuePut(g_cmsisMqId, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_2], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osMessageQueueGetCount(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, MSGQUEUE_PUT_COUNT, uwRet);

    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1700
 * @tc.name      : message queue get count with mq_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGetCount002, Function | MediumTest | Level1)
{
    UINT32 uwRet = osMessageQueueGetCount(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1800
 * @tc.name      : message queue operation for get space
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGetSpace001, Function | MediumTest | Level1)
{
    UINT32 uwRet;
    g_cmsisMqId = osMessageQueueNew(MSGQUEUE_COUNT, MSG_SIZE, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisMqId, NULL, g_cmsisMqId);

    uwRet = osMessageQueuePut(g_cmsisMqId, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_0], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osMessageQueuePut(g_cmsisMqId, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_1], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osMessageQueuePut(g_cmsisMqId, g_cmsisMessageInfo[MSGQUEUE_COUNT_INDEX_2], 0, 0);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);

    uwRet = osMessageQueueGetSpace(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, MSGQUEUE_SPACE_COUNT, uwRet);

    uwRet = osMessageQueueDelete(g_cmsisMqId);
    ICUNIT_ASSERT_EQUAL(uwRet, osOK, uwRet);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MSG_OPERATION_1900
 * @tc.name      : message queue get space with mq_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMsgFuncTestSuite, testOsMessageQueueGetSpace002, Function | MediumTest | Level1)
{
    UINT32 uwRet = osMessageQueueGetSpace(NULL);
    ICUNIT_ASSERT_EQUAL(uwRet, 0, uwRet);
    return 0;
};

RUN_TEST_SUITE(CmsisMsgFuncTestSuite);

void CmsisMsgFuncTest(void)
{
    RUN_ONE_TESTCASE(testOsMessageQueueNew001);
    RUN_ONE_TESTCASE(testOsMessageQueueNew002);
    RUN_ONE_TESTCASE(testOsMessageQueueNew003);
    RUN_ONE_TESTCASE(testOsMessageQueueDelete001);
    RUN_ONE_TESTCASE(testOsMessageQueueDelete002);
    RUN_ONE_TESTCASE(testOsMessageQueuePut001);
    RUN_ONE_TESTCASE(testOsMessageQueuePut002);
    RUN_ONE_TESTCASE(testOsMessageQueuePut003);
    RUN_ONE_TESTCASE(testOsMessageQueueGet001);
    RUN_ONE_TESTCASE(testOsMessageQueueGet002);
    RUN_ONE_TESTCASE(testOsMessageQueueGet003);
    RUN_ONE_TESTCASE(testOsMessageQueueGetMsgSize001);
    RUN_ONE_TESTCASE(testOsMessageQueueGetMsgSize002);
    RUN_ONE_TESTCASE(testOsMessageQueueGetCapacity001);
    RUN_ONE_TESTCASE(testOsMessageQueueGetCapacity002);
    RUN_ONE_TESTCASE(testOsMessageQueueGetCount001);
    RUN_ONE_TESTCASE(testOsMessageQueueGetCount002);
    RUN_ONE_TESTCASE(testOsMessageQueueGetSpace001);
    RUN_ONE_TESTCASE(testOsMessageQueueGetSpace002);
}
