/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#if (LOS_KERNEL_MULTI_HWI_TEST == 1)
#include "it_los_hwi.h"

static VOID HwiF01(VOID)
{
    int index;
    for (index = OS_USER_HWI_MIN; index < OS_HWI_MAX_USED_NUM; index++) {
        TestHwiClear(index);
    }

    return;
}
#undef  HWI_NUM_INT0
#define HWI_NUM_INT0 HWI_NUM_TEST1
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 index;
    UINT32 loop;
    HWI_PRIOR_T hwiPrio = OS_HWI_PRIO_LOWEST;
    HWI_MODE_T mode = 0;
    HwiIrqParam irqParam;
    (void)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = 0;

    for (loop = 0; loop < HWI_LOOP_NUM; loop++) {
        for (index = 0; index < OS_HWI_MAX_USED_NUM; index++) {
            /* if not Interrupt number HWI_NUM_TEST1 + 3\4\32\35\36. */
            if ((index != 3) && (index != 4) && (index != 32) && (index != 35) && (index != 36)) {
                ret = LOS_HwiCreate(HWI_NUM_TEST1 + index, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01, &irqParam);
                ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
            }

            for (index = OS_USER_HWI_MIN; index < OS_HWI_MAX_USED_NUM; index++) {
                TestHwiDelete(HWI_NUM_TEST1 + index);
            }
        }
    }
    return LOS_OK;
}

VOID ItLosHwi033(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosHwi033", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_PRESSURE);
}
#endif

