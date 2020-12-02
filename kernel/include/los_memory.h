/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_MEMORY_H
#define _LOS_MEMORY_H

#include "los_config.h"
#include "los_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define IS_ALIGNED(value)                       ((((UINT32)(value)) & ((UINT32)((value) - 1))) == 0)
#define OS_MEM_ALIGN(value, align)              (((UINT32)(UINTPTR)(value) + (UINT32)((align) - 1)) & \
                                                (~(UINT32)((align) - 1)))
#define OS_MEM_ALIGN_FLAG                       0x80000000
#define OS_MEM_SET_ALIGN_FLAG(align)            ((align) = ((align) | OS_MEM_ALIGN_FLAG))
#define OS_MEM_GET_ALIGN_FLAG(align)            ((align) & OS_MEM_ALIGN_FLAG)
#define OS_MEM_GET_ALIGN_GAPSIZE(align)         ((align) & (~OS_MEM_ALIGN_FLAG))

/**
 * @ingroup los_memory
 * Memory pool information structure
 */
typedef struct {
    VOID *pPoolAddr;            /**< Starting address of a memory pool */
    UINT32 uwPoolSize;          /**< Memory pool size */
#if defined(OS_MEM_WATERLINE) && (OS_MEM_WATERLINE == YES)
    UINT32 uwPoolWaterLine;     /**< Maximum usage size in a memory pool */
    UINT32 uwPoolCurUsedSize;   /**< Current usage size in a memory pool */
#endif
//#ifdef LOSCFG_MEM_MUL_POOL
    VOID *pNextPool;
//#endif
} LOS_MEM_POOL_INFO;




typedef struct __s_LOS_MEM_STATUS {
    UINT32 totalSize;
    UINT32 usedSize;
    UINT32 freeSize;
    UINT32 allocCount;
    UINT32 freeCount;
} LOS_MEM_STATUS;


#define MEM_INFO_SIZE                   ((sizeof(MEM_INFO) * OS_SYS_MEM_NUM) + 4)
extern UINT8 g_memMang[];

enum _MEM_MANG_TYPE {
    MEM_MANG_MEMBOX,
    MEM_MANG_MEMORY,
    MEM_MANG_EMPTY,
};

enum _MEM_MANG_SOUCE {
    MEM_MANG_UNUSED,
    MEM_MANG_INIT,
    MEM_MANG_INT,
    MEM_MANG_TASK,
};

typedef struct _MEM_INFO {
    UINT32 uwType;
    UINT32 uwStartAddr;
    UINT32 uwSize;
    VOID * blkAddrArray;
}MEM_INFO;

typedef struct _SLAB_INFO {
    UINT32 item_sz;
    UINT32 item_cnt;
    UINT32 cur_usage;
}SLAB_INFO;

#define SLAB_CLASS_NUM                  (4U)
typedef struct _MEM_INFO_S {
    UINT32 uwType;
    UINT32 uwStartAddr;
    UINT32 uwSize;
    UINT32 uwFree;
    UINT32 uwBlockSize;
    UINT32 uwErrorAddr;
    UINT32 uwErrorLen;
    UINT32 uwErrorOwner;
    SLAB_INFO stSlabInfo[SLAB_CLASS_NUM];
}MEM_INFO_S;

