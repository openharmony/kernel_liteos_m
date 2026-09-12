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

#include "los_slab_pri.h"

#include "los_memory_pri.h"
#include "los_debug.h"
#include "los_hwi.h"
#include "securec.h"
#include "los_task_pri.h"

#ifdef LOSCFG_KERNEL_LMS
#include "los_lms_pri.h"
#endif

STATIC UINT32 g_slabAllocatorSizeCfg[SLAB_MEM_COUNT] = {
    [0 ... SLAB_MEM_COUNT - 1] = SLAB_MEM_ALLOCATOR_SIZE
};

STATIC VOID OsSlabSizeReset(VOID)
{
    INT32 idx;

    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
        if (g_slabAllocatorSizeCfg[idx] != SLAB_MEM_ALLOCATOR_SIZE) {
            g_slabAllocatorSizeCfg[idx] = SLAB_MEM_ALLOCATOR_SIZE;
        }
    }
}

STATIC VOID *OsSlabBlockHeadFill(OsSlabBlockNode *slabNode, UINT32 blkSz)
{
    UINT32 taskId = LOS_CurTaskIDGet();
    OS_SLAB_BLOCK_MAGIC_SET(slabNode);
    OS_SLAB_BLOCK_SIZE_SET(slabNode, blkSz);
    OS_SLAB_BLOCK_ID_SET(slabNode, taskId); /* Records the current task ID. */
    return (VOID *)(slabNode + 1);
}

#ifdef LOSCFG_KERNEL_LMS
STATIC INLINE VOID OsLmsSlabAllocInitMark(OsSlabAllocator *alloc, UINT32 blkCnt)
{
    if (g_lms == NULL) {
        return;
    }
    /* mark allocator head as redzone */
    g_lms->simpleMark((UINTPTR)alloc, (UINTPTR)alloc->dataChunks, LMS_SHADOW_REDZONE_U8);
    /* mark datachunk as free */
    g_lms->simpleMark((UINTPTR)alloc->dataChunks, (UINTPTR)alloc->dataChunks + alloc->itemSz * blkCnt,
        LMS_SHADOW_AFTERFREE_U8);
}

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
STATIC INLINE VOID OsLmsSlabAllocDestoryMark(OsSlabAllocator *alloc, UINT32 blkCnt)
{
    /* mark allocator as access to do OsSlabAllocatorDestory */
    if (g_lms == NULL) {
        return;
    }
    g_lms->simpleMark((UINTPTR)alloc, (UINTPTR)alloc->dataChunks + alloc->itemSz * blkCnt,
        LMS_SHADOW_ACCESSABLE_U8);
}
#endif

STATIC INLINE VOID OsLmsSlabHeadMark(struct LosSlabControlHeader *slabMemHead, UINT32 bucketNum)
{
    if (g_lms == NULL) {
        return;
    }
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabAllocator *bucket = slabMemHead->allocatorBucket;
    /* mark allocatorBucket all as redzone */
    g_lms->simpleMark((UINTPTR)bucket, (UINTPTR)bucket->dataChunks + bucket->itemSz * bucketNum,
        LMS_SHADOW_REDZONE_U8);
#else
    (VOID)bucketNum;
#endif
    g_lms->simpleMark((UINTPTR)slabMemHead, (UINTPTR)slabMemHead + sizeof(struct LosSlabControlHeader),
        LMS_SHADOW_REDZONE_U8);
}

