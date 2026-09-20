/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
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

/* 用例简要描述: LOS_HwiBhworkAdd 边界—ISR 内连续投递 WORK_LIMIT+1 个 bottom half，
 * 前 WORK_LIMIT 次 LOS_OK，第 WORK_LIMIT+1 次空闲链表耗尽返回 LOS_ERRNO_HWI_NO_MEMORY；
 * ISR 退出后 BH 任务排空，handler 恰执行 WORK_LIMIT 次。只调对外接口。 */

#define TEST_BH_LOOP_DELAY_TICKS 10 // 10, wait ticks for the bottom half task to drain.

static volatile UINT32 g_bhCount;
static UINT32 g_bhRet[LOSCFG_HWI_BOTTOM_HALF_WORK_LIMIT + 1];

static VOID BhHandler(VOID *data)
{
    (VOID)data;
    g_bhCount++;
    return;
}

static VOID HwiF01(VOID)
{
    INT32 i;
    TestHwiClear(HWI_NUM_TEST);
    /* ISR 内独占：BH 任务（任务上下文）无法抢占 ISR，空闲链表只能被本 ISR 消耗。
     * 前 WORK_LIMIT 次成功，第 WORK_LIMIT+1 次返回 NO_MEMORY。 */
    for (i = 0; i <= LOSCFG_HWI_BOTTOM_HALF_WORK_LIMIT; i++) {
        g_bhRet[i] = LOS_HwiBhworkAdd(BhHandler, NULL);
    }
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    INT32 i;
    HWI_PRIOR_T hwiPrio = 2; // 2, Set hwi priority.
    HWI_MODE_T mode = 0;
    HwiIrqParam irqParam;
    (void)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = 0;

    g_bhCount = 0;
    (void)memset_s(g_bhRet, sizeof(g_bhRet), 0, sizeof(g_bhRet));

    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01, &irqParam);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TestHwiTrigger(HWI_NUM_TEST);

    /* 校验 ISR 内记录的返回码：前 WORK_LIMIT 次 LOS_OK，最后一次 NO_MEMORY。 */
    for (i = 0; i < LOSCFG_HWI_BOTTOM_HALF_WORK_LIMIT; i++) {
        ICUNIT_GOTO_EQUAL(g_bhRet[i], LOS_OK, g_bhRet[i], EXIT);
    }
    ret = g_bhRet[LOSCFG_HWI_BOTTOM_HALF_WORK_LIMIT];
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_HWI_NO_MEMORY, ret, EXIT);

    /* Bottom half runs asynchronously in the Int_Bottom_Half_Task. Wait. */
    (VOID)LOS_TaskDelay(TEST_BH_LOOP_DELAY_TICKS);
    /* 排空后 handler 恰好执行 WORK_LIMIT 次（被拒的那次未入队）。 */
    ICUNIT_GOTO_EQUAL(g_bhCount, LOSCFG_HWI_BOTTOM_HALF_WORK_LIMIT, g_bhCount, EXIT);

EXIT:
    TestHwiDelete(HWI_NUM_TEST);
    return LOS_OK;
}

VOID ItLosHwi048(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosHwi048", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_PRESSURE);
}
#endif
