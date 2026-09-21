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

#include "It_los_shell.h"

/* 用例简要描述: LOS_LkPrint（shell LK 对外接口）输出路径：1) level <= 默认 trace 级别
 * （TRACE_DEFAULT=TRACE_ERROR=2）→ OsLkDefaultFunc 主路径 → LkDprintf 控制台输出；
 * 2) level > trace 级别且 <= dmesg 级别 → DMESG 记录路径 → DmesgPrintf（需
 * LOSCFG_SHELL_DMESG）。抬高的 dmesg 级别经 EXIT 统一恢复，异常路径不留痕。 */

#if defined(LOSCFG_SHELL_LK)

static UINT32 Testcase(VOID)
{
#ifdef LOSCFG_SHELL_DMESG
    UINT32 ret;
#endif

    /* F-Pos: level=TRACE_ERROR(2) <= 默认 trace 级别(2) -> LkDprintf 主输出路径
     *（含级别前缀打印分支：level!=COMMON 且 0<level<DEBUG）。 */
    LOS_LkPrint(TRACE_ERROR, "ItLosShell009", __LINE__, "lk error path %d\n", 1);

    /* F-Pos: level=TRACE_EMG(0) <= trace 级别且命中"无前缀"分支（level==EMG 跳过前缀）-> LkDprintf。 */
    LOS_LkPrint(TRACE_EMG, "ItLosShell009", __LINE__, "lk emg path %d\n", 2);

#ifndef LOSCFG_SHELL_DMESG
    /* 无 DMESG 的板：无长驻现场（trace 级别未被本用例修改），直接返回。 */
    return LOS_OK;
#else
    /* shell init 将 trace/dmesg 级别均设为 TRACE_DEFAULT(2)，不存在同时满足
     * ">trace 且 <=dmesg" 的 level。先用公共 API 抬高 dmesg 级别再触发记录路径。 */
    ret = LOS_DmesgLvSet(TRACE_DEBUG);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Pos: level=TRACE_WARN(3) > trace(2) 且 <= dmesg(5) -> OsLogCycleRecord + DmesgPrintf。 */
    LOS_LkPrint(TRACE_WARN, "ItLosShell009", __LINE__, "dmesg record path %d\n", 3);

    /* F-Pos: level=TRACE_DEBUG(5) > trace(2) 且 <= dmesg(5) -> 亦走 DmesgPrintf（无前缀边界）。 */
    LOS_LkPrint(TRACE_DEBUG, "ItLosShell009", __LINE__, "dmesg debug path %d\n", 4);

    return LOS_OK;

EXIT:
    /* 异常路径同样恢复 dmesg 默认打印级别（TRACE_ERROR == shell_lk.c 私有 TRACE_DEFAULT），不留痕。 */
    (VOID)LOS_DmesgLvSet(TRACE_ERROR);
    return LOS_OK;
#endif
}

VOID ItLosShell009(VOID)
{
    TEST_ADD_CASE("ItLosShell009", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL1, TEST_FUNCTION);
}
#endif /* LOSCFG_SHELL_LK */
