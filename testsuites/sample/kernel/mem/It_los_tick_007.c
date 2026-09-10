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

/* ItLosTick007: LOS_CurrNanosec 单调非减 + 经时量级校验(UDelay 1ms 后增量 ≈ 1e6 ns) */
/* 用例简要描述: LOS_CurrNanosec 单调与 1ms 经时≥800000ns 下界 */
static UINT32 TestCase(VOID)
{
    UINT64 n1;
    UINT64 n2;
    UINT64 delta;

    /* F-Pos: 首次读取 > 0 */
    n1 = LOS_CurrNanosec();
    ICUNIT_ASSERT_EQUAL((n1 > 0), 1, (UINT32)n1);

    /* UDelay 1ms,确保纳秒推进 */
    LOS_UDelay(1000);

    /* F-Pos: 二次读取单调 */
    n2 = LOS_CurrNanosec();
    ICUNIT_ASSERT_EQUAL((n2 >= n1), 1, (UINT32)(n2 - n1));

    /* F-Pos: 1ms 经时,纳秒增量 >= 800000(下界 80% 容差,忙等可能被中断拉长,无严格上界) */
    delta = n2 - n1;
    ICUNIT_ASSERT_EQUAL((delta >= 800000), 1, (UINT32)delta);

    return LOS_OK;
}

VOID ItLosTick007(VOID)
{
    TEST_ADD_CASE("ItLosTick007", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}
