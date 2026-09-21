/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided under the following conditions:
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
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF THE POSSIBILITY OF SUCH
 * DAMAGE IS ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "It_los_shell.h"
#include "shmsg.h"

#if defined(LOSCFG_SHELL) && defined(LOSCFG_SHELL_LK)

/* 测试副作用声明: osCmdReg 无卸载接口,本用例注册的 "tc_exec7"/"tc_execs7" 命令将永久
 * 保留在命令表中;命令名使用 "tc_" 前缀,不会与真实命令冲突,无碍后续用例 */
static volatile UINT32 g_execCalls;    /* CMD_TYPE_EX 命令回调计数 */
static volatile UINT32 g_execStdCalls; /* CMD_TYPE_STD 命令回调计数 */
static UINT32 g_execArgc;
static CHAR g_execArg0[CMD_KEY_LEN];
static CHAR g_execArg1[CMD_KEY_LEN];
static UINT32 g_execStdArgc;
static CHAR g_execStdArg0[CMD_KEY_LEN];
static CHAR g_execStdArg1[CMD_KEY_LEN];

/* 安全拷贝: 逐字符拷贝并保证结尾 '\0',不依赖 strcpy/securec */
static VOID ShellTestArgCpy(CHAR *dst, const CHAR *src, UINT32 size)
{
    UINT32 i;

    for (i = 0; (i + 1) < size && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

/* EX 型命令回调: argv[0] 起为实际参数(OsCmdParseOneToken 对 index==0 且非 CMD_TYPE_STD 跳过) */
static UINT32 ShellExecTestHook(UINT32 argc, const CHAR **argv)
{
    g_execCalls++;
    g_execArgc = argc;
    g_execArg0[0] = '\0';
    g_execArg1[0] = '\0';
    if (argv != NULL) {
        if (argv[0] != NULL) {
            ShellTestArgCpy(g_execArg0, argv[0], sizeof(g_execArg0));
        }
        if (argv[1] != NULL) {
            ShellTestArgCpy(g_execArg1, argv[1], sizeof(g_execArg1));
        }
    }
    return LOS_OK;
}

/* STD 型命令回调: 命令名本身作为 argv[0] 传入(OsCmdParseOneToken 对 CMD_TYPE_STD 的 index==0 也入表) */
static UINT32 ShellExecStdTestHook(UINT32 argc, const CHAR **argv)
{
    g_execStdCalls++;
    g_execStdArgc = argc;
    g_execStdArg0[0] = '\0';
    g_execStdArg1[0] = '\0';
    if (argv != NULL) {
        if (argv[0] != NULL) {
            ShellTestArgCpy(g_execStdArg0, argv[0], sizeof(g_execStdArg0));
        }
        if (argv[1] != NULL) {
            ShellTestArgCpy(g_execStdArg1, argv[1], sizeof(g_execStdArg1));
        }
    }
    return LOS_OK;
}

/* 用例简要描述: ExecCmdline→OsCmdParse→OsCmdExec 命令行解析分发链: 无参/带参/空格压缩分发,
   EX 与 STD 型 argv 语义差异,command not found 负路径与空行/NULL 容错(均为副作用断言) */
static UINT32 Testcase(VOID)
{
    UINT32 ret;

    /* 前置: ExecCmdline(shmsg.c) 内部惰性调 OsShellInit(g_shellSourceFlag 幂等);
     * 此处显式调用一次确保命令注册表就绪,再走纯分发路径 */
    ret = OsShellInit();
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    g_execCalls = 0;
    g_execStdCalls = 0;

    /* F-Pos: 注册 EX/STD 两个自定义命令 */
    ret = osCmdReg(CMD_TYPE_EX, "tc_exec7", XARGS, ShellExecTestHook);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = osCmdReg(CMD_TYPE_STD, "tc_execs7", XARGS, ShellExecStdTestHook);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Pos: ExecCmdline 无参分发,回调计数+1 且 argc=0 */
    ExecCmdline("tc_exec7");
    ICUNIT_ASSERT_EQUAL(g_execCalls, 1, g_execCalls);
    ICUNIT_ASSERT_EQUAL(g_execArgc, 0, g_execArgc);

    /* F-Pos: ExecCmdline 带参分发,argc=2 且 argv 内容正确(EX 型: argv[0] 起为实际参数) */
    ExecCmdline("tc_exec7 aa bb");
    ICUNIT_ASSERT_EQUAL(g_execCalls, 2, g_execCalls);
    ICUNIT_ASSERT_EQUAL(g_execArgc, 2, g_execArgc);
    ICUNIT_ASSERT_STRING_EQUAL(g_execArg0, "aa", 0);
    ICUNIT_ASSERT_STRING_EQUAL(g_execArg1, "bb", 0);

    /* F-Pos: 前导/连续/尾部多余空格被压缩后仍正确分发(OsCmdKeyShift 路径) */
    ExecCmdline("   tc_exec7    cc ");
    ICUNIT_ASSERT_EQUAL(g_execCalls, 3, g_execCalls);
    ICUNIT_ASSERT_EQUAL(g_execArgc, 1, g_execArgc);
    ICUNIT_ASSERT_STRING_EQUAL(g_execArg0, "cc", 0);

    /* F-Pos: STD 型命令分发,argv[0] 为命令名本身(与 EX 型语义差异) */
    ExecCmdline("tc_execs7 zz");
    ICUNIT_ASSERT_EQUAL(g_execStdCalls, 1, g_execStdCalls);
    ICUNIT_ASSERT_EQUAL(g_execStdArgc, 2, g_execStdArgc);
    ICUNIT_ASSERT_STRING_EQUAL(g_execStdArg0, "tc_execs7", 0);
    ICUNIT_ASSERT_STRING_EQUAL(g_execStdArg1, "zz", 0);

    /* F-Inv: command not found 负路径: 仅打印提示不崩溃,两个回调计数均不变 */
    ExecCmdline("tc_nosuch7");
    ICUNIT_ASSERT_EQUAL(g_execCalls, 3, g_execCalls);
    ICUNIT_ASSERT_EQUAL(g_execStdCalls, 1, g_execStdCalls);

    /* F-Inv: 空串容错(PreHandleCmdline 返回 SH_NOK 直接返回),回调计数不变 */
    ExecCmdline("");
    ICUNIT_ASSERT_EQUAL(g_execCalls, 3, g_execCalls);

    /* F-Inv: 纯空格容错(压缩后长度为 0),回调计数不变 */
    ExecCmdline("   ");
    ICUNIT_ASSERT_EQUAL(g_execCalls, 3, g_execCalls);

    /* F-Inv: NULL 入参容错(ExecCmdline 起始即返回),回调计数不变 */
    ExecCmdline(NULL);
    ICUNIT_ASSERT_EQUAL(g_execCalls, 3, g_execCalls);

    return LOS_OK;
}

/*
 * @tc.name     ItLosShell007
 * @tc.desc     验证 ExecCmdline→OsCmdParse→OsCmdExec 命令行解析分发链:
 *              自定义命令无参/带参分发(断言回调计数与 argc/argv 传递)、
 *              EX/STD 型命令 argv 语义差异、command not found 负路径不崩溃、
 *              空串/纯空格/NULL 容错(ExecCmdline 返回 VOID,均为副作用断言)
 */
VOID ItLosShell007(VOID)
{
    TEST_ADD_CASE("ItLosShell007", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL1, TEST_FUNCTION);
}

#endif /* LOSCFG_SHELL && LOSCFG_SHELL_LK */
