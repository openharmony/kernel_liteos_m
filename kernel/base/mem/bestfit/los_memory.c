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

#include "los_memory_internal.h"
#include "los_init.h"
#include "los_memory_pri.h"
#include "securec.h"

#ifdef LOSCFG_LIB_LIBC
#include "stdio.h"
#endif
#include "los_multipledlinkhead_pri.h"
#include "los_slab_pri.h"
#include "los_task_pri.h"
#include "los_exc.h"
#include "los_spinlock.h"
#include "los_trace.h"
#include "los_exc_pri.h"

#ifdef LOSCFG_KERNEL_LMS
#include "los_lms_pri.h"
#endif

#define NODEDUMPSIZE  64  /* the dump size of current broken node when memcheck error */
#define COLUMN_NUM    8   /* column num of the output info of mem node */

#define OS_CHECK_NULL_RETURN(param) do {              \
    if ((param) == NULL) {                            \
        PRINT_ERR("%s %d\n", __FUNCTION__, __LINE__); \
        return;                                       \
    }                                                 \
} while (0)

/* spinlock for mem module, only available on SMP mode */
LITE_OS_SEC_BSS  SPIN_LOCK_INIT(g_memSpin);

UINT8 *m_aucSysMem0 = NULL;
UINT8 *m_aucSysMem1 = NULL;

MALLOC_HOOK g_MALLOC_HOOK = NULL;

#ifdef LOSCFG_MEM_TASK_STAT
TaskMemUsedInfo *g_osTaskMemstats = NULL;
#endif

#ifdef LOSCFG_BASE_MEM_NODE_SIZE_CHECK
STATIC UINT8 g_memCheckLevel = LOS_MEM_CHECK_LEVEL_DEFAULT;
#endif

#ifdef LOSCFG_MEM_MUL_MODULE
UINT32 g_moduleMemUsedSize[MEM_MODULE_MAX + 1] = { 0 };
#endif

#ifdef LOSCFG_MEM_HEAD_BACKUP
STATIC VOID OsMemNodeSave(LosMemDynNode *node);
#endif

STATIC VOID *OsMemAllocWithCheck(LosMemPoolInfo *pool, UINT32 size);

USED STATIC INLINE VOID OS_MEM_TASKID_SET(LosMemDynNode *node, UINT32 taskId)
{
    node->selfNode.taskId = taskId;

#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
}

USED STATIC INLINE UINT32 OS_MEM_TASKID_GET(const LosMemDynNode *node)
{
    return (UINT32)node->selfNode.taskId;
}

#ifdef LOSCFG_MEM_MUL_MODULE

STATIC INLINE VOID OS_MEM_MODID_SET(LosMemDynNode *node, UINT32 moduleId)
{
    node->selfNode.moduleId = moduleId;

#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
}

STATIC INLINE UINT32 OS_MEM_MODID_GET(const LosMemDynNode *node)
{
    return (UINT32)node->selfNode.moduleId;
}

#endif

/*
 * Description : set magic & taskid
 * Input       : node -- the node which will be set magic & taskid
 */
STATIC INLINE VOID OsMemSetMagicNumAndTaskID(LosMemDynNode *node)
{
#if defined(LOSCFG_MEM_DEBUG) || defined(LOSCFG_MEM_TASK_STAT)
    LosTaskCB *runTask = OsCurrTaskGet();

    OS_MEM_SET_MAGIC(node->selfNode.magic);

    /*
     * If the operation occurred before task initialization(runTask was not assigned)
     * or in interrupt, make the value of taskid of node to  (TASK_NUM - 1) which is
     * the id of the extra task. We can record those memory use with that.
     */
    if ((runTask != NULL) && OS_INT_INACTIVE) {
        OS_MEM_TASKID_SET(node, runTask->taskId);
    } else {
        /* If the task mode does not initialize, the field is the 0xffffffff */
        OS_MEM_TASKID_SET(node, TASK_NUM - 1);
    }
#else
    (VOID)node;
#endif
}

#ifdef LOSCFG_BASE_MEM_NODE_SIZE_CHECK
const VOID *OsMemFindNodeCtrl(const VOID *pool, const VOID *ptr);
#endif
#ifdef LOSCFG_MEM_HEAD_BACKUP
#define CHECKSUM_MAGICNUM    0xDEADBEEF
#define OS_MEM_NODE_CHECKSUM_CALCULATE(ctlNode)    \
    (((UINTPTR)(ctlNode)->freeNodeInfo.pstPrev) ^  \
    ((UINTPTR)(ctlNode)->freeNodeInfo.pstNext) ^   \
    ((UINTPTR)(ctlNode)->preNode) ^                \
    (ctlNode)->gapSize ^                           \
    (ctlNode)->sizeAndFlag ^                       \
    CHECKSUM_MAGICNUM)

STATIC INLINE VOID OsMemDispCtlNode(const LosMemCtlNode *ctlNode)
{
    UINTPTR checksum;

    checksum = OS_MEM_NODE_CHECKSUM_CALCULATE(ctlNode);

    PRINTK("node:%p checksum=%p[%p] freeNodeInfo.pstPrev=%p "
           "freeNodeInfo.pstNext=%p preNode=%p gapSize=0x%x sizeAndFlag=0x%x\n",
           ctlNode,
           ctlNode->checksum,
           checksum,
           ctlNode->freeNodeInfo.pstPrev,
           ctlNode->freeNodeInfo.pstNext,
           ctlNode->preNode,
           ctlNode->gapSize,
           ctlNode->sizeAndFlag);
}

STATIC INLINE VOID OsMemDispMoreDetails(const LosMemDynNode *node)
{
    UINT32 taskId;
    LosTaskCB *taskCB = NULL;

    PRINT_ERR("************************************************\n");
    OsMemDispCtlNode(&node->selfNode);
    PRINT_ERR("the address of node :%p\n", node);

    if (!OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        PRINT_ERR("this is a FREE node\n");
        PRINT_ERR("************************************************\n\n");
        return;
    }

    taskId = OS_MEM_TASKID_GET(node);
    if (taskId >= g_taskMaxNum) {
        PRINT_ERR("The task [ID: 0x%x] is ILLEGAL\n", taskId);
        if (taskId == g_taskMaxNum) {
            PRINT_ERR("PROBABLY alloc by SYSTEM INIT, NOT IN ANY TASK\n");
        }
        PRINT_ERR("************************************************\n\n");
        return;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    if ((taskCB->taskStatus & OS_TASK_STATUS_UNUSED) ||
        (taskCB->taskEntry == NULL) ||
        (taskCB->taskName == NULL)) {
        PRINT_ERR("The task [ID: 0x%x] is NOT CREATED(ILLEGAL)\n", taskId);
        PRINT_ERR("************************************************\n\n");
        return;
    }

    PRINT_ERR("allocated by task: %s [ID = 0x%x]\n", taskCB->taskName, taskId);
#ifdef LOSCFG_MEM_MUL_MODULE
    PRINT_ERR("allocated by moduleId: %lu\n", OS_MEM_MODID_GET(node));
#endif

    PRINT_ERR("************************************************\n\n");
}

STATIC INLINE VOID OsMemDispWildPointerMsg(const LosMemDynNode *node, const VOID *ptr)
{
    PRINT_ERR("*****************************************************\n");
    PRINT_ERR("find an control block at: %p, gap size: 0x%x, sizeof(LosMemDynNode): 0x%x\n", node,
              node->selfNode.gapSize, sizeof(LosMemDynNode));
    PRINT_ERR("the pointer should be: %p\n",
              ((UINTPTR)node + node->selfNode.gapSize + sizeof(LosMemDynNode)));
    PRINT_ERR("the pointer given is: %p\n", ptr);
    PRINT_ERR("PROBABLY A WILD POINTER\n");
    OsBackTrace();
    PRINT_ERR("*****************************************************\n\n");
}

STATIC INLINE VOID OsMemChecksumSet(LosMemCtlNode *ctlNode)
{
    ctlNode->checksum = OS_MEM_NODE_CHECKSUM_CALCULATE(ctlNode);
}

STATIC INLINE BOOL OsMemChecksumVerify(const LosMemCtlNode *ctlNode)
{
    return ctlNode->checksum == OS_MEM_NODE_CHECKSUM_CALCULATE(ctlNode);
}

STATIC INLINE VOID OsMemBackupSetup(const LosMemDynNode *node)
{
    LosMemDynNode *nodePre = node->selfNode.preNode;
    if (nodePre != NULL) {
        nodePre->backupNode.freeNodeInfo.pstNext = node->selfNode.freeNodeInfo.pstNext;
        nodePre->backupNode.freeNodeInfo.pstPrev = node->selfNode.freeNodeInfo.pstPrev;
        nodePre->backupNode.preNode = node->selfNode.preNode;
        nodePre->backupNode.checksum = node->selfNode.checksum;
        nodePre->backupNode.gapSize = node->selfNode.gapSize;
        nodePre->backupNode.sizeAndFlag = node->selfNode.sizeAndFlag;
    }
}

LosMemDynNode *OsMemNodeNextGet(const VOID *pool, const LosMemDynNode *node)
{
    const LosMemPoolInfo *poolInfo = (const LosMemPoolInfo *)pool;

    if (node == OS_MEM_END_NODE(pool, poolInfo->poolSize)) {
        return OS_MEM_FIRST_NODE(pool);
    } else {
        return OS_MEM_NEXT_NODE(node);
    }
}

STATIC INLINE UINT32 OsMemBackupSetup4Next(const VOID *pool, LosMemDynNode *node)
{
    LosMemDynNode *nodeNext = OsMemNodeNextGet(pool, node);

    if (!OsMemChecksumVerify(&nodeNext->selfNode)) {
        PRINT_ERR("[%s]the next node is broken!!\n", __FUNCTION__);
        OsMemDispCtlNode(&(nodeNext->selfNode));
        PRINT_ERR("Current node details:\n");
        OsMemDispMoreDetails(node);

        return LOS_NOK;
    }

    if (!OsMemChecksumVerify(&node->backupNode)) {
        node->backupNode.freeNodeInfo.pstNext = nodeNext->selfNode.freeNodeInfo.pstNext;
        node->backupNode.freeNodeInfo.pstPrev = nodeNext->selfNode.freeNodeInfo.pstPrev;
        node->backupNode.preNode = nodeNext->selfNode.preNode;
        node->backupNode.checksum = nodeNext->selfNode.checksum;
        node->backupNode.gapSize = nodeNext->selfNode.gapSize;
        node->backupNode.sizeAndFlag = nodeNext->selfNode.sizeAndFlag;
    }
    return LOS_OK;
}

UINT32 OsMemBackupDoRestore(VOID *pool, const LosMemDynNode *nodePre, LosMemDynNode *node)
{
    if (node == NULL) {
        PRINT_ERR("the node is NULL.\n");
        return LOS_NOK;
    }
    PRINT_ERR("the backup node information of current node in previous node:\n");
    OsMemDispCtlNode(&nodePre->backupNode);
    PRINT_ERR("the detailed information of previous node:\n");
    OsMemDispMoreDetails(nodePre);

    node->selfNode.freeNodeInfo.pstNext = nodePre->backupNode.freeNodeInfo.pstNext;
    node->selfNode.freeNodeInfo.pstPrev = nodePre->backupNode.freeNodeInfo.pstPrev;
    node->selfNode.preNode = nodePre->backupNode.preNode;
    node->selfNode.checksum = nodePre->backupNode.checksum;
    node->selfNode.gapSize = nodePre->backupNode.gapSize;
    node->selfNode.sizeAndFlag = nodePre->backupNode.sizeAndFlag;

    /* we should re-setup next node's backup on current node */
    return OsMemBackupSetup4Next(pool, node);
}

STATIC LosMemDynNode *OsMemFirstNodePrevGet(const LosMemPoolInfo *poolInfo)
{
    LosMemDynNode *nodePre = NULL;

    nodePre = OS_MEM_END_NODE(poolInfo, poolInfo->poolSize);
    if (!OsMemChecksumVerify(&(nodePre->selfNode))) {
        PRINT_ERR("the current node is THE FIRST NODE !\n");
        PRINT_ERR("[%s]: the node information of previous node is bad !!\n", __FUNCTION__);
        OsMemDispCtlNode(&(nodePre->selfNode));
    }
    if (!OsMemChecksumVerify(&(nodePre->backupNode))) {
        PRINT_ERR("the current node is THE FIRST NODE !\n");
        PRINT_ERR("[%s]: the backup node information of current node in previous Node is bad !!\n", __FUNCTION__);
        OsMemDispCtlNode(&(nodePre->backupNode));
        return NULL;
    }

    return nodePre;
}

LosMemDynNode *OsMemNodePrevGet(VOID *pool, const LosMemDynNode *node)
{
    LosMemDynNode *nodeCur = NULL;
    LosMemDynNode *nodePre = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *endNode = NULL;

    if (node == OS_MEM_FIRST_NODE(pool)) {
        return OsMemFirstNodePrevGet(poolInfo);
    }

    endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    for (nodeCur = OS_MEM_FIRST_NODE(pool); nodeCur < endNode; nodeCur = OS_MEM_NEXT_NODE(nodeCur)) {
        if (!OsMemChecksumVerify(&(nodeCur->selfNode))) {
            PRINT_ERR("[%s]: the node information of current node is bad !!\n", __FUNCTION__);
            OsMemDispCtlNode(&(nodeCur->selfNode));

            if (nodePre == NULL) {
                return NULL;
            }

            PRINT_ERR("the detailed information of previous node:\n");
            OsMemDispMoreDetails(nodePre);

            /* due to the every step's checksum verify, nodePre is trustful */
            if (OsMemBackupDoRestore(pool, nodePre, nodeCur) != LOS_OK) {
                return NULL;
            }
        }

        if (!OsMemChecksumVerify(&(nodeCur->backupNode))) {
            PRINT_ERR("[%s]: the backup node information in current node is bad !!\n", __FUNCTION__);
            OsMemDispCtlNode(&(nodeCur->backupNode));

            if (nodePre != NULL) {
                PRINT_ERR("the detailed information of previous node:\n");
                OsMemDispMoreDetails(nodePre);
            }

            if (OsMemBackupSetup4Next(pool, nodeCur) != LOS_OK) {
                return NULL;
            }
        }

        if (OS_MEM_NEXT_NODE(nodeCur) == node) {
            return nodeCur;
        }

        if (OS_MEM_NEXT_NODE(nodeCur) > node) {
            break;
        }

        nodePre = nodeCur;
    }

    return NULL;
}

LosMemDynNode *OsMemNodePrevTryGet(VOID *pool, LosMemDynNode **node, const VOID *ptr)
{
    UINTPTR nodeShouldBe;
    LosMemDynNode *nodeCur = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *nodePre = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    LosMemDynNode *endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);

    for (nodeCur = OS_MEM_FIRST_NODE(pool); nodeCur < endNode; nodeCur = OS_MEM_NEXT_NODE(nodeCur)) {
        if (!OsMemChecksumVerify(&(nodeCur->selfNode))) {
            PRINT_ERR("[%s]: the node information of current node is bad !!\n", __FUNCTION__);
            OsMemDispCtlNode(&(nodeCur->selfNode));

            PRINT_ERR("the detailed information of previous node:\n");
            OsMemDispMoreDetails(nodePre);

            /* due to the every step's checksum verify, nodePre is trustful */
            if (OsMemBackupDoRestore(pool, nodePre, nodeCur) != LOS_OK) {
                return NULL;
            }
        }

        if (!OsMemChecksumVerify(&(nodeCur->backupNode))) {
            PRINT_ERR("[%s]: the backup node information in current node is bad !!\n", __FUNCTION__);
            OsMemDispCtlNode(&(nodeCur->backupNode));

            if (nodePre != NULL) {
                PRINT_ERR("the detailed information of previous node:\n");
                OsMemDispMoreDetails(nodePre);
            }

            if (OsMemBackupSetup4Next(pool, nodeCur) != LOS_OK) {
                return NULL;
            }
        }

        nodeShouldBe = (UINTPTR)nodeCur + nodeCur->selfNode.gapSize + sizeof(LosMemDynNode);
        if (nodeShouldBe == (UINTPTR)ptr) {
            *node = nodeCur;
            return nodePre;
        }

        if (OS_MEM_NEXT_NODE(nodeCur) > (LosMemDynNode *)ptr) {
            OsMemDispWildPointerMsg(nodeCur, ptr);
            break;
        }

        nodePre = nodeCur;
    }

    return NULL;
}

