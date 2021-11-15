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

#include "los_secure_heap.h"
#include "los_secure_macros.h"
#include "los_config.h"
#include "los_list.h"

#pragma data_alignment=0x4
STATIC UINT8 g_secureHeap[LOSCFG_SECURE_HEAP_SIZE] = {0};
STATIC LOS_DL_LIST g_secureHeapFreeList = {NULL, NULL};

struct OsSecureHeapNode {
    LOS_DL_LIST freeNode;
    struct OsSecureHeapNode *preNode;
    UINT32 size : 24;
    UINT32 used : 8;
};

#define OS_SECURE_HEAP_NODE_HEAD_SIZE   sizeof(struct OsSecureHeapNode)
#define OS_SECURE_HEAP_ALIGN_SIZE       sizeof(UINTPTR)

#define OS_SECURE_HEAP_NODE_USED        1
#define OS_SECURE_HEAP_NODE_FREE        0

#define OS_SECURE_HEAP_FIRST_NODE     ((struct OsSecureHeapNode *)g_secureHeap)
#define OS_SECURE_HEAP_NEXT_NODE(node) \
    ((struct OsSecureHeapNode *)((UINT8 *)(node) + (node)->size))
#define OS_SECURE_HEAP_END_NODE \
    ((struct OsSecureHeapNode *)((UINT8 *)g_secureHeap + LOSCFG_SECURE_HEAP_SIZE - OS_SECURE_HEAP_NODE_HEAD_SIZE))

STATIC INLINE VOID OsSecureHeapListInit(LOS_DL_LIST *head)
{
    head->pstPrev = head;
    head->pstNext = head;
}

STATIC INLINE VOID OsSecureHeapListDelete(LOS_DL_LIST *node)
{
    node->pstNext->pstPrev = node->pstPrev;
    node->pstPrev->pstNext = node->pstNext;
    node->pstNext = NULL;
    node->pstPrev = NULL;
}

STATIC INLINE VOID OsSecureHeapListAdd(LOS_DL_LIST *listNode, LOS_DL_LIST *node)
{
    node->pstNext = listNode->pstNext;
    node->pstPrev = listNode;
    listNode->pstNext->pstPrev = node;
    listNode->pstNext = node;
}

STATIC struct OsSecureHeapNode *OsSecureHeapFindSuitableFreeBlock(UINT32 allocSize)
{
    LOS_DL_LIST *listNodeHead = &g_secureHeapFreeList;
    struct OsSecureHeapNode *tmpNode = NULL;

    LOS_DL_LIST_FOR_EACH_ENTRY(tmpNode, listNodeHead, struct OsSecureHeapNode, freeNode) {
        if (tmpNode->size >= allocSize) {
            return tmpNode;
        }
    }

    return NULL;
}

STATIC INLINE VOID OsSecureHeapClearNode(struct OsSecureHeapNode *node)
{
    node->preNode = NULL;
    node->size = 0;
    node->used = 0;
    node->freeNode.pstPrev = NULL;
    node->freeNode.pstNext = NULL;
}

STATIC INLINE VOID OsSecureHeapMergeNode(struct OsSecureHeapNode *node)
{
    struct OsSecureHeapNode *nextNode = NULL;

    node->preNode->size += node->size;
    nextNode = (struct OsSecureHeapNode *)((UINTPTR)node + node->size);
    nextNode->preNode = node->preNode;
    OsSecureHeapClearNode(node);
}

STATIC INLINE VOID OsSecureHeapSplitNode(struct OsSecureHeapNode *allocNode, UINT32 allocSize)
{
    struct OsSecureHeapNode *newFreeNode = NULL;
    struct OsSecureHeapNode *nextNode = NULL;

    newFreeNode = (struct OsSecureHeapNode *)((UINT8 *)allocNode + allocSize);
    newFreeNode->preNode = allocNode;
    newFreeNode->size = allocNode->size - allocSize;
    newFreeNode->used = OS_SECURE_HEAP_NODE_FREE;
    allocNode->size = allocSize;
    nextNode = OS_SECURE_HEAP_NEXT_NODE(newFreeNode);
    nextNode->preNode = newFreeNode;
    if (nextNode->used == OS_SECURE_HEAP_NODE_FREE) {
        OsSecureHeapListDelete(&nextNode->freeNode);
        OsSecureHeapMergeNode(nextNode);
    }

    OsSecureHeapListAdd(&g_secureHeapFreeList, &newFreeNode->freeNode);
}

