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

/* 用例简要描述: 通过 LOS_LmsAddrProtect 投毒后, 编译器插桩的 2/4 字节越界读写
 * 触发 __asan_load2/load4/store2 回调并命中错误钩子; 解除投毒后访问正常,
 * 覆盖 __asan_load2_noabort/__asan_store2_noabort 真实调用路径。 */

static volatile UINT32 g_lms029HookHit = 0;
static UINT32 g_lms029LastErrMod = 0xFFFF;

static VOID Lms029ErrHook(UINTPTR p, UINT32 size, UINT32 errMod)
{
    (VOID)p;
    (VOID)size;
    g_lms029HookHit++;
    g_lms029LastErrMod = errMod;
}

static UINT32 TestCase(VOID)
{
    UINT32 hits;
    CHAR *mem = (CHAR *)LOS_MemAlloc(g_testLmsPool, 64); /* 64: chunk size */
    ICUNIT_GOTO_NOT_EQUAL(mem, NULL, mem, EXIT);

    OsLmsErrorHookSet(Lms029ErrHook);
    g_lms029HookHit = 0;
    g_lms029LastErrMod = 0xFFFF;

    /* Poison the first 32 bytes of our own chunk: accesses below are reported
     * by LMS while the writes still land inside the allocation, so no real
     * memory corruption happens (safe on both bestfit and TLSF). */
    LOS_LmsAddrProtect((UINTPTR)mem, (UINTPTR)(mem + 32)); /* 32: poison len */

    /* 2-byte OOB load -> __asan_load2_noabort */
    volatile UINT16 *p16 = (UINT16 *)mem;
    UINT16 v16 = *p16;
    PRINTK("v16 = 0x%x\n", v16);

    /* 4-byte OOB load -> __asan_load4_noabort */
    volatile UINT32 *p32 = (UINT32 *)(mem + 4); /* 4: offset */
    UINT32 v32 = *p32;
    PRINTK("v32 = 0x%x\n", v32);

    /* 2-byte OOB store (lands inside our chunk) -> __asan_store2_noabort */
    volatile UINT16 *p16w = (UINT16 *)(mem + 16); /* 16: offset */
    *p16w = 0xABCD;

    hits = g_lms029HookHit;
    ICUNIT_GOTO_EQUAL((hits >= 3), 1, hits, EXIT); /* 3: load2 + load4 + store2 */
    ICUNIT_GOTO_EQUAL(g_lms029LastErrMod, STORE_ERRMODE, g_lms029LastErrMod, EXIT);

    /* Un-poison: subsequent accesses must not be reported. */
    LOS_LmsAddrDisableProtect((UINTPTR)mem, (UINTPTR)(mem + 32)); /* 32: poison len */
    g_lms029HookHit = 0;
    volatile UINT16 *p16ok = (UINT16 *)mem;
    UINT16 v16ok = *p16ok;
    PRINTK("v16ok = 0x%x\n", v16ok);
    hits = g_lms029HookHit;
    ICUNIT_GOTO_EQUAL(hits, 0, hits, EXIT);

EXIT:
    LOS_LmsAddrDisableProtect((UINTPTR)mem, (UINTPTR)(mem + 32)); /* 32: poison len */
    OsLmsErrorHookSet(NULL);
    (VOID)LOS_MemFree(g_testLmsPool, mem);
    return LOS_OK;
}

/* LmsTestPoisonedAccessCallbacks */
VOID ItLosLms029(void)
{
    TEST_ADD_CASE("ItLosLms029", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}
