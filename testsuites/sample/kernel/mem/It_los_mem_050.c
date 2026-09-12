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


#if (LOSCFG_MEM_MUL_POOL == 1)

STATIC UINT8 g_memPool_TC50[TEST_POOL_SIZE];

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 poolCountBefore;
    UINT32 poolCountAfter;
    void *p = NULL;

    MemStart();

    MemInit();

    /* Get pool count before adding our pool - at least system pool exists */
    poolCountBefore = LOS_MemPoolList();
    ICUNIT_GOTO_NOT_EQUAL(poolCountBefore, 0, poolCountBefore, EXIT);

    /* Use a static array as the new pool (avoids nested-pool conflict under LMS) */
    p = g_memPool_TC50;

#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
    /* Use LOS_MemPoolInit(attr=0) so slab is NOT enabled on the test pool. */
    ret = LOS_MemPoolInit(p, TEST_POOL_SIZE, 0);
#else
    ret = LOS_MemInit(p, TEST_POOL_SIZE);
#endif
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Pool count should have increased */
    poolCountAfter = LOS_MemPoolList();
    ICUNIT_GOTO_EQUAL(poolCountAfter, poolCountBefore + 1, poolCountAfter, EXIT);

    /* DeInit the pool we added */
    ret = LOS_MemDeInit(p);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Pool count should be back to original */
    poolCountAfter = LOS_MemPoolList();
    ICUNIT_GOTO_EQUAL(poolCountAfter, poolCountBefore, poolCountAfter, EXIT);

EXIT:
    MemFree();
    MemEnd();
    return LOS_OK;
}

#endif /* LOSCFG_MEM_MUL_POOL == 1 */

VOID ItLosMem050(void)
{
#if (LOSCFG_MEM_MUL_POOL == 1)
    TEST_ADD_CASE("ItLosMem050", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