STATIC INLINE VOID OsSecureHeapFreeNode(struct OsSecureHeapNode *node)
{
    struct OsSecureHeapNode *nextNode = NULL;

    if ((node->preNode != NULL) && (node->preNode->used == OS_SECURE_HEAP_NODE_FREE)) {
        struct OsSecureHeapNode *preNode = node->preNode;
        OsSecureHeapMergeNode(node);
        nextNode = OS_SECURE_HEAP_NEXT_NODE(preNode);
        if (nextNode->used == OS_SECURE_HEAP_NODE_FREE) {
            OsSecureHeapListDelete(&nextNode->freeNode);
            OsSecureHeapMergeNode(nextNode);
        }

        OsSecureHeapListDelete(&preNode->freeNode);
        preNode->used = OS_SECURE_HEAP_NODE_FREE;
        OsSecureHeapListAdd(&g_secureHeapFreeList, &preNode->freeNode);
    } else {
        nextNode = OS_SECURE_HEAP_NEXT_NODE(node);
        if (nextNode->used == OS_SECURE_HEAP_NODE_FREE) {
            OsSecureHeapListDelete(&nextNode->freeNode);
            OsSecureHeapMergeNode(nextNode);
        }

        node->used = OS_SECURE_HEAP_NODE_FREE;
        OsSecureHeapListAdd(&g_secureHeapFreeList, &node->freeNode);
    }
}

STATIC INLINE VOID *OsSecureHeapAllocNode(UINT32 size)
{
    struct OsSecureHeapNode *allocNode = NULL;
    UINT32 allocSize;

    allocSize = LOS_Align(size + OS_SECURE_HEAP_NODE_HEAD_SIZE, OS_SECURE_HEAP_ALIGN_SIZE);
    allocNode = OsSecureHeapFindSuitableFreeBlock(allocSize);
    if (allocNode == NULL) {
        return NULL;
    }
    if ((allocSize + OS_SECURE_HEAP_NODE_HEAD_SIZE + OS_SECURE_HEAP_ALIGN_SIZE) <= allocNode->size) {
        OsSecureHeapSplitNode(allocNode, allocSize);
    }
    OsSecureHeapListDelete(&allocNode->freeNode);
    allocNode->used = OS_SECURE_HEAP_NODE_USED;

    return (allocNode + 1);
}

STATIC INLINE VOID OsSecureHeapInit(VOID)
{
    struct OsSecureHeapNode *newNode = NULL;
    struct OsSecureHeapNode *endNode = NULL;

    newNode = OS_SECURE_HEAP_FIRST_NODE;
    newNode->size = LOSCFG_SECURE_HEAP_SIZE - OS_SECURE_HEAP_NODE_HEAD_SIZE;
    newNode->preNode = OS_SECURE_HEAP_END_NODE;
    newNode->used = OS_SECURE_HEAP_NODE_FREE;

    OsSecureHeapListInit(&g_secureHeapFreeList);
    OsSecureHeapListAdd(&g_secureHeapFreeList, &newNode->freeNode);

    endNode = OS_SECURE_HEAP_END_NODE;
    endNode->preNode = newNode;
    endNode->size = OS_SECURE_HEAP_NODE_HEAD_SIZE;
    endNode->used = OS_SECURE_HEAP_NODE_USED;
}

OS_CMSE_NS_ENTRY VOID *HalSecureMalloc(UINT32 size)
{
    if (size == 0) {
        return NULL;
    }

    if ((g_secureHeapFreeList.pstPrev == NULL) &&
        (g_secureHeapFreeList.pstNext == NULL)) {
        OsSecureHeapInit();
    }

    return OsSecureHeapAllocNode(size);
}

OS_CMSE_NS_ENTRY VOID HalSecureFree(VOID *ptr)
{
    struct OsSecureHeapNode *node = NULL;

    if (ptr == NULL) {
        return;
    }

    node = (struct OsSecureHeapNode *)((UINTPTR)ptr - OS_SECURE_HEAP_NODE_HEAD_SIZE);
    if (node->used != OS_SECURE_HEAP_NODE_USED) {
        return;
    }

    OsSecureHeapFreeNode(node);
}

