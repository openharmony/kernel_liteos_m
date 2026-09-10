/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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

/**
 * @defgroup los_memory  Dynamic memory
 * @ingroup kernel
 */

#ifndef _LOS_MEMORY_H
#define _LOS_MEMORY_H

#include "los_config.h"
#include "los_list.h"
#include "los_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#if (LOSCFG_PLATFORM_EXC == 1)
UINT32 OsMemExcInfoGet(UINT32 memNumMax, MemInfoCB *memExcInfo);
#endif

/**
 * @ingroup los_memory
 * Starting address of the memory.
 */
#define OS_SYS_MEM_ADDR     LOSCFG_SYS_HEAP_ADDR

#ifndef OS_MEM_ALIGN_SIZE
#define OS_MEM_ALIGN_SIZE   (sizeof(UINTPTR))
#endif
#ifndef OS_MEM_ALIGN
#define OS_MEM_ALIGN(p, alignSize)   (((UINTPTR)(p) + (alignSize) - 1) & ~((UINTPTR)((alignSize) - 1)))
#endif
typedef VOID (*MALLOC_HOOK)(VOID);
extern MALLOC_HOOK g_MALLOC_HOOK;

#ifndef LOS_MEM_INIT_ATTR_SLAB_MSK
#define LOS_MEM_INIT_ATTR_SLAB_MSK       0x1U
#endif

#if (LOSCFG_MEM_LEAKCHECK == 1)
/**
 * @ingroup los_memory
 * @brief Print function call stack information of all used nodes.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to print function call stack information of all used nodes.</li>
 * </ul>
 *
 * @param pool          [IN] Starting address of memory.
 *
 * @retval none.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
extern VOID LOS_MemUsedNodeShow(VOID *pool);
#endif

/**
 * @ingroup los_memory
 * @brief Deinitialize dynamic memory.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to deinitialize the dynamic memory of a doubly linked list.</li>
 * </ul>
 *
 * @param pool          [IN] Starting address of memory.
 *
 * @retval #OS_ERROR   The dynamic memory fails to be deinitialized.
 * @retval #LOS_OK     The dynamic memory is successfully deinitialized.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
extern UINT32 LOS_MemDeInit(VOID *pool);

#if (LOSCFG_MEM_MUL_POOL == 1)
/**
 * @ingroup los_memory
 * @brief Print information about all pools.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to print information about all pools.</li>
 * </ul>
 *
 * @retval #UINT32   The pool number.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
extern UINT32 LOS_MemPoolList(VOID);
#endif

#if (LOSCFG_MEM_FREE_BY_TASKID == 1)
/**
 * @ingroup los_memory
 * @brief Free memory nodes allocated by the specified task.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to free all memory nodes allocated by the specified task.</li>
 * </ul>
 *
 * @param pool           [IN] The memory pool address.
 * @param taskId         [IN] The task ID and all memory nodes allocated by this task will be freed.
 *
 * @retval #OS_ERROR     The memory pool is NULL or the task ID is invalid.
 * @retval #LOS_OK       All memory nodes allocated by this task are freed successfully.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
extern UINT32 LOS_MemFreeByTaskID(VOID *pool, UINT32 taskId);
#endif

#if (LOSCFG_MEM_MUL_REGIONS == 1)
typedef struct {
    VOID *startAddress;
    UINT32 length;
} LosMemRegion;

/**
 * @ingroup los_memory
 * @brief Initialize multiple non-continuous memory regions.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to initialize multiple non-continuous memory regions. If the starting address of a pool is
 *  specified, the memory regions will be linked to the pool as free nodes. Otherwise, the first memory region will
 *  be initialized as a *  new pool, and the rest regions will be linked as free nodes to the new pool.</li>
 * </ul>
 *
 * @attention
 * <ul>
 * <li>If the starting address of a memory pool is specified, the start address of the non-continuous memory regions should be
 *  greater than the end address of the memory pool.</li>
 * <li>The multiple non-continuous memory regions shouldn't conflict with each other.</li>
 * </ul>
 *
 * @param pool           [IN] The memory pool address. If NULL is specified, the start address of first memory region will be
 *                            initialized as the memory pool address. If not NULL, it should be a valid address of a memory pool.
 * @param memRegions     [IN] The LosMemRegion array that contains multiple non-continuous memory regions. The start address
 *                           of the memory regions are placed in ascending order.
 * @param memRegionCount [IN] The count of non-continuous memory regions, and it should be the length of the LosMemRegion array.
 *
 * @retval #LOS_NOK    The multiple non-continuous memory regions fails to be initialized.
 * @retval #LOS_OK     The multiple non-continuous memory regions is initialized successfully.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
extern UINT32 LOS_MemRegionsAdd(VOID *pool, const LosMemRegion * const memRegions, UINT32 memRegionCount);
#endif

/**
 * @ingroup los_memory
 * Memory pool extern information structure
 */