STATIC INLINE UINT32 OsMemBackupTryRestore(VOID *pool, const VOID *ptr)
{
    LosMemDynNode *nodeHead = NULL;
    LosMemDynNode *nodePre = OsMemNodePrevTryGet(pool, &nodeHead, ptr);
    if (nodePre == NULL) {
        return LOS_NOK;
    }

    return OsMemBackupDoRestore(pool, nodePre, nodeHead);
}

STATIC INLINE UINT32 OsMemBackupRestore(VOID *pool, LosMemDynNode *node)
{
    LosMemDynNode *nodePre = OsMemNodePrevGet(pool, node);
    if (nodePre == NULL) {
        return LOS_NOK;
    }

    return OsMemBackupDoRestore(pool, nodePre, node);
}

STATIC INLINE UINT32 OsMemBackupCheckAndRestore(VOID *pool, const VOID *ptr, LosMemDynNode *node)
{
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *startNode = OS_MEM_FIRST_NODE(pool);
    LosMemDynNode *endNode   = OS_MEM_END_NODE(pool, poolInfo->poolSize);

    if (OS_MEM_MIDDLE_ADDR(startNode, node, endNode)) {
        /* GapSize is bad or node is broken, we need to verify & try to restore */
        if (!OsMemChecksumVerify(&node->selfNode)) {
            return OsMemBackupTryRestore(pool, ptr);
        }
    }
    return LOS_OK;
}

STATIC INLINE VOID OsMemSetGapSize(LosMemCtlNode *ctlNode, UINT32 gapSize)
{
    ctlNode->gapSize = gapSize;
}

STATIC VOID OsMemNodeSave(LosMemDynNode *node)
{
    OsMemSetGapSize(&(node->selfNode), 0);
    OsMemChecksumSet(&(node->selfNode));
    OsMemBackupSetup(node);
}

STATIC VOID OsMemNodeSaveWithGapSize(LosMemDynNode *node, UINT32 gapSize)
{
    OsMemSetGapSize(&(node->selfNode), gapSize);
    OsMemChecksumSet(&(node->selfNode));
    OsMemBackupSetup(node);
}

STATIC VOID OsMemListDelete(LOS_DL_LIST *node, const VOID *firstNode)
{
    LosMemDynNode *dynNode = NULL;

    node->pstNext->pstPrev = node->pstPrev;
    node->pstPrev->pstNext = node->pstNext;

    if ((VOID *)(node->pstNext) >= firstNode) {
        dynNode = LOS_DL_LIST_ENTRY(node->pstNext, LosMemDynNode, selfNode.freeNodeInfo);
        OsMemNodeSave(dynNode);
    }

    if ((VOID *)(node->pstPrev) >= firstNode) {
        dynNode = LOS_DL_LIST_ENTRY(node->pstPrev, LosMemDynNode, selfNode.freeNodeInfo);
        OsMemNodeSave(dynNode);
    }

    node->pstNext = NULL;
    node->pstPrev = NULL;

    dynNode = LOS_DL_LIST_ENTRY(node, LosMemDynNode, selfNode.freeNodeInfo);
    OsMemNodeSave(dynNode);
}

STATIC VOID OsMemListAdd(LOS_DL_LIST *listNode, LOS_DL_LIST *node, const VOID *firstNode)
{
    LosMemDynNode *dynNode = NULL;

    node->pstNext = listNode->pstNext;
    node->pstPrev = listNode;

    dynNode = LOS_DL_LIST_ENTRY(node, LosMemDynNode, selfNode.freeNodeInfo);
    OsMemNodeSave(dynNode);

    listNode->pstNext->pstPrev = node;
    if ((VOID *)(listNode->pstNext) >= firstNode) {
        dynNode = LOS_DL_LIST_ENTRY(listNode->pstNext, LosMemDynNode, selfNode.freeNodeInfo);
        OsMemNodeSave(dynNode);
    }

    listNode->pstNext = node;
}

VOID LOS_MemBadNodeShow(VOID *pool)
{
    LosMemDynNode *nodePre = NULL;
    LosMemDynNode *tmpNode = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *endNode = NULL;
    UINT32 intSave;

    if (pool == NULL) {
        return;
    }

    MEM_LOCK(intSave, pool);

    endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode <= endNode; tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        OsMemDispCtlNode(&tmpNode->selfNode);

        if (OsMemChecksumVerify(&tmpNode->selfNode)) {
            continue;
        }

        nodePre = OsMemNodePrevGet(pool, tmpNode);
        if (nodePre == NULL) {
            PRINT_ERR("the current node is invalid, but cannot find its previous Node\n");
            continue;
        }

        PRINT_ERR("the detailed information of previous node:\n");
        OsMemDispMoreDetails(nodePre);
    }

    MEM_UNLOCK(intSave, pool);
    PRINTK("check finish\n");
}

#else  /* without LOSCFG_MEM_HEAD_BACKUP */

STATIC VOID OsMemListDelete(LOS_DL_LIST *node, const VOID *firstNode)
{
    (VOID)firstNode;
    node->pstNext->pstPrev = node->pstPrev;
    node->pstPrev->pstNext = node->pstNext;
    node->pstNext = NULL;
    node->pstPrev = NULL;
}

STATIC VOID OsMemListAdd(LOS_DL_LIST *listNode, LOS_DL_LIST *node, const VOID *firstNode)
{
    (VOID)firstNode;
    node->pstNext = listNode->pstNext;
    node->pstPrev = listNode;
    listNode->pstNext->pstPrev = node;
    listNode->pstNext = node;
}

#endif

#ifdef LOSCFG_MEM_LEAKCHECK
__attribute__((always_inline)) STATIC inline VOID OsMemLinkRegisterRecord(LosMemDynNode *node)
{
    (VOID)memset(node->selfNode.linkReg, 0, sizeof(node->selfNode.linkReg));
    (VOID)OsBackTraceHookCall(node->selfNode.linkReg, LOSCFG_MEM_RECORD_LR_CNT, LOSCFG_MEM_OMIT_LR_CNT, 0);
    return;
}

#if (LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK == 1)
STATIC VOID OsMemNodeBacktraceInfo(const LosMemDynNode *tmpNode, const LosMemDynNode *preNode)
{
    INT32 i;
    EXCINFO_PRINTK("\n broken node head LR info: \n");
    for (i = 0; i < LOSCFG_MEM_RECORD_LR_CNT; i++) {
        EXCINFO_PRINTK(" LR[%d]:%p\n", i, tmpNode->selfNode.linkReg[i]);
    }
    EXCINFO_PRINTK("\n pre node head LR info: \n");
    for (i = 0; i < LOSCFG_MEM_RECORD_LR_CNT; i++) {
        EXCINFO_PRINTK(" LR[%d]:%p\n", i, preNode->selfNode.linkReg[i]);
    }
}
#endif

LITE_OS_SEC_TEXT_MINOR VOID OsMemUsedNodeShow(VOID *pool)
{
    LosMemDynNode *tmpNode = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *endNode = NULL;
    UINT32 intSave;
    UINT32 count;

    if (pool == NULL) {
        PRINTK("input param is NULL\n");
        return;
    }
    if (LOS_MemIntegrityCheck(pool)) {
        PRINTK("LOS_MemIntegrityCheck error\n");
        return;
    }
    MEM_LOCK(intSave, pool);
#ifdef __LP64__
    PRINTK("node                size      ");
#else
    PRINTK("node        size      ");
#endif
    for (count = 0; count < LOSCFG_MEM_RECORD_LR_CNT; count++) {
#ifdef __LP64__
        PRINTK("        LR[%u]       ", count);
#else
        PRINTK("    LR[%u]   ", count);
#endif
    }
    PRINTK("\n");
    endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode < endNode;
         tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
#ifdef __LP64__
            PRINTK("%-18p  %-8u: ", tmpNode, OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag));
#else
            PRINTK("%-10p  %-8u: ", tmpNode, OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag));
#endif
            for (count = 0; count < LOSCFG_MEM_RECORD_LR_CNT; count++) {
#ifdef __LP64__
                PRINTK(" %-18p ", tmpNode->selfNode.linkReg[count]);
#else
                PRINTK(" %-10p ", tmpNode->selfNode.linkReg[count]);
#endif
            }
            PRINTK("\n");
        }
    }
    MEM_UNLOCK(intSave, pool);
    return;
}

#endif

