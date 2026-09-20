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
#include "los_slab.h"
#include "los_slab_pri.h"

#if (LOSCFG_KERNEL_MEM_SLAB_EXTENTION == 1)
/* 用例简要描述: SLAB_MSK 专属池验证(LOS_SlabSizeCfg 配置后建池生效, 小对象 alloc/free 命中 slab 类经行为断言验证, 超 SLAB_MEM_MAX_SIZE 回落堆, 出口恢复默认配置) */
/* 说明: A2 建议的"配置小桶"不可行——cfg 值 < SLAB_MEM_ALLOCATOR_SIZE 会被钳制到 512(los_slabmem.c:600-604), 只可调大; 故改为配大桶 + 专属池行为断言, 钳制路径已由 ItLosMem301 覆盖 */
#define MEMBOX_TC312_POOL_SIZE  0x2000  /* 8192, >= SLAB_BASIC_NEED_SIZE(0x1000), 满足 slab 建桶最小池要求 */
#define MEMBOX_TC312_SMALL_SIZE 0x20    /* 32, 命中 slab 类(blkSz 16/32/64/128) */
#define MEMBOX_TC312_LARGE_SIZE 0x100   /* 256, > SLAB_MEM_MAX_SIZE(128), 必走堆路径(los_slabmem.c:333) */

STATIC UINT8 g_slabPoolTC312[MEMBOX_TC312_POOL_SIZE] __attribute__((aligned(8)));

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 i;
    BOOL poolInit = FALSE;
    VOID *smallBlk = NULL;
    VOID *largeBlk = NULL;
    UINT32 usedBefore = 0;
    UINT32 usedAfter = 0;
    /* idx0 配 2 倍默认值, 验证 LOS_SlabSizeCfg 配置生效路径(非钳制, 钳制路径由 ItLosMem301 覆盖) */
    UINT32 cfgLarge[SLAB_MEM_COUNT] = {2 * SLAB_MEM_ALLOCATOR_SIZE, SLAB_MEM_ALLOCATOR_SIZE,
                                       SLAB_MEM_ALLOCATOR_SIZE, SLAB_MEM_ALLOCATOR_SIZE};
    UINT32 cfgDefault[SLAB_MEM_COUNT];

    /* F-Pos: 配置接口合法入参(仅影响下一次建池, 建池后配置自动恢复默认) */
    LOS_SlabSizeCfg(cfgLarge, SLAB_MEM_COUNT);

    /* F-Pos: bestfit_little 下 LOS_MemPoolInit(attr=SLAB_MSK) 建专属 slab 池 */
    ret = LOS_MemPoolInit(g_slabPoolTC312, MEMBOX_TC312_POOL_SIZE, LOS_MEM_INIT_ATTR_SLAB_MSK);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    poolInit = TRUE;

    /* 注: slab 专属池内部分配对 LOS_MemTotalUsedGet 水位透明(建池时整个 slab 区已是
     * 一个已用堆节点, slab 类内部分配/释放不改堆节点标记), 故不使用水位对比断言。 */
    usedBefore = LOS_MemTotalUsedGet(g_slabPoolTC312);

    /* F-Pos: 小对象(0x20 <= SLAB_MEM_MAX_SIZE=128)命中 slab 类, 分配+写+释放行为验证 */
    smallBlk = LOS_MemAlloc(g_slabPoolTC312, MEMBOX_TC312_SMALL_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(smallBlk, NULL, 0, EXIT);
    (VOID)memset_s(smallBlk, MEMBOX_TC312_SMALL_SIZE, 0x5A, MEMBOX_TC312_SMALL_SIZE);
    ret = LOS_MemFree(g_slabPoolTC312, smallBlk);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    smallBlk = NULL;

    /* F-BVA: 重复小对象分配释放循环(slabs 生命周期稳定性, 5: 循环次数) */
    for (i = 0; i < 5; i++) {
        smallBlk = LOS_MemAlloc(g_slabPoolTC312, MEMBOX_TC312_SMALL_SIZE);
        ICUNIT_GOTO_NOT_EQUAL(smallBlk, NULL, i, EXIT);
        ret = LOS_MemFree(g_slabPoolTC312, smallBlk);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
        smallBlk = NULL;
    }

    /* F-BVA: 超过 slab 类上限(SLAB_MEM_MAX_SIZE=128)的请求走堆路径, 分配+释放行为验证 */
    largeBlk = LOS_MemAlloc(g_slabPoolTC312, MEMBOX_TC312_LARGE_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(largeBlk, NULL, 0, EXIT);
    (VOID)memset_s(largeBlk, MEMBOX_TC312_LARGE_SIZE, 0xA5, MEMBOX_TC312_LARGE_SIZE);
    ret = LOS_MemFree(g_slabPoolTC312, largeBlk);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    largeBlk = NULL;
    usedAfter = LOS_MemTotalUsedGet(g_slabPoolTC312);
    ICUNIT_GOTO_EQUAL(usedAfter, usedBefore, usedAfter - usedBefore, EXIT);

EXIT:
    if (smallBlk != NULL) {
        (VOID)LOS_MemFree(g_slabPoolTC312, smallBlk);
        smallBlk = NULL;
    }
    if (largeBlk != NULL) {
        (VOID)LOS_MemFree(g_slabPoolTC312, largeBlk);
        largeBlk = NULL;
    }
    if (poolInit == TRUE) {
#if (LOSCFG_MEM_MUL_POOL == 1)
        (VOID)LOS_MemDeInit(g_slabPoolTC312);
#endif
    }
    /* D1: 出口恢复编译默认配置 */
    for (i = 0; i < SLAB_MEM_COUNT; i++) {
        cfgDefault[i] = SLAB_MEM_ALLOCATOR_SIZE;
    }
    LOS_SlabSizeCfg(cfgDefault, SLAB_MEM_COUNT);
    return LOS_OK;
}
#endif

VOID ItLosMem312(VOID)
{
#if (LOSCFG_KERNEL_MEM_SLAB_EXTENTION == 1)
    TEST_ADD_CASE("ItLosMem312", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
