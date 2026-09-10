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

#if (LOSCFG_HWI_PRE_POST_PROCESS == 1)

static volatile UINT32 g_preHookHit;
static volatile UINT32 g_postHookHit;
static volatile HWI_HANDLE_T g_preHookHwiNum;
static volatile HWI_HANDLE_T g_postHookHwiNum;
/* Monotonic execution sequence: pre hook marks 1, handler marks 2, post hook marks 3. */
static volatile UINT32 g_execSeq;
static volatile UINT32 g_handlerSeq;

static VOID PreHookFunc(HWI_HANDLE_T hwiNum)
{
    g_preHookHit++;
    g_preHookHwiNum = hwiNum;
    g_execSeq = 1;
}

static VOID PostHookFunc(HWI_HANDLE_T hwiNum)
{
    g_postHookHit++;
    g_postHookHwiNum = hwiNum;
    g_execSeq = 3;
}

static VOID HwiF01(VOID *arg)
{
    (VOID)arg;
    TestHwiClear(HWI_NUM_TEST);
    g_testCount++;
    g_handlerSeq = g_execSeq; /* record whether pre hook has already run */
    g_execSeq = 2;
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    HWI_PRIOR_T hwiPrio = 2;
    HWI_MODE_T mode = 0;

    g_preHookHit = 0;
    g_postHookHit = 0;
    g_testCount = 0;
    g_execSeq = 0;
    g_handlerSeq = 0;

    /* 1) Register pre/post hooks. */
    LOS_HwiPreHookReg(PreHookFunc);
    LOS_HwiPostHookReg(PostHookFunc);

    /* 2) Create and trigger an interrupt. */
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TestHwiTrigger(HWI_NUM_TEST);

    /* 3) Verify the handler ran. */
    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

    /* 4) Verify the pre hook ran with correct hwiNum.
     * OsIntHandle passes the vector index (IRQ + OS_SYS_VECTOR_CNT) to the
     * pre/post hooks, not the raw IRQ number. */
    ICUNIT_GOTO_EQUAL(g_preHookHit, 1, g_preHookHit, EXIT);
    ICUNIT_GOTO_EQUAL(g_preHookHwiNum, HWI_NUM_TEST + OS_SYS_VECTOR_CNT, g_preHookHwiNum, EXIT);

    /* 5) Verify the post hook ran with correct hwiNum. */
    ICUNIT_GOTO_EQUAL(g_postHookHit, 1, g_postHookHit, EXIT);
    ICUNIT_GOTO_EQUAL(g_postHookHwiNum, HWI_NUM_TEST + OS_SYS_VECTOR_CNT, g_postHookHwiNum, EXIT);

    /* 6) Verify ordering: handler saw pre hook already executed (g_handlerSeq == 1),
     * and post hook ran after handler (g_execSeq == 3). */
    ICUNIT_GOTO_EQUAL(g_handlerSeq, 1, g_handlerSeq, EXIT);
    ICUNIT_GOTO_EQUAL(g_execSeq, 3, g_execSeq, EXIT);

EXIT:
    TestHwiDelete(HWI_NUM_TEST);
    LOS_HwiPreHookReg(NULL);
    LOS_HwiPostHookReg(NULL);
    return LOS_OK;
}

VOID ItLosHwi044(VOID)
{
    TEST_ADD_CASE("ItLosHwi044", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_FUNCTION);
}
#endif