typedef struct {
    UINT32 totalUsedSize;
    UINT32 totalFreeSize;
    UINT32 maxFreeNodeSize;
    UINT32 usedNodeNum;
    UINT32 freeNodeNum;
#if (LOSCFG_MEM_WATERLINE == 1)
    UINT32 usageWaterLine;
#endif
} LOS_MEM_POOL_STATUS;

/**
 * @ingroup los_memory
 * @brief Initialize dynamic memory.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to initialize the dynamic memory of a doubly linked list.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The size parameter value should match the following two conditions :
 * 1) Be less than or equal to the Memory pool size;
 * 2) Be greater than the size of OS_MEM_MIN_POOL_SIZE.</li>
 * <li>Call this API when dynamic memory needs to be initialized during the startup of Huawei LiteOS.</li>
 * <li>The parameter input must be four byte-aligned.</li>
 * <li>The init area [pool, pool + size] should not conflict with other pools.</li>
 * </ul>
 *
 * @param pool         [IN] Starting address of memory.
 * @param size         [IN] Memory size.
 *
 * @retval #OS_ERROR   The dynamic memory fails to be initialized.
 * @retval #LOS_OK     The dynamic memory is successfully initialized.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
extern UINT32 LOS_MemInit(VOID *pool, UINT32 size);

/**
 * @ingroup los_memory
 * @brief Allocate dynamic memory.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to allocate a memory block of which the size is specified.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The size of the input parameter size can not be greater than the memory pool size that specified at the second
 * input parameter of LOS_MemInit.</li>
 * <li>The size of the input parameter size must be four byte-aligned.</li>
 * </ul>
 *
 * @param  pool    [IN] Pointer to the memory pool that contains the memory block to be allocated.
 * @param  size    [IN] Size of the memory block to be allocated (unit: byte).
 *
 * @retval #NULL          The memory fails to be allocated.
 * @retval #VOID*         The memory is successfully allocated with the starting address of the allocated memory block
 *                        returned.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemRealloc | LOS_MemAllocAlign | LOS_MemFree
 */
extern VOID *LOS_MemAlloc(VOID *pool, UINT32 size);

/**
 * @ingroup los_memory
 * @brief Free dynamic memory.
 *
 * @par Description:
 * <li>This API is used to free specified dynamic memory that has been allocated.</li>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The input ptr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign or LOS_MemRealloc.</li>
 * </ul>
 *
 * @param  pool  [IN] Pointer to the memory pool that contains the dynamic memory block to be freed.
 * @param  ptr   [IN] Starting address of the memory block to be freed.
 *
 * @retval #LOS_NOK          The memory block fails to be freed because the starting address of the memory block is
 *                           invalid, or the memory overwriting occurs.
 * @retval #LOS_OK           The memory block is successfully freed.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemAlloc | LOS_MemRealloc | LOS_MemAllocAlign
 */
extern UINT32 LOS_MemFree(VOID *pool, VOID *ptr);

/**
 * @ingroup los_memory
 * @brief Re-allocate a memory block.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to allocate a new memory block of which the size is specified by size if the original memory
 * block size is insufficient. The new memory block will copy the data in the original memory block of which the
 * address is specified by ptr. The size of the new memory block determines the maximum size of data to be copied.
 * After the new memory block is created, the original one is freed.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The input ptr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 * <li>The size of the input parameter size can not be greater than the memory pool size that specified at the second
 * input parameter of LOS_MemInit.</li>
 * <li>The size of the input parameter size must be aligned as follows: 1) if the ptr is allocated by LOS_MemAlloc,
 * it must be four byte-aligned; 2) if the ptr is allocated by LOS_MemAllocAlign, it must be aligned with the size of
 * the input parameter boundary of LOS_MemAllocAlign.</li>
 * </ul>
 *
 * @param  pool     [IN] Pointer to the memory pool that contains the original and new memory blocks.
 * @param  ptr      [IN] Address of the original memory block.
 * @param  size     [IN] Size of the new memory block.
 *
 * @retval #NULL    The memory fails to be re-allocated.
 * @retval #VOID*   The memory is successfully re-allocated with the starting address of the new memory block returned.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemAlloc | LOS_MemAllocAlign | LOS_MemFree
 */
