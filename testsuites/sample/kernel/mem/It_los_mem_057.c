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


#if defined(LOSCFG_KERNEL_MEM_BESTFIT) && (LOSCFG_MEM_MUL_MODULE == 1)

STATIC UINT8 g_memPool_TC57[TEST_MEM_SIZE];

/*
 * testcasename: bestfit 多模块内存计量 API（LOS_MemMalloc/MallocAlign/Mfree/Mrealloc/MusedGet）功能与边界验证
 * testcasecode: ItLosMem057
 * level:        LEVEL1
 * type:         FUNCTION
 *
 * precond（预置条件）
 *   1. g_testCount = 0
 *   2. LOSCFG_KERNEL_MEM_BESTFIT && LOSCFG_MEM_MUL_MODULE == 1（仅 bestfit+多模块下编译/注册）
 *   3. 静态数组 g_memPool_TC57 已用 LOS_MemPoolInit(attr=0) 初始化（不使能 slab）
 *   4. 模块 A(moduleId=0) 与模块 B(moduleId=1) 起始用量均为 0
 *
 * step（测试步骤）
 *   1. LOS_MemMusedGet(modA) 取基线
 *   2. LOS_MemMalloc(g_memPool_TC57, 0x100, modA)
 *   3. LOS_MemMallocAlign(g_memPool_TC57, 0x40, 32, modB)
 *   4. LOS_MemMalloc(g_memPool_TC57, 0x80, modA) 后 LOS_MemMrealloc(g_memPool_TC57, p1, 0x200, modA)（扩容）
 *   5. LOS_MemMrealloc(g_memPool_TC57, newPtr, 0x40, modA)（缩容）
 *   6. LOS_MemMfree(g_memPool_TC57, p, modA)
 *   7. LOS_MemMfree(g_memPool_TC57, newPtr, modA) 与 LOS_MemMfree(g_memPool_TC57, pa, modB)
 *   8. LOS_MemMalloc(g_memPool_TC57, 0x40, MEM_MODULE_MAX + 1) 与 LOS_MemMusedGet(MEM_MODULE_MAX + 1)（边界：非法 moduleId）
 *   9. LOS_MemMfree(g_memPool_TC57, NULL, modA)（边界：NULL 指针）
 *  10. LOS_MemMrealloc(g_memPool_TC57, NULL, 0x40, modA) 后 LOS_MemMfree(g_memPool_TC57, p2, modA)（边界：NULL 等价 malloc）
 *
 * exp（预期结果）
 *   1. modA 用量 == 0
 *   2. 返回 != NULL；modA 用量较基线增加；modB 用量 == 0
 *   3. 返回 != NULL；地址 32 字节对齐；modB 用量 > 0
 *   4. Mrealloc 返回 != NULL；modA 用量较扩容前增加
 *   5. 返回 != NULL；modA 用量较缩容前不增加（<=）
 *   6. 返回 LOS_OK；modA 用量较释放前减少
 *   7. 两次 Mfree 均返回 LOS_OK；modA、modB 用量均回到 0
 *   8. Malloc 返回 NULL；MusedGet 返回 OS_NULL_INT
 *   9. 返回 LOS_NOK
 *  10. Mrealloc 返回 != NULL（等价 malloc）；modA 用量增加；Mfree 返回 LOS_OK
 *
 * rstenv（环境清理）
 *   1. EXIT 标签执行 LOS_MemDeInit(g_memPool_TC57) 反初始化池
 *   2. 成功路径已释放 p/p1/pa/newPtr/p2，无跨用例节点泄漏
 */

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 size;
    UINT32 usedBefore;
    UINT32 usedAfter;
    void *p = NULL;
    void *p1 = NULL;
    void *p2 = NULL;
    void *pa = NULL;
    void *newPtr = NULL;
    UINT32 modA = 0;          /* module ID 0 */
    UINT32 modB = 1;          /* module ID 1 */

    /* Use LOS_MemPoolInit(attr=0) so slab is NOT enabled, avoiding slab
     * intercepting the small (0x40/0x80) module allocs that MEM_MUL_MODULE must track. */
    ret = LOS_MemPoolInit(g_memPool_TC57, TEST_MEM_SIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* --- LOS_MemMalloc + LOS_MemMusedGet --- */
    usedBefore = LOS_MemMusedGet(modA);
    ICUNIT_GOTO_EQUAL(usedBefore, 0, usedBefore, EXIT);

    size = 0x100;
    p = LOS_MemMalloc(g_memPool_TC57, size, modA);
    ICUNIT_GOTO_NOT_EQUAL(p, NULL, 0, EXIT);

    usedAfter = LOS_MemMusedGet(modA);
    ICUNIT_GOTO_EQUAL((usedAfter > usedBefore), 1, 0, EXIT);

    /* Module B should still be 0 */
    ICUNIT_GOTO_EQUAL(LOS_MemMusedGet(modB), 0, LOS_MemMusedGet(modB), EXIT);

    /* --- LOS_MemMallocAlign --- */
    pa = LOS_MemMallocAlign(g_memPool_TC57, 0x40, 32, modB);
    ICUNIT_GOTO_NOT_EQUAL(pa, NULL, 0, EXIT);
    ICUNIT_GOTO_EQUAL(IS_ALIGNED_SIZE((UINT32)(UINTPTR)pa, 32), 1, 0, EXIT);

    /* Module B should now have non-zero usage */
    ICUNIT_GOTO_EQUAL((LOS_MemMusedGet(modB) > 0), 1, 0, EXIT);

    /* --- LOS_MemMrealloc (grow) --- */
    p1 = LOS_MemMalloc(g_memPool_TC57, 0x80, modA);
    ICUNIT_GOTO_NOT_EQUAL(p1, NULL, 0, EXIT);

    usedBefore = LOS_MemMusedGet(modA);

    newPtr = LOS_MemMrealloc(g_memPool_TC57, p1, 0x200, modA);
    ICUNIT_GOTO_NOT_EQUAL(newPtr, NULL, 0, EXIT);

    usedAfter = LOS_MemMusedGet(modA);
    ICUNIT_GOTO_EQUAL((usedAfter > usedBefore), 1, 0, EXIT);

    /* --- LOS_MemMrealloc (shrink) --- */
    usedBefore = LOS_MemMusedGet(modA);

    newPtr = LOS_MemMrealloc(g_memPool_TC57, newPtr, 0x40, modA);
    ICUNIT_GOTO_NOT_EQUAL(newPtr, NULL, 0, EXIT);

    usedAfter = LOS_MemMusedGet(modA);
    ICUNIT_GOTO_EQUAL((usedAfter <= usedBefore), 1, 0, EXIT);

    /* --- LOS_MemMfree --- */
    usedBefore = LOS_MemMusedGet(modA);
    ret = LOS_MemMfree(g_memPool_TC57, p, modA);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    usedAfter = LOS_MemMusedGet(modA);
    ICUNIT_GOTO_EQUAL((usedAfter < usedBefore), 1, 0, EXIT);

    /* Free remaining allocations */
    ret = LOS_MemMfree(g_memPool_TC57, newPtr, modA);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_MemMfree(g_memPool_TC57, pa, modB);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* All module usage should be back to 0 */
    ICUNIT_GOTO_EQUAL(LOS_MemMusedGet(modA), 0, LOS_MemMusedGet(modA), EXIT);
    ICUNIT_GOTO_EQUAL(LOS_MemMusedGet(modB), 0, LOS_MemMusedGet(modB), EXIT);

    /* --- Boundary: invalid module ID --- */
    p2 = LOS_MemMalloc(g_memPool_TC57, 0x40, MEM_MODULE_MAX + 1);
    ICUNIT_GOTO_EQUAL(p2, NULL, p2, EXIT);

    ret = LOS_MemMusedGet(MEM_MODULE_MAX + 1);
    ICUNIT_GOTO_EQUAL(ret, OS_NULL_INT, ret, EXIT);

    /* --- Boundary: NULL ptr to LOS_MemMfree --- */
    ret = LOS_MemMfree(g_memPool_TC57, NULL, modA);
    ICUNIT_GOTO_EQUAL(ret, LOS_NOK, ret, EXIT);

    /* --- Boundary: NULL ptr to LOS_MemMrealloc (acts as malloc) --- */
    p2 = LOS_MemMrealloc(g_memPool_TC57, NULL, 0x40, modA);
    ICUNIT_GOTO_NOT_EQUAL(p2, NULL, 0, EXIT);
    ICUNIT_GOTO_EQUAL((LOS_MemMusedGet(modA) > 0), 1, 0, EXIT);

    ret = LOS_MemMfree(g_memPool_TC57, p2, modA);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    LOS_MemDeInit(g_memPool_TC57);
    return LOS_OK;
}

#endif /* LOSCFG_KERNEL_MEM_BESTFIT && LOSCFG_MEM_MUL_MODULE == 1 */

VOID ItLosMem057(void)
{
#if defined(LOSCFG_KERNEL_MEM_BESTFIT) && (LOSCFG_MEM_MUL_MODULE == 1)
    TEST_ADD_CASE("ItLosMem057", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
