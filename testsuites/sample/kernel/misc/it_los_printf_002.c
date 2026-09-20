/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
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

#include "it_los_misc.h"
#include "los_printf.h"

/* 用例简要描述: LOS_Printf（对外接口）负路径：1) 日志级别高于 PRINT_LEVEL 被
 * 过滤直接返回；2) fmt 为 NULL 时 vsnprintf_s 失败进入 ErrorMsg 提示路径。 */

static UINT32 TestCase(VOID)
{
    /* F-Pos: 常规级别正常输出 */
    LOS_Printf(LOG_MODULE_KERNEL, LOG_COMMON_LEVEL, "ItLosPrintf002 common %d\n", 1);

    /* F-Inv: 级别 > PRINT_LEVEL → OsLogLevelCheck 失败，直接返回不输出 */
    LOS_Printf(LOG_MODULE_KERNEL, LOG_DEBUG_LEVEL + 1, "ItLosPrintf002 filtered\n");

    /* F-Inv: fmt = NULL → vsnprintf_s 返回 -1 → ErrorMsg 提示 */
    LOS_Printf(LOG_MODULE_KERNEL, LOG_COMMON_LEVEL, NULL);

    return LOS_OK;
}

VOID ItLosPrintf002(VOID)
{
    TEST_ADD_CASE("ItLosPrintf002", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL1, TEST_FUNCTION);
}
