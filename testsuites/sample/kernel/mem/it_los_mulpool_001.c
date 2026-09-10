/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of the following disclaimer in the documentation and/or materials provided with the
 *    distribution.
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

#include "It_los_mem.h"

#ifdef LOSCFG_MEM_MUL_POOL_ALLOC

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    VOID *ptr1 = NULL;
    VOID *ptr2 = NULL;
    VOID *ptr3 = NULL;
    UINT32 allocSize = TEST_EACH_POOL_SIZE / 2;

    g_poolNum = 0;

    ret = LOS_MulPoolRegister(TestGetAddrSize, TestReleaseAddrSize);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* ptr1: small alloc triggers expand pool 0 */
    ptr1 = LOS_MulPoolAlloc(64);
    ICUNIT_ASSERT_NOT_EQUAL(ptr1, NULL, 0);
    ICUNIT_ASSERT_WITHIN_EQUAL((UINTPTR)ptr1,
        (UINTPTR)g_poolMem[0], (UINTPTR)g_poolMem[0] + TEST_EACH_POOL_SIZE - 1, 0);

    /* ptr2: large alloc still fits in pool 0 */
    ptr2 = LOS_MulPoolAlloc(allocSize);
    ICUNIT_ASSERT_NOT_EQUAL(ptr2, NULL, 0);
    ICUNIT_ASSERT_WITHIN_EQUAL((UINTPTR)ptr2,
        (UINTPTR)g_poolMem[0], (UINTPTR)g_poolMem[0] + TEST_EACH_POOL_SIZE - 1, 0);

    /* ptr3: pool 0 full, triggers expand pool 1 */
    ptr3 = LOS_MulPoolAlloc(allocSize);
    ICUNIT_ASSERT_NOT_EQUAL(ptr3, NULL, 0);
    ICUNIT_ASSERT_WITHIN_EQUAL((UINTPTR)ptr3,
        (UINTPTR)g_poolMem[1], (UINTPTR)g_poolMem[1] + TEST_EACH_POOL_SIZE - 1, 0);

    (VOID)memset_s(ptr1, 64, 0xAA, 64);
    ICUNIT_ASSERT_EQUAL(*((UINT8 *)ptr1), 0xAA, 0);
    (VOID)memset_s(ptr3, allocSize, 0xCC, allocSize);
    ICUNIT_ASSERT_EQUAL(*((UINT8 *)ptr3), 0xCC, 0);

    ret = LOS_MulPoolFree(ptr1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_MulPoolFree(ptr2);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_MulPoolFree(ptr3);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_MulPoolShrink();
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TEST_MulPoolUnRegister();
    return LOS_OK;
}

/**
 * @ingroup TEST_MISC
 * @par TestCase_Number
 * ItLosMulPool001
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_MulPoolRegister/LOS_MulPoolAlloc/LOS_MulPoolFree/LOS_MulPoolShrink
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Pre-allocate TEST_POOL_MAX_NUM buffers from system pool for expand.
 * step2: Register hooks, allocate small + large + large to trigger at least 2 pool expansions.
 * step3: Verify each allocation lands in the expected expand pool by address range check.
 * step4: Free all, shrink, unregister, return pre-allocated buffers.
 * @par TestCase_Expected_Result
 * 1.LOS_MulPoolAlloc returns pointers in g_poolAddr[0] for first two allocations.
 * 2.LOS_MulPoolAlloc returns pointer in g_poolAddr[1] for third allocation (pool 0 full).
 * 3.LOS_MulPoolFree and LOS_MulPoolShrink return LOS_OK.
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

#endif /* LOSCFG_MEM_MUL_POOL_ALLOC */

VOID ItLosMulPool001(VOID)
{
#ifdef LOSCFG_MEM_MUL_POOL_ALLOC
    TEST_ADD_CASE("ItLosMulPool001", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL0, TEST_FUNCTION);
#endif
}