STATIC INLINE VOID OsLmsSlabAllocMark(const VOID *ptr, UINT32 blkSz)
{
    UINTPTR ptrAddr = (UINTPTR)ptr;

    if (g_lms == NULL) {
        return;
    }
    /* mark slab nodehead as redzone */
    g_lms->simpleMark(ptrAddr - sizeof(OsSlabBlockNode), ptrAddr, LMS_SHADOW_REDZONE_U8);
    /* mark slab datachunk as available */
    g_lms->simpleMark(ptrAddr, ptrAddr + blkSz, LMS_SHADOW_ACCESSABLE_U8);
}
#endif

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
STATIC BOOL OsSlabMemBucketInit(VOID *pool, UINT32 size)
{
    UINT32 bucketNum, idx, i, blkSz, blkCnt;
    struct LosSlabControlHeader *slabMemHead = OsSlabCtrlHdrGet(pool);

    bucketNum = (size / SLAB_MEM_ALLOCATOR_SIZE) >> SLAB_MEM_BUCKET_SIZE_TYPE;
    if ((slabMemHead == NULL) ||
        (SLAB_MEM_BUCKET_SIZE_TYPE >= SLAB_BUCKET_SIZE_TYPE_MAX) ||
        (bucketNum < SLAB_MEM_COUNT)) {
        PRINT_ERR("invalid para, size type %d, bucketNum %u\n", SLAB_MEM_BUCKET_SIZE_TYPE, bucketNum);
        return FALSE;
    }

    slabMemHead->allocatorBucket = OsSlabAllocatorNew(pool, sizeof(OsSlabMemAllocator),
                                                      (UINT32)sizeof(UINT32), bucketNum);
    if (slabMemHead->allocatorBucket == NULL) {
        return FALSE;
    }
    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
        blkSz = (SLAB_MEM_CLASS_STEP_SIZE << idx);
        blkCnt = SLAB_MEM_ALLOCATOR_SIZE / blkSz;
        slabMemHead->slabClass[idx].blkSz      = blkSz;
        slabMemHead->slabClass[idx].blkCnt     = blkCnt;
        slabMemHead->slabClass[idx].blkUsedCnt = 0;

        slabMemHead->slabClass[idx].bucket = OsSlabAllocatorAlloc(slabMemHead->allocatorBucket);
        if (slabMemHead->slabClass[idx].bucket == NULL) {
            goto BUCKET_ALLOC_FAIL;
        }
        slabMemHead->slabClass[idx].bucket->slabAlloc =
            OsSlabAllocatorNew(pool, blkSz + sizeof(OsSlabBlockNode), (UINT32)sizeof(CHAR *), blkCnt);
        if (slabMemHead->slabClass[idx].bucket->slabAlloc == NULL){
            goto ALLOCATOR_ALLOC_FAIL;
        }

        slabMemHead->slabClass[idx].bucket->next      = NULL;
        slabMemHead->slabClass[idx].allocatorCnt      = 1;
#if LOSCFG_KERNEL_LMS
        OsLmsSlabAllocInitMark(slabMemHead->slabClass[idx].bucket->slabAlloc, blkCnt);
#endif
    }
#ifdef LOSCFG_KERNEL_LMS
    OsLmsSlabHeadMark(slabMemHead, bucketNum);
#endif
    return TRUE;

ALLOCATOR_ALLOC_FAIL:
    for (i = 0; i < idx; i++) {
        (VOID)OsSlabAllocatorFree(slabMemHead->allocatorBucket, slabMemHead->slabClass[i].bucket);
    }

BUCKET_ALLOC_FAIL:
    OsSlabAllocatorDestroy(pool, slabMemHead->allocatorBucket);
    return FALSE;
}

