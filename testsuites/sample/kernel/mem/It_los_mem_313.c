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
/* 用例简要描述: bestfit_little 防御与退化行为(AllocAlign size=0/非法对齐/NULL池返NULL, PoolSizeGet/TotalUsedGet/IntegrityCheck NULL 防御返 OS_NULL_INT, UsedBlksGet/TaskIdGet 明确不支持恒返 OS_INVALID) */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    VOID *ptr = NULL;

    /* F-Pos: 正常对齐分配锚点(boundary=32 合法 2 幂) */
    ptr = LOS_MemAllocAlign(LOSCFG_SYS_HEAP_ADDR, 0x40, 32); /* 32, legal power-of-2 boundary */
    ICUNIT_GOTO_NOT_EQUAL(ptr, NULL, 0, EXIT);
    ICUNIT_GOTO_EQUAL((UINT32)(UINTPTR)ptr % 32, 0, (UINT32)(UINTPTR)ptr, EXIT);
    ret = LOS_MemFree(LOSCFG_SYS_HEAP_ADDR, ptr);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ptr = NULL;

    /* F-Inv: size=0 返回 NULL(OsHeapAllocAlign los_heap.c:357) */
    ptr = LOS_MemAllocAlign(LOSCFG_SYS_HEAP_ADDR, 0, 32);
    ICUNIT_GOTO_EQUAL(ptr, NULL, 0, EXIT);

    /* F-Inv: boundary < sizeof(VOID *) 返回 NULL(los_heap.c:357) */
    ptr = LOS_MemAllocAlign(LOSCFG_SYS_HEAP_ADDR, 0x40, 2);
    ICUNIT_GOTO_EQUAL(ptr, NULL, 0, EXIT);

    /* F-Inv: boundary 非 2 幂返回 NULL(los_heap.c:357 IS_ALIGNED(boundary, boundary) 检查) */
    ptr = LOS_MemAllocAlign(LOSCFG_SYS_HEAP_ADDR, 0x40, 6);
    ICUNIT_GOTO_EQUAL(ptr, NULL, 0, EXIT);

    /* F-Inv: pool=NULL 返回 NULL(los_heap.c:357, MEM_LOCK 为全局 spinlock 不解引用 pool) */
    ptr = LOS_MemAllocAlign(NULL, 0x40, 32);
    ICUNIT_GOTO_EQUAL(ptr, NULL, 0, EXIT);

    /* F-Inv: PoolSizeGet(NULL) 返回 OS_NULL_INT(bestfit_little/los_memory.c:321-327) */
    ret = LOS_MemPoolSizeGet(NULL);
    ICUNIT_GOTO_EQUAL(ret, OS_NULL_INT, ret, EXIT);

    /* F-Inv: TotalUsedGet(NULL) 返回 OS_NULL_INT(bestfit_little/los_memory.c:300-308) */
    ret = LOS_MemTotalUsedGet(NULL);
    ICUNIT_GOTO_EQUAL(ret, OS_NULL_INT, ret, EXIT);

    /* F-Inv: IntegrityCheck(NULL) 返回 OS_NULL_INT(bestfit_little/los_memory.c:333-340) */
    ret = LOS_MemIntegrityCheck(NULL);
    ICUNIT_GOTO_EQUAL(ret, OS_NULL_INT, ret, EXIT);

    /* F-Inv: bestfit_little 不支持已用块计数, 恒返 OS_INVALID(bestfit_little/los_memory.c:365-370 显式注释 do not support) */
    ret = LOS_MemUsedBlksGet(LOSCFG_SYS_HEAP_ADDR);
    ICUNIT_GOTO_EQUAL(ret, OS_INVALID, ret, EXIT);

    /* F-Inv: bestfit_little 不支持节点属主查询, 恒返 OS_INVALID(bestfit_little/los_memory.c:372-377 显式注释 do not support) */
    ptr = LOS_MemAlloc(LOSCFG_SYS_HEAP_ADDR, 0x20);
    ICUNIT_GOTO_NOT_EQUAL(ptr, NULL, 0, EXIT);
    ret = LOS_MemTaskIdGet(ptr);
    ICUNIT_GOTO_EQUAL(ret, OS_INVALID, ret, EXIT);
    ret = LOS_MemFree(LOSCFG_SYS_HEAP_ADDR, ptr);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ptr = NULL;

EXIT:
    if (ptr != NULL) {
        (VOID)LOS_MemFree(LOSCFG_SYS_HEAP_ADDR, ptr);
    }
    return LOS_OK;
}
#endif

VOID ItLosMem313(VOID)
{
#if (LOSCFG_KERNEL_MEM_BESTFIT_LITTLE == 1)
    TEST_ADD_CASE("ItLosMem313", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