/**
 * @ingroup los_memboxcheck
 * @brief Get the information of the exc memory.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the information of the exc memory.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param memNum       [IN]     Type #UINT32  Memory pool number.
 * @param memExcInfo   [IN/OUT] Type #MEM_INFO_S *  information of the exc memory.
 *
 * @retval UINT32 Get information result.
 * @par Dependency:
 * <ul>
 * <li>los_memboxcheck.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
UINT32 LOS_MemExcInfoGet(UINT32 memNum, MEM_INFO_S *memExcInfo);

//#if (LOSCFG_MEMORY_BESTFIT == YES)


#if (LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK == YES)
extern UINT8 g_memMang[];
#endif
#if (LOSCFG_PLATFORM_EXC == YES)
#define OS_MEM_ENABLE_ALLOC_CHECK
#endif
#if (LOSCFG_BASE_MEM_NODE_SIZE_CHECK == YES)
#define OS_MEM_CHECK_DEBUG
#endif

#ifdef LOSCFG_MEM_MUL_POOL
extern VOID *g_memPoolHead;
#endif

typedef VOID (*MALLOC_HOOK)(VOID);

extern MALLOC_HOOK g_mallocHook;

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
 * @brief Get the number of free memory nodes.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the number of free memory nodes in memory pool.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * </ul>
 *
 * @param  pool           [IN] A pointer pointed to the memory pool.
 *
 * @retval #LOS_NOK        The incoming parameter pool is NULL.
 * @retval #UINT32         The number of free memory nodes.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_MemFreeBlksGet(VOID *pool);

/**
 * @ingroup los_memory
 * @brief Get the number of used memory nodes.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the number of used memory nodes in memory pool.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * </ul>
 *
 * @param  pool           [IN] A pointer pointed to the memory pool.
 *
 * @retval #LOS_NOK        The incoming parameter pool is NULL.
 * @retval #UINT32         The number of used memory nodes.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_MemUsedBlksGet(VOID *pool);

/**
 * @ingroup los_memory
 * @brief Get the task ID of a used memory node.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the task ID of a used memory node.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input ptr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 * <li>This interface only support obtain the task ID of a used memory node which is allocated from
 * the system memory pool (OS_SYS_MEM_ADDR) at present.</li>
 * </ul>
 *
 * @param  pool               [IN] A used memory node.
 *
 * @retval #OS_INVALID        The incoming parameter ptr is illegal.
 * @retval #UINT32            The task ID of used memory node ptr.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_MemTaskIdGet(const VOID *pool);

/**
 * @ingroup los_memory
 * @brief Get the address of last node.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the address of last node.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The last node of memory pool is not the end node.</li>
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
extern UINT32 LOS_MemLastUsedGet(VOID *pool);

/**
 * @ingroup los_memory
 * @brief Check the memory pool Integrity.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to check the memory pool Integrity.</li>
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
extern UINT32 LOS_MemIntegrityCheck(VOID *pool);

/**
 * @ingroup los_memory
 *  Define a mem size check intensity
 *
 *  Lowest mem check.
 */
#define LOS_MEM_CHECK_LEVEL_LOW     0

/**
 * @ingroup los_memory
 * Define a mem size check intensity
 *
 * Highest mem check.
 */
#define LOS_MEM_CHECK_LEVEL_HIGH    1

/**
 * @ingroup los_memory
 * Define a mem size check intensity
 *
 * disable mem check.
 */
#define LOS_MEM_CHECK_LEVEL_DISABLE 0xff

/**
 * @ingroup los_memory
 * Define a mem size check intensity
 *
 * default intensity set mem check.
 */
#define LOS_MEM_CHECK_LEVEL_DEFAULT LOS_MEM_CHECK_LEVEL_DISABLE

