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
static volatile UINT32 g_exc007HookCalled;
static UINT32  g_exc007RecType;
static UINT32  g_exc007RecMcause;
static UINT32  g_exc007RecMtval;
static UINTPTR g_exc007RecMepc;
static UINTPTR g_exc007RecStart;
static UINT32  g_exc007RecSpace;
static UINT32  g_exc007RecRw;
static UINTPTR g_exc007RecBuf;
static CHAR g_exc007Buf[256];

static VOID Exc007DumpHook(UINTPTR startAddr, UINT32 space, UINT32 rwFlag, CHAR *buf)
{
    g_exc007RecStart = startAddr;
    g_exc007RecSpace = space;
    g_exc007RecRw    = rwFlag;
    g_exc007RecBuf   = (UINTPTR)buf;
    g_exc007HookCalled = 1;
    PRINTF("[EXC-TEST007] dump hook: addr=0x%x space=%d rw=%d buf=0x%x\n",
           (UINT32)startAddr, space, rwFlag, (UINTPTR)buf);
}

static VOID Exc007ExcHook(UINT32 type, LosExcContext *context)
{
    g_exc007RecType   = type;
    g_exc007RecMcause = context->mcause;
    g_exc007RecMtval  = context->mtval;
    g_exc007RecMepc   = context->taskContext.mepc;
    PRINTF("[EXC-TEST007] exc hook: type=%u mcause=%u (expect 1 instruction access) mtval=0x%x mepc=0x%x\n",
           type, context->mcause, context->mtval, context->taskContext.mepc);
}

static UINT32 Testcase(VOID)
{
    void (*volatile fp)(void) = (void (*)(void))0;

    LOS_ExcInfoRegHook((UINTPTR)0x12340000, sizeof(g_exc007Buf), g_exc007Buf, Exc007DumpHook);
    LOS_ExcRegHook(Exc007ExcHook);
    PRINTF("[EXC-TEST007] triggering instruction access fault (call NULL)...\n");
    fp();
    return LOS_OK;
}

/**
 * @ingroup TEST_EXC
 * @par TestCase_Number
 * ItLosExc007
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_ExcInfoRegHook
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: LOS_ExcInfoRegHook(0x12340000, 256, buf, dumpHook) + LOS_ExcRegHook(excHook)
 * step2: fp() (NULL function pointer call) triggers instruction access fault (mcause=1)
 * @par TestCase_Expected_Result
 * 1. exc hook entered with type=1, mcause=1, mtval=0x0
 * 2. dump hook called with addr=0x12340000 space=256 rw=0 buf=<buf>
 * 3. system hangs in while(1), watchdog reboots ~10s later
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * false
 * @par TestCase_Remark
 * DESTRUCTIVE/NON-RETURNING: needs LOS_KERNEL_TEST_MANUAL + LOSCFG_SHELL_EXCINFO_DUMP.
 */
VOID ItLosExc007(VOID)
{
    TEST_ADD_CASE("ItLosExc007", Testcase, TEST_LOS, TEST_EXC, TEST_LEVEL0, TEST_FUNCTION);
}
#endif /* LOSCFG_SHELL_EXCINFO_DUMP */
