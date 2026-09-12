/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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
 * "AS IS" AND ANY EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, TORT OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "It_los_mem.h"

/* ItLosTick006: LOS_GetCpuCycle 正常路径(hi/lo 合成值在读窗内) + NULL 入参不崩溃 */
/* 用例简要描述: LOS_GetCpuCycle 合成值落在读窗内+三组 NULL 不崩溃 */
static UINT32 TestCase(VOID)
{
    UINT32 hi = 0;
    UINT32 lo = 0;
    UINT64 c1;
    UINT64 c2;
    UINT64 combined;

    /* F-Pos: 合法入参,hi/lo 合成 64 位 cycle,应落在前后两次 LOS_SysCycleGet 读窗内 */
    c1 = LOS_SysCycleGet();
    LOS_GetCpuCycle(&hi, &lo);
    c2 = LOS_SysCycleGet();

    combined = ((UINT64)hi << OS_SYS_MV_32_BIT) | lo;
    ICUNIT_ASSERT_EQUAL((combined >= c1), 1, (UINT32)combined);
    ICUNIT_ASSERT_EQUAL((combined <= c2), 1, (UINT32)combined);
    ICUNIT_ASSERT_EQUAL((combined > 0), 1, (UINT32)combined);

    /* F-Inv: NULL highCnt,早返不崩溃(void 返回,无错误码) */
    LOS_GetCpuCycle(NULL, &lo);

    /* F-Inv: NULL lowCnt,早返不崩溃 */
    LOS_GetCpuCycle(&hi, NULL);

    /* F-Inv: 双 NULL,早返不崩溃 */
    LOS_GetCpuCycle(NULL, NULL);

    return LOS_OK;
}

VOID ItLosTick006(VOID)
{
    TEST_ADD_CASE("ItLosTick006", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}