/**
 * @ingroup los_memory
 * @brief Check the size of memory node specified.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to check the size of memory node.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The input ptr parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign.</li>
 * <li>The function will be called by function specified, such as memset or memcpy.</li>
 * <li>The feature can be enabled when you set the macro value of LOSCFG_BASE_MEM_NODE_SIZE_CHECK as YES.</li>
 * <li>You had better set memory check level as LOS_MEM_CHECK_LEVEL_DISABLE when copy bin file.</li>
 * </ul>
 *
 * @param  pool              [IN]  A pointer pointed to the memory pool.
 * @param  ptr               [IN]  A pointer pointed to the source node.
 * @param  totalSize         [OUT] A pointer to save total size, must point to valid memory.
 * @param  availSize         [OUT] A pointer to save available size, must point to valid memory.
 *
 * @retval #OS_ERRNO_MEMCHECK_DISABLED           Memcheck function does not open.
 * @retval #OS_ERRNO_MEMCHECK_NOT_INIT           Memcheck function does not init.
 * @retval #OS_ERRNO_MEMCHECK_PARA_NULL          The pool or ptr is NULL.
 * @retval #OS_ERRNO_MEMCHECK_OUTSIDE            The ptr address is not in the reasonable range.
 * @retval #OS_ERRNO_MEMCHECK_NO_HEAD            Can't find the control head node from ptr.
 * @retval #OS_ERRNO_MEMCHECK_WRONG_LEVEL        The memory check level is illegal.
 * @retval #LOS_OK                               Success to get total size and available
 *                                               size of the memory node (ptr).
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
 * <ul>
 * <li>This API is used to set the memory check level.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>There are three level you can set.</li>
 * <li>The legal level are LOS_MEM_CHECK_LEVEL_LOW, LOS_MEM_CHECK_LEVEL_HIGH, LOS_MEM_CHECK_LEVEL_DISABLE.</li>
 * </ul>
 *
 * @param  level                                  [IN] The level what you want to set.
 *
 * @retval #LOS_ERRNO_MEMCHECK_WRONG_LEVEL           The memory check level what you want to set is illegal.
 * @retval #LOS_OK                                  Success to set the memory check level.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemNodeSizeCheck | LOS_MemCheckLevelGet
 */
extern UINT32 LOS_MemCheckLevelSet(UINT8 level);

/**
 * @ingroup los_memory
 * @brief Get the memory check level.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the current memory check level.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  None
 *
 * @retval #UINT8           The current memory check level.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemNodeSizeCheck | LOS_MemCheckLevelSet
 */
extern UINT8 LOS_MemCheckLevelGet(VOID);
/**
 * @ingroup los_memory
 * @brief Get the memory pool information.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to get the current memory pool used information.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  pool        [IN] A pointer pointed to the memory pool.
 * @param  status      [IN] A pointer for storage the pool status.
 *
 * @retval #LOS_OK                                  Success to get the memory pool information.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 */
extern UINT32 LOS_MemInfoGet(VOID *pool, LOS_MEM_STATUS *status);

/**
 * @ingroup los_memory
 * @brief calculate heap information.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to calculate heap information.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>One parameter of this interface is a pointer, it should be a correct value, otherwise,
 * the system may be abnormal.</li>
 * </ul>
 *
 * @param  pool          [IN]  Pointer to the memory pool that contains the memory block to be allocated.
 * @param  status        [OUT] Type  #LOS_MEM_STATUS* Pointer to the heap status structure to be obtained.
 *
 * @retval #LOS_OK        The heap status calculate success.
 * @retval #LOS_NOK       The heap status calculate with some error.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemAlloc | LOS_MemRealloc | LOS_MemFree
 */
extern UINT32 LOS_MemStatisticsGet(VOID *pool, LOS_MEM_STATUS *status);

/**
 * @ingroup los_memory
 * @brief calculate heap max free block size.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to calculate heap max free block size.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  pool          [IN] Pointer to memory pool.
 *
 * @retval #UINT32        The  max free block size.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemAlloc | LOS_MemRealloc | LOS_MemFree
 */
