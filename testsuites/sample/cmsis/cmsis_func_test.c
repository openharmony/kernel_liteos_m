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
    attr.priority = osPriorityNormal;
    attr.attr_bits = osThreadDetached;
    threadId = osThreadNew((osThreadFunc_t)CmsisStackFunc01, NULL, &attr);
    ICUNIT_GOTO_NOT_EQUAL(threadId, 0, threadId, EXIT);

    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);
EXIT:
    free(stackAddr);
    return LOS_OK;
};

void CmsisFuncTestSuite(void)
{
    ADD_TEST_CASE(TestCmsis001);
}

