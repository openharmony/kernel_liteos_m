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

#ifdef LOSCFG_SHELL_DMESG

#define SHELL_TEST_DMESG_A_SIZE 2048
#define SHELL_TEST_DMESG_B_SIZE 1024
#define SHELL_TEST_DMESG_X_LEN  64
#define SHELL_TEST_DMESG_Y_LEN  100

static CHAR g_dmesgBufA[SHELL_TEST_DMESG_A_SIZE];            /* 重装用静态日志池 A */
static CHAR g_dmesgBufB[SHELL_TEST_DMESG_B_SIZE];            /* 重装用静态日志池 B */
static CHAR g_dmesgRead[SHELL_TEST_DMESG_A_SIZE];            /* 读回缓冲 */
static CHAR g_dmesgFill[SHELL_TEST_DMESG_A_SIZE + 512];      /* 写入填充缓冲(可超一圈) */

/* 用例简要描述: dmesg 边界深化: 环绕写入后读回内容一致性(全量覆盖+精确字节序)、
   LvSet/LvGet 各档往返一致、Clear 后重装新 buffer 旧数据不残留且 Read 返回 0 */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 savedLevel;
    INT32 readRet;
    UINT32 logAreaA = sizeof(g_dmesgBufA) - sizeof(DmesgInfo);
    UINT32 logAreaB = sizeof(g_dmesgBufB) - sizeof(DmesgInfo);
    UINT32 i;
    UINT32 round;
    UINT32 ok;
    UINT32 badIdx;

    savedLevel = OsDmesgLvGet();

    /* step1: 安装 2KB 静态日志池并清空 */
    ret = LOS_DmesgMemSet(g_dmesgBufA, sizeof(g_dmesgBufA));
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_DmesgClear();

    /* step2: 环绕全量覆盖: 写满一圈 'A' 后继续写超过容量的 'B',
     * 读回应为满 logAreaA 字节且全为 'B'(最新),不含 'A'(最旧,已被环绕覆盖) */
    (VOID)memset(g_dmesgFill, 'A', logAreaA);
    (VOID)OsLogRecordStr(g_dmesgFill, logAreaA);
    (VOID)memset(g_dmesgFill, 'B', logAreaA + 256);
    (VOID)OsLogRecordStr(g_dmesgFill, logAreaA + 256);

    readRet = LOS_DmesgRead(g_dmesgRead, logAreaA);
    ICUNIT_GOTO_EQUAL(readRet, (INT32)logAreaA, readRet, EXIT);
    ok = 1;
    for (i = 0; i < logAreaA; i++) {
        if (g_dmesgRead[i] != 'B') {
            ok = 0;
            break;
        }
    }
    ICUNIT_GOTO_EQUAL(ok, 1, i, EXIT);

    /* step3: 环绕字节序精确性: 连写 3 段(每段 logAreaA,总量 3*logAreaA > 2 圈),
     * 第 round 段内容为 '0'+((i+round)%10); 因总量恰为圈长整数倍,环绕结束后
     * 缓冲区内容应恰为最后一段(round=2) */
    LOS_DmesgClear();
    for (round = 0; round < 3; round++) {
        for (i = 0; i < logAreaA; i++) {
            g_dmesgFill[i] = (CHAR)('0' + ((i + round) % 10));
        }
        (VOID)OsLogRecordStr(g_dmesgFill, logAreaA);
    }
    readRet = LOS_DmesgRead(g_dmesgRead, logAreaA);
    ICUNIT_GOTO_EQUAL(readRet, (INT32)logAreaA, readRet, EXIT);
    badIdx = 0;
    for (i = 0; i < logAreaA; i++) {
        if (g_dmesgRead[i] != (CHAR)('0' + ((i + 2) % 10))) {
            badIdx = i + 1;
            break;
        }
    }
    ICUNIT_GOTO_EQUAL(badIdx, 0, badIdx, EXIT);

    /* step4: LvSet/LvGet 往返一致性(0~5 各档) */
    for (i = 0; i <= 5; i++) {
        ret = LOS_DmesgLvSet(i);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
        ret = OsDmesgLvGet();
        ICUNIT_GOTO_EQUAL(ret, i, ret, EXIT);
    }

    /* step5: LvSet 越界(6 > 5)返回 LOS_NOK 且级别保持为 5 */
    ret = LOS_DmesgLvSet(6);
    ICUNIT_GOTO_EQUAL(ret, LOS_NOK, ret, EXIT);
    ret = OsDmesgLvGet();
    ICUNIT_GOTO_EQUAL(ret, 5, ret, EXIT);

    /* step6: 写入 marker('X') → Clear → 重装新 buffer(1KB 静态池 B):
     * Read 返回 0 且新池日志区无 'X'/旧数据残留(重装前将池 B 清零做基线) */
    (VOID)memset(g_dmesgFill, 'X', SHELL_TEST_DMESG_X_LEN);
    (VOID)OsLogRecordStr(g_dmesgFill, SHELL_TEST_DMESG_X_LEN);
    LOS_DmesgClear();
    (VOID)memset(g_dmesgBufB, 0, sizeof(g_dmesgBufB));
    ret = LOS_DmesgMemSet(g_dmesgBufB, sizeof(g_dmesgBufB));
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    readRet = LOS_DmesgRead(g_dmesgRead, logAreaB);
    ICUNIT_GOTO_EQUAL(readRet, 0, readRet, EXIT);
    ok = 1;
    for (i = 0; i < logAreaB; i++) {
        if (g_dmesgBufB[sizeof(DmesgInfo) + i] != 0) {
            ok = 0;
            break;
        }
    }
    ICUNIT_GOTO_EQUAL(ok, 1, i, EXIT);

    /* step7: 重装后新池可正常写入/读回('Y' marker 往返一致) */
    (VOID)memset(g_dmesgFill, 'Y', SHELL_TEST_DMESG_Y_LEN);
    (VOID)OsLogRecordStr(g_dmesgFill, SHELL_TEST_DMESG_Y_LEN);
    readRet = LOS_DmesgRead(g_dmesgRead, SHELL_TEST_DMESG_Y_LEN);
    ICUNIT_GOTO_EQUAL(readRet, SHELL_TEST_DMESG_Y_LEN, readRet, EXIT);
    ok = 1;
    for (i = 0; i < SHELL_TEST_DMESG_Y_LEN; i++) {
        if (g_dmesgRead[i] != 'Y') {
            ok = 0;
            break;
        }
    }
    ICUNIT_GOTO_EQUAL(ok, 1, i, EXIT);

EXIT:
    /* 恢复(参照 001 模式): 清空当前日志 → 恢复保存的级别 → 重装默认 8KB malloc 池 */
    LOS_DmesgClear();
    (VOID)LOS_DmesgLvSet(savedLevel);
    ret = LOS_DmesgMemSet(NULL, KERNEL_LOG_BUF_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

/*
 * @tc.name     ItLosShell008
 * @tc.desc     dmesg 边界深化: 环形 buffer 写满回绕后读回内容一致性(全量
 *              覆盖断言+逐字节序断言)、LOS_DmesgLvSet/OsDmesgLvGet 各档
 *              往返一致与越界拒绝、Clear 后 LOS_DmesgMemSet 重装新 buffer
 *              旧数据不残留(Read 返回 0)且新池写入读回正常
 */
VOID ItLosShell008(VOID)
{
    TEST_ADD_CASE("ItLosShell008", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL1, TEST_FUNCTION);
}

#endif /* LOSCFG_SHELL_DMESG */
