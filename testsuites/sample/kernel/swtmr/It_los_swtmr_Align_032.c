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
#include "It_los_swtmr.h"

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)

/* 用例简要描述: 对齐扫描跳过非对齐节点。先启动一个"不对齐"周期定时器
 * （SENSITIVE，alignEnable=FALSE，运行中 isAligned=0），再启动一个"需对齐"
 * 周期定时器（INSENSITIVE，alignEnable=TRUE）→ OsSwtmrStart 触发
 * OsSwtmrFindAlignTime 遍历，遇到非对齐节点走 continue 分支（los_swtmr.c
 * L236-237/L264）。只调对外接口。 */

static VOID CaseFunc032(UINT32 arg)
{
    (VOID)arg;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 idNonAlign;
    UINT32 idAlign;

    /* 非对齐定时器：SENSITIVE 语义为"不需对齐"（alignEnable=FALSE）。 */
    ret = LOS_SwtmrCreate(5, LOS_SWTMR_MODE_PERIOD, CaseFunc032, &idNonAlign, 0xffff,
                          OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_SENSITIVE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_SwtmrStart(idNonAlign);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* 需对齐定时器：INSENSITIVE 语义为"需对齐"（alignEnable=TRUE），
     * 启动时触发 FindAlignTime 遍历，跳过运行中的非对齐节点。 */
    ret = LOS_SwtmrCreate(10, LOS_SWTMR_MODE_PERIOD, CaseFunc032, &idAlign, 0xffff,
                          OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SwtmrStart(idAlign);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    (VOID)LOS_TaskDelay(2); // 2, let both timers tick once.

EXIT:
    (VOID)LOS_SwtmrStop(idAlign);
    (VOID)LOS_SwtmrDelete(idAlign);
    (VOID)LOS_SwtmrStop(idNonAlign);
    (VOID)LOS_SwtmrDelete(idNonAlign);
    return LOS_OK;
}

VOID ItLosSwtmrAlign032(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosSwtmrAlign032", Testcase, TEST_LOS, TEST_SWTMR, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