#ifdef LOSCFG_KERNEL_LMS
STATIC INLINE VOID OsLmsFirstNodeMark(VOID *pool, LosMemDynNode *node)
{
    if (g_lms == NULL) {
        return;
    }

    g_lms->simpleMark((UINTPTR)pool, (UINTPTR)node, LMS_SHADOW_PAINT_U8);
    g_lms->simpleMark((UINTPTR)node, (UINTPTR)node + OS_MEM_NODE_HEAD_SIZE, LMS_SHADOW_REDZONE_U8);
    g_lms->simpleMark((UINTPTR)OS_MEM_NEXT_NODE(node), (UINTPTR)OS_MEM_NEXT_NODE(node) + OS_MEM_NODE_HEAD_SIZE,
        LMS_SHADOW_REDZONE_U8);
    g_lms->simpleMark((UINTPTR)node + OS_MEM_NODE_HEAD_SIZE, (UINTPTR)OS_MEM_NEXT_NODE(node),
        LMS_SHADOW_AFTERFREE_U8);
}

STATIC INLINE VOID OsLmsAllocAlignMark(VOID *ptr, VOID *alignedPtr, UINT32 size)
{
    LosMemDynNode *allocNode = NULL;

    if (g_lms == NULL) {
        return;
    }
    allocNode = (LosMemDynNode *)ptr - 1;
    if (ptr != alignedPtr) {
        g_lms->simpleMark((UINTPTR)ptr, (UINTPTR)ptr + sizeof(UINT32), LMS_SHADOW_PAINT_U8);
        g_lms->simpleMark((UINTPTR)ptr + sizeof(UINT32), (UINTPTR)alignedPtr, LMS_SHADOW_REDZONE_U8);
    }

    /* mark remind as redzone */
    g_lms->simpleMark(LMS_ADDR_ALIGN((UINTPTR)alignedPtr + size), (UINTPTR)OS_MEM_NEXT_NODE(allocNode),
        LMS_SHADOW_REDZONE_U8);
}

STATIC INLINE VOID OsLmsReallocMergeNodeMark(LosMemDynNode *node)
{
    if (g_lms == NULL) {
        return;
    }

    g_lms->simpleMark((UINTPTR)node + OS_MEM_NODE_HEAD_SIZE, (UINTPTR)OS_MEM_NEXT_NODE(node),
        LMS_SHADOW_ACCESSABLE_U8);
}

STATIC INLINE VOID OsLmsReallocSplitNodeMark(LosMemDynNode *node)
{
    if (g_lms == NULL) {
        return;
    }
    /* mark next node */
    g_lms->simpleMark((UINTPTR)OS_MEM_NEXT_NODE(node),
        (UINTPTR)OS_MEM_NEXT_NODE(node) + OS_MEM_NODE_HEAD_SIZE, LMS_SHADOW_REDZONE_U8);
    g_lms->simpleMark((UINTPTR)OS_MEM_NEXT_NODE(node) + OS_MEM_NODE_HEAD_SIZE,
        (UINTPTR)OS_MEM_NEXT_NODE(OS_MEM_NEXT_NODE(node)), LMS_SHADOW_AFTERFREE_U8);
}

STATIC INLINE VOID OsLmsReallocResizeMark(LosMemDynNode *node, UINT32 resize)
{
    if (g_lms == NULL) {
        return;
    }
    /* mark remained as readZone */
    g_lms->simpleMark((UINTPTR)node + resize, (UINTPTR)OS_MEM_NEXT_NODE(node), LMS_SHADOW_REDZONE_U8);
}
#endif

#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
STATIC VOID *OsMemReallocSlab(VOID *pool, VOID *ptr, BOOL *isSlabMem, UINT32 size)
{
    errno_t rc;
    UINT32 blkSz;
    VOID *newPtr = NULL;
    VOID *freePtr = ptr;

    blkSz = OsSlabMemCheck(pool, ptr);
    if (blkSz == (UINT32)-1) {
        *isSlabMem = FALSE;
        return NULL;
    }
    *isSlabMem = TRUE;

    if (size <= blkSz) {
        return ptr;
    }

    /* Unlock the memory spin, to allow the memory alloc API to be called */
    LOS_SpinUnlock(&g_memSpin);

    newPtr = LOS_MemAlloc(pool, size);
    if (newPtr != NULL) {
        rc = memcpy_s(newPtr, size, ptr, blkSz);
        if (rc != EOK) {
            PRINT_ERR("%s[%d] memcpy_s failed, error type = %d\n", __FUNCTION__, __LINE__, rc);
            freePtr = newPtr;
            newPtr = NULL;
        }
        if (LOS_MemFree((VOID *)pool, freePtr) != LOS_OK) {
            PRINT_ERR("%s, %d\n", __FUNCTION__, __LINE__);
        }
    }

    /* Reacquire the spin */
    LOS_SpinLock(&g_memSpin);

    return newPtr;
}

VOID *OsMemAlloc(VOID *pool, UINT32 size)
{
    return OsMemAllocWithCheck(pool, size);
}

#else

STATIC VOID *OsMemReallocSlab(VOID *pool, const VOID *ptr, BOOL *isSlabMem, UINT32 size)
{
    (VOID)pool;
    (VOID)ptr;
    (VOID)size;

    *isSlabMem = FALSE;
    return NULL;
}

#endif

#ifdef LOSCFG_EXC_INTERACTION
LITE_OS_SEC_TEXT_INIT UINT32 OsMemExcInteractionInit(UINTPTR memStart)
{
    UINT32 ret;
    m_aucSysMem0 = (UINT8 *)memStart;
    g_excInteractMemSize = EXC_INTERACT_MEM_SIZE;
    ret = LOS_MemInit(m_aucSysMem0, g_excInteractMemSize);
    PRINT_INFO("LiteOS kernel exc interaction memory size:0x%x\n", g_excInteractMemSize);
    return ret;
}
#endif

LITE_OS_SEC_TEXT_INIT UINT32 OsMemSystemInit(VOID)
{
    UINT32 ret;
    UINT32 poolSize;

#if (LOSCFG_SYS_EXTERNAL_HEAP == 0)
    extern UINT8 g_memStart[];
    m_aucSysMem1 = g_memStart;
#else
    m_aucSysMem1 = (UINT8 *)LOSCFG_SYS_HEAP_ADDR;
#endif
    poolSize = LOSCFG_SYS_HEAP_SIZE;
    ret = LOS_MemInit(m_aucSysMem1, poolSize);
    PRINT_INFO("LiteOS system heap memory size:0x%x\n", poolSize);
#ifndef LOSCFG_EXC_INTERACTION
    m_aucSysMem0 = m_aucSysMem1;
#endif
    return ret;
}
LOS_SYS_INIT(OsMemSystemInit, SYS_INIT_LEVEL_AHEAD, SYS_INIT_SYNC_0);

/*
 * Description : find suitable free block use "best fit" algorithm
 * Input       : pool      --- Pointer to memory pool
 *               allocSize --- Size of memory in bytes which note need allocate
 * Return      : NULL      --- no suitable block found
 *               tmpNode   --- pointer a suitable free block
 */
STATIC INLINE LosMemDynNode *OsMemFindSuitableFreeBlock(VOID *pool, UINT32 allocSize)
{
    LOS_DL_LIST *listNodeHead = NULL;
    LosMemDynNode *tmpNode = NULL;

#ifdef LOSCFG_MEM_HEAD_BACKUP
    UINT32 ret = LOS_OK;
#endif
    for (listNodeHead = OS_MEM_HEAD(pool, allocSize); listNodeHead != NULL;
         listNodeHead = OsDLnkNextMultiHead(OS_MEM_HEAD_ADDR(pool), listNodeHead)) {
        LOS_DL_LIST_FOR_EACH_ENTRY(tmpNode, listNodeHead, LosMemDynNode, selfNode.freeNodeInfo) {
#ifdef LOSCFG_MEM_HEAD_BACKUP
            if (!OsMemChecksumVerify(&tmpNode->selfNode)) {
                PRINT_ERR("[%s]: the node information of current node is bad !!\n", __FUNCTION__);
                OsMemDispCtlNode(&tmpNode->selfNode);
                ret = OsMemBackupRestore(pool, tmpNode);
            }
            if (ret != LOS_OK) {
                break;
            }
#endif

#ifdef LOSCFG_MEM_DEBUG
            if (((UINTPTR)tmpNode < (UINTPTR)pool) ||
                ((UINTPTR)tmpNode > ((UINTPTR)pool + ((LosMemPoolInfo *)pool)->poolSize)) ||
                (((UINTPTR)tmpNode & (OS_MEM_ALIGN_SIZE - 1)) != 0)) {
                LOS_PANIC("[%s:%d]Mem node data error:OS_MEM_HEAD_ADDR(pool)=%p, listNodeHead:%p,"
                          "allocSize=%u, tmpNode=%p\n",
                          __FUNCTION__, __LINE__, OS_MEM_HEAD_ADDR(pool), listNodeHead, allocSize, tmpNode);
                break;
            }
#endif
            if (tmpNode->selfNode.sizeAndFlag >= allocSize) {
                return tmpNode;
            }
        }
    }

    return NULL;
}

/*
 * Description : clear a mem node, set every member to NULL
 * Input       : node    --- Pointer to the mem node which will be cleared up
 */
STATIC INLINE VOID OsMemClearNode(LosMemDynNode *node)
{
    (VOID)memset((VOID *)node, 0, sizeof(LosMemDynNode));
}

/*
 * Description : merge this node and pre node, then clear this node info
 * Input       : node    --- Pointer to node which will be merged
 */
STATIC INLINE VOID OsMemMergeNode(LosMemDynNode *node)
{
    LosMemDynNode *nextNode = NULL;

    node->selfNode.preNode->selfNode.sizeAndFlag += node->selfNode.sizeAndFlag;
    nextNode = (LosMemDynNode *)((UINTPTR)node + node->selfNode.sizeAndFlag);
    nextNode->selfNode.preNode = node->selfNode.preNode;
#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node->selfNode.preNode);
    OsMemNodeSave(nextNode);
#endif
    OsMemClearNode(node);
}

/*
 * Description : split new node from allocNode, and merge remainder mem if necessary
 * Input       : pool      -- Pointer to memory pool
 *               allocNode -- the source node which the new node will split from it.
 *                            After pick up it's node info, change to point to the new node
 *               allocSize -- the size of new node
 * Output      : allocNode -- save new node addr
 */
STATIC INLINE VOID OsMemSplitNode(VOID *pool,
                                  LosMemDynNode *allocNode, UINT32 allocSize)
{
    LosMemDynNode *newFreeNode = NULL;
    LosMemDynNode *nextNode = NULL;
    LOS_DL_LIST *listNodeHead = NULL;
    const VOID *firstNode = (const VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE);

    newFreeNode = (LosMemDynNode *)(VOID *)((UINT8 *)allocNode + allocSize);
    newFreeNode->selfNode.preNode = allocNode;
    newFreeNode->selfNode.sizeAndFlag = allocNode->selfNode.sizeAndFlag - allocSize;
    allocNode->selfNode.sizeAndFlag = allocSize;
    nextNode = OS_MEM_NEXT_NODE(newFreeNode);
    nextNode->selfNode.preNode = newFreeNode;
    BOOL isNextNodeFree = !OS_MEM_NODE_GET_USED_FLAG(nextNode->selfNode.sizeAndFlag);
    if (isNextNodeFree) {
        OsMemListDelete(&nextNode->selfNode.freeNodeInfo, firstNode);
        OsMemMergeNode(nextNode);
    }
#ifdef LOSCFG_MEM_HEAD_BACKUP
    else {
        OsMemNodeSave(nextNode);
    }
#endif
    listNodeHead = OS_MEM_HEAD(pool, newFreeNode->selfNode.sizeAndFlag);
    OS_CHECK_NULL_RETURN(listNodeHead);

    OsMemListAdd(listNodeHead, &newFreeNode->selfNode.freeNodeInfo, firstNode);
#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(newFreeNode);
#endif
}

/*
 * Description : free the node from memory & if there are free node beside, merger them.
 *               at last update "listNodeHead' which saved all free node control head
 * Input       : node -- the node which need be freed
 *               pool -- Pointer to memory pool
 */
STATIC INLINE VOID OsMemFreeNode(LosMemDynNode *node, LosMemPoolInfo *pool)
{
    LosMemDynNode *nextNode = NULL;
    LOS_DL_LIST *listNodeHead = NULL;
    const VOID *firstNode = (const VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE);

    OS_MEM_REDUCE_USED(&pool->stat, OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag), OS_MEM_TASKID_GET(node));
    node->selfNode.sizeAndFlag = OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);
