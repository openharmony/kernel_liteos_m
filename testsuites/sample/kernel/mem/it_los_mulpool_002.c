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
    VOID *ptr = NULL;
    VOID *newPtr = NULL;
    VOID *filler = NULL;
    UINT32 ret;

    g_poolNum = 0;

    ret = LOS_MulPoolRegister(TestGetAddrSize, TestReleaseAddrSize);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    g_poolNum = 0;

    /* alloc 64 bytes on pool 0, fill with 0xBB */
    ptr = LOS_MulPoolAlloc(64);
    ICUNIT_ASSERT_NOT_EQUAL(ptr, NULL, 0);
    (VOID)memset_s(ptr, 64, 0xBB, 64);

    /* large alloc fills pool 0 */
    filler = LOS_MulPoolAlloc(TEST_EACH_POOL_SIZE / 2);
    ICUNIT_ASSERT_NOT_EQUAL(filler, NULL, 0);

    /* pool 0 full: new alloc triggers expand pool 1 */
    newPtr = LOS_MulPoolAlloc(TEST_EACH_POOL_SIZE / 2);
    ICUNIT_ASSERT_NOT_EQUAL(newPtr, NULL, 0);
    ICUNIT_ASSERT_WITHIN_EQUAL((UINTPTR)newPtr,
        (UINTPTR)g_poolMem[1], (UINTPTR)g_poolMem[1] + TEST_EACH_POOL_SIZE - 1, 0);

    /* manual cross-pool copy: memcpy from pool 0 to pool 1 */
    (VOID)memcpy_s(newPtr, 64, ptr, 64);
    ICUNIT_ASSERT_EQUAL(*((UINT8 *)newPtr), 0xBB, 0);

    ret = LOS_MulPoolFree(newPtr);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_MulPoolFree(ptr);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_MulPoolFree(filler);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_MulPoolShrink();
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    TEST_MulPoolUnRegister();
    return LOS_OK;
}

/**
 * @ingroup TEST_MISC
 * @par TestCase_Number
 * ItLosMulPool002
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_MulPoolRealloc (cross-pool copy)
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: Pre-allocate buffers from system pool, register hooks.
 * step2: Allocate 64 bytes (pool 0), fill with 0xBB, then allocate large block to fill pool 0.
 * step3: Realloc the 64-byte block to half-pool size — pool 0 cannot fit, triggers expand pool 1 + data copy.
 * step4: Verify data preserved and new pointer in g_poolAddr[1] range.
 * step5: Free all, shrink, unregister, return buffers.
 * @par TestCase_Expected_Result
 * 1.LOS_MulPoolRealloc returns non-NULL pointer in g_poolAddr[1] with original data preserved.
 * 2.LOS_MulPoolFree and LOS_MulPoolShrink return LOS_OK.
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */

#endif /* LOSCFG_MEM_MUL_POOL_ALLOC */

VOID ItLosMulPool002(VOID)
{
#ifdef LOSCFG_MEM_MUL_POOL_ALLOC
    TEST_ADD_CASE("ItLosMulPool002", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL0, TEST_FUNCTION);
#endif
}
