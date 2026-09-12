/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
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
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
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
/* 用例简要描述: LOS_SlabSizeCfg NULL/cnt 错误早返+钳制配置+堆健康+恢复默认 */
static UINT32 TestCase(VOID)
{
    /* 256 < 512 验证钳制路径,其余保持编译默认值(系统行为零扰动) */
    UINT32 cfgClamp[SLAB_MEM_COUNT] = {256, SLAB_MEM_ALLOCATOR_SIZE, SLAB_MEM_ALLOCATOR_SIZE,
                                       SLAB_MEM_ALLOCATOR_SIZE};
    UINT32 cfgDefault[SLAB_MEM_COUNT];
    UINT32 sizes[3] = {100, 128, 256}; /* 3, health check sizes(小尺寸,避开测试池碎片敏感区) */
    VOID *ptr = NULL;
    UINT32 ret;
    UINT32 i;

    /* F-Inv: NULL cfg 早返不崩溃(los_slabmem.c:594) */
    LOS_SlabSizeCfg(NULL, SLAB_MEM_COUNT);

    /* F-Inv: cnt 错误早返不崩溃 */
    LOS_SlabSizeCfg(cfgClamp, SLAB_MEM_COUNT + 1);

    /* F-Pos: 合法配置(256 钳到 512,其余等价默认) */
    LOS_SlabSizeCfg(cfgClamp, SLAB_MEM_COUNT);

    /* 配置后系统堆分配健康检查(系统堆充足,不受 g_memPool 测试池碎片影响) */
    for (i = 0; i < 3; i++) { /* 3, sizes array length */
        ptr = LOS_MemAlloc(LOSCFG_SYS_HEAP_ADDR, sizes[i]);
        ICUNIT_GOTO_EQUAL((ptr != NULL), 1, (UINT32)(UINTPTR)ptr, EXIT);
        (VOID)memset_s(ptr, sizes[i], 0x5A, sizes[i]);
        ret = LOS_MemFree(LOSCFG_SYS_HEAP_ADDR, ptr);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    }

EXIT:
    /* D1: 出口恢复编译默认(los_slabmem.c:41-42 全 SLAB_MEM_ALLOCATOR_SIZE) */
    for (i = 0; i < SLAB_MEM_COUNT; i++) {
        cfgDefault[i] = SLAB_MEM_ALLOCATOR_SIZE;
    }
    LOS_SlabSizeCfg(cfgDefault, SLAB_MEM_COUNT);
    return LOS_OK;
}
#endif

VOID ItLosMem301(VOID)
{
#if (LOSCFG_KERNEL_MEM_SLAB_EXTENTION == 1)
    TEST_ADD_CASE("ItLosMem301", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