extern VOID *LOS_MemRealloc(VOID *pool, VOID *ptr, UINT32 size);

/**
 * @ingroup los_memory
 * @brief Allocate aligned memory.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to allocate memory blocks of specified size and of which the starting addresses are aligned on
 * a specified boundary.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The size of the input parameter size can not be greater than the memory pool size that specified at the second
 * input parameter of LOS_MemInit.</li>
 * <li>The alignment parameter value must be a power of 2 with the minimum value being 4.</li>
 * </ul>
 *
 * @param  pool      [IN] Pointer to the memory pool that contains the memory blocks to be allocated.
 * @param  size      [IN] Size of the memory to be allocated.
 * @param  boundary  [IN] Boundary on which the memory is aligned.
 *
 * @retval #NULL    The memory fails to be allocated.
 * @retval #VOID*   The memory is successfully allocated with the starting address of the allocated memory returned.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemAlloc | LOS_MemRealloc | LOS_MemFree
 */
extern VOID *LOS_MemAllocAlign(VOID *pool, UINT32 size, UINT32 boundary);

/**
 * @ingroup los_memory
 * @brief Get the size of memory pool's size.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the size of memory pool' total size.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * </ul>
 *
 * @param  pool           [IN] A pointer pointed to the memory pool.
 *
 * @retval #LOS_NOK        The incoming parameter pool is NULL.
 * @retval #UINT32         The size of the memory pool.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_MemPoolSizeGet(const VOID *pool);

/**
 * @ingroup los_memory
 * @brief Get the size of memory totally used.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the size of memory totally used in memory pool.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * </ul>
 *
 * @param  pool           [IN] A pointer pointed to the memory pool.
 *
 * @retval #LOS_NOK        The incoming parameter pool is NULL.
 * @retval #UINT32         The size of the memory pool used.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_MemTotalUsedGet(VOID *pool);

/**
 * @ingroup los_memory
 * @brief Get the information of memory pool.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the information of memory pool.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * </ul>
 *
 * @param  pool                 [IN] A pointer pointed to the memory pool.
 * @param  poolStatus           [IN] A pointer for storage the pool status
 *
 * @retval #LOS_NOK           The incoming parameter pool is NULL or invalid.
 * @retval #LOS_OK            Success to get memory information.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_MemInfoGet(VOID *pool, LOS_MEM_POOL_STATUS *poolStatus);

/**
 * @ingroup los_memory
 * @brief Get the number of free node in every size.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the number of free node in every size.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * </ul>
 *
 * @param  pool               [IN] A pointer pointed to the memory pool.
 *
 * @retval #LOS_NOK           The incoming parameter pool is NULL.
 * @retval #UINT32            The address of the last used node that casts to UINT32.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_MemFreeNodeShow(VOID *pool);

/**
 * @ingroup los_memory
 * @brief Check the memory pool integrity.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to check the memory pool integrity.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>LOS_MemIntegrityCheck will be called by malloc function when the macro of LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK
 * is defined in LiteOS.</li>
 * <li>LOS_MemIntegrityCheck function can be called by user anytime.</li>
 * </ul>
 *
 * @param  pool              [IN] A pointer pointed to the memory pool.
 *
 * @retval #LOS_NOK           The memory pool (pool) is impaired.
 * @retval #LOS_OK            The memory pool (pool) is integrated.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_MemIntegrityCheck(const VOID *pool);

/**
 * @ingroup los_memory
 * @brief Enable memory pool to support no internal lock during using interfaces.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to enable memory pool to support no internal lock during using interfaces,</li>
 * <li>such as LOS_MemAlloc/LOS_MemAllocAlign/LOS_MemRealloc/LOS_MemFree and so on.
 * </ul>
 * @attention
 * <ul>
 * <li>The memory pool does not support multi-threaded concurrent application scenarios.
 * <li>If you want to use this function, you need to call this interface before the memory
 * pool is used, it cannot be called during the trial period.</li>
 * </ul>
 *
 * @param pool         [IN] Starting address of memory.
 *
 * @retval node.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */

/* Supposing a Second Level Index: SLI = 3. */
#define OS_MEM_SLI                      3
/* Giving 1 free list for each small bucket: 4, 8, 12, up to 124. */
#define OS_MEM_SMALL_BUCKET_COUNT       31
#define OS_MEM_SMALL_BUCKET_MAX_SIZE    128
/* Giving 2^OS_MEM_SLI free lists for each large bucket. */
#define OS_MEM_LARGE_BUCKET_COUNT       24
/* OS_MEM_SMALL_BUCKET_MAX_SIZE to the power of 2 is 7. */
#define OS_MEM_LARGE_START_BUCKET       7

/* The count of free list. */
#define OS_MEM_FREE_LIST_COUNT  (OS_MEM_SMALL_BUCKET_COUNT + (OS_MEM_LARGE_BUCKET_COUNT << OS_MEM_SLI))
/* The bitmap is used to indicate whether the free list is empty, 1: not empty, 0: empty. */
#define OS_MEM_BITMAP_WORDS     ((OS_MEM_FREE_LIST_COUNT >> 5) + 1)

struct OsMemNodeHead {
#if (LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK == 1)
    UINT32 magic;
#endif
#if (LOSCFG_MEM_LEAKCHECK == 1)
    UINTPTR linkReg[LOSCFG_MEM_RECORD_LR_CNT];
#endif
    union {
        struct OsMemNodeHead *prev; /* The prev is used for current node points to the previous node */
        struct OsMemNodeHead *next; /* The next is used for sentinel node points to the expand node */
    } ptr;
#if (LOSCFG_TASK_MEM_USED == 1)
    UINT32 taskId;
    UINT32 sizeAndFlag;
#elif (LOSCFG_MEM_FREE_BY_TASKID == 1)
    UINT32 taskId : 6;
    UINT32 sizeAndFlag : 26;
#else
    UINT32 sizeAndFlag;
#endif
};

struct OsMemFreeNodeHead {
    struct OsMemNodeHead header;
    struct OsMemFreeNodeHead *prev;
    struct OsMemFreeNodeHead *next;
};

struct OsMemPoolInfo {
    VOID *pool;
    UINT32 totalSize;
    UINT32 attr;
#if (LOSCFG_MEM_WATERLINE == 1)
    UINT32 waterLine;   /* Maximum usage size in a memory pool */
    UINT32 curUsedSize; /* Current usage size in a memory pool */
#endif
#if (LOSCFG_MEM_MUL_REGIONS == 1)
    UINT32 totalGapSize;
#endif
};

struct OsMemPoolHead {
    struct OsMemPoolInfo info;
    UINT32 freeListBitmap[OS_MEM_BITMAP_WORDS];
    struct OsMemFreeNodeHead *freeList[OS_MEM_FREE_LIST_COUNT];
#if (LOSCFG_MEM_MUL_POOL == 1)
    VOID *nextPool;
#endif
};

extern VOID LOS_MemUnlockEnable(VOID *pool);

#if (LOSCFG_MEM_MUL_MODULE == 1)
/**
 * @ingroup los_memory
 * The memory usage statistics depend on module, the default max module number 32.
 * Note that this macro is defined only when LOSCFG_MEM_MUL_MODULE is defined.
 */
#define MEM_MODULE_MAX LOSCFG_MEM_MODULE_MAX

/**
 * @ingroup los_memory
 * @brief Allocate dynamic memory.
 *
 * @par Description:
 * This API is used to allocate a memory block of which the size is specified and update module mem used.
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The size of the input parameter size can not be greater than the memory pool size that specified at the second
 * input parameter of LOS_MemInit.</li>
 * <li>The size of the input parameter size must be four byte-aligned.</li>
 * <li>This function is defined only when LOSCFG_MEM_MUL_MODULE is defined.</li>
 * </ul>
 *
 * @param  pool     [IN] Pointer to the memory pool that contains the memory block to be allocated.
 * @param  size     [IN] Size of the memory block to be allocated (unit: byte).
 * @param  moduleId [IN] module ID (0~MODULE_MAX).
 *
 * @retval #NULL       The memory fails to be allocated.
 * @retval #VOID*      The memory is successfully allocated, and the API returns the pointer to
 *                     the allocated memory block.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemMrealloc | LOS_MemMallocAlign | LOS_MemMfree
 */