STATIC BOOL OsSlabMemAllocStrollBuckets(VOID *pool, VOID **outMem,
    OsSlabMem *slabClass)
{
    struct LosSlabControlHeader *slabMem = OsSlabCtrlHdrGet(pool);
    OsSlabMem *slabMemClass = slabClass;
    OsSlabMemAllocator **bucket = &slabClass->bucket; /* slabMemClass->bucket must not be NULL */

    *outMem = NULL;

    while (*bucket != NULL) {
        *outMem = OsSlabAllocatorAlloc((*bucket)->slabAlloc);
        if (*outMem != NULL) {
            /* alloc success */
            return TRUE;
        }
        bucket = &((*bucket)->next);
    }

    *bucket = OsSlabAllocatorAlloc(slabMem->allocatorBucket);
    if (*bucket == NULL) {
        return FALSE;
    }

    (*bucket)->slabAlloc =
        OsSlabAllocatorNew(pool, slabMemClass->blkSz + sizeof(OsSlabBlockNode),
                           (UINT32)sizeof(CHAR *), slabMemClass->blkCnt);
    if ((*bucket)->slabAlloc == NULL) {
        (VOID)OsSlabAllocatorFree(slabMem->allocatorBucket, *bucket);
        *bucket = NULL;
        return FALSE;
    }

    (*bucket)->next = NULL;
    slabMemClass->allocatorCnt++;
#ifdef LOSCFG_KERNEL_LMS
    OsLmsSlabAllocInitMark((*bucket)->slabAlloc, slabMemClass->blkCnt);
#endif
    *outMem = OsSlabAllocatorAlloc((*bucket)->slabAlloc);

    return TRUE;
}


STATIC BOOL OsSlabMemFreeStrollBuckets(VOID *pool, OsSlabBlockNode *slabNode,
    OsSlabMem *slabClass)
{
    struct LosSlabControlHeader *slabMem = OsSlabCtrlHdrGet(pool);
    OsSlabMem *slabMemClass = slabClass;
    OsSlabMemAllocator **bucket = &(slabClass->bucket); /* slabMemClass->bucket must not be NULL */
    OsSlabAllocator *slabAlloc = NULL;

    while (*bucket != NULL) {
        slabAlloc = (*bucket)->slabAlloc;
        if (OsSlabAllocatorFree(slabAlloc, slabNode) == TRUE) {
#ifdef LOSCFG_KERNEL_LMS
            if (g_lms != NULL) {
                g_lms->freeMark((VOID *)slabNode, (VOID *)((UINTPTR)slabNode + slabAlloc->itemSz),
                    sizeof(OsSlabBlockNode));
            }
#endif
            /* try to destroy slabAllocator if needed */
            if ((slabMemClass->allocatorCnt > SLAB_MEM_DEFAULT_BUCKET_CNT) &&
                (OsSlabAllocatorEmpty(slabAlloc) == TRUE)) {
                OsSlabMemAllocator *alloc = *bucket;
                (*bucket) = (*bucket)->next; /* delete node in list */
#ifdef LOSCFG_KERNEL_LMS
                OsLmsSlabAllocDestoryMark(alloc->slabAlloc, slabMemClass->blkCnt);
#endif
                OsSlabAllocatorDestroy(pool, alloc->slabAlloc); /* destroy slabAllocator */
                (VOID)OsSlabAllocatorFree(slabMem->allocatorBucket, alloc);

                slabMemClass->allocatorCnt--;
            }
            return TRUE;
        }
        bucket = &((*bucket)->next);
    }

    return FALSE;
}

STATIC UINT32 OsGetBlkSzStrollBuckets(const OsSlabBlockNode *slabNode, const OsSlabMem *slabClass)
{
    OsSlabMem *slabMemClass = (OsSlabMem *)slabClass;
    OsSlabMemAllocator *bucket = slabMemClass->bucket; /* slabMemClass->bucket must not be NULL */
    OsSlabAllocator *slabAlloc = NULL;
    UINT32 retBlkSz = (UINT32)-1;

    while (bucket != NULL) {
        slabAlloc = bucket->slabAlloc;
        if (OsSlabAllocatorCheck(slabAlloc, (VOID *)slabNode) == TRUE) {
            retBlkSz = slabMemClass->blkSz;
            return retBlkSz;
        }
        bucket = bucket->next;
    }

    return retBlkSz;
}
#endif