extern UINT32 LOS_MemGetMaxFreeBlkSize(VOID *pool);
//#endif

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
 * <li>The size parameter value should match the following two conditions : 1) Be less than or equal to
 * the Memory pool size; 2) Be greater than the size of OS_MEM_MIN_POOL_SIZE.</li>
 * <li>Call this API when dynamic memory needs to be initialized during the startup of Huawei LiteOS.</li>
 * <li>The parameter input must be four byte-aligned.</li>
 * <li>The init area [pool, pool + size] should not conflict with other pools.</li>
 * </ul>
 *
 * @param pool         [IN] Starting address of memory.
 * @param size         [IN] Memory size.
 *
 * @retval #LOS_NOK    The dynamic memory fails to be initialized.
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
 * <li>The size of the input parameter size can not be greater than the memory pool size
 * that specified at the second input parameter of LOS_MemInit.</li>
 * <li>The size of the input parameter size must be four byte-aligned.</li>
 * </ul>
 *
 * @param  pool    [IN] Pointer to the memory pool that contains the memory block to be allocated.
 * @param  size    [IN] Size of the memory block to be allocated (unit: byte).
 *
 * @retval #NULL          The memory fails to be allocated.
 * @retval #VOID*         The memory is successfully allocated with the starting address of
 *                        the allocated memory block returned.
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
 * <li>The input mem parameter must be allocated by LOS_MemAlloc or LOS_MemAllocAlign or LOS_MemRealloc.</li>
 * </ul>
 *
 * @param  pool  [IN] Pointer to the memory pool that contains the dynamic memory block to be freed.
 * @param  mem   [IN] Starting address of the memory block to be freed.
 *
 * @retval #LOS_NOK          The memory block fails to be freed
 * @retval #LOS_OK           The memory block is successfully freed.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemAlloc | LOS_MemRealloc | LOS_MemAllocAlign
 */
extern UINT32 LOS_MemFree(VOID *pool, VOID *mem);

/**
 * @ingroup los_memory
 * @brief Re-allocate a memory block.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to allocate a new memory block of which the size is specified by size if the original
 * memory block size is insufficient. The new memory block will copy the data in the original memory block of
 * which the address is specified by ptr. The size of the new memory block determines the maximum size of data
 * to be copied. After the new memory block is created, the original one is freed.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>The input pool parameter must be initialized via func LOS_MemInit.</li>
 * <li>The input ptr parameter must be allocated by LOS_MemAlloc.</li>
 * <li>The size of the input parameter size can not be greater than the memory pool size that specified at the
 * second input parameter of LOS_MemInit.</li>
 * <li>The size of the input parameter size must be aligned as follows: 1) if the ptr is allocated by LOS_MemAlloc,
 * it must be four byte-aligned; 2) if the ptr is allocated by LOS_MemAllocAlign, it must be aligned with the size
 * of the input parameter uwBoundary of LOS_MemAllocAlign.</li>
 * </ul>
 *
 * @param  pool      [IN] Pointer to the memory pool that contains the original and new memory blocks.
 * @param  ptr       [IN] Address of the original memory block.
 * @param  size      [IN] Size of the new memory block.
 *
 * @retval #NULL          The memory fails to be re-allocated.
 * @retval #VOID*         The memory is successfully re-allocated with the starting address of the new memory block
 *                        returned.
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
 * <li>This API is used to allocate memory blocks of specified size and of which the starting addresses are aligned
 * on a specified boundary.</li>
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
 * @retval #NULL          The memory fails to be allocated.
 * @retval #VOID*         The memory is successfully allocated with the starting address of the allocated memory
 *                        returned.
 * @par Dependency:
 * <ul><li>los_memory.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_MemAlloc | LOS_MemRealloc | LOS_MemFree
 */
extern VOID *LOS_MemAllocAlign(VOID *pool, UINT32 size, UINT32 boundary);

#if (LOSCFG_MEM_MUL_POOL == YES)
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
 * @retval #LOS_NOK    The dynamic memory fails to be deinitialized.
 * @retval #LOS_OK     The dynamic memory is successfully deinitialized.
 * @par Dependency:
 * <ul>
 * <li>los_memory.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
extern UINT32 LOS_MemDeInit(const VOID *pool);

