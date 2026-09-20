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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include "osTest.h"
#include "It_los_lms.h"

/* 用例简要描述: 仅通过对外接口 LOS_LmsAddrProtect/LOS_LmsAddrDisableProtect 将
 * 本用例自有分配块投毒, 再以 1/2/4/8 字节宽度的自然内存读写验证 LMS 检测:
 * 编译器 -fsanitize=kernel-address 插桩会按访问宽度生成对应的 asan 回调,
 * 越界读写命中错误钩子; 解除保护后相同访问不再触发报告。所有写操作
 * 都落在自己的分配块内, 不产生真实越界(bestfit 与 TLSF 均安全)。 */

static volatile UINT32 g_lms030HookHit = 0;
static UINT32 g_lms030LastErrMod = 0xFFFF;

static VOID Lms030ErrHook(UINTPTR p, UINT32 size, UINT32 errMod)
{
    (VOID)p;
    (VOID)size;
    g_lms030HookHit++;
    g_lms030LastErrMod = errMod;
}

static UINT32 TestCase(VOID)
{
    UINT32 hits;
    UINTPTR base;
    CHAR *mem = (CHAR *)LOS_MemAlloc(g_testLmsPool, 64); /* 64: chunk size */
    ICUNIT_GOTO_NOT_EQUAL(mem, NULL, mem, EXIT);

    OsLmsErrorHookSet(Lms030ErrHook);
    g_lms030HookHit = 0;
    g_lms030LastErrMod = 0xFFFF;

    LOS_LmsAddrProtect((UINTPTR)mem, (UINTPTR)(mem + 64)); /* 64: poison len */
    base = ((UINTPTR)mem + 7) & ~(UINTPTR)0x7; /* 8-byte aligned access base */

    /* 8/4/2/1 字节读: __asan_load8/load4/load2/load1 */
    volatile UINT64 *p64 = (UINT64 *)base;
    volatile UINT32 *p32 = (UINT32 *)(base + 8); /* 8: offset */
    volatile UINT16 *p16 = (UINT16 *)(base + 16); /* 16: offset */
    volatile UINT8 *p8 = (UINT8 *)(base + 24); /* 24: offset */
    UINT64 v64 = *p64;
    UINT32 v32 = *p32;
    UINT16 v16 = *p16;
    UINT8 v8 = *p8;
    PRINTK("lms030 read: 0x%x 0x%x 0x%x 0x%x\n", (UINT32)v64, v32, v16, v8);

    hits = g_lms030HookHit;
    ICUNIT_GOTO_EQUAL((hits >= 4), 1, hits, EXIT); /* 4: four width loads */
    ICUNIT_GOTO_EQUAL(g_lms030LastErrMod, LOAD_ERRMODE, g_lms030LastErrMod, EXIT);

    /* 上面 4 次读各触发一份关中断的 LMS 错误报告, 先喂狗再触发写报告,
     * 避免 8 份报告连续关中断打印耗尽看门狗窗口(仅 WS63+LMS 生效)。 */
    LMS_FEED_WDT();

    /* 8/4/2/1 字节写(落在自己块内): __asan_store8/store4/store2/store1 */
    *p64 = 0x1122334455667788ULL;
    *p32 = 0xAA55AA55;
    *p16 = 0xBCBC;
    *p8 = 0x5A;

    hits = g_lms030HookHit;
    ICUNIT_GOTO_EQUAL((hits >= 8), 1, hits, EXIT); /* 8: loads + stores */
    ICUNIT_GOTO_EQUAL(g_lms030LastErrMod, STORE_ERRMODE, g_lms030LastErrMod, EXIT);

    /* 解除保护后相同访问不再触发报告 */
    LOS_LmsAddrDisableProtect((UINTPTR)mem, (UINTPTR)(mem + 64)); /* 64: poison len */
    g_lms030HookHit = 0;
    UINT32 vok = *p32;
    *p32 = vok;
    hits = g_lms030HookHit;
    ICUNIT_GOTO_EQUAL(hits, 0, hits, EXIT);

EXIT:
    LOS_LmsAddrDisableProtect((UINTPTR)mem, (UINTPTR)(mem + 64)); /* 64: poison len */
    OsLmsErrorHookSet(NULL);
    (VOID)LOS_MemFree(g_testLmsPool, mem);
    return LOS_OK;
}

/* LmsTestPoisonedMultiWidthAccess */
VOID ItLosLms030(void)
{
    TEST_ADD_CASE("ItLosLms030", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}