VOID OsSlabMemInit(VOID *pool, UINT32 size)
{
    struct LosSlabControlHeader *slabMemHead = OsSlabCtrlHdrGet(pool);

    (VOID)memset(slabMemHead, 0, sizeof(struct LosSlabControlHeader));
    /*
     * Since if the size is not enough to create a SLAB pool, the memory pool
     * still works. We choose not treat that as failure.
     */
    if (size < SLAB_BASIC_NEED_SIZE) {
        PRINT_INFO("bad input size\n");
        return;
    }

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    if (!OsSlabMemBucketInit(pool, size)) {
        PRINT_INFO("slab bucket init failed\n");
    }
#else
    UINT32 idx, blkSz, blkCnt;

    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
        blkSz = (SLAB_MEM_CLASS_STEP_SIZE << idx);
        blkCnt = g_slabAllocatorSizeCfg[idx] / blkSz;
        slabMemHead->slabClass[idx].blkSz = blkSz;
        slabMemHead->slabClass[idx].blkCnt = blkCnt;
        slabMemHead->slabClass[idx].blkUsedCnt = 0;
        slabMemHead->slabClass[idx].alloc =
            OsSlabAllocatorNew(pool, blkSz + sizeof(OsSlabBlockNode), (UINT32)sizeof(CHAR *), blkCnt);
        if (slabMemHead->slabClass[idx].alloc == NULL) {
            PRINT_ERR("slab alloc failed\n");
#ifdef LOSCFG_KERNEL_LMS
        } else {
            OsLmsSlabAllocInitMark(slabMemHead->slabClass[idx].alloc, blkCnt);
        }
#else
        }
#endif
    }
#ifdef LOSCFG_KERNEL_LMS
    /* mark slab head as redzone */
    OsLmsSlabHeadMark(slabMemHead, 0);
#endif
    /* for multi pool, cfg size should reset */
    OsSlabSizeReset();
#endif
}

VOID *OsSlabMemAlloc(VOID *pool, UINT32 size)
{
    VOID *ret = NULL;
    struct LosSlabControlHeader *slabMem = OsSlabCtrlHdrGet(pool);
    UINT32 idx;

    if ((slabMem->enabled == FALSE) || (size > (SLAB_MEM_CLASS_STEP_SIZE << (SLAB_MEM_COUNT - 1)))) {
        return NULL;
    }

    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
        if (size <= slabMem->slabClass[idx].blkSz) {
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
            /* if slab mem alloc from current bucket successful, goto ALLOC_SUCC */
            /* otherwise, should alloc in slabAlloc with new bucket */
            if (OsSlabMemAllocStrollBuckets(pool, &ret, &(slabMem->slabClass[idx])) == FALSE) {
                return NULL;
            }
#else
            if (slabMem->slabClass[idx].blkUsedCnt >= slabMem->slabClass[idx].blkCnt) {
                return NULL;
            }

            if (slabMem->slabClass[idx].alloc == NULL) {
                return NULL;
            }

            ret = OsSlabAllocatorAlloc(slabMem->slabClass[idx].alloc);
#endif
            if (ret != NULL) {
                /* alloc success */
                ret = OsSlabBlockHeadFill((OsSlabBlockNode *)ret, slabMem->slabClass[idx].blkSz);
                slabMem->slabClass[idx].blkUsedCnt++;
#ifdef LOSCFG_KERNEL_LMS
                OsLmsSlabAllocMark(ret, slabMem->slabClass[idx].blkSz);
#endif
            }
            return ret;
        }
    }
    return NULL;
}

BOOL OsSlabMemFree(VOID *pool, VOID *ptr)
{
    struct LosSlabControlHeader *slabMem = OsSlabCtrlHdrGet(pool);
#ifndef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabAllocator *slabAlloc = NULL;
#endif
    UINT32 idx;
    OsSlabBlockNode *slabNode = OS_SLAB_BLOCK_HEAD_GET(ptr);

    if ((slabMem->enabled == FALSE) || (!OS_ALLOC_FROM_SLAB_CHECK(slabNode))) {
        return FALSE;
    }

    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
        if (slabMem->slabClass[idx].blkSz < OS_SLAB_BLOCK_SIZE_GET(slabNode)) {
            continue;
        }
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
        if (OsSlabMemFreeStrollBuckets(pool, slabNode, &(slabMem->slabClass[idx])) == FALSE) {
            break;
        }
#else
        slabAlloc = slabMem->slabClass[idx].alloc;
        if (OsSlabAllocatorFree(slabAlloc, slabNode) == FALSE) {
            break;
        }
#ifdef LOSCFG_KERNEL_LMS
        if (g_lms != NULL) {
            g_lms->freeMark((VOID *)slabNode, (VOID *)((UINTPTR)slabNode + slabAlloc->itemSz), sizeof(OsSlabBlockNode));
        }
#endif
#endif
        slabMem->slabClass[idx].blkUsedCnt--;
        return TRUE;
    }
    return FALSE;
}