#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
#ifdef LOSCFG_MEM_LEAKCHECK
    OsMemLinkRegisterRecord(node);
#endif
#ifdef LOSCFG_KERNEL_LMS
    LosMemDynNode *nextNodeBackup = OS_MEM_NEXT_NODE(node);
    if (g_lms != NULL) {
        g_lms->check((UINTPTR)node + OS_MEM_NODE_HEAD_SIZE, TRUE);
    }
#endif
    if ((node->selfNode.preNode != NULL) &&
        !OS_MEM_NODE_GET_USED_FLAG(node->selfNode.preNode->selfNode.sizeAndFlag)) {
        LosMemDynNode *preNode = node->selfNode.preNode;
        OsMemMergeNode(node);
        nextNode = OS_MEM_NEXT_NODE(preNode);
        BOOL isNextNodeFree = !OS_MEM_NODE_GET_USED_FLAG(nextNode->selfNode.sizeAndFlag);
        if (isNextNodeFree) {
            OsMemListDelete(&nextNode->selfNode.freeNodeInfo, firstNode);
            OsMemMergeNode(nextNode);
        }

        OsMemListDelete(&(preNode->selfNode.freeNodeInfo), firstNode);
        listNodeHead = OS_MEM_HEAD(pool, preNode->selfNode.sizeAndFlag);
        OS_CHECK_NULL_RETURN(listNodeHead);

        OsMemListAdd(listNodeHead, &preNode->selfNode.freeNodeInfo, firstNode);
    } else {
        nextNode = OS_MEM_NEXT_NODE(node);
        BOOL isNextNodeFree = !OS_MEM_NODE_GET_USED_FLAG(nextNode->selfNode.sizeAndFlag);
        if (isNextNodeFree) {
            OsMemListDelete(&nextNode->selfNode.freeNodeInfo, firstNode);
            OsMemMergeNode(nextNode);
        }

        listNodeHead = OS_MEM_HEAD(pool, node->selfNode.sizeAndFlag);
        OS_CHECK_NULL_RETURN(listNodeHead);

        OsMemListAdd(listNodeHead, &node->selfNode.freeNodeInfo, firstNode);
    }
#ifdef LOSCFG_KERNEL_LMS
    if (g_lms != NULL) {
        g_lms->freeMark(node, nextNodeBackup, OS_MEM_NODE_HEAD_SIZE);
    }
#endif
}

#ifdef LOSCFG_MEM_DEBUG
/*
 * Description : check the result if pointer memory node belongs to pointer memory pool
 * Input       : pool -- Pointer to memory pool
 *               node -- the node which need be checked
 * Return      : LOS_OK or LOS_NOK
 */
STATIC INLINE BOOL OsMemIsNodeValid(const LosMemDynNode *node, const LosMemDynNode *startNode,
                                    const LosMemDynNode *endNode,
                                    const UINT8 *startPool, const UINT8 *endPool)
{
    if (!OS_MEM_MIDDLE_ADDR(startNode, node, endNode)) {
        return FALSE;
    }

    if (OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        if (!OS_MEM_MAGIC_VALID(node->selfNode.magic)) {
            return FALSE;
        }
        return TRUE;
    }

    if (!OS_MEM_MIDDLE_ADDR_OPEN_END(startPool, node->selfNode.freeNodeInfo.pstPrev, endPool)) {
        return FALSE;
    }

    if (node->selfNode.freeNodeInfo.pstPrev->pstNext != &node->selfNode.freeNodeInfo) {
        return FALSE;
    }

    return TRUE;
}

STATIC INLINE VOID OsMemCheckUsedNode(const VOID *pool, const LosMemDynNode *node)
{
    const LosMemPoolInfo *poolInfo = (const LosMemPoolInfo *)pool;
    const LosMemDynNode *startNode = (const LosMemDynNode *)OS_MEM_FIRST_NODE(pool);
    const LosMemDynNode *endNode = (const LosMemDynNode *)OS_MEM_END_NODE(pool, poolInfo->poolSize);
    const UINT8 *endPool = (const UINT8 *)pool + poolInfo->poolSize;
    const LosMemDynNode *nextNode = NULL;
    if (!OsMemIsNodeValid(node, startNode, endNode, (UINT8 *)pool, endPool)) {
        LOS_PANIC("cur node:%p has been damaged! pre node:%p \n", node, node->selfNode.preNode);
    }

    nextNode = OS_MEM_NEXT_NODE(node);
    if ((!OsMemIsNodeValid(nextNode, startNode, endNode, (UINT8 *)pool, endPool)) ||
        (nextNode->selfNode.preNode != node)) {
        LOS_PANIC("node:%p has been damaged! pre node:%p \n", nextNode, node);
    }

    if (node != startNode) {
        if ((!OsMemIsNodeValid(node->selfNode.preNode, startNode, endNode, (UINT8 *)pool, endPool)) ||
            (OS_MEM_NEXT_NODE(node->selfNode.preNode) != node)) {
            LOS_PANIC("The node:%p has been damaged!\n", node->selfNode.preNode);
        }
    }
}
#endif

#if (LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK == 1)
LITE_OS_SEC_TEXT_MINOR STATIC INLINE UINT32 OsMemPoolDlinkcheck(const LosMemPoolInfo *pool, LOS_DL_LIST listHead)
{
    if (((UINTPTR)listHead.pstPrev < (UINTPTR)(pool + 1)) ||
        ((UINTPTR)listHead.pstPrev >= ((UINTPTR)pool + pool->poolSize)) ||
        ((UINTPTR)listHead.pstNext < (UINTPTR)(pool + 1)) ||
        ((UINTPTR)listHead.pstNext >= ((UINTPTR)pool + pool->poolSize)) ||
        !IS_ALIGNED(listHead.pstPrev, sizeof(VOID *)) ||
        !IS_ALIGNED(listHead.pstNext, sizeof(VOID *))) {
        return LOS_NOK;
    }

    return LOS_OK;
}

/*
 * Description : show mem pool header info
 * Input       : pool --Pointer to memory pool
 */
LITE_OS_SEC_TEXT_MINOR STATIC VOID OsMemPoolHeadInfoPrint(const VOID *pool)
{
    const LosMemPoolInfo *poolInfo = (const LosMemPoolInfo *)pool;
    UINT32 dlinkNum;
    UINT32 flag = 0;
    const LosMultipleDlinkHead *dlinkHead = NULL;

    if (!IS_ALIGNED(poolInfo, sizeof(VOID *))) {
        EXCINFO_PRINT_ERR("wrong mem pool addr: %p, func:%s,line:%d\n", poolInfo, __FUNCTION__, __LINE__);
        return;
    }

    dlinkHead = (const LosMultipleDlinkHead *)(VOID *)(poolInfo + 1);
    for (dlinkNum = 0; dlinkNum < OS_MULTI_DLNK_NUM; dlinkNum++) {
        if (OsMemPoolDlinkcheck(pool, dlinkHead->listHead[dlinkNum])) {
            flag = 1;
            EXCINFO_PRINT_ERR("DlinkHead[%u]: pstPrev:%p, pstNext:%p\n",
                              dlinkNum, dlinkHead->listHead[dlinkNum].pstPrev, dlinkHead->listHead[dlinkNum].pstNext);
        }
    }
    if (flag) {
        EXCINFO_PRINTK("mem pool info: poolAddr:%p, poolSize:0x%x\n", poolInfo->pool, poolInfo->poolSize);
#ifdef LOSCFG_MEM_TASK_STAT
        EXCINFO_PRINTK("mem pool info: poolWaterLine:0x%x, poolCurUsedSize:0x%x\n", poolInfo->stat.memTotalPeak,
                       poolInfo->stat.memTotalUsed);
#endif
    }
}

STATIC UINT32 OsMemIntegrityCheck(const VOID *pool, LosMemDynNode **tmpNode, LosMemDynNode **preNode)
{
    const LosMemPoolInfo *poolInfo = (const LosMemPoolInfo *)pool;
    const UINT8 *endPool = (const UINT8 *)pool + poolInfo->poolSize;
    LosMemDynNode *endNode = NULL;

    OsMemPoolHeadInfoPrint(pool);

    *preNode = OS_MEM_FIRST_NODE(pool);
    endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    for (*tmpNode = OS_MEM_FIRST_NODE(pool); *tmpNode < endNode;
         *tmpNode = OS_MEM_NEXT_NODE(*tmpNode)) {
        if (OS_MEM_NODE_GET_USED_FLAG((*tmpNode)->selfNode.sizeAndFlag)) {
            if (!OS_MEM_MAGIC_VALID((*tmpNode)->selfNode.magic)) {
                EXCINFO_PRINT_ERR("[%s], %d, memory check error!\n"
                                  "memory used but magic num wrong, freeNodeInfo.pstPrev(magic num):%p \n",
                                  __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstPrev);
                return LOS_NOK;
            }
        } else { /* is free node, check free node range */
            if (!OS_MEM_MIDDLE_ADDR_OPEN_END(pool, (*tmpNode)->selfNode.freeNodeInfo.pstPrev, endPool)) {
                EXCINFO_PRINT_ERR("[%s], %d, memory check error!\n"
                                  " freeNodeInfo.pstPrev:%p is out of legal mem range[%p, %p]\n",
                                  __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstPrev, pool, endPool);
                return LOS_NOK;
            }
            if (!OS_MEM_MIDDLE_ADDR_OPEN_END(pool, (*tmpNode)->selfNode.freeNodeInfo.pstNext, endPool)) {
                EXCINFO_PRINT_ERR("[%s], %d, memory check error!\n"
                                  " freeNodeInfo.pstNext:%p is out of legal mem range[%p, %p]\n",
                                  __FUNCTION__, __LINE__, (*tmpNode)->selfNode.freeNodeInfo.pstNext, pool, endPool);
                return LOS_NOK;
            }
        }

        *preNode = *tmpNode;
    }
    return LOS_OK;
}

STATIC VOID OsMemNodeInfo(const LosMemDynNode *tmpNode,
                          const LosMemDynNode *preNode)
{
    if (tmpNode == preNode) {
        EXCINFO_PRINTK("\n the broken node is the first node\n");
    }
    EXCINFO_PRINTK("\n broken node head: %p  %p  %p  0x%x, pre node head: %p  %p  %p  0x%x\n",
                   tmpNode->selfNode.freeNodeInfo.pstPrev, tmpNode->selfNode.freeNodeInfo.pstNext,
                   tmpNode->selfNode.preNode, tmpNode->selfNode.sizeAndFlag,
                   preNode->selfNode.freeNodeInfo.pstPrev, preNode->selfNode.freeNodeInfo.pstNext,
                   preNode->selfNode.preNode, preNode->selfNode.sizeAndFlag);
#ifdef LOSCFG_MEM_LEAKCHECK
    OsMemNodeBacktraceInfo(tmpNode, preNode);
#endif

    PRINTK("\n---------------------------------------------\n");
    UINTPTR dumpEnd = (((UINTPTR)tmpNode + NODEDUMPSIZE) > (UINTPTR)tmpNode) ?
                      ((UINTPTR)tmpNode + NODEDUMPSIZE) : (UINTPTR)(UINTPTR_MAX);
    UINT32 dumpSize = (UINT32)(dumpEnd - (UINTPTR)tmpNode);
    PRINTK(" dump mem tmpNode:%p ~ %p\n", tmpNode, (VOID *)dumpEnd);
    OsDumpMemByte(dumpSize, (UINTPTR)tmpNode);
    PRINTK("\n---------------------------------------------\n");
    if (preNode != tmpNode) {
        PRINTK(" dump mem :%p ~ tmpNode:%p\n", (VOID *)((UINTPTR)tmpNode - NODEDUMPSIZE), tmpNode);
        OsDumpMemByte(NODEDUMPSIZE, ((UINTPTR)tmpNode - NODEDUMPSIZE));
        PRINTK("\n---------------------------------------------\n");
    }
}

STATIC VOID OsMemIntegrityCheckError(const LosMemDynNode *tmpNode,
                                     const LosMemDynNode *preNode)
{
    LosTaskCB *taskCB = NULL;
    UINT32 taskId;

    OsMemNodeInfo(tmpNode, preNode);

    taskId = OS_MEM_TASKID_GET(preNode);
    if (taskId >= g_taskMaxNum) {
        EXCINFO_PANIC("Task ID %u in pre node is invalid!\n", taskId);
        return;
    }

    taskCB = OS_TCB_FROM_TID(taskId);
    if ((taskCB->taskStatus & OS_TASK_STATUS_UNUSED) ||
        (taskCB->taskEntry == NULL) || (taskCB->taskName == NULL)) {
        EXCINFO_PANIC("\r\nTask ID %u in pre node is not created or deleted!\n", taskId);
        return;
    }
    EXCINFO_PANIC("cur node: %p\npre node: %p\npre node was allocated by task:%s\n",
                  tmpNode, preNode, taskCB->taskName);
    return;
}

