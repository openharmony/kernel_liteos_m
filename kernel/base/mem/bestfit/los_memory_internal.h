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

#ifndef _LOS_MEMORY_INTERNAL_H
#define _LOS_MEMORY_INTERNAL_H

#include "los_typedef.h"
#include "los_memory_pri.h"
#include "los_multipledlinkhead_pri.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Memory linked list control node structure */
typedef struct {
    union {
        LOS_DL_LIST freeNodeInfo;         /* Free memory node */
        struct {
            UINTPTR magic;
            UINTPTR taskId   : 16;
#ifdef LOSCFG_MEM_MUL_MODULE
            UINTPTR moduleId : 16;
#endif
        };
    };

    struct tagLosMemDynNode *preNode; /* Pointer to the previous memory node */

#ifdef LOSCFG_MEM_HEAD_BACKUP
    UINT32 gapSize;
    UINTPTR checksum; /* magic = xor checksum */
#endif

#ifdef LOSCFG_MEM_LEAKCHECK
    UINTPTR linkReg[LOSCFG_MEM_RECORD_LR_CNT];
#endif

#ifdef LOSCFG_AARCH64
    UINT32 reserve2; /* 64-bit alignment */
#endif
    /* Size and flag of the current node (the high two bits represent a flag,and the rest bits specify the size) */
    UINT32 sizeAndFlag;
} LosMemCtlNode;

/* Memory linked list node structure */
typedef struct tagLosMemDynNode {
#ifdef LOSCFG_MEM_HEAD_BACKUP
    LosMemCtlNode backupNode;
#endif
    LosMemCtlNode selfNode;
} LosMemDynNode;

typedef LosMemDynNode* MEM_NODE_STRUCT;
#define OS_MEM_NODE_HEAD_SIZE               sizeof(LosMemDynNode)
/* Requires at least 1 byte of allocated space */
#define OS_MEM_MIN_POOL_SIZE                (OS_DLNK_HEAD_SIZE + (2 * OS_MEM_NODE_HEAD_SIZE) + \
                                            sizeof(LosMemPoolInfo) + 1)
/* The upper two bits [31:30] are used as a flag. */
#define OS_MEM_MAX_POOL_SIZE                0x3FFFFFFFU
#define IS_POW_TWO(value)                   ((((UINTPTR)(value)) & ((UINTPTR)(value) - 1)) == 0)
#define POOL_ADDR_ALIGNSIZE                 64
#define OS_MEM_NODE_USED_FLAG               0x80000000U
#define OS_MEM_NODE_ALIGNED_FLAG            0x40000000U
#define OS_MEM_NODE_ALIGNED_AND_USED_FLAG   (OS_MEM_NODE_USED_FLAG | OS_MEM_NODE_ALIGNED_FLAG)

#define OS_MEM_NODE_GET_ALIGNED_FLAG(sizeAndFlag) \
    ((sizeAndFlag) & OS_MEM_NODE_ALIGNED_FLAG)
#define OS_MEM_NODE_SET_ALIGNED_FLAG(sizeAndFlag) \
    ((sizeAndFlag) = ((sizeAndFlag) | OS_MEM_NODE_ALIGNED_FLAG))
#define OS_MEM_NODE_GET_ALIGNED_GAPSIZE(sizeAndFlag) \
    ((sizeAndFlag) & ~OS_MEM_NODE_ALIGNED_FLAG)
#define OS_MEM_NODE_GET_USED_FLAG(sizeAndFlag) \
    ((sizeAndFlag) & OS_MEM_NODE_USED_FLAG)
#define OS_MEM_NODE_SET_USED_FLAG(sizeAndFlag) \
    ((sizeAndFlag) = ((sizeAndFlag) | OS_MEM_NODE_USED_FLAG))
#define OS_MEM_NODE_GET_SIZE(sizeAndFlag) \
    ((sizeAndFlag) & ~OS_MEM_NODE_ALIGNED_AND_USED_FLAG)
#define OS_MEM_HEAD(pool, size) \
    OsDLnkMultiHead(OS_MEM_HEAD_ADDR(pool), size)
#define OS_MEM_HEAD_ADDR(pool) \
    ((VOID *)((UINTPTR)(pool) + sizeof(LosMemPoolInfo)))
#define OS_MEM_NEXT_NODE(node) \
    ((LosMemDynNode *)(VOID *)((UINT8 *)(node) + OS_MEM_NODE_GET_SIZE((node)->selfNode.sizeAndFlag)))
#define OS_MEM_FIRST_NODE(pool) \
    ((LosMemDynNode *)(VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE))
#define OS_MEM_END_NODE(pool, size) \
    ((LosMemDynNode *)(VOID *)(((UINT8 *)(pool) + (size)) - OS_MEM_NODE_HEAD_SIZE))
#define OS_MEM_MIDDLE_ADDR_OPEN_END(startAddr, middleAddr, endAddr) \
    (((UINT8 *)(startAddr) <= (UINT8 *)(middleAddr)) && ((UINT8 *)(middleAddr) < (UINT8 *)(endAddr)))
#define OS_MEM_MIDDLE_ADDR(startAddr, middleAddr, endAddr) \
    (((UINT8 *)(startAddr) <= (UINT8 *)(middleAddr)) && ((UINT8 *)(middleAddr) <= (UINT8 *)(endAddr)))
#define OS_MEM_SET_MAGIC(value) \
    ((value) = (UINTPTR)&(value) ^ (UINTPTR)(-1))
#define OS_MEM_MAGIC_VALID(value) \
    (((UINTPTR)(value) ^ (UINTPTR)&(value)) == (UINTPTR)(-1))

#ifdef LOSCFG_MEM_MUL_POOL_ALLOC
VOID *OsMulPoolCopyToNewNode(VOID *pool, VOID *ptr, UINT32 size, VOID *newPtr);
#endif

LosMemDynNode *OsGetRealPtr(const VOID *pool, const VOID *ptr);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_MEMORY_INTERNAL_H */