/**
 * @ingroup los_memory
 * @brief Print infomation about all pools.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to print infomation about all pools.</li>
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

#if (LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK == YES)
/**
 * @ingroup los_memboxcheck
 * @brief Update the information of the memory.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to update the information of the memory.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param pool       [IN/OUT]  Type #VOID  *  Memory pool number.
 * @param size       [IN] Type #UINT32   Memory size.
 * @param type       [IN] Type #UINT32   Memory mang type.
 *
 * @retval UINT32 Updateinformation result.
 * @par Dependency:
 * <ul>
 * <li>los_memboxcheck_pri.h: the header file that contains the API declaration.</li>
 * </ul>
 * @see None.
 */
UINT32 OsMemInfoUpdate(VOID *pool, UINT32 size, UINT32 type);
#endif

#define OS_MEM_ENABLE_MEM_STATISTICS

/*
 * memcheck error code: the stack have not inited
 * Value: 0x02000100
 * Solution: do memcheck must after stack mem init
 */
#define  OS_ERRNO_MEMCHECK_NOT_INIT      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x0)

/*
 *  memcheck error code: the pPtr is NULL
 *  Value: 0x02000101
 *  Solution: don't give a NULL parameter
 */
#define  OS_ERRNO_MEMCHECK_PARA_NULL      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x1)

/*
 *  memcheck error code: the pPtr addr not in the suit range
 *  Value: 0x02000102
 *  Solution: check pPtr and comfirm it included by stack
 */
#define  OS_ERRNO_MEMCHECK_OUTSIDE      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x2)

/*
 *  memcheck error code: can't find the ctrl node
 *  Value: 0x02000103
 *  Solution: confirm the pPtr if this node has been freed or has not been alloced
 */
#define  OS_ERRNO_MEMCHECK_NO_HEAD      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x3)

/*
 *  memcheck error code: the para level is wrong
 *  Value: 0x02000104
 *  Solution: checkout the memcheck level by the func "OS_GetMemCheck_Level"
 */
#define  OS_ERRNO_MEMCHECK_WRONG_LEVEL      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x4)

/*
 *  memcheck error code: memcheck func not open
 *  Value: 0x02000105
 *  Solution: enable memcheck by the func "OS_SetMemCheck_Level"
 */
#define  OS_ERRNO_MEMCHECK_DISABLED      LOS_ERRNO_OS_ERROR(LOS_MOD_MEM, 0x5)

/**
 * @ingroup los_memory
 * @brief Initialization the memory system.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to initialization the memory system.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  None.
 *
 * @retval UINT32 Initialization result.
 * @par Dependency:
 * <ul><li>los_memory_pri.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 OsMemSystemInit(VOID);

/**
 * @ingroup los_memory
 * Memory linked list node structure
 */
typedef struct tagLosMemDynNode {
    LOS_DL_LIST freeNodeInfo;         /**< Free memory node  */
    struct tagLosMemDynNode *preNode; /**< Pointer to the previous memory node */
    UINT32 sizeAndFlag;               /**< Size and flag of the current node(the highest bit
                                           represents a flag, and the rest bits specify the size) */
} LosMemDynNode;

#define OS_MEM_TASKID_SET(node, ID) \
    do { \
        UINTPTR tmp = (UINTPTR)(((LosMemDynNode *)(node))->freeNodeInfo.pstNext); \
        tmp = tmp & 0xffff0000; \
        tmp |= (ID); \
        ((LosMemDynNode *)(node))->freeNodeInfo.pstNext = (LOS_DL_LIST *)(tmp); \
    } while (0)

#define OS_MEM_TASKID_GET(node) ((UINT32)(UINTPTR)(((LosMemDynNode *)(node))->freeNodeInfo.pstNext) & 0xffff)

#define OS_MEM_MODID_SET(node, ID) \
    do { \
        UINTPTR tmp = (UINTPTR)(((LosMemDynNode *)(node))->freeNodeInfo.pstNext); \
        tmp = tmp & 0xffff; \
        tmp |= (ID) << 16; \
        ((LosMemDynNode *)(node))->freeNodeInfo.pstNext = (LOS_DL_LIST *)(tmp); \
    } while (0)

