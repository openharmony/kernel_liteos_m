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

/* ItLosTick008: LOS_UDelay 经时校验(cycle 换算 us) + 入参 0 立即返回不崩溃 */
/* 用例简要描述: LOS_UDelay 0 立即返回与 1ms 经时≥800us 下界 */
static UINT32 TestCase(VOID)
{
    UINT64 c1;
    UINT64 c2;
    UINT64 elapsedUs;
    UINT32 expectUs = 1000; /* 1ms */

    /* F-Inv: 0 入参立即返回不崩溃(los_tick.c:442) */
    LOS_UDelay(0);

    /* F-Pos: 前后 LOS_SysCycleGet 差值换算 us >= 800(≈1000us,下界 80% 容差) */
    c1 = LOS_SysCycleGet();
    LOS_UDelay(expectUs);
    c2 = LOS_SysCycleGet();
    ICUNIT_ASSERT_EQUAL((c2 > c1), 1, (UINT32)(c2 - c1));

    elapsedUs = (c2 - c1) * OS_SYS_US_PER_SECOND / g_sysClock;
    ICUNIT_ASSERT_EQUAL((elapsedUs >= 800), 1, (UINT32)elapsedUs);

    return LOS_OK;
}

VOID ItLosTick008(VOID)
{
    TEST_ADD_CASE("ItLosTick008", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}
