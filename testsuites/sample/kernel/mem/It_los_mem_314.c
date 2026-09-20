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

#include "It_los_mem.h"

#if (LOSCFG_KERNEL_MEM_BESTFIT_LITTLE == 1)
/* 用例简要描述: bestfit_little LOS_MemRealloc 边界(ptr=NULL 等价 malloc / size=0 等价 free / 缩容必换址且数据保留 / 扩容失败原块完好) */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 i;
    UINT8 *p1 = NULL;
    UINT8 *p2 = NULL;
    VOID *pool = LOSCFG_SYS_HEAP_ADDR;

    /* F-BVA: ptr=NULL 等价 malloc(bestfit_little/los_memory.c:200-201) */
    p1 = (UINT8 *)LOS_MemRealloc(pool, NULL, 0x40);
    ICUNIT_GOTO_NOT_EQUAL(p1, NULL, 0, EXIT);
    (VOID)memset_s(p1, 0x40, 0x5A, 0x40);
    ret = LOS_MemFree(pool, p1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    p1 = NULL;

    /* F-BVA: size=0 等价 free(bestfit_little/los_memory.c:195-199): 返回 NULL 且原块确已释放(重复释放被拒) */
    p1 = (UINT8 *)LOS_MemAlloc(pool, 0x40);
    ICUNIT_GOTO_NOT_EQUAL(p1, NULL, 0, EXIT);
    p2 = (UINT8 *)LOS_MemRealloc(pool, p1, 0);
    ICUNIT_GOTO_EQUAL(p2, NULL, 0, EXIT);
    ret = LOS_MemFree(pool, p1);
    ICUNIT_GOTO_EQUAL(ret, LOS_NOK, ret, EXIT);
    p1 = NULL;

    /* F-Pos: 缩容走 alloc+copy+free(bestfit_little 无原地缩容, 与 it_los_mem.c 跳过 018-020 的原因对应): 必换址(申请时原块仍 used, 不可能复用), 前 min(old,new)=0x40 字节数据保留(los_memory.c:217,224) */
    p1 = (UINT8 *)LOS_MemAlloc(pool, 0x100);
    ICUNIT_GOTO_NOT_EQUAL(p1, NULL, 0, EXIT);
    (VOID)memset_s(p1, 0x100, 0xB7, 0x100);
    p2 = (UINT8 *)LOS_MemRealloc(pool, p1, 0x40);
    ICUNIT_GOTO_NOT_EQUAL(p2, NULL, 0, EXIT);
    ICUNIT_GOTO_NOT_EQUAL((UINTPTR)p2, (UINTPTR)p1, 0, EXIT);
    for (i = 0; i < 0x40; i++) { /* 0x40, cpySize = min(0x100, 0x40) */
        ICUNIT_GOTO_EQUAL(p2[i], 0xB7, i, EXIT);
    }
    ret = LOS_MemFree(pool, p2);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    p2 = NULL;
    p1 = NULL; /* realloc 成功路径中原块已由内核释放(los_memory.c:232), 不可再 free */

    /* F-Inv: 扩容失败返回 NULL 且原块未被释放、数据完好(los_memory.c:223 retPtr==NULL 时不触发 free) */
    p1 = (UINT8 *)LOS_MemAlloc(pool, 0x40);
    ICUNIT_GOTO_NOT_EQUAL(p1, NULL, 0, EXIT);
    (VOID)memset_s(p1, 0x40, 0xC3, 0x40);
    p2 = (UINT8 *)LOS_MemRealloc(pool, p1, 0x7FFFFFF0); /* 0x7FFFFFF0, 远超系统池容量, alloc 失败 */
    ICUNIT_GOTO_EQUAL(p2, NULL, 0, EXIT);
    for (i = 0; i < 0x40; i++) {
        ICUNIT_GOTO_EQUAL(p1[i], 0xC3, i, EXIT);
    }
    ret = LOS_MemFree(pool, p1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    p1 = NULL;

EXIT:
    if (p1 != NULL) {
        (VOID)LOS_MemFree(pool, p1);
    }
    if (p2 != NULL) {
        (VOID)LOS_MemFree(pool, p2);
    }
    return LOS_OK;
}
#endif

VOID ItLosMem314(VOID)
{
#if (LOSCFG_KERNEL_MEM_BESTFIT_LITTLE == 1)
    TEST_ADD_CASE("ItLosMem314", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