#define OS_MEM_MODID_GET(node) ((UINT32)(((LosMemDynNode *)(node))->freeNodeInfo.pstNext) >> 16)

#define OS_MEM_NODE_HEAD_SIZE    sizeof(LosMemDynNode)
#define OS_MEM_MIN_POOL_SIZE     (OS_DLNK_HEAD_SIZE + (2 * OS_MEM_NODE_HEAD_SIZE) + sizeof(LOS_MEM_POOL_INFO))
#define OS_MEM_ALIGN_SIZE        4
#define OS_MEM_NODE_USED_FLAG    0x80000000
#define OS_MEM_NODE_ALIGNED_FLAG 0x40000000
#define OS_MEM_NODE_ALIGN_SIZE   64
#define OS_MEM_NODE_NUM          2
#define OS_MEM_NODE_DATA_SIZE    4
#define OS_MEM_NODE_COUNT_NUM    8
#define OS_MULTI_DLNK_SIZE       (OS_MAX_MULTI_DLNK_LOG2 - OS_MIN_MULTI_DLNK_LOG2)

#define OS_MEM_NODE_GET_ALIGNED_FLAG(sizeAndFlag) ((sizeAndFlag) & OS_MEM_NODE_ALIGNED_FLAG)
#define OS_MEM_NODE_SET_ALIGNED_FLAG(sizeAndFlag) ((sizeAndFlag) = ((sizeAndFlag) | OS_MEM_NODE_ALIGNED_FLAG))
#define OS_MEM_NODE_GET_ALIGNED_GAPSIZE(sizeAndFlag) ((sizeAndFlag) & (~OS_MEM_NODE_ALIGNED_FLAG))
#define OS_MEM_NODE_GET_USED_FLAG(sizeAndFlag) ((sizeAndFlag) & OS_MEM_NODE_USED_FLAG)
#define OS_MEM_NODE_SET_USED_FLAG(sizeAndFlag) ((sizeAndFlag) = ((sizeAndFlag) | OS_MEM_NODE_USED_FLAG))
#define OS_MEM_NODE_GET_SIZE(sizeAndFlag) ((sizeAndFlag) & (~OS_MEM_NODE_USED_FLAG))
#define OS_MEM_IS_NODE_NEXT_EXIST(node, poolInfo) (((UINT32)(node) + (node)->sizeAndFlag) < \
                                                         ((UINT32)(poolInfo) + (poolInfo)->uwPoolSize))
#define OS_MEM_HEAD(pool, size) OS_DLNK_HEAD(OS_MEM_HEAD_ADDR(pool), size)
#define OS_MEM_HEAD_ADDR(pool) ((VOID *)((UINT32)(UINTPTR)(pool) + sizeof(LOS_MEM_POOL_INFO)))
#define OS_MEM_NEXT_NODE(node) ((LosMemDynNode *)((UINT8 *)(node) + \
                                   OS_MEM_NODE_GET_SIZE((node)->sizeAndFlag)))
#define OS_MEM_FIRST_NODE(pool) ((LosMemDynNode *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE))
#define OS_MEM_END_NODE(pool, size) ((LosMemDynNode *)(((UINT8 *)(pool) + (size)) - OS_MEM_NODE_HEAD_SIZE))
#define OS_MEM_MIDDLE_ADDR_OPEN_END(startAddr, middleAddr, endAddr)  \
    (((UINT8 *)(startAddr) <= (UINT8 *)(middleAddr)) && ((UINT8 *)(middleAddr) < (UINT8 *)(endAddr)))
#define OS_MEM_MIDDLE_ADDR(startAddr, middleAddr, endAddr) (((UINT8 *)(startAddr) <= (UINT8 *)(middleAddr)) && \
                                                            ((UINT8 *)(middleAddr) <= (UINT8 *)(endAddr)))
