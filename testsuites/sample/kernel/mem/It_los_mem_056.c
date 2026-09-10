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


#if defined(LOSCFG_KERNEL_MEM_BESTFIT) && (LOSCFG_BASE_MEM_NODE_SIZE_CHECK == 1)

#ifdef LOSCFG_MEM_HEAD_BACKUP
extern VOID LOS_MemBadNodeShow(VOID *pool);
#endif

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 size;
    UINT32 totalSize = 0;
    UINT32 availSize = 0;
    UINT8 checkLevel;
    void *p = NULL;

    MemStart();

    MemInit();

    /* Get initial check level - should be disabled by default */
    checkLevel = LOS_MemCheckLevelGet();
    ICUNIT_GOTO_EQUAL(checkLevel, LOS_MEM_CHECK_LEVEL_DISABLE, checkLevel, EXIT);

    /* Set check level to LOW */
    ret = LOS_MemCheckLevelSet(LOS_MEM_CHECK_LEVEL_LOW);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    checkLevel = LOS_MemCheckLevelGet();
    ICUNIT_GOTO_EQUAL(checkLevel, LOS_MEM_CHECK_LEVEL_LOW, checkLevel, EXIT);

    /* Set check level to HIGH */
    ret = LOS_MemCheckLevelSet(LOS_MEM_CHECK_LEVEL_HIGH);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    checkLevel = LOS_MemCheckLevelGet();
    ICUNIT_GOTO_EQUAL(checkLevel, LOS_MEM_CHECK_LEVEL_HIGH, checkLevel, EXIT);

    /* Set check level to DISABLE */
    ret = LOS_MemCheckLevelSet(LOS_MEM_CHECK_LEVEL_DISABLE);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    checkLevel = LOS_MemCheckLevelGet();
    ICUNIT_GOTO_EQUAL(checkLevel, LOS_MEM_CHECK_LEVEL_DISABLE, checkLevel, EXIT);

    /* Test invalid check level */
    ret = LOS_MemCheckLevelSet(0x42);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_MEMCHECK_WRONG_LEVEL, ret, EXIT);

    /* Enable check and test LOS_MemNodeSizeCheck */
    ret = LOS_MemCheckLevelSet(LOS_MEM_CHECK_LEVEL_HIGH);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    size = 0x100;
    p = LOS_MemAlloc(g_memPool, size);
    ICUNIT_GOTO_NOT_EQUAL(p, NULL, 0, EXIT);

    /* Check node size of the allocated pointer */
    ret = LOS_MemNodeSizeCheck(g_memPool, p, &totalSize, &availSize);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(totalSize, 0, totalSize, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(availSize, 0, availSize, EXIT);
    ICUNIT_GOTO_EQUAL((totalSize >= availSize), 1, 0, EXIT);

#ifdef LOSCFG_MEM_HEAD_BACKUP
    /* LOS_MemBadNodeShow walks all nodes and prints any whose HEAD_BACKUP
     * checksum no longer matches (corrupted node). On a clean pool it finds
     * none; the call here covers the API and verifies it does not fault. */
    LOS_MemBadNodeShow(g_memPool);
#endif

    /* Test NULL pool */
    ret = LOS_MemNodeSizeCheck(NULL, p, &totalSize, &availSize);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_MEMCHECK_PARA_NULL, ret, EXIT);

    /* Test NULL ptr */
    ret = LOS_MemNodeSizeCheck(g_memPool, NULL, &totalSize, &availSize);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_MEMCHECK_PARA_NULL, ret, EXIT);

    ret = LOS_MemFree(g_memPool, p);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Restore default check level */
    (VOID)LOS_MemCheckLevelSet(LOS_MEM_CHECK_LEVEL_DEFAULT);

EXIT:
    MemFree();
    MemEnd();
    return LOS_OK;
}

#endif /* LOSCFG_KERNEL_MEM_BESTFIT && LOSCFG_BASE_MEM_NODE_SIZE_CHECK == 1 */

VOID ItLosMem056(void)
{
#if defined(LOSCFG_KERNEL_MEM_BESTFIT) && (LOSCFG_BASE_MEM_NODE_SIZE_CHECK == 1)
    TEST_ADD_CASE("ItLosMem056", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
