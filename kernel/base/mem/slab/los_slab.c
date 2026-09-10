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

#include "securec.h"

#define NUM_BITS_IN_ONE_WORD              32
#define LOW_BITS_MASK                     31U
#define NUMBITS_TO_NUMBYTES(numBits)      (((numBits) + LOW_BITS_MASK) / 8)

STATIC VOID OsAtomicBitsetInit(struct AtomicBitset *set, UINT32 numBits)
{
    /* mark all high bits so that OsAtomicBitsetFindClearAndSet() is simpler */
    if (numBits & LOW_BITS_MASK) {
        set->words[numBits / NUM_BITS_IN_ONE_WORD] =
            ((UINT32)((INT32) -1LL)) << (numBits & LOW_BITS_MASK);
    }
}

STATIC UINT32 OsAtomicBitsetGetNumBits(const struct AtomicBitset *set)
{
    return set->numBits;
}

STATIC BOOL OsAtomicBitsetGetBit(const struct AtomicBitset *set, UINT32 num)
{
    /* any value is as good as the next */
    if (num >= set->numBits) {
        return FALSE;
    }
    return !!((set->words[num / NUM_BITS_IN_ONE_WORD]) & (1U << (num & LOW_BITS_MASK)));
}

STATIC VOID OsAtomicBitsetClearBit(struct AtomicBitset *set, UINT32 num)
{
    UINT32 *wordPtr = set->words + (num / NUM_BITS_IN_ONE_WORD);

    if (num >= set->numBits) {
        return;
    }
    (*wordPtr) &= ~(1U << (num & LOW_BITS_MASK));
}

/* find from the high bit to high bit return the address of the first available bit */
STATIC INT32 OsAtomicBitsetFindClearAndSet(struct AtomicBitset *set)
{
    UINT32 idx;
    UINT32 numWords = (set->numBits + LOW_BITS_MASK) / NUM_BITS_IN_ONE_WORD;
    UINT32 *wordPtr = set->words;
    UINT32 tmpWord;
    UINT32 count = 0;

    for (idx = 0; idx < numWords; idx++, wordPtr++) {
        if (*wordPtr == 0xFFFFFFFF) {
            continue;
        }

        tmpWord = ~(*wordPtr);

        while (tmpWord) {
            tmpWord = tmpWord >> 1U;
            count++;
        }

        *wordPtr |= (1U << (count - 1));

        return (INT32)(idx * NUM_BITS_IN_ONE_WORD + count - 1);
    }

    return -1;
}

STATIC BOOL OsAtomicBitsetEmpty(const struct AtomicBitset *bitset)
{
    UINT32 idx;
    for (idx = 0; idx < (bitset->numBits / NUM_BITS_IN_ONE_WORD); idx++) {
        if (bitset->words[idx] != 0) {
            return FALSE;
        }
    }
    if (bitset->numBits & LOW_BITS_MASK) {
        if (bitset->words[idx] & ~((UINT32)0xFFFFFFFF << (bitset->numBits & LOW_BITS_MASK))) {
            return FALSE;
        }
    }
    return TRUE;
}

OsSlabAllocator *OsSlabAllocatorNew(VOID *pool, UINT32 itemSz, UINT32 itemAlign, UINT32 numItems)
{
    OsSlabAllocator *allocator = NULL;
    UINT32 atomicBitSetSz;
    UINT32 bitSetSz;
    UINT32 dataSz;
    UINT32 itemSize;

    /* calculate size */
    bitSetSz = NUMBITS_TO_NUMBYTES(numItems);
    atomicBitSetSz = (UINT32)sizeof(struct AtomicBitset) + bitSetSz;

    atomicBitSetSz = (atomicBitSetSz + itemAlign - 1) & (~(itemAlign - 1));
    itemSize = (itemSz + itemAlign - 1) & (~(itemAlign - 1));
    dataSz = itemSize * numItems;
    allocator = (OsSlabAllocator*)OsMemAlloc(pool, sizeof(OsSlabAllocator) + atomicBitSetSz + dataSz);
    if (allocator != NULL) {
        allocator->itemSz = itemSize;

        allocator->bitset = (struct AtomicBitset *)(VOID *)((UINT8*)allocator + sizeof(OsSlabAllocator));
        allocator->dataChunks = ((UINT8*)allocator->bitset) + atomicBitSetSz;

        struct AtomicBitset *set = allocator->bitset;
        set->numBits = numItems;
        (VOID)memset(set->words, 0, bitSetSz);
        OsAtomicBitsetInit(set, numItems);
    }

    return allocator;
}

VOID OsSlabAllocatorDestroy(VOID *pool, OsSlabAllocator *allocator)
{
    (VOID)OsMemFree(pool, allocator);
}

VOID *OsSlabAllocatorAlloc(OsSlabAllocator *allocator)
{
    INT32 itemIdx = OsAtomicBitsetFindClearAndSet(allocator->bitset);
    if (itemIdx < 0) {
        return NULL;
    }

    return allocator->dataChunks + allocator->itemSz * (UINT32)itemIdx;
}

BOOL OsSlabAllocatorFree(OsSlabAllocator *allocator, VOID* ptr)
{
    UINT8 *ptrTmp = (UINT8*)ptr;
    UINT32 itemOffset = (UINT32)(ptrTmp - allocator->dataChunks);
    UINT32 itemIdx = itemOffset / allocator->itemSz;

    /* check for invalid inputs */
    if ((itemOffset % allocator->itemSz) || (itemIdx >= OsAtomicBitsetGetNumBits(allocator->bitset)) ||
         !(OsAtomicBitsetGetBit(allocator->bitset, itemIdx))) {
        return FALSE;
    }

    OsAtomicBitsetClearBit(allocator->bitset, itemIdx);
    return TRUE;
}

VOID *OsSlabAllocatorGetIdxP(const OsSlabAllocator *allocator, UINT32 idx)
{
    return allocator->dataChunks + allocator->itemSz * idx;
}

BOOL OsSlabAllocatorEmpty(const OsSlabAllocator *allocator)
{
    return OsAtomicBitsetEmpty(allocator->bitset);
}

UINT32 OsSlabAllocatorGetUsedItemCnt(const OsSlabAllocator *allocator)
{
    UINT32 used;
    UINT32 idx;
    struct AtomicBitset *bitset = allocator->bitset;
    for (used = 0, idx = 0; idx < bitset->numBits; idx++) {
        if (OsAtomicBitsetGetBit(bitset, idx)) {
            used++;
        }
    }
    return used;
}

VOID OsSlabAllocatorGetSlabInfo(const OsSlabAllocator *allocator, UINT32 *itemSize, UINT32 *itemCnt, UINT32 *curUsage)
{
    *itemSize = allocator->itemSz;
    *itemCnt = OsAtomicBitsetGetNumBits(allocator->bitset);
    *curUsage = OsSlabAllocatorGetUsedItemCnt(allocator);
}

BOOL OsSlabAllocatorCheck(const OsSlabAllocator *allocator, const VOID* ptr)
{
    UINT8 *ptrTmp = (UINT8*)ptr;
    UINT32 itemOffset = (UINT32)(ptrTmp - allocator->dataChunks);
    UINT32 itemIdx = itemOffset / allocator->itemSz;

    /* check for invalid inputs */
    if ((itemOffset % allocator->itemSz) || (itemIdx >= OsAtomicBitsetGetNumBits(allocator->bitset)) ||
        !(OsAtomicBitsetGetBit(allocator->bitset, itemIdx))) {
        return FALSE;
    }

    return TRUE;
}
