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

/* ItLosTick010: LOS_TickTimerRegister 只测错误路径(成功路径会 memcpy 覆盖运行中 g_sysTickTimer 崩溃) */
/* 用例简要描述: LOS_TickTimerRegister 双 NULL/零 freq 早返错误码 */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    ArchTickTimer timer = {0}; /* freq=0,TickTimerCheck 在 freq 校验早返,不触发 memcpy */

    /* F-Inv: 双 NULL 入参 → LOS_ERRNO_SYS_PTR_NULL(los_tick.c:214) */
    ret = LOS_TickTimerRegister(NULL, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_PTR_NULL, ret);

    /* F-Inv: timer 非空但 freq=0 → TickTimerCheck 早返 LOS_ERRNO_SYS_CLOCK_INVALID
       (los_tick.c:138-141),不进入 memcpy 分支,不破坏运行系统 */
    ret = LOS_TickTimerRegister(&timer, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_CLOCK_INVALID, ret);

    return LOS_OK;
}

VOID ItLosTick010(VOID)
{
    TEST_ADD_CASE("ItLosTick010", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}
