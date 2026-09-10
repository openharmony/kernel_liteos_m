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
 *    of conditions and the following disclaimer in the documentation and/or other materials
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

#include "osTest.h"
#include "It_los_mem.h"


#ifdef LOSCFG_KERNEL_MEM_TLSF

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 size;
    void *p = NULL;

    MemStart();

    MemInit();

    /* Enable no-lock mode on the pool */
    LOS_MemUnlockEnable(g_memPool);

    /* Alloc and free without internal lock - should work in single-threaded test */
    size = 0x100;
    p = LOS_MemAlloc(g_memPool, size);
    ICUNIT_GOTO_NOT_EQUAL(p, NULL, 0, EXIT);

    (void)memset_s(p, size, 0xAA, size);

    ret = LOS_MemFree(g_memPool, p);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Test realloc without lock */
    p = LOS_MemAlloc(g_memPool, 0x80);
    ICUNIT_GOTO_NOT_EQUAL(p, NULL, 0, EXIT);

    p = LOS_MemRealloc(g_memPool, p, 0x100);
    ICUNIT_GOTO_NOT_EQUAL(p, NULL, 0, EXIT);

    ret = LOS_MemFree(g_memPool, p);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Test aligned alloc without lock */
    p = LOS_MemAllocAlign(g_memPool, 0x40, 32);
    ICUNIT_GOTO_NOT_EQUAL(p, NULL, 0, EXIT);
    ICUNIT_GOTO_EQUAL(IS_ALIGNED_SIZE((UINT32)(UINTPTR)p, 32), 1, 0, EXIT);

    ret = LOS_MemFree(g_memPool, p);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    MemFree();
    MemEnd();
    return LOS_OK;
}

#endif /* LOSCFG_KERNEL_MEM_TLSF */

VOID ItLosMem051(void)
{
#ifdef LOSCFG_KERNEL_MEM_TLSF
    TEST_ADD_CASE("ItLosMem051", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
