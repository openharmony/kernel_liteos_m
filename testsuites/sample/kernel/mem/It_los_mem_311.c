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
#include "los_membox.h"

#if (LOSCFG_KERNEL_MEMBOX == 1)
/* 用例简要描述: LOS_MemboxClr 将已分配块内容清零(逐字节断言) + 块间清零独立性 + NULL 入参防御不崩溃, 兼容 static/dynamic 双实现 */
#define MEMBOX_TC311_BLK_SIZE   0x20
#define MEMBOX_TC311_FILL_BYTE  0xAA
#define MEMBOX_TC311_FILL_BYTE2 0x5A

#ifdef LOSCFG_KERNEL_MEMBOX_STATIC
#define MEMBOX_TC311_POOL_SIZE  LOS_MEMBOX_SIZE(MEMBOX_TC311_BLK_SIZE, 2) /* 2, static: 精确 2 块池 */
#else
#define MEMBOX_TC311_POOL_SIZE  (MEMBOX_TC311_BLK_SIZE * 2) /* 2, dynamic: uwBlkNum = poolSize / blkSize */
#endif

STATIC UINT8 g_memboxPoolTC311[MEMBOX_TC311_POOL_SIZE] __attribute__((aligned(8)));

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 i;
    UINT8 *box1 = NULL;
    UINT8 *box2 = NULL;

    ret = LOS_MemboxInit(g_memboxPoolTC311, MEMBOX_TC311_POOL_SIZE, MEMBOX_TC311_BLK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    box1 = (UINT8 *)LOS_MemboxAlloc(g_memboxPoolTC311);
    ICUNIT_GOTO_NOT_EQUAL(box1, NULL, 0, EXIT);

    /* F-Inv: NULL 入参防御, 不清任何内存、不崩溃(实现早退 los_membox.c:220 / los_membox_dyn.c:116; VOID 返回无值可断言, 由后续逐字节断言兜底) */
    LOS_MemboxClr(NULL, box1);
    LOS_MemboxClr(g_memboxPoolTC311, NULL);

    /* F-Pos: 写满 0xAA 后 Clr, 逐字节断言为 0(STATIC 清用户区 uwBlkSize-NODE_HEAD >= blkSize los_membox.c:225; DYNAMIC 清 uwBlkSize=blkSize los_membox_dyn.c:122) */
    (VOID)memset_s(box1, MEMBOX_TC311_BLK_SIZE, MEMBOX_TC311_FILL_BYTE, MEMBOX_TC311_BLK_SIZE);
    LOS_MemboxClr(g_memboxPoolTC311, box1);
    for (i = 0; i < MEMBOX_TC311_BLK_SIZE; i++) {
        ICUNIT_GOTO_EQUAL(box1[i], 0, i, EXIT);
    }

    /* F-BVA: 块间独立性, Clr box2 不影响 box1 内容 */
    box2 = (UINT8 *)LOS_MemboxAlloc(g_memboxPoolTC311);
    ICUNIT_GOTO_NOT_EQUAL(box2, NULL, 0, EXIT);
    (VOID)memset_s(box1, MEMBOX_TC311_BLK_SIZE, MEMBOX_TC311_FILL_BYTE, MEMBOX_TC311_BLK_SIZE);
    (VOID)memset_s(box2, MEMBOX_TC311_BLK_SIZE, MEMBOX_TC311_FILL_BYTE2, MEMBOX_TC311_BLK_SIZE);
    LOS_MemboxClr(g_memboxPoolTC311, box2);
    for (i = 0; i < MEMBOX_TC311_BLK_SIZE; i++) {
        ICUNIT_GOTO_EQUAL(box1[i], MEMBOX_TC311_FILL_BYTE, i, EXIT);
    }
    for (i = 0; i < MEMBOX_TC311_BLK_SIZE; i++) {
        ICUNIT_GOTO_EQUAL(box2[i], 0, i, EXIT);
    }

EXIT:
    /* D1: 归还全部持有块 */
    if (box1 != NULL) {
        (VOID)LOS_MemboxFree(g_memboxPoolTC311, box1);
        box1 = NULL;
    }
    if (box2 != NULL) {
        (VOID)LOS_MemboxFree(g_memboxPoolTC311, box2);
        box2 = NULL;
    }
    return LOS_OK;
}
#endif

VOID ItLosMem311(VOID)
{
#if (LOSCFG_KERNEL_MEMBOX == 1)
    TEST_ADD_CASE("ItLosMem311", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