#define OS_MEM_SET_MAGIC(value) (value) = (LOS_DL_LIST *)(UINTPTR)((UINT32)(UINTPTR)(&(value)) ^ 0xffffffff)
#define OS_MEM_MAGIC_VALID(value) ((((UINT32)(UINTPTR)(value)) ^ ((UINT32)(UINTPTR)(&(value)))) == 0xffffffff)

#define OS_MAX_MULTI_DLNK_LOG2              30
#define OS_MIN_MULTI_DLNK_LOG2              4
#define OS_MULTI_DLNK_NUM                   ((OS_MAX_MULTI_DLNK_LOG2 - OS_MIN_MULTI_DLNK_LOG2) + 1)
#define OS_DLNK_HEAD_SIZE                   OS_MULTI_DLNK_HEAD_SIZE
#define OS_DLNK_INIT_HEAD                   OsDLnkInitMultiHead
#define OS_DLNK_HEAD                        OsDLnkMultiHead
#define OS_DLNK_NEXT_HEAD                   OsDLnkNextMultiHead
#define OS_DLNK_FIRST_HEAD                  OsDLnkFirstMultiHead
#define OS_MULTI_DLNK_HEAD_SIZE             sizeof(LosMultipleDlinkHead)

typedef struct {
    LOS_DL_LIST listHead[OS_MULTI_DLNK_NUM];
} LosMultipleDlinkHead;

STATIC_INLINE LOS_DL_LIST *OsDLnkNextMultiHead(VOID *headAddr, LOS_DL_LIST *listHead)
{
    LosMultipleDlinkHead *head = (LosMultipleDlinkHead *)headAddr;

    return (&(head->listHead[OS_MULTI_DLNK_NUM - 1]) == listHead) ? NULL : (listHead + 1);
}

STATIC_INLINE LOS_DL_LIST *OsDLnkFirstMultiHead(VOID *headAddr)
{
    return (LOS_DL_LIST *)headAddr;
}

extern VOID OsDLnkInitMultiHead(VOID *headAddr);
extern LOS_DL_LIST *OsDLnkMultiHead(VOID *headAddr, UINT32 size);

#if (LOSCFG_MEMORY_BESTFIT == YES)

extern VOID OsTaskMemUsedInc(UINT32 usedSize, UINT32 taskID);
extern VOID OsTaskMemUsedDec(UINT32 usedSize, UINT32 taskID);
extern UINT32 OsTaskMemUsage(UINT32 taskID);
extern VOID  OsTaskMemClear(UINT32 taskID);

#ifdef OS_MEM_ENABLE_MEM_STATISTICS
#define OS_MEM_ADD_USED(usedSize, taskID)         OsTaskMemUsedInc(usedSize, taskID)
#define OS_MEM_REDUCE_USED(usedSize, taskID)      OsTaskMemUsedDec(usedSize, taskID)
#define OS_MEM_CLEAR(taskID)                      OsTaskMemClear(taskID)
#else
#define OS_MEM_ADD_USED(usedSize, taskID)
#define OS_MEM_REDUCE_USED(usedSize, taskID)
#define OS_MEM_CLEAR(taskID)
#endif

#else

#if (LOSCFG_KERNEL_MEM_STATISTICS == YES)
typedef struct {
    UINT32 memUsed;
    UINT32 memPeak;
} TaskMemUsedInfo;

extern VOID OsTaskMemStatInit(TaskMemUsedInfo *memStats);
extern VOID OsTaskMemUsedInc(TaskMemUsedInfo *memStats, UINT32 usedSize, UINT32 taskID);
extern VOID OsTaskMemUsedDec(TaskMemUsedInfo *memStats, UINT32 usedSize, UINT32 taskID);
extern UINT32 OsTaskMemUsage(TaskMemUsedInfo *memStats, UINT32 taskID);
extern VOID OsTaskMemClear(TaskMemUsedInfo *memStats, UINT32 taskID);
#endif
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_MEMORY_H */
