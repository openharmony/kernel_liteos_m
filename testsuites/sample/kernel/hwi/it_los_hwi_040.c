/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

#include "osTest.h"
#include "it_los_hwi.h"

static UINT32 TesttriggerIrq(HWI_HANDLE_T hwiNum)
{
    g_testCount++;
    printf("This is TesttriggerIrq\n");
    return LOS_OK;
}

static UINT32 TestclearIrq(HWI_HANDLE_T hwiNum)
{
    g_testCount++;
    printf("This is TestclearIrq\n");
    return LOS_OK;
}

static UINT32 TestenableIrq(HWI_HANDLE_T hwiNum)
{
    g_testCount++;
    printf("This is TestenableIrq\n");
    return LOS_OK;
}

static UINT32 TestdisableIrq(HWI_HANDLE_T hwiNum)
{
    g_testCount++;
    printf("This is TestdisableIrq\n");
    return LOS_OK;
}

static UINT32 TestsetIrqPriority(HWI_HANDLE_T hwiNum, UINT8 priority)
{
    g_testCount++;
    printf("This is TestsetIrqPriority\n");
    return LOS_OK;
}

static UINT32 TestgetCurIrqNum(VOID)
{
    g_testCount++;
    printf("This is TestgetCurIrqNum\n");
    return HWI_NUM_TEST;
}

static VOID HwiF01(VOID)
{
    TestHwiClear(HWI_NUM_TEST);
    g_testCount++;
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret = LOS_OK;
    UINT32 irqNum = 0;
    HWI_PRIOR_T hwiPrio = 2;  // 2, Set hwi priority.
    HWI_MODE_T mode = 0;
    HWI_ARG_T arg = 0;
    HwiControllerOps *ops = NULL;
    HwiControllerOps *opsBac = (HwiControllerOps *)malloc(sizeof(HwiControllerOps));
    if (opsBac == NULL) {
        ret = LOS_NOK;
    }
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    (VOID)memset_s(opsBac, sizeof(HwiControllerOps), 0, sizeof(HwiControllerOps));

    g_testCount = 0;
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01, arg);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_HwiTrigger(LOSCFG_PLATFORM_HWI_LIMIT + 1);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_HWI_NUM_INVALID, ret);
    ret = LOS_HwiEnable(LOSCFG_PLATFORM_HWI_LIMIT + 1);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_HWI_NUM_INVALID, ret);
    ret = LOS_HwiDisable(LOSCFG_PLATFORM_HWI_LIMIT + 1);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_HWI_NUM_INVALID, ret);
    ret = LOS_HwiClear(LOSCFG_PLATFORM_HWI_LIMIT + 1);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_HWI_NUM_INVALID, ret);
    hwiPrio = 3; // 3, Set hwi priority.
    ret = LOS_HwiSetPriority(LOSCFG_PLATFORM_HWI_LIMIT + 1, hwiPrio);
    ICUNIT_ASSERT_NOT_EQUAL(ret, LOS_OK, ret);

    ops = LOS_HwiOpsGet();
    (VOID)memcpy_s(opsBac, sizeof(HwiControllerOps), ops, sizeof(HwiControllerOps));
    ops->triggerIrq = TesttriggerIrq;
    ops->enableIrq = TestenableIrq;
    ops->disableIrq = TestdisableIrq;
    ops->setIrqPriority = TestsetIrqPriority;
    ops->getCurIrqNum = TestgetCurIrqNum;
    ops->clearIrq = TestclearIrq;

    LOS_HwiTrigger(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT); // Compare wiht the expected value 1.
    LOS_HwiEnable(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_testCount, 2, g_testCount, EXIT); // Compare wiht the expected value 2.
    LOS_HwiDisable(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_testCount, 3, g_testCount, EXIT); // Compare wiht the expected value 3.
    LOS_HwiClear(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_testCount, 4, g_testCount, EXIT); // Compare wiht the expected value 4.
    LOS_HwiSetPriority(HWI_NUM_TEST, 3); // 3, Set hwi priority.
    ICUNIT_GOTO_EQUAL(g_testCount, 5, g_testCount, EXIT); // Compare wiht the expected value 5.
    irqNum = LOS_HwiCurIrqNum();
    ICUNIT_ASSERT_EQUAL(irqNum, HWI_NUM_TEST, irqNum);
    ICUNIT_GOTO_EQUAL(g_testCount, 6, g_testCount, EXIT); // Compare wiht the expected value 6.

    ops->triggerIrq = opsBac->triggerIrq;
    ops->enableIrq = opsBac->enableIrq;
    ops->disableIrq = opsBac->disableIrq;
    ops->setIrqPriority = opsBac->setIrqPriority;
    ops->getCurIrqNum = opsBac->getCurIrqNum;
    ops->clearIrq = opsBac->clearIrq;

    free(opsBac);
    ret = LOS_HwiDisable(HWI_NUM_TEST);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_HwiTrigger(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_testCount, 6, g_testCount, EXIT); // Compare wiht the expected value 6.
    ret = LOS_HwiEnable(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_testCount, 7, g_testCount, EXIT); // Compare wiht the expected value 7.
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_HwiTrigger(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_testCount, 8, g_testCount, EXIT); // Compare wiht the expected value 8.

    TestHwiDelete(HWI_NUM_TEST);
    return LOS_OK;

EXIT:
    TestHwiDelete(HWI_NUM_TEST);
    free(opsBac);
    return LOS_OK;
}

VOID ItLosHwi040(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosHwi040", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_PRESSURE);
}
