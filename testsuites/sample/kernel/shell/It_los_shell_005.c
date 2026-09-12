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


#include "It_los_shell.h"
#include "shcmd.h"

/* 用例简要描述: DumpTask/Free/Date 命令正路径+DumpTask 多参负路径 */
static UINT32 Testcase(VOID)
{
    INT32 ret;
    UINT32 retU;
    const CHAR *argvDate[1] = {"date"};

    /* F-Pos: task 命令无参模式(全部任务信息),OsShellCmdTskInfoGet 路径 */
    ret = OsShellCmdDumpTask(0, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Pos: swtmr 命令无参模式(全部定时器信息),受 LOSCFG_DEBUG_SWTMR 守卫(两板未开,编译旁路) */
#if defined(LOSCFG_DEBUG_SWTMR)
    retU = OsShellCmdSwtmrInfoGet(0, NULL);
    ICUNIT_ASSERT_EQUAL(retU, LOS_OK, retU);
#endif

    /* F-Pos: free 命令无参模式(内存水位信息) */
    retU = OsShellCmdFree(0, NULL);
    ICUNIT_ASSERT_EQUAL(retU, LOS_OK, retU);

    /* F-Pos: date 命令单参模式(打印当前时间,gettimeofday 路径) */
    ret = OsShellCmdDate(1, argvDate);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Inv: task 命令多参模式(Usage 提示+错误返回) */
    ret = OsShellCmdDumpTask(2, NULL);
    ICUNIT_ASSERT_EQUAL(ret, (INT32)OS_ERROR, ret);

    return LOS_OK;
}

VOID ItLosShell005(VOID)
{
    TEST_ADD_CASE("ItLosShell005", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL1, TEST_FUNCTION);
}
