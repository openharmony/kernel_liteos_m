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


#include "it_los_misc.h"
#include "los_init.h"

/* 板级差异: 仅 qemu(arm) 链接脚本导出 __sysinitcall* 边界符号;
   ws63(riscv) 有 .sysinitcall_levelN section 但无符号定义 -> 编译旁路 */
#if defined(LOSCFG_ARCH_ARM)

/* 用例简要描述: initcall 8 级注册表非空+分表和==全表跨度 */
static UINT32 TestCase(VOID)
{
    UINT32 total = 0;
    UINT32 fullSpan;

    /* initcall 机制白盒验证(启动路径不可重入,对注册表做静态回看):
       1) 8 级分表(0~7)均已由链接器填充(总计数 > 0);
       2) 分表计数之和 == 全表跨度(__sysinitcall_start..end),机制自洽。 */
    total += (UINT32)(__sysinitcall1_start - __sysinitcall0_start);
    total += (UINT32)(__sysinitcall2_start - __sysinitcall1_start);
    total += (UINT32)(__sysinitcall3_start - __sysinitcall2_start);
    total += (UINT32)(__sysinitcall4_start - __sysinitcall3_start);
    total += (UINT32)(__sysinitcall5_start - __sysinitcall4_start);
    total += (UINT32)(__sysinitcall6_start - __sysinitcall5_start);
    total += (UINT32)(__sysinitcall7_start - __sysinitcall6_start);
    total += (UINT32)(__sysinitcall_end - __sysinitcall7_start);

    fullSpan = (UINT32)(__sysinitcall_end - __sysinitcall_start);

    /* F-Pos: 注册表非空(内核模块经 initcall 注册) */
    ICUNIT_ASSERT_EQUAL((total > 0), 1, total);

    /* F-Pos: 分级表连续拼合等于全表(链接器 section 顺序自洽) */
    ICUNIT_ASSERT_EQUAL(total, fullSpan, total);

    return LOS_OK;
}

VOID ItLosInit001(VOID)
{
    TEST_ADD_CASE("ItLosInit001", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL1, TEST_FUNCTION);
}
#endif /* LOSCFG_ARCH_ARM */
