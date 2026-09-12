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

/* ItLosTick004: LOS_CyclePerTickGet 返回值 == g_sysClock/TICK_PER_SECOND == OS_CYCLE_PER_TICK */
/* 用例简要描述: LOS_CyclePerTickGet==g_sysClock/每秒与 OS_CYCLE_PER_TICK 宏 */
static UINT32 TestCase(VOID)
{
    UINT32 cyclePerTick;
    UINT32 expect;

    /* F-Pos: cycle/tick == g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND */
    cyclePerTick = LOS_CyclePerTickGet();
    expect = g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    ICUNIT_ASSERT_EQUAL(cyclePerTick, expect, cyclePerTick);

    /* F-Pos: 等价 OS_CYCLE_PER_TICK 宏(los_tick.h:169) */
    ICUNIT_ASSERT_EQUAL(cyclePerTick, OS_CYCLE_PER_TICK, cyclePerTick);

    return LOS_OK;
}

VOID ItLosTick004(VOID)
{
    TEST_ADD_CASE("ItLosTick004", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}
