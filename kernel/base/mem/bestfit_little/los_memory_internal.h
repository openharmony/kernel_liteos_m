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

/*
 * @defgroup los_heap Heap
 * @ingroup kernel
 */

#ifndef _LOS_MEMORY_INTERNAL_H
#define _LOS_MEMORY_INTERNAL_H

#include "los_typedef.h"
#include "los_memory_pri.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define OS_MEM_ALIGN_FLAG                     0x80000000
#define OS_MEM_SET_ALIGN_FLAG(align)          ((align) = ((align) | OS_MEM_ALIGN_FLAG))
#define OS_MEM_GET_ALIGN_FLAG(align)          ((align) & OS_MEM_ALIGN_FLAG)
#define OS_MEM_GET_ALIGN_GAPSIZE(align)       ((align) & (~OS_MEM_ALIGN_FLAG))
#define OS_MEM_NODE_HEAD_SIZE                 (UINT32)sizeof(struct LosHeapNode)
/* Requires at least 1 byte of allocated space */
#define OS_MEM_MIN_POOL_SIZE                  (OS_MEM_NODE_HEAD_SIZE + sizeof(struct LosHeapManager) + 1)
/* The upper two bits [31:30] are used as a flag. */
#define OS_MEM_MAX_POOL_SIZE                  0x3FFFFFFFU

typedef struct tagLosHeapStatus {
    UINT32 totalUsedSize;
    UINT32 totalFreeSize;
    UINT32 maxFreeNodeSize;
    UINT32 usedNodeNum;
    UINT32 freeNodeNum;
#ifdef LOSCFG_MEM_TASK_STAT
    UINT32 usageWaterLine;
#endif
} LosHeapStatus;

struct LosHeapNode {
    struct LosHeapNode *prev;
#ifdef LOSCFG_MEM_TASK_STAT
    UINT32 taskId;
#endif
#if (defined(LOSCFG_AARCH64) && !defined(LOSCFG_MEM_TASK_STAT))
    UINT32 res;
#endif
    UINT32 size     : 30;
    UINT32 used     : 1;
    UINT32 align    : 1;
    UINT8  data[0];
};

typedef struct LosHeapNode* MEM_NODE_STRUCT;

_Static_assert(((sizeof(struct LosHeapNode) % OS_MEM_ALIGN_SIZE) == 0), "invalid mem node alignment");

extern struct LosHeapNode *OsHeapPrvGetNext(const struct LosHeapManager *heapMan, const struct LosHeapNode *node);
extern BOOL OsHeapInit(VOID *pool, UINT32 size);
extern VOID* OsHeapAlloc(VOID *pool, UINT32 size);
extern VOID* OsHeapAllocAlign(VOID *pool, UINT32 size, UINT32 boundary);
extern BOOL OsHeapFree(VOID *pool, const VOID* ptr);
extern UINT32 OsHeapStatisticsGet(VOID *pool, LosHeapStatus *status);
extern UINT32 OsHeapIntegrityCheck(struct LosHeapManager *heap);
extern UINTPTR OsHeapLastUsedGet(VOID *pool);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_MEMORY_INTERNAL_H */