extern VOID *LOS_MemMalloc(VOID *pool, UINT32 size, UINT32 moduleId);

/**
 * @ingroup los_memory
 * @brief Allocate aligned memory.
 *
 * @par Description:
 * This API is used to allocate memory blocks of specified size and of which the starting addresses are aligned on
 * a specified boundary and update module mem used.
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The size of the input parameter size can not be greater than the memory pool size that specified at the second
 * input parameter of LOS_MemInit.</li>
 * <li>The alignment parameter value must be a power of 2 with the minimum value being 4.</li>
 * <li>This function is defined only when LOSCFG_MEM_MUL_MODULE is defined.</li>
 * </ul>
 *
 * @param  pool      [IN] Pointer to the memory pool that contains the memory blocks to be allocated.
 * @param  size      [IN] Size of the memory to be allocated.
 * @param  boundary  [IN] Boundary on which the memory is aligned.
 * @param  moduleId  [IN] module ID (0~MODULE_MAX).
 *
 * @retval #NULL          The memory fails to be allocated.
 * @retval #VOID*         The memory is successfully allocated, and the API returns the pointer to the allocated memory.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemMalloc | LOS_MemRealloc | LOS_MemMfree
 */
extern VOID *LOS_MemMallocAlign(VOID *pool, UINT32 size, UINT32 boundary, UINT32 moduleId);

/**
 * @ingroup los_memory
 * @brief Free dynamic memory.
 *
 * @par Description:
 * This API is used to free specified dynamic memory that has been allocated and update module mem used.
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The input ptr parameter must be allocated by LOS_MemMalloc or LOS_MemMallocAlign or LOS_MemMrealloc.</li>
 * <li>This function is defined only when LOSCFG_MEM_MUL_MODULE is defined.</li>
 * </ul>
 *
 * @param  pool     [IN] Pointer to the memory pool that contains the dynamic memory block to be freed.
 * @param  ptr      [IN] Starting address of the memory block to be freed.
 * @param  moduleId [IN] module ID (0~MODULE_MAX).
 *
 * @retval #LOS_NOK          The memory block fails to be freed because the starting address of the memory block is
 * invalid, or the memory overwriting occurs.
 * @retval #LOS_OK           The memory block is freed successfully.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemMalloc | LOS_MemMrealloc | LOS_MemMallocAlign
 */
extern UINT32 LOS_MemMfree(VOID *pool, VOID *ptr, UINT32 moduleId);

/**
 * @ingroup los_memory
 * @brief Re-allocate a memory block.
 *
 * @par Description:
 * This API is used to allocate a new memory block of which the size is specified by size if the original memory
 * block size is insufficient. The new memory block will copy the data in the original memory block of which the
 * address is specified by ptr. The size of the new memory block determines the maximum size of data to be copied.
 * After the new memory block is created, the original one is freed. And update module mem used.
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The input ptr parameter must be allocated by LOS_MemMalloc or LOS_MemMallocAlign.</li>
 * <li>The size of the input parameter size can not be greater than the memory pool size that specified at the second
 * input parameter of LOS_MemInit.</li>
 * <li>The size of the input parameter size must be aligned as follows: 1) if the ptr is allocated by LOS_MemAlloc,
 * it must be four byte-aligned; 2) if the ptr is allocated by LOS_MemMallocAlign, it must be aligned with the size of
 * the input parameter boundary of LOS_MemMallocAlign.</li>
 * <li>This function is defined only when LOSCFG_MEM_MUL_MODULE is defined.</li>
 * </ul>
 *
 * @param  pool      [IN] Pointer to the memory pool that contains the original and new memory blocks.
 * @param  ptr       [IN] Address of the original memory block.
 * @param  size      [IN] Size of the new memory block.
 * @param  moduleId  [IN] module ID (0~MODULE_MAX).
 *
 * @retval #NULL          The memory fails to be re-allocated.
 * @retval #VOID*         The memory is successfully re-allocated, and the API returns the pointer to
 *                        the new memory block.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemMalloc | LOS_MemMallocAlign | LOS_MemMfree
 */
