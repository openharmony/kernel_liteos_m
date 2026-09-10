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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "It_los_cpup.h"

#if (LOSCFG_CPUP_START_STOP == 1)
/* 用例简要描述: LOS_CpupStart/Stop 闭环+UsageMonitor 成功+恢复现场 */
static UINT32 TestCase008(VOID)
{
    UINT32 ret;

    /* F-Pos: 启动统计(幂等,已启动早返) */
    LOS_CpupStart();

    /* 让统计跑几个 tick */
    (VOID)LOS_TaskDelay(3); /* 3, delay ticks */

    /* 统计有效: 系统级 1s 档查询成功 */
    ret = LOS_CpupUsageMonitor(SYS_CPU_USAGE, CPUP_IN_1S, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Pos: 停止统计 */
    LOS_CpupStop();

    /* F-Pos: 再次启动(恢复现场,幂等) */
    LOS_CpupStart();

    return LOS_OK;
}
#endif

VOID ItLosCpup008(VOID)
{
#if (LOSCFG_CPUP_START_STOP == 1)
    TEST_ADD_CASE("ItLosCpup008", TestCase008, TEST_LOS, TEST_CPUP, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
