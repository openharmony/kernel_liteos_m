/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of the conditions and the following disclaimer in the documentation and/or other materials
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

#include "it_los_exc.h"

#ifdef LOSCFG_SHELL_EXCINFO_DUMP
static volatile UINT32 g_exc004HookCalled;
static UINTPTR g_exc004RecStart;
static UINT32  g_exc004RecSpace;
static UINT32  g_exc004RecRw;
static UINTPTR g_exc004RecBuf;
static CHAR g_exc004Buf[256];

static VOID Exc004DumpHook(UINTPTR startAddr, UINT32 space, UINT32 rwFlag, CHAR *buf)
{
    g_exc004RecStart = startAddr;
    g_exc004RecSpace = space;
    g_exc004RecRw    = rwFlag;
    g_exc004RecBuf   = (UINTPTR)buf;
    g_exc004HookCalled = 1;
    PRINTF("[EXC-TEST004] dump hook: addr=0x%x space=%d rw=%d buf=0x%x\n",
           (UINT32)startAddr, space, rwFlag, (UINTPTR)buf);
}

static VOID Exc004ExcHook(UINT32 type, LosExcContext *context)
{
    (VOID)type;
    (VOID)context;
    PRINTF("[EXC-TEST004] exc hook entered, type=%u\n", type);
}

static UINT32 Testcase(VOID)
{
    LOS_ExcInfoRegHook((UINTPTR)0x12340000, sizeof(g_exc004Buf), g_exc004Buf, Exc004DumpHook);
    LOS_ExcRegHook(Exc004ExcHook);
    PRINTF("[EXC-TEST004] triggering fault (wild store to NULL)...\n");
    *(volatile UINT32 *)0 = 0xDEAD;
    return LOS_OK;
}

/**
 * @ingroup TEST_EXC
 * @par TestCase_Number
 * ItLosExc004
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_ExcInfoRegHook
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: LOS_ExcInfoRegHook(0x12340000, 256, buf, dumpHook) + LOS_ExcRegHook(excHook)
 * step2: *(volatile UINT32 *)0 = 0xDEAD triggers store access fault (mcause=7)
 * @par TestCase_Expected_Result
 * 1. exc hook entered with type=7
 * 2. dump hook called with addr=0x12340000 space=256 rw=0 buf=<buf>
 * 3. system hangs in while(1), watchdog reboots ~10s later
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * false
 * @par TestCase_Remark
 * DESTRUCTIVE/NON-RETURNING: needs LOS_KERNEL_TEST_MANUAL + LOSCFG_SHELL_EXCINFO_DUMP.
 */
VOID ItLosExc004(VOID)
{
    TEST_ADD_CASE("ItLosExc004", Testcase, TEST_LOS, TEST_EXC, TEST_LEVEL0, TEST_FUNCTION);
}
#endif /* LOSCFG_SHELL_EXCINFO_DUMP */