/*
 * Description : memory pool integrity checking
 * Input       : pool --Pointer to memory pool
 * Return      : LOS_OK --memory pool integrate or LOS_NOK--memory pool impaired
 */
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemIntegrityCheck(const VOID *pool)
{
    LosMemDynNode *tmpNode = NULL;
    LosMemDynNode *preNode = NULL;
    UINT32 intSave;

    if (pool == NULL) {
        return LOS_NOK;
    }

    MEM_LOCK(intSave, pool);
    if (OsMemIntegrityCheck(pool, &tmpNode, &preNode)) {
        goto ERROR_OUT;
    }
    MEM_UNLOCK(intSave, pool);
    return LOS_OK;

ERROR_OUT:
    OsMemIntegrityCheckError(tmpNode, preNode);
    MEM_UNLOCK(intSave, pool);
    return LOS_NOK;
}

VOID OsMemIntegrityMultiCheck(VOID)
{
    if (LOS_MemIntegrityCheck(m_aucSysMem1) == LOS_OK) {
        EXCINFO_PRINTK("system memcheck over, all passed!\n");
    }
#ifdef LOSCFG_EXC_INTERACTION
    if (LOS_MemIntegrityCheck(m_aucSysMem0) == LOS_OK) {
        EXCINFO_PRINTK("exc interaction memcheck over, all passed!\n");
    }
#endif
}
#else
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemIntegrityCheck(const VOID *pool)
{
    (VOID)pool;
    return LOS_OK;
}

VOID OsMemIntegrityMultiCheck(VOID)
{
}
#endif

STATIC INLINE VOID OsMemNodeDebugOperate(LosMemDynNode *allocNode)
{
#if !defined(LOSCFG_MEM_HEAD_BACKUP) && !defined(LOSCFG_MEM_LEAKCHECK)
    (VOID)allocNode;
#endif

#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(allocNode);
#endif

#ifdef LOSCFG_MEM_LEAKCHECK
    OsMemLinkRegisterRecord(allocNode);
#endif
}

STATIC UINT32 OsMemInfoGet(const VOID *pool, LOS_MEM_POOL_STATUS *poolStatus)
{
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *tmpNode = NULL;
    LosMemDynNode *endNode = NULL;
    UINT32 totalUsedSize = sizeof(LosMemPoolInfo) + OS_MULTI_DLNK_HEAD_SIZE;
    UINT32 totalFreeSize = 0;
    UINT32 maxFreeNodeSize = 0;
    UINT32 usedNodeNum = 0;
    UINT32 freeNodeNum = 0;

#if defined(LOSCFG_MEM_DEBUG) || defined(LOSCFG_MEM_TASK_STAT)
    tmpNode = (LosMemDynNode *)OS_MEM_END_NODE(pool, poolInfo->poolSize);
    tmpNode = (LosMemDynNode *)OS_MEM_ALIGN(tmpNode, OS_MEM_ALIGN_SIZE);
    if (!OS_MEM_MAGIC_VALID(tmpNode->selfNode.magic)) {
        PRINT_ERR("wrong mem pool addr! line:%d\n", __LINE__);
        PRINT_DEBUG("wrong addr: %p\n", poolInfo);
        return LOS_NOK;
    }
#endif

    endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    for (tmpNode = OS_MEM_FIRST_NODE(poolInfo); tmpNode < endNode; tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (!OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
            ++freeNodeNum;
            totalFreeSize += OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag);
            if (maxFreeNodeSize < OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag)) {
                maxFreeNodeSize = OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag);
            }
        } else {
            ++usedNodeNum;
            totalUsedSize += OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag);
        }
    }

    poolStatus->totalUsedSize = totalUsedSize;
    poolStatus->totalFreeSize = totalFreeSize;
    poolStatus->maxFreeNodeSize = maxFreeNodeSize;
    poolStatus->usedNodeNum = usedNodeNum;
    poolStatus->freeNodeNum = freeNodeNum;
#ifdef LOSCFG_MEM_TASK_STAT
    poolStatus->usageWaterLine = poolInfo->stat.memTotalPeak;
#endif

    return LOS_OK;
}

VOID OsMemInfoPrint(const VOID *pool)
{
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LOS_MEM_POOL_STATUS status = {0};

    if (OsMemInfoGet(pool, &status) == LOS_NOK) {
        return;
    }

#ifdef LOSCFG_MEM_TASK_STAT
    PRINTK("pool addr          pool size    used size     free size    "
           "max free node size   used node num     free node num      UsageWaterLine\n");
    PRINTK("---------------    --------     -------       --------     "
           "--------------       -------------      ------------      ------------\n");
    PRINTK("0x%-14lx   0x%-8x   0x%-8x    0x%-8x   0x%-16x   0x%-13x    0x%-13x    0x%-13x\n",
           (UINTPTR)poolInfo->pool, poolInfo->poolSize, status.totalUsedSize,
           status.totalFreeSize, status.maxFreeNodeSize, status.usedNodeNum,
           status.freeNodeNum, status.usageWaterLine);

#else
    PRINTK("pool addr          pool size    used size     free size    "
           "max free node size   used node num     free node num\n");
    PRINTK("---------------    --------     -------       --------     "
           "--------------       -------------      ------------\n");
    PRINTK("%-16p   0x%-8x   0x%-8x    0x%-8x   0x%-16x   0x%-13x    0x%-13x\n",
           poolInfo->pool, poolInfo->poolSize, status.totalUsedSize,
           status.totalFreeSize, status.maxFreeNodeSize, status.usedNodeNum,
           status.freeNodeNum);
#endif
}

STATIC INLINE VOID OsMemInfoAlert(const VOID *pool, UINT32 allocSize)
{
#ifdef LOSCFG_MEM_DEBUG
    PRINT_ERR("---------------------------------------------------"
              "--------------------------------------------------------\n");
    OsMemInfoPrint(pool);
    PRINT_ERR("[%s] No suitable free block, require free node size: 0x%x\n", __FUNCTION__, allocSize);
    PRINT_ERR("----------------------------------------------------"
              "-------------------------------------------------------\n");
#else
    (VOID)pool;
    (VOID)allocSize;
#endif
}

/*
 * Description : Allocate node from Memory pool
 * Input       : pool  --- Pointer to memory pool
 *               size  --- Size of memory in bytes to allocate
 * Return      : Pointer to allocated memory
 */
