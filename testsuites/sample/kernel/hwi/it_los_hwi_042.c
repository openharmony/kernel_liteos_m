/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
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

#if (LOSCFG_HWI_BOTTOM_HALF == 1)

#define TEST_BH_DELAY_TICKS 10 // 10, wait ticks for the bottom half task to drain.

/* g_isrRan is set in the ISR; g_bhRan is set in the bottom half handler.
 * The bh handler also receives &g_bhRan as data to prove the data pointer
 * propagates correctly. */
static volatile UINT32 g_isrRan;
static volatile UINT32 g_bhRan;

static VOID BhHandler(VOID *data)
{
    UINT32 *p = (UINT32 *)data;
    if (p != NULL) {
        (*p)++;
    }
    return;
}

static VOID HwiF01(VOID)
{
    UINT32 ret;
    TestHwiClear(HWI_NUM_TEST);
    g_isrRan = 1;
    /* LOS_HwiBhworkAdd must succeed in interrupt context. */
    ret = LOS_HwiBhworkAdd(BhHandler, (VOID *)&g_bhRan);
    ICUNIT_ASSERT_EQUAL_VOID(ret, LOS_OK, ret);
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    HWI_PRIOR_T hwiPrio = 2; // 2, Set hwi priority.
    HWI_MODE_T mode = 0;
    HwiIrqParam irqParam;
    (void)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = 0;

    g_isrRan = 0;
    g_bhRan = 0;
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01, &irqParam);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* Calling LOS_HwiBhworkAdd from task context must be rejected. */
    ret = LOS_HwiBhworkAdd(BhHandler, NULL);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_HWI_NOT_INTERRUPT_CONTEXT, ret, EXIT);

    /* Trigger the IRQ: the ISR runs and enqueues the bottom half. */
    TestHwiTrigger(HWI_NUM_TEST);
    ICUNIT_GOTO_EQUAL(g_isrRan, 1, g_isrRan, EXIT); // 1, ISR fired.

    /* Bottom half runs asynchronously in the Int_Bottom_Half_Task. Wait. */
    (VOID)LOS_TaskDelay(TEST_BH_DELAY_TICKS);
    ICUNIT_GOTO_EQUAL(g_bhRan, 1, g_bhRan, EXIT); // 1, bottom half executed.

    TestHwiDelete(HWI_NUM_TEST);
    return LOS_OK;

EXIT:
    TestHwiDelete(HWI_NUM_TEST);
    return LOS_OK;
}

VOID ItLosHwi042(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosHwi042", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_PRESSURE);
}
#endif
