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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

void *g_memPool = NULL;
void *g_testPool = NULL;

void MemStart(void)
{
    g_memPool = LOS_MemAlloc(LOSCFG_SYS_HEAP_ADDR, TEST_MEM_SIZE);
    g_testPool = LOS_MemAlloc(LOSCFG_SYS_HEAP_ADDR, TEST_POOL_SIZE);

    if ((g_memPool == NULL) || (g_testPool == NULL)) {
        PRINT_ERR("alloc failed , mem TestCase would be failed!!!!%p !!! %p", g_memPool, g_testPool);
    }
}

void MemEnd(void)
{
    (VOID)LOS_MemFree(LOSCFG_SYS_HEAP_ADDR, g_memPool);
    (VOID)LOS_MemFree(LOSCFG_SYS_HEAP_ADDR, g_testPool);
    g_memPool = NULL;
    g_testPool = NULL;
}

void MemInit(void)
{
    (void)LOS_MemInit(g_memPool, TEST_MEM_SIZE);
}

void MemFree(void)
{
#if (LOSCFG_MEM_MUL_POOL == 1)
    (void)LOS_MemDeInit(g_memPool);
#endif
}

UINT32 MemGetFreeSize(void *pool)
{
    return LOS_MemPoolSizeGet(pool) - LOS_MemTotalUsedGet(pool);
}

UINT32 CalPow(UINT32 exp)
{
    UINT32 pw = 1;
    pw <<= exp;
    return pw;
}

#ifdef LOSCFG_MEM_MUL_POOL_ALLOC
UINT8 g_poolMem[TEST_POOL_MAX_NUM][TEST_EACH_POOL_SIZE] __attribute__((aligned(32)));
UINT32 g_poolNum;

VOID TestGetAddrSize(UINTPTR *poolAddr, UINT32 *poolSize)
{
    *poolAddr = (UINTPTR)g_poolMem[g_poolNum];
    *poolSize = TEST_EACH_POOL_SIZE;
    g_poolNum++;
}

VOID TestReleaseAddrSize(VOID *pool)
{
    (VOID)pool;
    g_poolNum--;
}

VOID TEST_MulPoolUnRegister(VOID)
{
    extern UINT32 g_poolFlag;
    extern UINT32 g_mulPoolMux;

    (VOID)LOS_MuxDelete(g_mulPoolMux);
    g_poolFlag = 0;
    g_mulPoolMux = 0;
}
#endif

VOID ItSuiteLosMem(void)
{
    MemStart();

    ItLosMem001();
    ItLosMem002();
    ItLosMem003();
#ifdef LOSCFG_KERNEL_MEM_TLSF
    ItLosMem004();
#endif
    ItLosMem006();
    ItLosMem007();
    ItLosMem008();
    ItLosMem009();
    ItLosMem010();
    ItLosMem011();
#ifndef LOSCFG_MEM_DEBUG
    ItLosMem012();
    ItLosMem013();
#endif
#if !defined(LOSCFG_MEM_HEAD_BACKUP) && !defined(LOSCFG_MEM_TASK_STAT)
    /* When LOSCFG_MEM_HEAD_BACKUP is enabled, the TEST_MEM_SIZE is not enough. */
    ItLosMem014();
#endif
    ItLosMem015();
    ItLosMem016();
    ItLosMem017();
#ifndef LOSCFG_KERNEL_MEM_BESTFIT_LITTLE
    /*
     * bestfit_little's LOS_MemRealloc always moves (alloc+copy+free), it does
     * not support in-place shrink, so these in-place-realloc cases are skipped.
     */
    ItLosMem018();
    ItLosMem019();
    ItLosMem020();
#endif
#if (LOSCFG_TEST_MUCH_LOG == 1) // when open this， Too many logs will printed
    ItLosMem035();
    ItLosMem036();
    ItLosMem037();
    ItLosMem038();
#endif
#ifdef LOSCFG_KERNEL_MEM_TLSF
    ItLosMem040();
#endif
    ItLosMem045();
#if (LOSCFG_MEM_MUL_REGIONS == 1)
    ItLosMem046();
    ItLosMem047();
#endif

    /* Common memory API tests (work on both TLSF and bestfit) */
    ItLosMem048(); /* LOS_MemInfoGet */
#if defined(LOSCFG_KERNEL_MEM_TLSF) || defined(LOSCFG_KERNEL_MEM_BESTFIT)
    ItLosMem049(); /* LOS_MemFreeNodeShow */
#endif
#if (LOSCFG_MEM_MUL_POOL == 1)
    ItLosMem050(); /* LOS_MemPoolList + LOS_MemDeInit */
#endif
#ifdef LOSCFG_KERNEL_MEM_TLSF
    ItLosMem051(); /* LOS_MemUnlockEnable (TLSF only) */
#if (LOSCFG_MEM_LEAKCHECK == 1)
    ItLosMem052(); /* LOS_MemUsedNodeShow */
#endif
#endif
#ifdef LOSCFG_KERNEL_MEM_BESTFIT
    ItLosMem053(); /* LOS_MemPoolInit */
    ItLosMem054(); /* LOS_MemFreeBlksGet + LOS_MemUsedBlksGet */
    ItLosMem055(); /* LOS_MemTaskIdGet + LOS_MemLastUsedGet */
#endif
#if defined(LOSCFG_KERNEL_MEM_BESTFIT) && (LOSCFG_BASE_MEM_NODE_SIZE_CHECK == 1)
    ItLosMem056(); /* LOS_MemNodeSizeCheck + LOS_MemCheckLevelSet/Get */
#endif
#if defined(LOSCFG_KERNEL_MEM_BESTFIT) && (LOSCFG_MEM_MUL_MODULE == 1)
    ItLosMem057(); /* LOS_MemMalloc/MallocAlign/Mfree/Mrealloc/MusedGet */
#endif
#if (LOSCFG_KERNEL_MEM_TLSF == 1) && (LOSCFG_MEM_FREE_BY_TASKID == 1)
    ItLosMem058(); /* LOS_MemFreeByTaskID */
#endif

#if (LOS_KERNEL_TEST_FULL == 1)
    ItLosTick001();
    ItLosTick002();
    ItLosTick003();
    ItLosTick004();
    ItLosTick005();
    ItLosTick006();
    ItLosTick007();
    ItLosTick008();
    ItLosTick009();
    ItLosTick010();
    ItLosTick011();
    ItLosMem301();
#endif
#ifdef LOSCFG_MEM_MUL_POOL_ALLOC
    ItLosMulPool001();
    ItLosMulPool002();
#endif
    MemEnd();
}