STATIC VOID *OsMemAllocWithCheck(LosMemPoolInfo *pool, UINT32 size)
{
    LosMemDynNode *allocNode = NULL;
    UINT32 allocSize;

#if (LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK == 1)
    LosMemDynNode *tmpNode = NULL;
    LosMemDynNode *preNode = NULL;
#endif
    const VOID *firstNode = (const VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE);

#if (LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK == 1)
    if (OsMemIntegrityCheck(pool, &tmpNode, &preNode)) {
        OsMemIntegrityCheckError(tmpNode, preNode);
        return NULL;
    }
#endif

    allocSize = OS_MEM_ALIGN(size + OS_MEM_NODE_HEAD_SIZE, OS_MEM_ALIGN_SIZE);
    allocNode = OsMemFindSuitableFreeBlock(pool, allocSize);
    if (allocNode == NULL) {
        OsMemInfoAlert(pool, allocSize);
        return NULL;
    }
    if ((allocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= allocNode->selfNode.sizeAndFlag) {
        OsMemSplitNode(pool, allocNode, allocSize);
    }
    OsMemListDelete(&allocNode->selfNode.freeNodeInfo, firstNode);
    OsMemSetMagicNumAndTaskID(allocNode);
    OS_MEM_NODE_SET_USED_FLAG(allocNode->selfNode.sizeAndFlag);
    OS_MEM_ADD_USED(&pool->stat, OS_MEM_NODE_GET_SIZE(allocNode->selfNode.sizeAndFlag),
                    OS_MEM_TASKID_GET(allocNode));
    OsMemNodeDebugOperate(allocNode);

#ifdef LOSCFG_KERNEL_LMS
    if (g_lms != NULL) {
        g_lms->mallocMark(allocNode, OS_MEM_NEXT_NODE(allocNode), OS_MEM_NODE_HEAD_SIZE);
    }
#endif
    return (allocNode + 1);
}

/*
 * Description : reAlloc a smaller memory node
 * Input       : pool      --- Pointer to memory pool
 *               allocSize --- the size of new node which will be alloced
 *               node      --- the node which will be reallocated
 *               nodeSize  --- the size of old node
 * Output      : node      --- pointer to the new node after realloc
 */
STATIC INLINE VOID OsMemReAllocSmaller(LosMemPoolInfo *pool, UINT32 allocSize, LosMemDynNode *node, UINT32 nodeSize)
{
    if ((allocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= nodeSize) {
        node->selfNode.sizeAndFlag = nodeSize;
        OsMemSplitNode(pool, node, allocSize);
        OS_MEM_NODE_SET_USED_FLAG(node->selfNode.sizeAndFlag);
#ifdef LOSCFG_MEM_HEAD_BACKUP
        OsMemNodeSave(node);
#endif

        OS_MEM_REDUCE_USED(&pool->stat, nodeSize - allocSize, OS_MEM_TASKID_GET(node));
#ifdef LOSCFG_KERNEL_LMS
        OsLmsReallocSplitNodeMark(node);
    } else {
        OsLmsReallocResizeMark(node, allocSize);
#endif
    }

#ifdef LOSCFG_MEM_LEAKCHECK
    OsMemLinkRegisterRecord(node);
#endif
}

/*
 * Description : reAlloc a Bigger memory node after merge node and nextNode
 * Input       : pool      --- Pointer to memory pool
 *               allocSize --- the size of new node which will be alloced
 *               node      --- the node which will be reallocated
 *               nodeSize  --- the size of old node
 *               nextNode  --- pointer next node which will be merged
 * Output      : node      --- pointer to the new node after realloc
 */
STATIC INLINE VOID OsMemMergeNodeForReAllocBigger(LosMemPoolInfo *pool, UINT32 allocSize, LosMemDynNode *node,
                                                  UINT32 nodeSize, LosMemDynNode *nextNode)
{
    const VOID *firstNode = (const VOID *)((UINT8 *)OS_MEM_HEAD_ADDR(pool) + OS_DLNK_HEAD_SIZE);

    node->selfNode.sizeAndFlag = nodeSize;
    OsMemListDelete(&nextNode->selfNode.freeNodeInfo, firstNode);
    OsMemMergeNode(nextNode);

#ifdef LOSCFG_KERNEL_LMS
    OsLmsReallocMergeNodeMark(node);
#endif
    if ((allocSize + OS_MEM_NODE_HEAD_SIZE + OS_MEM_ALIGN_SIZE) <= node->selfNode.sizeAndFlag) {
        OsMemSplitNode(pool, node, allocSize);
#ifdef LOSCFG_KERNEL_LMS
        OsLmsReallocSplitNodeMark(node);
    } else {
        OsLmsReallocResizeMark(node, allocSize);
#endif
    }
    OS_MEM_ADD_USED(&pool->stat, node->selfNode.sizeAndFlag - nodeSize, OS_MEM_TASKID_GET(node));

    OS_MEM_NODE_SET_USED_FLAG(node->selfNode.sizeAndFlag);
#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(node);
#endif
#ifdef LOSCFG_MEM_LEAKCHECK
    OsMemLinkRegisterRecord(node);
#endif
}

STATIC UINT32 OsMemInit(VOID *pool, UINT32 size)
{
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *newNode = NULL;
    LosMemDynNode *endNode = NULL;
    LOS_DL_LIST *listNodeHead = NULL;
    UINT32 poolSize = size;

#ifdef LOSCFG_KERNEL_LMS
    UINT32 resize = 0;
    if (g_lms != NULL) {
        /*
         * resize == 0, shadow memory init failed, no shadow memory for this pool, set poolSize as original size.
         * resize != 0, shadow memory init successful, set poolSize as resize.
         */
        resize = g_lms->init(pool, size);
        poolSize = (resize == 0) ? size : resize;
        if (poolSize < OS_MEM_MIN_POOL_SIZE) {
            return LOS_NOK;
        }
    }
#endif
    poolInfo->pool = pool;
    poolInfo->poolSize = poolSize;

    OsDLnkInitMultiHead(OS_MEM_HEAD_ADDR(pool));
    newNode = OS_MEM_FIRST_NODE(pool);
    newNode->selfNode.sizeAndFlag = (poolSize - (UINT32)((UINTPTR)newNode - (UINTPTR)pool) - OS_MEM_NODE_HEAD_SIZE);
    newNode->selfNode.preNode = (LosMemDynNode *)OS_MEM_END_NODE(pool, poolSize);
    listNodeHead = OS_MEM_HEAD(pool, newNode->selfNode.sizeAndFlag);
    if (listNodeHead == NULL) {
        return LOS_NOK;
    }

    LOS_ListTailInsert(listNodeHead, &(newNode->selfNode.freeNodeInfo));
    endNode = (LosMemDynNode *)OS_MEM_END_NODE(pool, poolSize);
    (VOID)memset(endNode, 0, sizeof(*endNode));
    endNode->selfNode.preNode = newNode;
    endNode->selfNode.sizeAndFlag = OS_MEM_NODE_HEAD_SIZE;
    OS_MEM_NODE_SET_USED_FLAG(endNode->selfNode.sizeAndFlag);
    OsMemSetMagicNumAndTaskID(endNode);
#ifdef LOSCFG_MEM_TASK_STAT
    UINT32 statSize = sizeof(poolInfo->stat);
    (VOID)memset(&poolInfo->stat, 0, statSize);
    poolInfo->stat.memTotalUsed = sizeof(LosMemPoolInfo) + OS_MULTI_DLNK_HEAD_SIZE +
                                  OS_MEM_NODE_GET_SIZE(endNode->selfNode.sizeAndFlag);
    poolInfo->stat.memTotalPeak = poolInfo->stat.memTotalUsed;
    poolInfo->stat.taskMemstats = g_osTaskMemstats;
#endif

#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(newNode);
    OsMemNodeSave(endNode);
#endif
#ifdef LOSCFG_KERNEL_LMS
    if (resize != 0) { /* mark first node only when shadow memory init successful */
        OsLmsFirstNodeMark(pool, newNode);
    }
#endif
    return LOS_OK;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_MemPoolInit(VOID *pool, UINT32 size, UINT32 attr)
{
    UINT32 intSave;
    UINT32 alignSize = size;

    if ((pool == NULL) || (size < OS_MEM_MIN_POOL_SIZE) || (size > OS_MEM_MAX_POOL_SIZE) ||
        (((UINTPTR)pool + (UINTPTR)size) < (UINTPTR)pool)) {
        return LOS_NOK;
    }

    if (!IS_ALIGNED(size, OS_MEM_ALIGN_SIZE) || !IS_ALIGNED(pool, OS_MEM_ALIGN_SIZE)) {
        PRINT_WARN("pool size 0x%x should be aligned with OS_MEM_ALIGN_SIZE\n", size);
        alignSize = (UINT32)(OS_MEM_ALIGN(size, OS_MEM_ALIGN_SIZE) - OS_MEM_ALIGN_SIZE);
    }

    MEM_LOCK(intSave, pool);
    if (OsMemMulPoolInit(pool, alignSize)) {
        MEM_UNLOCK(intSave, pool);
        return LOS_NOK;
    }

    if (OsMemInit(pool, alignSize) != LOS_OK) {
        (VOID)OsMemMulPoolDeinit(pool);
        MEM_UNLOCK(intSave, pool);
        return LOS_NOK;
    }

    OsSlabMemProcInitFlag(pool, alignSize, attr & LOS_MEM_INIT_ATTR_SLAB_MSK);
    MEM_UNLOCK(intSave, pool);

    LOS_TRACE(MEM_INFO_REQ, pool, size);
    return LOS_OK;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_MemInit(VOID *pool, UINT32 size)
{
    return LOS_MemPoolInit(pool, size, LOS_MEM_INIT_ATTR_SLAB_MSK);
}

LITE_OS_SEC_TEXT VOID *LOS_MemAlloc(VOID *pool, UINT32 size)
{
    VOID *ptr = NULL;
    UINT32 intSave;
    MALLOC_HOOK mallocHook;

    if ((pool == NULL) || (size == 0) ||
        OS_MEM_NODE_GET_USED_FLAG(size) ||
        OS_MEM_NODE_GET_ALIGNED_FLAG(size)) {
        return NULL;
    }

    mallocHook = g_MALLOC_HOOK;
    if (mallocHook != NULL) {
        mallocHook();
    }
    MEM_LOCK(intSave, pool);
    ptr = OsSlabMemAlloc(pool, size);
    if (ptr == NULL) {
        ptr = OsMemAllocWithCheck(pool, size);
    }
    MEM_UNLOCK(intSave, pool);

    LOS_TRACE(MEM_ALLOC, pool, (UINTPTR)ptr, size);
    return ptr;
}

UINT32 OsPoolInfoGet(VOID *pool, UINT32 *usedNodeNum, UINT32 *freeNodeNum, UINT32 *peak)
{
    LOS_MEM_POOL_STATUS status = {0};

    if (OsMemInfoGet(pool, &status) == LOS_NOK) {
        return LOS_NOK;
    }
    *usedNodeNum = status.usedNodeNum;
    *freeNodeNum = status.freeNodeNum;
#ifdef LOSCFG_MEM_TASK_STAT
    *peak = status.usageWaterLine;
#else
    (VOID)peak;
    PRINTK("Without LOSCFG_MEM_TASK_STAT, peak not tracked\n");
#endif
    return LOS_OK;
}

LITE_OS_SEC_TEXT VOID *LOS_MemAllocAlign(VOID *pool, UINT32 size, UINT32 boundary)
{
    UINT32 useSize;
    UINT32 gapSize;
    VOID *ptr = NULL;
    VOID *alignedPtr = NULL;
    LosMemDynNode *allocNode = NULL;
    UINT32 intSave;

    if ((pool == NULL) || (size == 0) || (boundary == 0) || !IS_POW_TWO(boundary) ||
        !IS_ALIGNED(boundary, sizeof(VOID *))) {
        return NULL;
    }

    MEM_LOCK(intSave, pool);
    do {
        /*
         * sizeof(gapSize) bytes stores offset between alignedPtr and ptr,
         * the ptr has been OS_MEM_ALIGN_SIZE(4 or 8) aligned, so maximum
         * offset between alignedPtr and ptr is boundary - OS_MEM_ALIGN_SIZE
         */
        if ((boundary - sizeof(gapSize)) > ((UINT32)(-1) - size)) {
            break;
        }

        useSize = (size + boundary) - sizeof(gapSize);
        if (OS_MEM_NODE_GET_USED_FLAG(useSize) || OS_MEM_NODE_GET_ALIGNED_FLAG(useSize)) {
            break;
        }

        ptr = OsMemAllocWithCheck(pool, useSize);
        if (ptr == NULL) {
            break;
        }

        alignedPtr = (VOID *)OS_MEM_ALIGN(ptr, boundary);
        if (ptr == alignedPtr) {
#ifdef LOSCFG_KERNEL_LMS
            OsLmsAllocAlignMark(ptr, alignedPtr, size);
#endif
            break;
        }

        /* store gapSize in address (ptr -4), it will be checked while free */
        gapSize = (UINT32)((UINTPTR)alignedPtr - (UINTPTR)ptr);
        allocNode = (LosMemDynNode *)ptr - 1;
        OS_MEM_NODE_SET_ALIGNED_FLAG(allocNode->selfNode.sizeAndFlag);

#ifdef LOSCFG_MEM_HEAD_BACKUP
        OsMemNodeSaveWithGapSize(allocNode, gapSize);
#endif

        OS_MEM_NODE_SET_ALIGNED_FLAG(gapSize);
        *(UINT32 *)((UINTPTR)alignedPtr - sizeof(gapSize)) = gapSize;

#ifdef LOSCFG_KERNEL_LMS
        OsLmsAllocAlignMark(ptr, alignedPtr, size);
#endif
        ptr = alignedPtr;
    } while (0);

    MEM_UNLOCK(intSave, pool);

    LOS_TRACE(MEM_ALLOC_ALIGN, pool, (UINTPTR)ptr, size, boundary);

    return ptr;
}

LosMemDynNode *OsGetRealPtr(const VOID *pool, const VOID *ptr)
{
    LosMemDynNode *realPtr = (LosMemDynNode *)((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE);
    UINT32 gapSize;
    UINT32 sizeAndFlag = *((UINT32 *)((UINTPTR)ptr - sizeof(gapSize)));

    if (!OS_MEM_NODE_GET_ALIGNED_FLAG(sizeAndFlag)) {
        return realPtr;
    }

    /* ptr is allocated by alignment. */
    if (OS_MEM_NODE_GET_USED_FLAG(sizeAndFlag)) {
        PRINT_ERR("[%s:%d]sizeAndFlag:0x%x error\n", __FUNCTION__, __LINE__, sizeAndFlag);
        return NULL;
    }

    gapSize = OS_MEM_NODE_GET_ALIGNED_GAPSIZE(sizeAndFlag);
    if ((gapSize & (OS_MEM_ALIGN_SIZE - 1)) ||
        (gapSize > ((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE - (UINTPTR)pool))) {
        PRINT_ERR("illegal gapSize: 0x%x\n", gapSize);
        return realPtr;
    }
    realPtr = (LosMemDynNode *)((UINTPTR)ptr - gapSize - OS_MEM_NODE_HEAD_SIZE);
    return realPtr;
}

UINT32 OsMemFree(VOID *pool, const VOID *ptr)
{
    LosMemDynNode *node = OsGetRealPtr(pool, ptr);

    if (node == NULL) {
        return LOS_NOK;
    }

#ifdef LOSCFG_MEM_HEAD_BACKUP
    UINT32 ret = OsMemBackupCheckAndRestore(pool, ptr, node);
    if (ret != LOS_OK) {
        return LOS_NOK;
    }
#endif

    if (!OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
#ifdef LOSCFG_MEM_DEBUG
        LOS_PANIC("The node:%p is not used!\n", node);
#endif
        return LOS_NOK;
    }

#ifdef LOSCFG_MEM_DEBUG
    OsMemCheckUsedNode(pool, node);
#endif
    OsMemFreeNode(node, pool);

    return LOS_OK;
}

LITE_OS_SEC_TEXT UINT32 LOS_MemFree(VOID *pool, VOID *ptr)
{
    UINT32 ret;
    UINT32 intSave;

    if ((pool == NULL) || (ptr == NULL) ||
        !IS_ALIGNED(pool, sizeof(VOID *)) || !IS_ALIGNED(ptr, sizeof(VOID *))) {
        return LOS_NOK;
    }

    if ((ptr < pool) || ((UINT8 *)ptr >= (UINT8 *)pool + ((LosMemPoolInfo *)pool)->poolSize)) {
        return LOS_NOK;
    }

    MEM_LOCK(intSave, pool);

    if (OsSlabMemFree(pool, ptr)) {
        ret = LOS_OK;
        goto OUT;
    }

    ret = OsMemFree(pool, ptr);
OUT:
    MEM_UNLOCK(intSave, pool);

    LOS_TRACE(MEM_FREE, pool, (UINTPTR)ptr);
    return ret;
}

STATIC VOID *OsMemRealloc(VOID *pool, VOID *ptr, UINT32 size)
{
    LosMemDynNode *node = NULL;
    LosMemDynNode *nextNode = NULL;
    VOID *tmpPtr = NULL;
    UINT32 nodeSize;
    UINT32 allocSize = OS_MEM_ALIGN(size + OS_MEM_NODE_HEAD_SIZE, OS_MEM_ALIGN_SIZE);

    node = OsGetRealPtr(pool, ptr);
    if (node == NULL) {
        return NULL;
    }

#ifdef LOSCFG_MEM_HEAD_BACKUP
    UINT32 ret = OsMemBackupCheckAndRestore(pool, ptr, node);
    if (ret != LOS_OK) {
        return NULL;
    }
#endif

    if (!OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
#ifdef LOSCFG_MEM_DEBUG
        LOS_PANIC("The node:%p is not used!\n", node);
#endif
        return NULL;
    }

#ifdef LOSCFG_MEM_DEBUG
    OsMemCheckUsedNode(pool, node);
#endif

    nodeSize = OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);
    if (nodeSize >= allocSize) {
        OsMemReAllocSmaller(pool, allocSize, node, nodeSize);
        return (VOID *)ptr;
    }

    nextNode = OS_MEM_NEXT_NODE(node);
    if (!OS_MEM_NODE_GET_USED_FLAG(nextNode->selfNode.sizeAndFlag) &&
        ((nextNode->selfNode.sizeAndFlag + nodeSize) >= allocSize)) {
        OsMemMergeNodeForReAllocBigger(pool, allocSize, node, nodeSize, nextNode);
        return (VOID *)ptr;
    }

    tmpPtr = OsMemAllocWithCheck(pool, size);
    if (tmpPtr != NULL) {
        if (memcpy_s(tmpPtr, size, ptr, (nodeSize - ((UINTPTR)ptr - (UINTPTR)node))) != EOK) {
            (VOID)OsMemFree((VOID *)pool, (VOID *)tmpPtr);
            return NULL;
        }
        OsMemFreeNode(node, pool);
    }
    return tmpPtr;
}

LITE_OS_SEC_TEXT_MINOR VOID *LOS_MemRealloc(VOID *pool, VOID *ptr, UINT32 size)
{
    UINT32 intSave;
    VOID *newPtr = NULL;
    BOOL isSlabMem = FALSE;

    if (OS_MEM_NODE_GET_USED_FLAG(size) || OS_MEM_NODE_GET_ALIGNED_FLAG(size) || (pool == NULL)) {
        return NULL;
    }

    if (ptr == NULL) {
        newPtr = LOS_MemAlloc(pool, size);
        goto OUT;
    }

    if (size == 0) {
        (VOID)LOS_MemFree(pool, ptr);
        goto OUT;
    }

    MEM_LOCK(intSave, pool);

    newPtr = OsMemReallocSlab(pool, ptr, &isSlabMem, size);
    if (isSlabMem == TRUE) {
        goto OUT_UNLOCK;
    }

    newPtr = OsMemRealloc(pool, ptr, size);

OUT_UNLOCK:
    MEM_UNLOCK(intSave, pool);
OUT:
    LOS_TRACE(MEM_REALLOC, pool, (UINTPTR)ptr, size);

    return newPtr;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemTotalUsedGet(VOID *pool)
{
    LosMemDynNode *tmpNode = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *endNode = NULL;
    UINT32 memUsed = 0;
    UINT32 intSave;

    if (pool == NULL) {
        return LOS_NOK;
    }

    MEM_LOCK(intSave, pool);

    endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode <= endNode; tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
            memUsed += OS_MEM_NODE_GET_SIZE(tmpNode->selfNode.sizeAndFlag);
        }
    }

    MEM_UNLOCK(intSave, pool);

    return memUsed;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemUsedBlksGet(VOID *pool)
{
    LosMemDynNode *tmpNode = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *endNode = NULL;
    UINT32 blkNums = 0;
    UINT32 intSave;

    if (pool == NULL) {
        return OS_INVALID;
    }

    MEM_LOCK(intSave, pool);

    endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode < endNode; tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
            blkNums++;
        }
    }

    MEM_UNLOCK(intSave, pool);

    return blkNums;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemTaskIdGet(const VOID *ptr)
{
    LosMemDynNode *tmpNode = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)(VOID *)m_aucSysMem1;
    LosMemDynNode *endNode = NULL;
    UINT32 intSave;
#ifdef LOSCFG_EXC_INTERACTION
    if (ptr < (VOID *)m_aucSysMem1) {
        poolInfo = (LosMemPoolInfo *)(VOID *)m_aucSysMem0;
    }
#endif

    if ((ptr == NULL) || (ptr < (VOID *)OS_MEM_FIRST_NODE(poolInfo)) ||
        (ptr > (VOID *)OS_MEM_END_NODE(poolInfo, poolInfo->poolSize))) {
        PRINT_ERR("input ptr is out of system memory range\n");
        PRINT_DEBUG("ptr %p is not in range[%p, %p]\n", ptr, OS_MEM_FIRST_NODE(poolInfo),
                    OS_MEM_END_NODE(poolInfo, poolInfo->poolSize));
        return OS_INVALID;
    }

    MEM_LOCK(intSave, poolInfo);

    endNode = OS_MEM_END_NODE(poolInfo, poolInfo->poolSize);
    for (tmpNode = OS_MEM_FIRST_NODE(poolInfo); tmpNode <= endNode; tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if ((UINTPTR)ptr < (UINTPTR)tmpNode) {
            if (OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.preNode->selfNode.sizeAndFlag)) {
                MEM_UNLOCK(intSave, poolInfo);
                return tmpNode->selfNode.preNode->selfNode.taskId;
            } else {
                MEM_UNLOCK(intSave, poolInfo);
                PRINT_ERR("input ptr is belong to a free mem node\n");
                PRINT_DEBUG("ptr address: %p\n", ptr);
                return OS_INVALID;
            }
        }
    }

    MEM_UNLOCK(intSave, poolInfo);
    return OS_INVALID;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemFreeBlksGet(VOID *pool)
{
    LosMemDynNode *tmpNode = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *endNode = NULL;
    UINT32 blkNums = 0;
    UINT32 intSave;

    if (pool == NULL) {
        return LOS_NOK;
    }

    MEM_LOCK(intSave, pool);

    endNode = OS_MEM_END_NODE(pool, poolInfo->poolSize);
    for (tmpNode = OS_MEM_FIRST_NODE(pool); tmpNode <= endNode; tmpNode = OS_MEM_NEXT_NODE(tmpNode)) {
        if (!OS_MEM_NODE_GET_USED_FLAG(tmpNode->selfNode.sizeAndFlag)) {
            blkNums++;
        }
    }

    MEM_UNLOCK(intSave, pool);

    return blkNums;
}

LITE_OS_SEC_TEXT_MINOR UINTPTR LOS_MemLastUsedGet(VOID *pool)
{
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    LosMemDynNode *node = NULL;

    if (pool == NULL) {
        return LOS_NOK;
    }

    node = OS_MEM_END_NODE(pool, poolInfo->poolSize)->selfNode.preNode;
    if (OS_MEM_NODE_GET_USED_FLAG(node->selfNode.sizeAndFlag)) {
        return (UINTPTR)((CHAR *)node + OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag) + sizeof(LosMemDynNode));
    } else {
        return (UINTPTR)((CHAR *)node + sizeof(LosMemDynNode));
    }
}

/*
 * Description : reset "end node"
 * Input       : pool    --- Pointer to memory pool
 *               preAddr --- Pointer to the pre Pointer of end node
 * Output      : endNode --- pointer to "end node"
 * Return      : the number of free node
 */
LITE_OS_SEC_TEXT_MINOR VOID OsMemResetEndNode(VOID *pool, UINTPTR preAddr)
{
    LosMemDynNode *endNode = (LosMemDynNode *)OS_MEM_END_NODE(pool, ((LosMemPoolInfo *)pool)->poolSize);
    endNode->selfNode.sizeAndFlag = OS_MEM_NODE_HEAD_SIZE;
    if (preAddr != 0) {
        endNode->selfNode.preNode = (LosMemDynNode *)(preAddr - sizeof(LosMemDynNode));
    }
    OS_MEM_NODE_SET_USED_FLAG(endNode->selfNode.sizeAndFlag);
    OsMemSetMagicNumAndTaskID(endNode);

#ifdef LOSCFG_MEM_HEAD_BACKUP
    OsMemNodeSave(endNode);
#endif
}

UINT32 LOS_MemPoolSizeGet(const VOID *pool)
{
    if (pool == NULL) {
        return LOS_NOK;
    }
    return ((LosMemPoolInfo *)pool)->poolSize;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemInfoGet(VOID *pool, LOS_MEM_POOL_STATUS *poolStatus)
{
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    UINT32 ret;
    UINT32 intSave;

    if (poolStatus == NULL) {
        PRINT_ERR("can't use NULL addr to save info\n");
        return LOS_NOK;
    }

    if ((poolInfo == NULL) || ((UINTPTR)pool != (UINTPTR)poolInfo->pool)) {
        PRINT_ERR("wrong mem pool addr! line:%d\n", __LINE__);
        PRINT_DEBUG("wrong addr: %p\n", poolInfo);
        return LOS_NOK;
    }

    MEM_LOCK(intSave, pool);
    ret = OsMemInfoGet(poolInfo, poolStatus);
    MEM_UNLOCK(intSave, pool);

    return ret;
}

STATIC INLINE VOID OsShowFreeNode(UINT32 index, UINT32 length, const UINT32 *countNum)
{
    UINT32 count = 0;
    PRINTK("\n    block size:  ");
    while (count < length) {
        PRINTK("2^%-5u", (index + OS_MIN_MULTI_DLNK_LOG2 + count));
        count++;
    }
    PRINTK("\n    node number: ");
    count = 0;
    while (count < length) {
        PRINTK("  %-5u", countNum[count + index]);
        count++;
    }
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemFreeNodeShow(VOID *pool)
{
    LOS_DL_LIST *listNodeHead = NULL;
    LosMultipleDlinkHead *headAddr = (LosMultipleDlinkHead *)((UINTPTR)pool + sizeof(LosMemPoolInfo));
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    UINT32 linkHeadIndex;
    UINT32 countNum[OS_MULTI_DLNK_NUM] = { 0 };
    UINT32 intSave;

    if ((poolInfo == NULL) || ((UINTPTR)pool != (UINTPTR)poolInfo->pool)) {
        PRINT_ERR("wrong mem pool addr! line:%d\n", __LINE__);
        PRINT_DEBUG("wrong addr: %p\n", poolInfo);
        return LOS_NOK;
    }

    PRINTK("\n   ************************ left free node number**********************");
    MEM_LOCK(intSave, pool);

    for (linkHeadIndex = 0; linkHeadIndex <= (OS_MULTI_DLNK_NUM - 1);
         linkHeadIndex++) {
        listNodeHead = headAddr->listHead[linkHeadIndex].pstNext;
        while (listNodeHead != &(headAddr->listHead[linkHeadIndex])) {
            listNodeHead = listNodeHead->pstNext;
            countNum[linkHeadIndex]++;
        }
    }

    linkHeadIndex = 0;
    while (linkHeadIndex < OS_MULTI_DLNK_NUM) {
        if (linkHeadIndex + COLUMN_NUM < OS_MULTI_DLNK_NUM) {
            OsShowFreeNode(linkHeadIndex, COLUMN_NUM, countNum);
            linkHeadIndex += COLUMN_NUM;
        } else {
            OsShowFreeNode(linkHeadIndex, (OS_MULTI_DLNK_NUM - 1 - linkHeadIndex), countNum);
            break;
        }
    }

    MEM_UNLOCK(intSave, pool);
    PRINTK("\n   ********************************************************************\n\n");

    return LOS_OK;
}

#ifdef LOSCFG_BASE_MEM_NODE_SIZE_CHECK
LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemNodeSizeCheck(VOID *pool, VOID *ptr, UINT32 *totalSize, UINT32 *availSize)
{
    const VOID *head = NULL;
    LosMemPoolInfo *poolInfo = (LosMemPoolInfo *)pool;
    UINT8 *endPool = NULL;

    if (g_memCheckLevel == LOS_MEM_CHECK_LEVEL_DISABLE) {
        return LOS_ERRNO_MEMCHECK_DISABLED;
    }

    if ((pool == NULL) || (ptr == NULL) || (totalSize == NULL) || (availSize == NULL)) {
        return LOS_ERRNO_MEMCHECK_PARA_NULL;
    }

    endPool = (UINT8 *)pool + poolInfo->poolSize;
    if (!(OS_MEM_MIDDLE_ADDR_OPEN_END(pool, ptr, endPool))) {
        return LOS_ERRNO_MEMCHECK_OUTSIDE;
    }

    if (g_memCheckLevel == LOS_MEM_CHECK_LEVEL_HIGH) {
        head = OsMemFindNodeCtrl(pool, ptr);
        if ((head == NULL) ||
            (OS_MEM_NODE_GET_SIZE(((LosMemDynNode *)head)->selfNode.sizeAndFlag) < ((UINTPTR)ptr - (UINTPTR)head))) {
            return LOS_ERRNO_MEMCHECK_NO_HEAD;
        }
        *totalSize = (UINT32)OS_MEM_NODE_GET_SIZE(((LosMemDynNode *)head)->selfNode.sizeAndFlag -
                                                  sizeof(LosMemDynNode));
        *availSize = (UINT32)OS_MEM_NODE_GET_SIZE(((LosMemDynNode *)head)->selfNode.sizeAndFlag -
                                                  ((UINTPTR)ptr - (UINTPTR)head));
        return LOS_OK;
    }
    if (g_memCheckLevel == LOS_MEM_CHECK_LEVEL_LOW) {
        if (ptr != (VOID *)OS_MEM_ALIGN(ptr, OS_MEM_ALIGN_SIZE)) {
            return LOS_ERRNO_MEMCHECK_NO_HEAD;
        }
        head = (const VOID *)((UINTPTR)ptr - sizeof(LosMemDynNode));
        if (OS_MEM_MAGIC_VALID(((LosMemDynNode *)head)->selfNode.magic)) {
            *totalSize = (UINT32)OS_MEM_NODE_GET_SIZE(((LosMemDynNode *)head)->selfNode.sizeAndFlag -
                                                      sizeof(LosMemDynNode));
            *availSize = (UINT32)OS_MEM_NODE_GET_SIZE(((LosMemDynNode *)head)->selfNode.sizeAndFlag -
                                                      sizeof(LosMemDynNode));
            return LOS_OK;
        } else {
            return LOS_ERRNO_MEMCHECK_NO_HEAD;
        }
    }

    return LOS_ERRNO_MEMCHECK_WRONG_LEVEL;
}

/*
 * Description : get a pool's memCtrl
 * Input       : ptr -- point to source ptr
 * Return      : search forward for ptr's memCtrl or "NULL"
 * attention : this func couldn't ensure the return memCtrl belongs to ptr it just find forward the most nearly one
 */
LITE_OS_SEC_TEXT_MINOR const VOID *OsMemFindNodeCtrl(const VOID *pool, const VOID *ptr)
{
    const VOID *head = ptr;

    if (ptr == NULL) {
        return NULL;
    }

    head = (const VOID *)OS_MEM_ALIGN(head, OS_MEM_ALIGN_SIZE);
    while (!OS_MEM_MAGIC_VALID(((LosMemDynNode *)head)->selfNode.magic)) {
        head = (const VOID *)((UINT8 *)head - sizeof(CHAR *));
        if (head <= pool) {
            return NULL;
        }
    }
    return head;
}

LITE_OS_SEC_TEXT_MINOR UINT32 LOS_MemCheckLevelSet(UINT8 checkLevel)
{
    if (checkLevel == LOS_MEM_CHECK_LEVEL_LOW) {
        PRINTK("%s: LOS_MEM_CHECK_LEVEL_LOW \n", __FUNCTION__);
    } else if (checkLevel == LOS_MEM_CHECK_LEVEL_HIGH) {
        PRINTK("%s: LOS_MEM_CHECK_LEVEL_HIGH \n", __FUNCTION__);
    } else if (checkLevel == LOS_MEM_CHECK_LEVEL_DISABLE) {
        PRINTK("%s: LOS_MEM_CHECK_LEVEL_DISABLE \n", __FUNCTION__);
    } else {
        PRINTK("%s: wrong param, setting failed !! \n", __FUNCTION__);
        return LOS_ERRNO_MEMCHECK_WRONG_LEVEL;
    }
    g_memCheckLevel = checkLevel;
    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT8 LOS_MemCheckLevelGet(VOID)
{
    return g_memCheckLevel;
}


UINT32 OsMemSysNodeCheck(VOID *dstAddr, VOID *srcAddr, UINT32 nodeLength, UINT8 pos)
{
    UINT32 ret;
    UINT32 totalSize = 0;
    UINT32 availSize = 0;
    UINT8 *pool = m_aucSysMem1;
#ifdef LOSCFG_EXC_INTERACTION
    if ((UINTPTR)dstAddr < ((UINTPTR)m_aucSysMem0 + g_excInteractMemSize)) {
        pool = m_aucSysMem0;
    }
#endif
    ret = LOS_MemNodeSizeCheck(pool, dstAddr, &totalSize, &availSize);
    if ((ret == LOS_OK) && (nodeLength > availSize)) {
        PRINT_ERR("---------------------------------------------\n"
                  "%s: dst inode availSize is not enough availSize = 0x%x, memcpy length = 0x%x\n",
                  ((pos == 0) ? "memset" : "memcpy"), availSize, nodeLength);
        OsBackTrace();
        PRINT_ERR("---------------------------------------------\n");
        return LOS_NOK;
    }
    if (pos == 1) {
#ifdef LOSCFG_EXC_INTERACTION
        if ((UINTPTR)srcAddr < ((UINTPTR)m_aucSysMem0 + g_excInteractMemSize)) {
            pool = m_aucSysMem0;
        } else {
            pool = m_aucSysMem1;
        }
#endif
        ret = LOS_MemNodeSizeCheck(pool, srcAddr, &totalSize, &availSize);
        if ((ret == LOS_OK) && (nodeLength > availSize)) {
            PRINT_ERR("---------------------------------------------\n");
            PRINT_ERR("memcpy: src inode availSize is not enough"
                      " availSize = 0x%x, memcpy length = 0x%x\n",
                      availSize, nodeLength);
            OsBackTrace();
            PRINT_ERR("---------------------------------------------\n");
            return LOS_NOK;
        }
    }
    return LOS_OK;
}
#endif /* LOSCFG_BASE_MEM_NODE_SIZE_CHECK */

#ifdef LOSCFG_MEM_MUL_MODULE
STATIC INLINE UINT32 OsMemModCheck(UINT32 moduleId)
{
    if (moduleId > MEM_MODULE_MAX) {
        PRINT_ERR("error module ID input!\n");
        return LOS_NOK;
    }
    return LOS_OK;
}

STATIC INLINE VOID *OsMemPtrToNode(VOID *ptr)
{
    UINT32 gapSize;

    if ((UINTPTR)ptr & (OS_MEM_ALIGN_SIZE - 1)) {
        PRINT_ERR("[%s:%d] pointer not align by 4byte\n", __FUNCTION__, __LINE__);
        return NULL;
    }

    gapSize = *((UINT32 *)((UINTPTR)ptr - sizeof(gapSize)));
    if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize) && OS_MEM_NODE_GET_USED_FLAG(gapSize)) {
        PRINT_ERR("[%s:%d]gapSize:0x%x error\n", __FUNCTION__, __LINE__, gapSize);
        return NULL;
    }
    if (OS_MEM_NODE_GET_ALIGNED_FLAG(gapSize)) {
        gapSize = OS_MEM_NODE_GET_ALIGNED_GAPSIZE(gapSize);
        if ((gapSize & (OS_MEM_ALIGN_SIZE - 1)) || (gapSize > ((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE))) {
            PRINT_ERR("[%s:%d]gapSize:0x%x error\n", __FUNCTION__, __LINE__, gapSize);
            return NULL;
        }

        ptr = (VOID *)((UINTPTR)ptr - gapSize);
    }

    return (VOID *)((UINTPTR)ptr - OS_MEM_NODE_HEAD_SIZE);
}

STATIC INLINE UINT32 OsMemNodeSizeGet(VOID *ptr)
{
    LosMemDynNode *node = (LosMemDynNode *)OsMemPtrToNode(ptr);
    if (node == NULL) {
        return 0;
    }

    return OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);
}

VOID *LOS_MemMalloc(VOID *pool, UINT32 size, UINT32 moduleId)
{
    UINT32 intSave;
    VOID *ptr = NULL;
    VOID *node = NULL;
    if (OsMemModCheck(moduleId) == LOS_NOK) {
        return NULL;
    }
    ptr = LOS_MemAlloc(pool, size);
    if (ptr != NULL) {
        MEM_LOCK(intSave, pool);
        g_moduleMemUsedSize[moduleId] += OsMemNodeSizeGet(ptr);
        node = OsMemPtrToNode(ptr);
        LOS_ASSERT(node != NULL);
        OS_MEM_MODID_SET(node, moduleId);

        MEM_UNLOCK(intSave, pool);
    }
    return ptr;
}

VOID *LOS_MemMallocAlign(VOID *pool, UINT32 size, UINT32 boundary, UINT32 moduleId)
{
    UINT32 intSave;
    VOID *ptr = NULL;
    VOID *node = NULL;
    if (OsMemModCheck(moduleId) == LOS_NOK) {
        return NULL;
    }
    ptr = LOS_MemAllocAlign(pool, size, boundary);
    if (ptr != NULL) {
        MEM_LOCK(intSave, pool);
        g_moduleMemUsedSize[moduleId] += OsMemNodeSizeGet(ptr);
        node = OsMemPtrToNode(ptr);
        LOS_ASSERT(node != NULL);
        OS_MEM_MODID_SET(node, moduleId);
        MEM_UNLOCK(intSave, pool);
    }
    return ptr;
}

UINT32 LOS_MemMfree(VOID *pool, VOID *ptr, UINT32 moduleId)
{
    UINT32 intSave;
    UINT32 ret;
    UINT32 size;
    LosMemDynNode *node = NULL;

    if ((OsMemModCheck(moduleId) == LOS_NOK) || (ptr == NULL) || (pool == NULL)) {
        return LOS_NOK;
    }

    node = (LosMemDynNode *)OsMemPtrToNode(ptr);
    if (node == NULL) {
        return LOS_NOK;
    }

    size = OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);

    if (moduleId != OS_MEM_MODID_GET(node)) {
        PRINT_ERR("node alloced in module %u, but free in module %u\n node's taskId: 0x%x\n",
                  OS_MEM_MODID_GET(node), moduleId, OS_MEM_TASKID_GET(node));
        moduleId = OS_MEM_MODID_GET(node);
    }

    ret = LOS_MemFree(pool, ptr);
    if (ret == LOS_OK) {
        MEM_LOCK(intSave, pool);
        g_moduleMemUsedSize[moduleId] -= size;
        MEM_UNLOCK(intSave, pool);
    }
    return ret;
}

VOID *LOS_MemMrealloc(VOID *pool, VOID *ptr, UINT32 size, UINT32 moduleId)
{
    VOID *newPtr = NULL;
    UINT32 oldNodeSize;
    UINT32 intSave;
    LosMemDynNode *node = NULL;
    UINT32 oldModuleId = moduleId;

    if ((OsMemModCheck(moduleId) == LOS_NOK) || (pool == NULL)) {
        return NULL;
    }

    if (ptr == NULL) {
        return LOS_MemMalloc(pool, size, moduleId);
    }

    node = (LosMemDynNode *)OsMemPtrToNode(ptr);
    if (node == NULL) {
        return NULL;
    }

    if (moduleId != OS_MEM_MODID_GET(node)) {
        PRINT_ERR("a node alloced in module %u, but realloc in module %u\n node's taskId: %u\n",
                  OS_MEM_MODID_GET(node), moduleId, OS_MEM_TASKID_GET(node));
        oldModuleId = OS_MEM_MODID_GET(node);
    }

    if (size == 0) {
        (VOID)LOS_MemMfree(pool, ptr, oldModuleId);
        return NULL;
    }

    oldNodeSize = OsMemNodeSizeGet(ptr);
    newPtr = LOS_MemRealloc(pool, ptr, size);
    if (newPtr != NULL) {
        MEM_LOCK(intSave, pool);
        g_moduleMemUsedSize[moduleId] += OsMemNodeSizeGet(newPtr);
        g_moduleMemUsedSize[oldModuleId] -= oldNodeSize;
        node = (LosMemDynNode *)OsMemPtrToNode(newPtr);
        OS_MEM_MODID_SET(node, moduleId);
        MEM_UNLOCK(intSave, pool);
    }
    return newPtr;
}

UINT32 LOS_MemMusedGet(UINT32 moduleId)
{
    if (OsMemModCheck(moduleId) == LOS_NOK) {
        return OS_NULL_INT;
    }
    return g_moduleMemUsedSize[moduleId];
}
#endif

#ifdef LOSCFG_MEM_MUL_POOL_ALLOC
VOID *OsMulPoolCopyToNewNode(VOID *pool, VOID *ptr, UINT32 size, VOID *newPtr)
{
    LosMemDynNode *node = NULL;
    UINT32 nodeSize;
    UINT32 intSave;

    MEM_LOCK(intSave, pool);
    node = OsGetRealPtr(pool, ptr);
    if (node == NULL) {
        MEM_UNLOCK(intSave, pool);
        return NULL;
    }
    nodeSize = OS_MEM_NODE_GET_SIZE(node->selfNode.sizeAndFlag);
    if (memcpy_s(newPtr, size, ptr, (nodeSize - ((UINTPTR)ptr - (UINTPTR)node))) != EOK) {
        MEM_UNLOCK(intSave, pool);
        return NULL;
    }
    MEM_UNLOCK(intSave, pool);

    return newPtr;
}
#endif