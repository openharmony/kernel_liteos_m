/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of the following disclaimer in the documentation and/or other materials
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

static volatile UINT32 g_isrActive;
static volatile UINT32 g_isrInactive;

static VOID HwiF01(VOID)
{
    TestHwiClear(HWI_NUM_TEST);
    g_isrActive = OS_INT_ACTIVE;
    g_isrInactive = OS_INT_INACTIVE;
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    HWI_PRIOR_T hwiPrio = 2;
    HWI_MODE_T mode = 0;

    /* 1) In task context: OS_INT_ACTIVE should be 0, OS_INT_INACTIVE should be non-zero. */
    ICUNIT_ASSERT_EQUAL(OS_INT_ACTIVE, 0, OS_INT_ACTIVE);
    ICUNIT_ASSERT_NOT_EQUAL(OS_INT_INACTIVE, 0, OS_INT_INACTIVE);

    /* 2) Create and trigger an interrupt. */
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    g_isrActive = 0;
    g_isrInactive = 1;

    TestHwiTrigger(HWI_NUM_TEST);

    /* 3) In ISR context: OS_INT_ACTIVE should be non-zero, OS_INT_INACTIVE should be 0. */
    ICUNIT_GOTO_NOT_EQUAL(g_isrActive, 0, g_isrActive, EXIT);
    ICUNIT_GOTO_EQUAL(g_isrInactive, 0, g_isrInactive, EXIT);

    /* 4) Back in task context: OS_INT_ACTIVE should be 0 again. */
    ICUNIT_GOTO_EQUAL(OS_INT_ACTIVE, 0, OS_INT_ACTIVE, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(OS_INT_INACTIVE, 0, OS_INT_INACTIVE, EXIT);

EXIT:
    TestHwiDelete(HWI_NUM_TEST);
    return LOS_OK;
}

VOID ItLosHwi046(VOID)
{
    TEST_ADD_CASE("ItLosHwi046", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_FUNCTION);
}
