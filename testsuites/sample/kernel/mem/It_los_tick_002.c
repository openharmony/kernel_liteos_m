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

/* ItLosTick002: LOS_MS2Tick 正常路径 + 边界/哨兵值 + 与 LOS_Tick2MS 往返一致 */
/* 用例简要描述: LOS_MS2Tick 转换契约:0/1000ms/50ms 截断/哨兵值/往返一致 */
static UINT32 TestCase(VOID)
{
    UINT32 tick;

    /* F-Pos: 0ms -> 0 tick */
    tick = LOS_MS2Tick(0);
    ICUNIT_ASSERT_EQUAL(tick, 0, tick);

    /* F-Pos: 1000ms(1s) -> LOSCFG_BASE_CORE_TICK_PER_SECOND tick(100) */
    tick = LOS_MS2Tick(1000);
    ICUNIT_ASSERT_EQUAL(tick, LOSCFG_BASE_CORE_TICK_PER_SECOND, tick);

    /* F-Pos: 截断取整校验,50ms -> 5 tick(5000/1000=5,B 侧截断非向上取整) */
    tick = LOS_MS2Tick(50);
    ICUNIT_ASSERT_EQUAL(tick, 5, tick);

    /* F-BVA: 哨兵值 OS_NULL_INT(0xFFFFFFFF) 原样直返(los_tick.c:336) */
    tick = LOS_MS2Tick(OS_NULL_INT);
    ICUNIT_ASSERT_EQUAL(tick, OS_NULL_INT, tick);

    /* F-Pos: 往返一致 LOS_Tick2MS(LOS_MS2Tick(1000)) == 1000 */
    tick = LOS_Tick2MS(LOS_MS2Tick(1000));
    ICUNIT_ASSERT_EQUAL(tick, 1000, tick);

    return LOS_OK;
}

VOID ItLosTick002(VOID)
{
    TEST_ADD_CASE("ItLosTick002", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}
