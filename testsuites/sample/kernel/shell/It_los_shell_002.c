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
 *    of the conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
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

#ifdef LOSCFG_SHELL_LK

static volatile UINT32 g_hookCalls;
static volatile UINT32 g_hook2Calls;
static INT32 g_lastLevel;
static const CHAR *g_lastFunc;
static INT32 g_lastLine;

static VOID ShellTestHook(INT32 level, const CHAR *func, INT32 line, const CHAR *fmt, va_list ap)
{
    (VOID)fmt;
    (VOID)ap;
    g_hookCalls++;
    g_lastLevel = level;
    g_lastFunc = func;
    g_lastLine = line;
}

static VOID ShellTestHook2(INT32 level, const CHAR *func, INT32 line, const CHAR *fmt, va_list ap)
{
    (VOID)level;
    (VOID)func;
    (VOID)line;
    (VOID)fmt;
    (VOID)ap;
    g_hook2Calls++;
}

static UINT32 Testcase(VOID)
{
    g_hookCalls = 0;
    g_hook2Calls = 0;
    LOS_LkRegHook(ShellTestHook);
    LOS_LkPrint(TRACE_ERROR, __func__, __LINE__, "test %d", 42);
    ICUNIT_GOTO_EQUAL(g_hookCalls, 1, g_hookCalls, EXIT);
    ICUNIT_GOTO_EQUAL(g_lastLevel, TRACE_ERROR, g_lastLevel, EXIT);
    ICUNIT_GOTO_EQUAL(g_lastFunc, __func__, 0, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(g_lastLine, 0, g_lastLine, EXIT);

    g_hookCalls = 0;
    g_hook2Calls = 0;
    LOS_LkRegHook(ShellTestHook2);
    LOS_LkPrint(TRACE_INFO, __func__, __LINE__, "re-register");
    ICUNIT_GOTO_EQUAL(g_hook2Calls, 1, g_hook2Calls, EXIT);
    ICUNIT_GOTO_EQUAL(g_hookCalls, 0, g_hookCalls, EXIT);

    g_hook2Calls = 0;
    LOS_LkRegHook(NULL);
    LOS_LkPrint(TRACE_DEBUG, __func__, __LINE__, "after null");
    ICUNIT_GOTO_EQUAL(g_hook2Calls, 0, g_hook2Calls, EXIT);

EXIT:
    OsLkLoggerInit("");

    return LOS_OK;
}

/**
 * @ingroup TEST_SHELL
 * @par TestCase_Number
 * ItLosShell002
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_LkRegHook / LOS_LkPrint
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: register a counting hook, call LOS_LkPrint, verify hook invoked with forwarded level/func/line
 * step2: re-register a second hook, call LOS_LkPrint, verify only the second hook is called
 * step3: register NULL hook, call LOS_LkPrint, verify no crash and hook not called
 * step4: cleanup (runs on pass and fail paths): restore the default logger
 *        hook via OsLkLoggerInit
 * @par TestCase_Expected_Result
 * 1. custom hook is invoked exactly once with matching level/func/line
 * 2. re-registration replaces the previous hook; old hook is not called
 * 3. NULL hook path does not crash
 * 4. default logger restored after the case on every path
 * @par TestCase_Level
 * Level 1
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */
VOID ItLosShell002(VOID)
{
    TEST_ADD_CASE("ItLosShell002", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL1, TEST_FUNCTION);
}

#endif /* LOSCFG_SHELL_LK */