extern VOID *LOS_MemMrealloc(VOID *pool, VOID *ptr, UINT32 size, UINT32 moduleId);

/**
 * @ingroup los_memory
 * @brief Get the used memory size of the specified module.
 *
 * @par Description:
 * This API is used to get the specified module's memory consume size.
 * @attention This function is defined only when LOSCFG_MEM_MUL_MODULE is defined.
 *
 * @param  moduleId   [IN] module ID (0~MODULE_MAX).
 *
 * @retval #UINT32         The size of the specified module's consumed memory.
 * @retval #OS_NULL_INT    The input module id is illegal.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINT32 LOS_MemMusedGet(UINT32 moduleId);
#endif /* LOSCFG_MEM_MUL_MODULE == 1 */


/**
 * @ingroup los_memory
 * @brief Initialize dynamic memory.
 *
 * @par Description:
 * This API is used to initialize the dynamic memory of a doubly linked list.
 * @attention
 * <ul>
 * <li>The size parameter value should match the following two conditions :
 * 1) Be less than or equal to the Memory pool size;
 * 2) Be greater than the size of OS_MEM_MIN_POOL_SIZE.</li>
 * <li>Call this API when dynamic memory needs to be initialized during the startup of Huawei LiteOS.</li>
 * <li>The parameter input must be OS_MEM_ALIGN_SIZE byte-aligned.</li>
 * <li>The init area [pool, pool + size] should not conflict with other pools.</li>
 * </ul>
 *
 * @param pool         [IN] Starting address of memory.
 * @param size         [IN] Memory size.
 * @param attr         [IN] Attributes to config memory pool, see LOS_MEM_INIT_ATTR_SLAB_MSK above.
 *
 * @retval #LOS_NOK    The dynamic memory fails to be initialized.
 * @retval #LOS_OK     The dynamic memory is successfully initialized.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 */
extern UINT32 LOS_MemPoolInit(VOID *pool, UINT32 size, UINT32 attr);

/**
 * @ingroup los_memory
 * @brief Get the number of free memory nodes.
 *
 * @par Description:
 * This API is used to get the number of free memory nodes in memory pool.
 * @attention
 * The input pool parameter must be initialized via func LOS_MemInit.
 *
 * @param  pool           [IN] A pointer pointed to the memory pool.
 *
 * @retval #LOS_NOK        The input parameter pool is NULL.
 * @retval #UINT32         The number of free memory nodes.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINT32 LOS_MemFreeBlksGet(VOID *pool);

/**
 * @ingroup los_memory
 * @brief Get the number of used memory nodes.
 *
 * @par Description:
 * This API is used to get the number of used memory nodes in memory pool.
 * @attention
 * The input pool parameter must be initialized via func LOS_MemInit.
 *
 * @param  pool           [IN] A pointer pointed to the memory pool.
 *
 * @retval #OS_INVALID     The input parameter pool is NULL.
 * @retval #UINT32         The number of used memory nodes.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINT32 LOS_MemUsedBlksGet(VOID *pool);

/**
 * @ingroup los_memory
 * @brief Get the task ID of a used memory node.
 *
 * @par Description:
 * This API is used to get the task ID of a used memory node.
 * @attention
 * <ul>
 * <li>The input ptr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 * <li>This interface only support obtain the task ID of a used memory node which is allocated from the system memory
 * pool (OS_SYS_MEM_ADDR) at present.</li>
 * <li>This API can be enabled when you set the macro value of LOSCFG_MEM_DEBUG or LOSCFG_MEM_TASK_STAT is defined
 * in LiteOS.</li>
 * </ul>
 *
 * @param  ptr               [IN] A used memory node.
 *
 * @retval #OS_INVALID        The input parameter ptr is illegal.
 * @retval #UINT32            The task ID of used memory node ptr.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINT32 LOS_MemTaskIdGet(const VOID *ptr);

/**
 * @ingroup los_memory
 * @brief Get the address of the next byte of the last used byte.
 *
 * @par Description:
 * This API is used to get the address of the next byte of the last used byte.
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The last used byte may the last byte of an used node or the last byte of an unused node head</li>
 * </ul>
 *
 * @param  pool               [IN] A pointer pointed to the memory pool.
 *
 * @retval #LOS_NOK           The input parameter pool is NULL.
 * @retval #UINTPTR           The pointer to the next byte of the last used byte.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINTPTR LOS_MemLastUsedGet(VOID *pool);

#if (LOSCFG_BASE_MEM_NODE_SIZE_CHECK == 1)
/**
 * @ingroup los_memory
 * Define a mem size check intensity.
 *
 * Lowest mem check.
 */