VOID OsSlabMemDeinit(VOID *pool)
{
    UINT32 idx;
    struct LosSlabControlHeader *slabMem = NULL;
    OsSlabAllocator *slabAlloc = NULL;
    UINT32 blkSz;
    UINT32 blkCnt;
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabMemAllocator **bucket = NULL;
#endif

    if (pool == NULL) {
        return;
    }
    slabMem = OsSlabCtrlHdrGet(pool);

    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
        blkSz = (SLAB_MEM_CLASS_STEP_SIZE << idx);
        blkCnt = SLAB_MEM_ALLOCATOR_SIZE / blkSz;
        slabMem->slabClass[idx].blkSz = blkSz;
        slabMem->slabClass[idx].blkCnt = blkCnt;

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
        bucket = &(slabMem->slabClass[idx].bucket);
        while (*bucket != NULL) {
            slabAlloc = (*bucket)->slabAlloc;
            (*bucket) = (*bucket)->next;
            OsSlabAllocatorDestroy(pool, slabAlloc);
        }
#else
        if (slabMem->slabClass[idx].alloc != NULL) {
            slabAlloc = slabMem->slabClass[idx].alloc;
            OsSlabAllocatorDestroy(pool, slabAlloc);
            slabMem->slabClass[idx].alloc = NULL;
        }
#endif
    }
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabAllocatorDestroy(pool, slabMem->allocatorBucket);
    slabMem->allocatorBucket = NULL;
#endif

    OsSlabSizeReset();

    return;
}

UINT32 OsSlabMemCheck(const VOID *pool, const VOID* ptr)
{
    struct LosSlabControlHeader *slabMem = OsSlabCtrlHdrGet(pool);
    UINT32 retBlkSz = (UINT32)-1;
#ifndef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabAllocator *slabAlloc = NULL;
#endif
    UINT32 idx;
    OsSlabBlockNode *slabNode = OS_SLAB_BLOCK_HEAD_GET(ptr);

    if ((!OS_ALLOC_FROM_SLAB_CHECK(slabNode)) ||
        (slabMem->slabClass[SLAB_MEM_COUNT - 1].blkSz < (OS_SLAB_BLOCK_SIZE_GET(slabNode)))) {
        return retBlkSz;
    }

    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
        if (slabMem->slabClass[idx].blkSz >= OS_SLAB_BLOCK_SIZE_GET(slabNode)) {
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
            retBlkSz = OsGetBlkSzStrollBuckets(slabNode, &(slabMem->slabClass[idx]));
#else
            slabAlloc = slabMem->slabClass[idx].alloc;
            if (OsSlabAllocatorCheck(slabAlloc, slabNode) == TRUE) {
                retBlkSz = slabMem->slabClass[idx].blkSz;
            }
#endif
            return retBlkSz;
        }
    }

    return retBlkSz;
}

VOID *OsSlabCtrlHdrGet(const VOID *pool)
{
    return &((LosMemPoolInfo *)pool)->slabCtrlHdr;
}

STATIC_INLINE VOID OsSlabStatusInit(LosSlabStatus *status, UINT32 totalMem, UINT32 totalUsage,
    UINT32 totalAllocCount, UINT32 totalFreeCount)
{
    status->totalSize  = totalMem;
    status->usedSize   = totalUsage;
    status->freeSize   = status->totalSize - status->usedSize;
    status->allocCount = totalAllocCount;
    status->freeCount  = totalFreeCount;
}

