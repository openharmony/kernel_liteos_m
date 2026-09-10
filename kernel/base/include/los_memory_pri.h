/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_MEMORY_PRI_H
#define _LOS_MEMORY_PRI_H

#include "los_memory.h"
#include "los_memstat_pri.h"
#include "los_slab_pri.h"
#include "los_spinlock.h"
#include "los_misc_pri.h"
#include "los_debug.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Memory pool information structure */
#if defined(LOSCFG_KERNEL_MEM_BESTFIT)

typedef struct {
    VOID        *pool;      /* Starting address of a memory pool */
    UINT32      poolSize;   /* Memory pool size */

#ifdef LOSCFG_MEM_TASK_STAT
    Memstat     stat;
#endif

#if (LOSCFG_MEM_MUL_POOL == 1)
    VOID        *nextPool;
#endif

#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
    struct LosSlabControlHeader slabCtrlHdr;
#endif
} LosMemPoolInfo;

#elif defined(LOSCFG_KERNEL_MEM_BESTFIT_LITTLE)

typedef struct LosHeapManager {
    struct LosHeapNode *head;
    struct LosHeapNode *tail;
    UINT32 size;

#ifdef LOSCFG_MEM_TASK_STAT
    Memstat stat;
#endif

#if (LOSCFG_MEM_MUL_POOL == 1)
    VOID *nextPool;
#endif

#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
    struct LosSlabControlHeader slabCtrlHdr;
#endif
} LosMemPoolInfo;

#elif defined(LOSCFG_KERNEL_MEM_TLSF)

/*
 * tlsf keeps its own pool head (struct OsMemPoolHead, declared in los_memory.h);
 * alias it so the common multipool helpers can manage the linked list uniformly.
 */
typedef struct OsMemPoolHead LosMemPoolInfo;

#endif

#define IS_ALIGNED(value, alignSize) ((((UINTPTR)(value)) & ((UINTPTR)((alignSize) - 1))) == 0)

#if !defined(LOSCFG_KERNEL_MEM_TLSF)
/* spinlock for mem module, only available on SMP mode */
extern SPIN_LOCK_S g_memSpin;
#define MEM_LOCK(intSave, pool)       LOS_SpinLockSave(&g_memSpin, &(intSave))
#define MEM_UNLOCK(intSave, pool)     LOS_SpinUnlockRestore(&g_memSpin, intSave)
#else
/* The memory pool support expand. */
#define OS_MEM_POOL_EXPAND_ENABLE   0x01
/* The memory pool support no lock. */
#define OS_MEM_POOL_UNLOCK_ENABLE   0x02
#define MEM_LOCK(pool, state)       do {                    \
    if (!(((struct OsMemPoolHead *)(pool))->info.attr & OS_MEM_POOL_UNLOCK_ENABLE)) { \
        (state) = LOS_IntLock();                            \
    }                                                       \
} while (0);
#define MEM_UNLOCK(pool, state)     do {                    \
    if (!(((struct OsMemPoolHead *)(pool))->info.attr & OS_MEM_POOL_UNLOCK_ENABLE)) { \
        LOS_IntRestore(state);                              \
    }                                                       \
} while (0);
#endif

extern UINTPTR g_sys_mem_addr_end;
extern UINT32 OsMemSystemInit(VOID);

/* SLAB extension needs memory algorithms provide following internal apis */
#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
extern VOID* OsMemAlloc(VOID *pool, UINT32 size);
extern UINT32 OsMemFree(VOID *pool, const VOID *ptr);
#endif /* LOSCFG_KERNEL_MEM_SLAB_EXTENTION */

#ifdef LOSCFG_MAX_POOL_NUM
#define MAX_POOL_NUM LOSCFG_MAX_POOL_NUM
#else
#define MAX_POOL_NUM 8
#endif

#if (LOSCFG_MEM_MUL_POOL == 1)
extern UINT32 OsMemMulPoolInit(VOID *pool, UINT32 size);
extern UINT32 OsMemMulPoolDeinit(const VOID *pool);
extern VOID *OsMemMulPoolHeadGet(VOID);
#else /* LOSCFG_MEM_MUL_POOL */
STATIC INLINE UINT32 OsMemMulPoolInit(VOID *pool, UINT32 size)
{
    (VOID)pool;
    (VOID)size;

    return LOS_OK;
}

STATIC INLINE UINT32 OsMemMulPoolDeinit(const VOID *pool)
{
    (VOID)pool;

    return LOS_OK;
}
#endif /* LOSCFG_MEM_MUL_POOL */

#if (LOSCFG_PLATFORM_EXC == 1)
/* tlsf-only: per-node integrity check filling errorAddr/errLen/errOwner */
extern VOID OsMemExcErrInfoGet(VOID *pool, MemInfoCB *memExcInfo);
#endif

extern UINT32 OsPoolInfoGet(VOID *pool, UINT32 *usedNodeNum, UINT32 *freeNodeNum, UINT32 *peak);

#ifdef LOSCFG_EXC_INTERACTION
extern UINT32 OsMemExcInteractionInit(UINTPTR memStart);
#endif

#ifdef LOSCFG_MEM_LEAKCHECK
extern VOID OsMemUsedNodeShow(VOID *pool);
#endif

#ifdef LOSCFG_MEM_DEBUG
VOID OsExcMemInfo(VOID);
VOID OsCheckPool(LosMemPoolInfo *poolInfo);
#endif

extern VOID OsMemResetEndNode(VOID *pool, UINTPTR preAddr);
extern VOID OsMemInfoPrint(const VOID *pool);
extern UINT32 OsShellCmdMemCheck(INT32 argc, const CHAR *argv[]);
extern VOID OsMemIntegrityMultiCheck(VOID);

#if (LOSCFG_TASK_MEM_USED == 1)
extern VOID OsTaskMemUsed(VOID *pool, UINT32 *tskMemInfoBuf, UINT32 tskMemInfoCnt);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_MEMORY_PRI_H */
