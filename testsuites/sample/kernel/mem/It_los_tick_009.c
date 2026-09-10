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

/* ItLosTick009: LOS_MDelay 经时校验(前后 tick 计数差 ≈ LOS_MS2Tick(请求值)) */
/* 用例简要描述: LOS_MDelay 30ms→3tick 上下界容差 */
static UINT32 TestCase(VOID)
{
    UINT64 t1;
    UINT64 t2;
    UINT32 delta;
    UINT32 expectTick;

    /* F-Pos: 30ms -> 3 tick(LOSCFG_BASE_CORE_TICK_PER_SECOND=100),
       忙等期间 tick 中断仍触发,tick 计数推进。
       注:MDelay(5)→0.5 tick 截断不可测,故取 30ms(3 tick)。 */
    expectTick = LOS_MS2Tick(30);
    t1 = LOS_TickCountGet();
    LOS_MDelay(30);
    t2 = LOS_TickCountGet();
    delta = (UINT32)(t2 - t1);

    /* 下界容差 1 tick,上界 +2 tick 防卡死抖动 */
    ICUNIT_ASSERT_EQUAL((delta >= (expectTick - 1)), 1, delta);
    ICUNIT_ASSERT_EQUAL((delta <= (expectTick + 2)), 1, delta);

    return LOS_OK;
}

VOID ItLosTick009(VOID)
{
    TEST_ADD_CASE("ItLosTick009", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}