UINT32 OsSlabStatisticsGet(const VOID *pool, LosSlabStatus *status)
{
    struct LosSlabControlHeader *slabMem = NULL;
    OsSlabAllocator *slabAlloc = NULL;
    UINT32 itemSz = 0;
    UINT32 itemCnt = 0;
    UINT32 curUsage = 0;
    UINT32 totalUsage = 0;
    UINT32 totalMem = 0;
    UINT32 totalAllocCount = 0;
    UINT32 totalFreeCount = 0;
    UINT32 idx;
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabMemAllocator *bucket = NULL;
#endif

    if ((status == NULL) || (pool == NULL)) {
        return LOS_NOK;
    }

    slabMem = OsSlabCtrlHdrGet(pool);

    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
        bucket = slabMem->slabClass[idx].bucket;
        while (bucket != NULL) {
            slabAlloc = bucket->slabAlloc;

            OsSlabAllocatorGetSlabInfo(slabAlloc, &itemSz, &itemCnt, &curUsage);
            totalUsage += (curUsage * itemSz);
            totalMem += (itemCnt * itemSz);

            bucket = bucket->next;
        }

        totalAllocCount += slabMem->slabClass[idx].blkUsedCnt;
        totalFreeCount  += (slabMem->slabClass[idx].blkCnt * slabMem->slabClass[idx].allocatorCnt) -
                                                slabMem->slabClass[idx].blkUsedCnt;
#else
        slabAlloc = slabMem->slabClass[idx].alloc;
        OsSlabAllocatorGetSlabInfo(slabAlloc, &itemSz, &itemCnt, &curUsage);
        totalUsage += (curUsage * itemSz);
        totalMem += (itemCnt * itemSz);
        totalAllocCount += slabMem->slabClass[idx].blkUsedCnt;
        totalFreeCount  += slabMem->slabClass[idx].blkCnt - slabMem->slabClass[idx].blkUsedCnt;
#endif
    }

    if (totalMem < totalUsage) {
        return LOS_NOK;
    }

    OsSlabStatusInit(status, totalMem, totalUsage, totalAllocCount, totalFreeCount);
    return LOS_OK;
}

UINT32 OsSlabGetMaxFreeBlkSize(const VOID *pool)
{
    struct LosSlabControlHeader *slabMem = OsSlabCtrlHdrGet(pool);
    OsSlabAllocator *slabAlloc = NULL;
    UINT32 itemSz = 0;
    UINT32 itemCnt = 0;
    UINT32 curUsage = 0;
    INT32 idx;

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabMemAllocator *bucket = NULL;
#endif

    for (idx = SLAB_MEM_COUNT - 1; idx >= 0; idx--) {
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
        bucket = slabMem->slabClass[idx].bucket;
        if (bucket == NULL) { continue; }
        slabAlloc = bucket->slabAlloc;
#else
        slabAlloc = slabMem->slabClass[idx].alloc;
#endif
        if (slabAlloc != NULL) {
            OsSlabAllocatorGetSlabInfo(slabAlloc, &itemSz, &itemCnt, &curUsage);
            if (curUsage != itemCnt) {
                return itemSz;
            }
        }
    }

    return 0;
}

VOID LOS_SlabSizeCfg(UINT32 *cfg, UINT32 cnt)
{
    INT32 idx;

    if ((cfg == NULL) || (cnt != SLAB_MEM_COUNT)) {
        PRINT_ERR("cfg is NULL, or cnt %u != SLAB_MEM_COUNT\n", cnt);
        return;
    }

    for (idx = 0; idx < SLAB_MEM_COUNT; idx++) {
        if (cfg[idx] < SLAB_MEM_ALLOCATOR_SIZE) {
            g_slabAllocatorSizeCfg[idx] = SLAB_MEM_ALLOCATOR_SIZE;
        } else {
            g_slabAllocatorSizeCfg[idx] = cfg[idx];
        }
    }
}