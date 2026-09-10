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
 * of conditions and the following disclaimer in the documentation and/or materials
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

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)

static volatile UINT32 g_withArgHit;
static volatile VOID *g_withArgDevId;

static VOID HwiF01WithArg(VOID *arg)
{
    TestHwiClear(HWI_NUM_TEST);
    g_withArgHit++;
    g_withArgDevId = arg;
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    HWI_PRIOR_T hwiPrio = 2;
    HWI_MODE_T mode = 0;
    HwiIrqParam irqParam;
    VOID *expectedDevId = (VOID *)0x1234;

    (void)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = expectedDevId;

    g_withArgHit = 0;
    g_withArgDevId = NULL;

    /* 1) Create interrupt with irqParam (WITH_ARG path). */
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01WithArg, &irqParam);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* 2) Trigger and verify handler received the device ID. */
    TestHwiTrigger(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_withArgHit, 1, g_withArgHit, EXIT);
    ICUNIT_GOTO_EQUAL(g_withArgDevId, expectedDevId, g_withArgDevId, EXIT);

    /* 3) Delete and verify handler no longer fires. */
    ret = LOS_HwiDelete(HWI_NUM_TEST, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    g_withArgHit = 0;
    TestHwiTrigger(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_withArgHit, 0, g_withArgHit, EXIT);

    return LOS_OK;

EXIT:
    TestHwiDelete(HWI_NUM_TEST);
    return LOS_OK;
}

VOID ItLosHwi045(VOID)
{
    TEST_ADD_CASE("ItLosHwi045", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_FUNCTION);
}
#endif