#define LOS_MEM_CHECK_LEVEL_LOW 0

/**
 * @ingroup los_memory
 * Define a mem size check intensity.
 *
 * Highest mem check.
 */
#define LOS_MEM_CHECK_LEVEL_HIGH 1

/**
 * @ingroup los_memory
 * Define a mem size check intensity.
 *
 * disable mem check.
 */
#define LOS_MEM_CHECK_LEVEL_DISABLE 0xff

/**
 * @ingroup los_memory
 * Define a mem size check intensity.
 *
 * default intensity set mem check.
 */
#define LOS_MEM_CHECK_LEVEL_DEFAULT LOS_MEM_CHECK_LEVEL_DISABLE

/**
 * @ingroup los_memory
 * memcheck error code: the pointer or pool is NULL.
 *
 * Value: 0x02000101.
 *
 * Solution: don't give a NULL parameter.
 */
#define LOS_ERRNO_MEMCHECK_PARA_NULL LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x1)

/**
 * @ingroup los_memory
 * memcheck error code: the pointer address is not in the suitable range.
 *
 * Value: 0x02000102.
 *
 * Solution: check pointer and confirm it is in stack.
 */
#define LOS_ERRNO_MEMCHECK_OUTSIDE LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x2)

/**
 * @ingroup los_memory
 * memcheck error code: can't find the control node.
 *
 * Value: 0x02000103.
 *
 * Solution: check if the node which the pointer points to has been freed or not been allocated.
 */
#define LOS_ERRNO_MEMCHECK_NO_HEAD LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x3)

/**
 * @ingroup los_memory
 * memcheck error code: the memcheck level is wrong.
 *
 * Value: 0x02000104.
 *
 * Solution: check the memcheck level by the function "LOS_MemCheckLevelGet".
 */
#define LOS_ERRNO_MEMCHECK_WRONG_LEVEL LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x4)

/**
 * @ingroup los_memory
 * memcheck error code: memcheck function is not enable.
 *
 * Value: 0x02000105.
 *
 * Solution: enable memcheck by the function "LOS_MemCheckLevelSet".
 */
#define LOS_ERRNO_MEMCHECK_DISABLED LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x5)

/**
 * @ingroup los_memory
 * @brief Check the size of the specified memory node.
 *
 * @par Description:
 * This API is used to check the size of memory node.
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The input ptr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 * <li>The function will be called by function specified, such as memset or memcpy.</li>
 * <li>The feature can be enabled when you set the macro value of LOSCFG_BASE_MEM_NODE_SIZE_CHECK as YES.</li>
 * <li>You had better set memory check level as LOS_MEM_CHECK_LEVEL_DISABLE when copy bin file.</li>
 * </ul>
 *
 * @param  pool            [IN]  A pointer pointed to the memory pool.
 * @param  ptr             [IN]  A pointer pointed to the source node.
 * @param  totalSize       [OUT] A pointer to save total size, must point to valid memory.
 * @param  availSize       [OUT] A pointer to save available size, must point to valid memory.
 *
 * @retval #LOS_ERRNO_MEMCHECK_DISABLED         Memcheck function does not open.
 * @retval #LOS_ERRNO_MEMCHECK_PARA_NULL        The pool or ptr is NULL.
 * @retval #LOS_ERRNO_MEMCHECK_OUTSIDE          The ptr address is not in the reasonable range.
 * @retval #LOS_ERRNO_MEMCHECK_NO_HEAD          Can't find the control head node from ptr.
 * @retval #LOS_ERRNO_MEMCHECK_WRONG_LEVEL      The memory check level is illegal.
 * @retval #LOS_OK                              Success to get total size and available size of the memory node (ptr).
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemCheckLevelSet | LOS_MemCheckLevelGet
 */
extern UINT32 LOS_MemNodeSizeCheck(VOID *pool, VOID *ptr, UINT32 *totalSize, UINT32 *availSize);

