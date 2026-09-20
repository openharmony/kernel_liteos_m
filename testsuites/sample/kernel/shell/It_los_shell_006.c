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
#include "shcmd.h"
#include "los_memory.h"

#if defined(LOSCFG_SHELL) && defined(LOSCFG_SHELL_LK)

/* 回调返回特征值基址,用于断言 OsCmdExec 对自定义命令回调返回值的透传 */
#define SHELL_REG_TEST_RET_BASE 0x5A00

/* 测试副作用声明: osCmdReg 无卸载接口,本用例注册的 "tc_reg006" 命令将永久保留在
 * 命令表中;命令名使用 "tc_" 前缀,不会与 g_shellcmdAll 中的真实命令冲突,无碍后续用例 */
static volatile UINT32 g_regCalls;
static UINT32 g_lastArgc;
static CHAR g_lastArg0[CMD_KEY_LEN];
static CHAR g_lastArg1[CMD_KEY_LEN];

/* 安全拷贝: 逐字符拷贝并保证结尾 '\0',不依赖 strcpy/securec */
static VOID ShellTestKeyCpy(CHAR *dst, const CHAR *src, UINT32 size)
{
    UINT32 i;

    for (i = 0; (i + 1) < size && src[i] != '\0'; i++) {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

/* 自定义命令回调: 签名对照 shell.h 中 CmdCallBackFunc 定义 (UINT32 argc, const CHAR **argv) */
static UINT32 ShellRegTestHook(UINT32 argc, const CHAR **argv)
{
    g_regCalls++;
    g_lastArgc = argc;
    g_lastArg0[0] = '\0';
    g_lastArg1[0] = '\0';
    if (argv != NULL) {
        if (argv[0] != NULL) {
            ShellTestKeyCpy(g_lastArg0, argv[0], sizeof(g_lastArg0));
        }
        if (argv[1] != NULL) {
            ShellTestKeyCpy(g_lastArg1, argv[1], sizeof(g_lastArg1));
        }
    }
    return SHELL_REG_TEST_RET_BASE + argc;
}

/* 用例简要描述: osCmdReg 运行时注册正/负路径 + OsCmdExec 触发自定义回调(计数/argc/argv/返回值透传) + OsShellCmdHelp 直调 */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    INT32 retHelp;
    CmdParsed cmdParsed;
    const CHAR *argvHelp[1] = {"help"};

    /* 前置: 确保 shell 命令表已初始化(show.c 中 OsShellInit 由 g_shellSourceFlag 保证幂等) */
    ret = OsShellInit();
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    g_regCalls = 0;
    g_lastArgc = 0xFFFFFFFFU;

    /* F-Pos: 运行时注册自定义命令(CMD_TYPE_EX + XARGS)成功 */
    ret = osCmdReg(CMD_TYPE_EX, "tc_reg006", XARGS, ShellRegTestHook);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Inv: 重复注册同名同类型命令,返回 OS_ERRNO_SHELL_CMDREG_CMD_EXIST */
    ret = osCmdReg(CMD_TYPE_EX, "tc_reg006", XARGS, ShellRegTestHook);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SHELL_CMDREG_CMD_EXIST, ret);

    /* F-Inv: cmdKey 为 NULL,参数防御 */
    ret = osCmdReg(CMD_TYPE_EX, NULL, XARGS, ShellRegTestHook);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SHELL_CMDREG_PARA_ERROR, ret);

    /* F-Inv: 回调为 NULL,参数防御 */
    ret = osCmdReg(CMD_TYPE_EX, "tc_null006", XARGS, NULL);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SHELL_CMDREG_PARA_ERROR, ret);

    /* F-Inv: cmdType 越界(== CMD_TYPE_BUTT),参数防御 */
    ret = osCmdReg(CMD_TYPE_BUTT, "tc_butt006", XARGS, ShellRegTestHook);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SHELL_CMDREG_PARA_ERROR, ret);

    /* F-Inv: cmdKey 长度超限(恰 16 字符 == CMD_KEY_LEN) */
    ret = osCmdReg(CMD_TYPE_EX, "tc0123456789abcd", XARGS, ShellRegTestHook);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SHELL_CMDREG_PARA_ERROR, ret);

    /* F-Inv: paraNum 超过 CMD_MAX_PARAS 且非 XARGS */
    ret = osCmdReg(CMD_TYPE_EX, "tc_para006", CMD_MAX_PARAS + 1, ShellRegTestHook);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SHELL_CMDREG_PARA_ERROR, ret);

    /* F-Inv: cmdKey 含非法字符(OsCmdKeyCheck 拒绝) */
    ret = osCmdReg(CMD_TYPE_EX, "tc_b@d!", XARGS, ShellRegTestHook);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SHELL_CMDREG_CMD_ERROR, ret);

    /* F-Inv: cmdKey 为纯数字(OsCmdKeyCheck 拒绝纯数字首态) */
    ret = osCmdReg(CMD_TYPE_EX, "123", XARGS, ShellRegTestHook);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SHELL_CMDREG_CMD_ERROR, ret);

    /* F-Pos: OsCmdExec 无参触发: 回调计数+argc=0+返回值透传 */
    (VOID)memset(&cmdParsed, 0, sizeof(cmdParsed));
    cmdParsed.cmdType = CMD_TYPE_EX;
    ShellTestKeyCpy(cmdParsed.cmdKeyword, "tc_reg006", sizeof(cmdParsed.cmdKeyword));
    cmdParsed.paramCnt = 0;
    ret = OsCmdExec(&cmdParsed);
    ICUNIT_ASSERT_EQUAL(ret, SHELL_REG_TEST_RET_BASE, ret);
    ICUNIT_ASSERT_EQUAL(g_regCalls, 1, g_regCalls);
    ICUNIT_ASSERT_EQUAL(g_lastArgc, 0, g_lastArgc);

    /* F-Pos: OsCmdExec 带参触发: argc/argv 内容正确传递
     * 注: OsCmdExec 内部以 LOS_MemFree(m_aucSysMem0) 释放 paramArray 各元素,
     * 故此处必须用 LOS_MemAlloc(m_aucSysMem0) 分配,与 OsCmdParseStrdup 同堆 */
    (VOID)memset(&cmdParsed, 0, sizeof(cmdParsed));
    cmdParsed.cmdType = CMD_TYPE_EX;
    ShellTestKeyCpy(cmdParsed.cmdKeyword, "tc_reg006", sizeof(cmdParsed.cmdKeyword));
    cmdParsed.paramArray[0] = (CHAR *)LOS_MemAlloc(m_aucSysMem0, 8); /* "hello" + '\0' */
    ICUNIT_ASSERT_EQUAL((cmdParsed.paramArray[0] != NULL), 1, LOS_NOK);
    ShellTestKeyCpy(cmdParsed.paramArray[0], "hello", 8);
    cmdParsed.paramCnt = 1;
    g_lastArgc = 0xFFFFFFFFU;
    ret = OsCmdExec(&cmdParsed);
    ICUNIT_ASSERT_EQUAL(ret, SHELL_REG_TEST_RET_BASE + 1, ret);
    ICUNIT_ASSERT_EQUAL(g_lastArgc, 1, g_lastArgc);
    ICUNIT_ASSERT_STRING_EQUAL(g_lastArg0, "hello", 0);

    /* F-Inv: OsCmdExec 分发未注册命令,返回 OS_ERROR */
    (VOID)memset(&cmdParsed, 0, sizeof(cmdParsed));
    cmdParsed.cmdType = CMD_TYPE_EX;
    ShellTestKeyCpy(cmdParsed.cmdKeyword, "tc_nosuch006", sizeof(cmdParsed.cmdKeyword));
    cmdParsed.paramCnt = 0;
    ret = OsCmdExec(&cmdParsed);
    ICUNIT_ASSERT_EQUAL(ret, (UINT32)OS_ERROR, ret);

    /* F-Inv: OsCmdExec NULL 入参防御,返回 OS_ERROR */
    ret = OsCmdExec(NULL);
    ICUNIT_ASSERT_EQUAL(ret, (UINT32)OS_ERROR, ret);

    /* F-Pos: OsShellCmdHelp 直调(argc=0),经注册表遍历打印全部命令,返回 0 */
    retHelp = OsShellCmdHelp(0, NULL);
    ICUNIT_ASSERT_EQUAL(retHelp, 0, retHelp);

    /* F-Inv: OsShellCmdHelp 带参直调,打印 Usage 并返回 OS_ERROR */
    retHelp = OsShellCmdHelp(1, argvHelp);
    ICUNIT_ASSERT_EQUAL(retHelp, (INT32)OS_ERROR, retHelp);

    /* 负路径均未误触自定义回调 */
    ICUNIT_ASSERT_EQUAL(g_regCalls, 2, g_regCalls);

    return LOS_OK;
}

/*
 * @tc.name     ItLosShell006
 * @tc.desc     验证 shell 框架 osCmdReg 运行时注册能力: 合法注册/重复注册返回
 *              OS_ERRNO_SHELL_CMDREG_CMD_EXIST/NULL cmdKey 等非法参数防御;
 *              注册后经 OsCmdExec 触发自定义回调,断言回调计数/argc/argv 内容
 *              与回调返回值透传; 附 OsShellCmdHelp 直调正负路径
 */
VOID ItLosShell006(VOID)
{
    TEST_ADD_CASE("ItLosShell006", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL1, TEST_FUNCTION);
}

#endif /* LOSCFG_SHELL && LOSCFG_SHELL_LK */
