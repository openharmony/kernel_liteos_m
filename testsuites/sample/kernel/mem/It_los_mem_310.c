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
/* 用例简要描述: membox Alloc/Free 生命周期 P0(满池第4次Alloc失败/释放后再分配/重复Free拒绝/池外指针Free拒绝) + StatisticsGet 与 Init 参数交叉断言 + ShowBox 冒烟, 兼容 static/dynamic 双实现 */
#define MEMBOX_TC310_BLK_SIZE   0x20
#define MEMBOX_TC310_MAX_BLKS   3

#ifdef LOSCFG_KERNEL_MEMBOX_STATIC
/* static: 池自管理, LOS_MEMBOX_SIZE 精确 3 块; uwBlkSize 为含节点头并对齐后的值(los_membox.c:136) */
#define MEMBOX_TC310_POOL_SIZE      LOS_MEMBOX_SIZE(MEMBOX_TC310_BLK_SIZE, MEMBOX_TC310_MAX_BLKS)
#define MEMBOX_TC310_EXP_BLK_SIZE   LOS_MEMBOX_ALIGNED(MEMBOX_TC310_BLK_SIZE + OS_MEMBOX_NODE_HEAD_SIZE)
#else
/* dynamic: uwBlkNum = poolSize / blkSize 不扣头(los_membox_dyn.c:57-58), 块从系统堆分配, uwBlkSize 为入参原值 */
#define MEMBOX_TC310_POOL_SIZE      (MEMBOX_TC310_BLK_SIZE * MEMBOX_TC310_MAX_BLKS)
#define MEMBOX_TC310_EXP_BLK_SIZE   MEMBOX_TC310_BLK_SIZE
#endif

STATIC UINT8 g_memboxPoolTC310[MEMBOX_TC310_POOL_SIZE] __attribute__((aligned(8)));
STATIC UINT8 g_memboxOutTC310[0x40] __attribute__((aligned(8))); /* 池外静态缓冲, 用于非法指针 Free 防御 */

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 maxBlk;
    UINT32 blkCnt;
    UINT32 blkSize;
    UINT32 i;
    VOID *blocks[MEMBOX_TC310_MAX_BLKS] = {NULL};

    ret = LOS_MemboxInit(g_memboxPoolTC310, MEMBOX_TC310_POOL_SIZE, MEMBOX_TC310_BLK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Inv: StatisticsGet 任一出参为 NULL 返回 LOS_NOK(los_membox.c:260 / los_membox_dyn.c:139) */
    ret = LOS_MemboxStatisticsGet(g_memboxPoolTC310, NULL, &blkCnt, &blkSize);
    ICUNIT_GOTO_EQUAL(ret, LOS_NOK, ret, EXIT);

    /* F-Pos: Init 参数交叉断言 maxBlk=3 / blkCnt=0 / blkSize 与实现口径一致 */
    ret = LOS_MemboxStatisticsGet(g_memboxPoolTC310, &maxBlk, &blkCnt, &blkSize);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(maxBlk, MEMBOX_TC310_MAX_BLKS, maxBlk, EXIT);
    ICUNIT_GOTO_EQUAL(blkCnt, 0, blkCnt, EXIT);
    ICUNIT_GOTO_EQUAL(blkSize, MEMBOX_TC310_EXP_BLK_SIZE, blkSize, EXIT);

    /* F-Pos: 连续 Alloc 满 3 块, 每块写满验证可用(STATIC 用户区 >= blkSize; DYNAMIC 恰 blkSize) */
    for (i = 0; i < MEMBOX_TC310_MAX_BLKS; i++) {
        blocks[i] = LOS_MemboxAlloc(g_memboxPoolTC310);
        ICUNIT_GOTO_NOT_EQUAL(blocks[i], NULL, i, EXIT);
        (VOID)memset_s(blocks[i], MEMBOX_TC310_BLK_SIZE, 0xA5, MEMBOX_TC310_BLK_SIZE);
    }

    /* F-BVA: 空池第 4 次 Alloc 返回 NULL(STATIC 空闲链空 los_membox.c:177; DYNAMIC blkCnt>=blkNum los_membox_dyn.c:80) */
    ICUNIT_GOTO_EQUAL(LOS_MemboxAlloc(g_memboxPoolTC310), NULL, 0, EXIT);

    ret = LOS_MemboxStatisticsGet(g_memboxPoolTC310, &maxBlk, &blkCnt, &blkSize);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(blkCnt, MEMBOX_TC310_MAX_BLKS, blkCnt, EXIT);

    /* F-Pos: 释放 1 块后计数回落, 再分配成功(STATIC 空闲链回收; DYNAMIC 归还系统堆后重申请) */
    ret = LOS_MemboxFree(g_memboxPoolTC310, blocks[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    blocks[0] = NULL;

    blocks[0] = LOS_MemboxAlloc(g_memboxPoolTC310);
    ICUNIT_GOTO_NOT_EQUAL(blocks[0], NULL, 0, EXIT);

    ret = LOS_MemboxStatisticsGet(g_memboxPoolTC310, &maxBlk, &blkCnt, &blkSize);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(blkCnt, MEMBOX_TC310_MAX_BLKS, blkCnt, EXIT);

    /* F-Inv: 重复 Free 第二次返回 LOS_NOK(STATIC magic 校验失败 los_membox.c:91; DYNAMIC 堆重复释放检查 los_heap.c:467) */
    ret = LOS_MemboxFree(g_memboxPoolTC310, blocks[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_MemboxFree(g_memboxPoolTC310, blocks[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_NOK, ret, EXIT);
    blocks[0] = NULL;

    /* F-Inv: Free 池外指针返回 LOS_NOK(STATIC offset/magic 校验 los_membox.c:82-91; DYNAMIC 系统堆范围检查 los_memory.c:251), 且池状态不受影响(重复 Free 已归还 1 块, blkCnt = 3-1 = 2) */
    ret = LOS_MemboxFree(g_memboxPoolTC310, (VOID *)(g_memboxOutTC310 + 8)); /* 8, 池外静态缓冲内部地址 */
    ICUNIT_GOTO_EQUAL(ret, LOS_NOK, ret, EXIT);
    ret = LOS_MemboxStatisticsGet(g_memboxPoolTC310, &maxBlk, &blkCnt, &blkSize);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(blkCnt, MEMBOX_TC310_MAX_BLKS - 1, blkCnt, EXIT);

    /* F-Pos: ShowBox 冒烟, 正常池与 NULL 均不崩溃(VOID 返回, 由后续清理链保证池完好) */
    LOS_ShowBox(g_memboxPoolTC310);
    LOS_ShowBox(NULL);

EXIT:
    /* D1: 归还全部持有块, STATIC 池/DYNAMIC 系统堆两侧无泄漏(blkCnt 归 0) */
    for (i = 0; i < MEMBOX_TC310_MAX_BLKS; i++) {
        if (blocks[i] != NULL) {
            (VOID)LOS_MemboxFree(g_memboxPoolTC310, blocks[i]);
            blocks[i] = NULL;
        }
    }
    return LOS_OK;
}
#endif

VOID ItLosMem310(VOID)
{
#if (LOSCFG_KERNEL_MEMBOX == 1)
    TEST_ADD_CASE("ItLosMem310", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
