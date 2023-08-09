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

UINT16 g_cmsisTestMutexCount;
osMutexId_t g_cmsisOsMutexId;
osMutexAttr_t g_cmsisMutexAttr;

LITE_TEST_SUIT(Cmsis, Cmsismutex, CmsisMutexFuncTestSuite);

static BOOL CmsisMutexFuncTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL CmsisMutexFuncTestSuiteTearDown(void)
{
    return TRUE;
}

static void CmsisMutexGetOwnerFunc001(void const *argument)
{
    (void)argument;
    osStatus_t ret;
    osThreadId_t id1;
    osThreadId_t id2;
    osThreadAttr_t attr;
    g_cmsisOsMutexId = osMutexNew(&g_cmsisMutexAttr);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisOsMutexId, NULL, g_cmsisOsMutexId);

    ret = osMutexAcquire(g_cmsisOsMutexId, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    id1 = osMutexGetOwner(g_cmsisOsMutexId);
    id2 = osThreadGetId();
    ICUNIT_ASSERT_STRING_EQUAL(id1, id2, id1);

    attr.name = osThreadGetName(id1);
    ICUNIT_ASSERT_STRING_EQUAL("testMutexGetOwner001", attr.name, attr.name);

    ret = osMutexRelease(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    osThreadExit();
}

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0100
 * @tc.name      : mutex operation for creat with NULL para
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexNew001, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisOsMutexId = osMutexNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisOsMutexId, NULL, g_cmsisOsMutexId);
    (void)osMutexDelete(g_cmsisOsMutexId);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0200
 * @tc.name      : mutex operation for creat
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexNew002, Function | MediumTest | Level1)
{
    osStatus_t status;
    g_cmsisOsMutexId = osMutexNew(&g_cmsisMutexAttr);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisOsMutexId, NULL, g_cmsisOsMutexId);
    (void)osMutexDelete(g_cmsisOsMutexId);
    status = osDelay(DELAY_TICKS_5);
    ICUNIT_ASSERT_EQUAL(status, osOK, status);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0300
 * @tc.name      : mutex operation for delete after creat mutex with NULL parameter
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexDelete001, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisOsMutexId = osMutexNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisOsMutexId, NULL, g_cmsisOsMutexId);
    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0400
 * @tc.name      : mutex operation for delete
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexDelete002, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisOsMutexId = osMutexNew(&g_cmsisMutexAttr);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisOsMutexId, NULL, g_cmsisOsMutexId);
    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0500
 * @tc.name      : mutex operation for delete after mutex acquire and release
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexDelete003, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisOsMutexId = osMutexNew(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisOsMutexId, NULL, g_cmsisOsMutexId);

    osMutexAcquire(g_cmsisOsMutexId, LOS_WAIT_FOREVER);
    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osErrorResource, ret);

    (void)osMutexRelease(g_cmsisOsMutexId);
    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0600
 * @tc.name      : mutex delete operation with mutex_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexDelete004, Function | MediumTest | Level1)
{
    osStatus_t ret = osMutexDelete(NULL);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0700
 * @tc.name      : mutex acquire operation with mutex_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexAcquire001, Function | MediumTest | Level1)
{
    osStatus_t ret = osMutexAcquire(NULL, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0800
 * @tc.name      : mutex operation for acquire
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexAcquire002, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisOsMutexId = osMutexNew(&g_cmsisMutexAttr);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisOsMutexId, NULL, g_cmsisOsMutexId);

    ret = osMutexAcquire(g_cmsisOsMutexId, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    (void)osMutexRelease(g_cmsisOsMutexId);
    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_0900
 * @tc.name      : mutex operation for release
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexRelease001, Function | MediumTest | Level1)
{
    osStatus_t ret;
    g_cmsisOsMutexId = osMutexNew(&g_cmsisMutexAttr);
    ICUNIT_ASSERT_NOT_EQUAL(g_cmsisOsMutexId, NULL, g_cmsisOsMutexId);

    ret = osMutexAcquire(g_cmsisOsMutexId, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osErrorResource, ret);

    ret = osMutexRelease(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);

    ret = osMutexDelete(g_cmsisOsMutexId);
    ICUNIT_ASSERT_EQUAL(ret, osOK, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_1000
 * @tc.name      : mutex release operation with mutex_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexRelease002, Function | MediumTest | Level1)
{
    osStatus_t ret = osMutexRelease(NULL);
    ICUNIT_ASSERT_EQUAL(ret, osErrorParameter, ret);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_1100
 * @tc.name      : mutex operation for get owner
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexGetOwner001, Function | MediumTest | Level1)
{
    osThreadId_t id;
    osThreadAttr_t osAttr;
    osAttr.name = "testMutexGetOwner001";
    osAttr.attr_bits = 0U;
    osAttr.cb_mem = NULL;
    osAttr.cb_size = 0U;
    osAttr.stack_mem = NULL;
    osAttr.stack_size = TEST_TASK_STACK_SIZE;
    osAttr.priority = osPriorityAboveNormal;
    id = osThreadNew((osThreadFunc_t)CmsisMutexGetOwnerFunc001, NULL, &osAttr);
    ICUNIT_ASSERT_NOT_EQUAL(id, NULL, id);
    return 0;
};

/**
 * @tc.number    : SUB_KERNEL_CMSIS_MUTEX_OPERATION_1200
 * @tc.name      : mutex get owner operation with mutex_id = NULL
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(CmsisMutexFuncTestSuite, testOsMutexGetOwner002, Function | MediumTest | Level1)
{
    osThreadId_t id = osMutexGetOwner(NULL);
    ICUNIT_ASSERT_EQUAL(id, NULL, id);
    return 0;
};

RUN_TEST_SUITE(CmsisMutexFuncTestSuite);

void CmsisMutexFuncTest(void)
{
    RUN_ONE_TESTCASE(testOsMutexNew001);
    RUN_ONE_TESTCASE(testOsMutexNew002);
    RUN_ONE_TESTCASE(testOsMutexDelete001);
    RUN_ONE_TESTCASE(testOsMutexDelete002);
    RUN_ONE_TESTCASE(testOsMutexDelete003);
    RUN_ONE_TESTCASE(testOsMutexDelete004);
    RUN_ONE_TESTCASE(testOsMutexAcquire001);
    RUN_ONE_TESTCASE(testOsMutexAcquire002);
    RUN_ONE_TESTCASE(testOsMutexRelease001);
    RUN_ONE_TESTCASE(testOsMutexRelease002);
    RUN_ONE_TESTCASE(testOsMutexGetOwner001);
    RUN_ONE_TESTCASE(testOsMutexGetOwner002);
}
