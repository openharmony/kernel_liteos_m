#ifndef _LOS_SLAB_PRI_H
#define _LOS_SLAB_PRI_H

#include "los_slab.h"
#include "los_toolchain.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION

/* step size of each class */
#define SLAB_MEM_CLASS_STEP_SIZE    0x10U

/* Power-of-2 stepping: 16,32,64,128 (4 classes, original scheme) */
#define SLAB_MEM_COUNT              4
#define SLAB_MEM_MAX_SIZE           (SLAB_MEM_CLASS_STEP_SIZE << (SLAB_MEM_COUNT - 1))

typedef struct tagLosSlabStatus {
    UINT32 totalSize;
    UINT32 usedSize;
    UINT32 freeSize;
    UINT32 allocCount;
    UINT32 freeCount;
} LosSlabStatus;

typedef struct tagOsSlabBlockNode {
    UINT16 magic;
    UINT8  blkSz;
    UINT8  recordId;
#ifdef LOSCFG_AARCH64
    UINT32 reserved;
#endif
} OsSlabBlockNode;

struct AtomicBitset {
    UINT32 numBits;
    UINT32 words[0];
};

typedef struct tagOsSlabAllocator {
    UINT32 itemSz;
    UINT8 *dataChunks;
    struct AtomicBitset *bitset;
} OsSlabAllocator;

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
typedef struct tagOsSlabMemAllocator {
    struct tagOsSlabMemAllocator *next;
    OsSlabAllocator *slabAlloc;
} OsSlabMemAllocator;
#endif

typedef struct tagOsSlabMem {
    UINT32 blkSz;
    UINT32 blkCnt;
    UINT32 blkUsedCnt;
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    UINT32 allocatorCnt;
    OsSlabMemAllocator *bucket;
#else
    OsSlabAllocator *alloc;
#endif
} OsSlabMem;

struct LosSlabControlHeader {
    BOOL enabled;
#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
    OsSlabAllocator *allocatorBucket;
#endif
    OsSlabMem slabClass[SLAB_MEM_COUNT];
};

#ifdef LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
#define SLAB_MEM_DEFAULT_BUCKET_CNT  0
#endif

#define OS_SLAB_MAGIC                              0xdede
#define OS_SLAB_BLOCK_HEAD_GET(ptr)                ((OsSlabBlockNode *)(VOID *)((UINT8 *)(ptr) - \
                                                      sizeof(OsSlabBlockNode)))
#define OS_SLAB_BLOCK_MAGIC_SET(slabNode)          (((OsSlabBlockNode *)(slabNode))->magic = (UINT16)OS_SLAB_MAGIC)
#define OS_SLAB_BLOCK_MAGIC_GET(slabNode)          (((OsSlabBlockNode *)(slabNode))->magic)
#define OS_SLAB_BLOCK_SIZE_SET(slabNode, size)     (((OsSlabBlockNode *)(slabNode))->blkSz = (UINT8)(size))
#define OS_SLAB_BLOCK_SIZE_GET(slabNode)           (((OsSlabBlockNode *)(slabNode))->blkSz)
#define OS_SLAB_BLOCK_ID_SET(slabNode, id)         (((OsSlabBlockNode *)(slabNode))->recordId = (id))
#define OS_SLAB_BLOCK_ID_GET(slabNode)             (((OsSlabBlockNode *)(slabNode))->recordId)
#define OS_ALLOC_FROM_SLAB_CHECK(slabNode)         (((OsSlabBlockNode *)(slabNode))->magic == (UINT16)OS_SLAB_MAGIC)

#define OS_SLAB_LOG2(value)                        ((UINT32)(32 - CLZ(value) - 1)) /* get highest bit one position */
#define OS_SLAB_CLASS_LEVEL_GET(size) \
        (OS_SLAB_LOG2((size - 1) >> (OS_SLAB_LOG2(SLAB_MEM_CLASS_STEP_SIZE - 1))))

extern OsSlabAllocator *OsSlabAllocatorNew(VOID *pool, UINT32 itemSz, UINT32 itemAlign, UINT32 numItems);
extern VOID OsSlabAllocatorDestroy(VOID *pool, OsSlabAllocator *allocator);
extern VOID *OsSlabAllocatorAlloc(OsSlabAllocator *allocator);
extern BOOL OsSlabAllocatorFree(OsSlabAllocator *allocator, VOID* ptr);
extern BOOL OsSlabAllocatorEmpty(const OsSlabAllocator *allocator);
extern VOID OsSlabAllocatorGetSlabInfo(const OsSlabAllocator *allocator, UINT32 *itemSize,
    UINT32 *itemCnt, UINT32 *curUsage);
extern BOOL OsSlabAllocatorCheck(const OsSlabAllocator *allocator, const VOID *ptr);
extern VOID OsSlabMemInit(VOID *pool, UINT32 size);
extern VOID OsSlabMemDeinit(VOID *pool);
extern VOID *OsSlabMemAlloc(VOID *pool, UINT32 size);
extern BOOL OsSlabMemFree(VOID *pool, VOID *ptr);
extern UINT32 OsSlabMemCheck(const VOID *pool, const VOID *ptr);
extern UINT32 OsSlabStatisticsGet(const VOID *pool, LosSlabStatus *status);
extern UINT32 OsSlabGetMaxFreeBlkSize(const VOID *pool);
extern VOID *OsSlabCtrlHdrGet(const VOID *pool);
extern VOID *OsSlabAllocatorGetIdxP(const OsSlabAllocator *allocator, UINT32 idx);


STATIC INLINE VOID OsSlabMemProcInitFlag(VOID *pool, UINT32 size, BOOL slabEnable)
{
    struct LosSlabControlHeader *header = (struct LosSlabControlHeader *)OsSlabCtrlHdrGet(pool);

    if (slabEnable) {
        OsSlabMemInit(pool, size);
    }

    /* flag Must be set after OsSlabMemInit */
    header->enabled = slabEnable;
}

#else /* !LOSCFG_KERNEL_MEM_SLAB_EXTENTION */

STATIC INLINE VOID OsSlabMemProcInitFlag(VOID *pool, UINT32 size, BOOL slabEnable)
{
    (VOID)pool;
    (VOID)size;
    (VOID)slabEnable;
}

STATIC INLINE VOID *OsSlabMemAlloc(VOID *pool, UINT32 size)
{
    (VOID)pool;
    (VOID)size;
    return NULL;
}

STATIC INLINE BOOL OsSlabMemFree(VOID *pool, VOID *ptr)
{
    (VOID)pool;
    (VOID)ptr;
    return FALSE;
}

STATIC INLINE UINT32 OsSlabMemCheck(const VOID *pool, const VOID *ptr)
{
    (VOID)pool;
    (VOID)ptr;
    return (UINT32)-1;
}

#endif /* LOSCFG_KERNEL_MEM_SLAB_EXTENTION */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_SLAB_PRI_H */
