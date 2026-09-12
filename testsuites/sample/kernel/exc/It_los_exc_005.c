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
static volatile UINT32 g_exc005HookCalled;
static UINT32  g_exc005RecType;
static UINT32  g_exc005RecMcause;
static UINT32  g_exc005RecMtval;
static UINTPTR g_exc005RecMepc;
static UINTPTR g_exc005RecStart;
static UINT32  g_exc005RecSpace;
static UINT32  g_exc005RecRw;
static UINTPTR g_exc005RecBuf;
static CHAR g_exc005Buf[256];

static VOID Exc005DumpHook(UINTPTR startAddr, UINT32 space, UINT32 rwFlag, CHAR *buf)
{
    g_exc005RecStart = startAddr;
    g_exc005RecSpace = space;
    g_exc005RecRw    = rwFlag;
    g_exc005RecBuf   = (UINTPTR)buf;
    g_exc005HookCalled = 1;
    PRINTF("[EXC-TEST005] dump hook: addr=0x%x space=%d rw=%d buf=0x%x\n",
           (UINT32)startAddr, space, rwFlag, (UINTPTR)buf);
}

static VOID Exc005ExcHook(UINT32 type, LosExcContext *context)
{
    g_exc005RecType   = type;
    g_exc005RecMcause = context->mcause;
    g_exc005RecMtval  = context->mtval;
    g_exc005RecMepc   = context->taskContext.mepc;
    PRINTF("[EXC-TEST005] exc hook: type=%u mcause=%u (expect 5 load access) mtval=0x%x mepc=0x%x\n",
           type, context->mcause, context->mtval, context->taskContext.mepc);
}

static UINT32 Testcase(VOID)
{
    LOS_ExcInfoRegHook((UINTPTR)0x12340000, sizeof(g_exc005Buf), g_exc005Buf, Exc005DumpHook);
    LOS_ExcRegHook(Exc005ExcHook);
    PRINTF("[EXC-TEST005] triggering load access fault (read from NULL)...\n");
    (VOID)*(volatile UINT32 *)0;
    return LOS_OK;
}

/**
 * @ingroup TEST_EXC
 * @par TestCase_Number
 * ItLosExc005
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_ExcInfoRegHook
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: LOS_ExcInfoRegHook(0x12340000, 256, buf, dumpHook) + LOS_ExcRegHook(excHook)
 * step2: (VOID)*(volatile UINT32 *)0 triggers load access fault (mcause=5)
 * @par TestCase_Expected_Result
 * 1. exc hook entered with type=5, mcause=5, mtval=0x0
 * 2. dump hook called with addr=0x12340000 space=256 rw=0 buf=<buf>
 * 3. system hangs in while(1), watchdog reboots ~10s later
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * false
 * @par TestCase_Remark
 * DESTRUCTIVE/NON-RETURNING: needs LOS_KERNEL_TEST_MANUAL + LOSCFG_SHELL_EXCINFO_DUMP.
 */
VOID ItLosExc005(VOID)
{
    TEST_ADD_CASE("ItLosExc005", Testcase, TEST_LOS, TEST_EXC, TEST_LEVEL0, TEST_FUNCTION);
}
#endif /* LOSCFG_SHELL_EXCINFO_DUMP */