/**
 * @ingroup los_memory
 * @brief Set the memory check level.
 *
 * @par Description:
 * This API is used to set the memory check level.
 * @attention
 * <ul>
 * <li>There are three level you can set.</li>
 * <li>The legal level are LOS_MEM_CHECK_LEVEL_LOW, LOS_MEM_CHECK_LEVEL_HIGH, LOS_MEM_CHECK_LEVEL_DISABLE.</li>
 * </ul>
 *
 * @param  checkLevel                               [IN] The level what you want to set.
 *
 * @retval #LOS_ERRNO_MEMCHECK_WRONG_LEVEL          The input memory check level is illegal.
 * @retval #LOS_OK                                  Set the memory check level successfully.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemNodeSizeCheck | LOS_MemCheckLevelGet
 */
extern UINT32 LOS_MemCheckLevelSet(UINT8 checkLevel);

/**
 * @ingroup los_memory
 * @brief Get the memory check level.
 *
 * @par Description:
 * This API is used to get the current memory check level.
 * @attention None.
 *
 * @param  None.
 *
 * @retval #UINT8           The current memory check level.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemNodeSizeCheck | LOS_MemCheckLevelSet
 */
extern UINT8 LOS_MemCheckLevelGet(VOID);
#endif /* LOSCFG_BASE_MEM_NODE_SIZE_CHECK == 1 */

#ifdef LOSCFG_MEM_MUL_POOL_ALLOC
typedef VOID (*ALLOC_PROC_FUNC)(UINTPTR *, UINT32 *);
typedef VOID (*FREE_PROC_FUNC)(VOID *);
/**
 * @ingroup los_memory
 * @brief Rregistration hook is used to apply for and release memory. The memory is used to initialize the memory pool.
 *
 * @par Description:
 * This API is a registration hook used to apply for and release memory.
 * @attention None.
 *
 * @param  allocFunc            [IN]  A hook for applying for memory pool initialization required memory.
 * @param  freeFunc             [IN]  A hook for freeing memory after the memory pool is deinitialized.
 *
 * @retval #LOS_NOK      The multi-memory pool hook is registration unsuccessfully.
 * @retval #LOS_OK       The multi-memory pool hook is registration successfully.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINT32 LOS_MulPoolRegister(ALLOC_PROC_FUNC allocFunc, FREE_PROC_FUNC freeFunc);

/**
 * @ingroup los_memory
 * @brief Alloc for memory in multiple memory pools.
 *
 * @par Description:
 * This API is used to alloc for memory in multiple memory pools.
 * @attention None.
 *
 * @param  size                 [IN] Size of the memory to be allocated.
 *
 * @retval #NULL                The memory fails to be allocated.
 * @retval #VOID*               The memory is successfully allocated, and the API returns the pointer to
 *                              the allocated memory block.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern VOID *LOS_MulPoolAlloc(UINT32 size);

/**
 * @ingroup los_memory
 * @brief Realloc for memory in multiple memory pools.
 *
 * @par Description:
 * This API is used to realloc for memory in multiple memory pools.
 * @attention None.
 *
 * @param  ptr       [IN] Address of the original memory block.
 * @param  size      [IN] Size of the memory to be allocated.
 *
 * @retval #NULL     The memory fails to be re-allocated.
 * @retval #VOID*    The memory is successfully re-allocated, and the API returns the pointer to the new memory block.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern VOID *LOS_MulPoolRealloc(VOID *ptr, UINT32 size);

/**
 * @ingroup los_memory
 * @brief Free for memory in multiple memory pools.
 *
 * @par Description:
 * This API is used to free for memory in multiple memory pools.
 * @attention None.
 *
 * @param  ptr      [IN] Starting address of the memory block to be freed.
 *
 * @retval #LOS_NOK      The memory block fails to be freed because the starting address of the memory block is
 *                       invalid, or the memory overwriting occurs.
 * @retval #LOS_OK       The memory block is successfully freed.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINT32 LOS_MulPoolFree(VOID *ptr);

/**
 * @ingroup los_memory
 * @brief Reduce unused memory pools.
 *
 * @par Description:
 * This API is used to reduce unused memory pools.
 * @attention None.
 *
 * @param  None
 *
 * @retval #LOS_NOK      The empty memory pool is released failed, failure due to an interrupt or
                         empty release hook.
 * @retval #LOS_OK       The empty memory pool is released successfully.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINT32 LOS_MulPoolShrink(VOID);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_MEMORY_H */
