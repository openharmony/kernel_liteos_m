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

#include "los_lmk.h"
#include "los_interrupt.h"
#if (LOSCFG_KERNEL_LMK_DEBUG == 1)
#include "los_debug.h"
#endif

#if (LOSCFG_KERNEL_LMK == 1)
STATIC LosLmkOps g_losLmkOps;

STATIC BOOL OsIsLmkOpsNodeRegistered(LosLmkOpsNode *lmkNode)
{
    LosLmkOpsNode *opsNode = NULL;

    if (LOS_ListEmpty(&g_losLmkOps.lmkOpsList)) {
        return FALSE;
    }
    LOS_DL_LIST_FOR_EACH_ENTRY(opsNode, &g_losLmkOps.lmkOpsList, LosLmkOpsNode, node) {
        if (lmkNode == opsNode) {
            return TRUE;
        }
    }
    return FALSE;
}

UINT32 LOS_LmkOpsNodeRegister(LosLmkOpsNode *lmkNode)
{
    UINT32 intSave;
    LosLmkOpsNode *opsNode = NULL;

    if (lmkNode == NULL) {
        return LOS_ERRNO_LMK_INVALID_PARAMETER;
    }

    intSave = LOS_IntLock();
    if (OsIsLmkOpsNodeRegistered(lmkNode)) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_LMK_ALREADY_REGISTERED;
    }
    if (LOS_ListEmpty(&g_losLmkOps.lmkOpsList)) {
        LOS_ListHeadInsert(&g_losLmkOps.lmkOpsList, &lmkNode->node);
        LOS_IntRestore(intSave);
        return LOS_OK;
    }

    // the priority of registered node <= the first node
    opsNode = LOS_DL_LIST_ENTRY(g_losLmkOps.lmkOpsList.pstNext, LosLmkOpsNode, node);
    if (lmkNode->priority <= opsNode->priority) {
        LOS_ListHeadInsert(&g_losLmkOps.lmkOpsList, &lmkNode->node);
        LOS_IntRestore(intSave);
        return LOS_OK;
    }

    // the priority of registered node > the last node
    opsNode = LOS_DL_LIST_ENTRY(g_losLmkOps.lmkOpsList.pstPrev, LosLmkOpsNode, node);
    if (lmkNode->priority >= opsNode->priority) {
        LOS_ListTailInsert(&g_losLmkOps.lmkOpsList, &lmkNode->node);
        LOS_IntRestore(intSave);
        return LOS_OK;
    }

    // the priority of registered node > the first node and < the last node
    LOS_DL_LIST_FOR_EACH_ENTRY(opsNode, &g_losLmkOps.lmkOpsList, LosLmkOpsNode, node) {
        if (lmkNode->priority < opsNode->priority) {
            LOS_ListHeadInsert((&opsNode->node)->pstPrev, &lmkNode->node);
            break;
        }
    }

    LOS_IntRestore(intSave);
    return LOS_OK;
}

UINT32 LOS_LmkOpsNodeUnregister(LosLmkOpsNode *lmkNode)
{
    UINT32 intSave;

    if (lmkNode == NULL) {
        return LOS_ERRNO_LMK_INVALID_PARAMETER;
    }

    intSave = LOS_IntLock();
    if (LOS_ListEmpty(&g_losLmkOps.lmkOpsList) || !OsIsLmkOpsNodeRegistered(lmkNode)) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_LMK_NOT_REGISTERED;
    }
    LOS_ListDelete(&lmkNode->node);
    LOS_IntRestore(intSave);
    return LOS_OK;
}

UINT32 LOS_LmkTasksKill(VOID)
{
    UINT32 intSave;
    UINT32 ret;
    LosLmkOpsNode *opsNode = NULL;
    FreeMemByKillingTask freeMem = NULL;

    intSave = LOS_IntLock();

    // if tasks already killed, no need to do it again.
    if (g_losLmkOps.isMemFreed) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_LMK_MEMORY_ALREADY_FREED;
    } else {
        g_losLmkOps.isMemFreed = TRUE;
    }
    LOS_DL_LIST_FOR_EACH_ENTRY(opsNode, &g_losLmkOps.lmkOpsList, LosLmkOpsNode, node) {
        freeMem = opsNode->freeMem;
        LOS_IntRestore(intSave);
        if (freeMem != NULL) {
            ret = freeMem();
            if (ret != LOS_OK) {
                return LOS_ERRNO_LMK_FREE_MEMORY_FAILURE;
            }
        }
        intSave = LOS_IntLock();
    }
    LOS_IntRestore(intSave);

    return LOS_OK;
}

UINT32 LOS_LmkTasksRestore(VOID)
{
    UINT32 intSave;
    UINT32 ret;
    LosLmkOpsNode *opsNode = NULL;
    RestoreKilledTask restore = NULL;

    intSave = LOS_IntLock();

    // if no tasks killed, no need to restore.
    if (!g_losLmkOps.isMemFreed) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_LMK_RESTORE_NOT_NEEDED;
    } else {
        g_losLmkOps.isMemFreed = FALSE;
    }
    LOS_DL_LIST_FOR_EACH_ENTRY(opsNode, &g_losLmkOps.lmkOpsList, LosLmkOpsNode, node) {
        restore = opsNode->restoreTask;
        LOS_IntRestore(intSave);
        if (restore != NULL) {
            ret = restore();
            if (ret != LOS_OK) {
                return LOS_ERRNO_LMK_RESTORE_TASKS_FAILURE;
            }
        }
        intSave = LOS_IntLock();
    }
    LOS_IntRestore(intSave);

    return LOS_OK;
}

#if (LOSCFG_KERNEL_LMK_DEBUG == 1)
VOID LOS_LmkOpsNodeInfoShow(VOID)
{
    UINT32 intSave;
    LosLmkOpsNode *opsNode = NULL;

    intSave = LOS_IntLock();
    LOS_DL_LIST_FOR_EACH_ENTRY(opsNode, &g_losLmkOps.lmkOpsList, LosLmkOpsNode, node) {
        PRINTK("Priority: %-4u Free:0x%-8x Restore:0x%-8x\n", opsNode->priority, 
               (UINT32)(UINTPTR)opsNode->freeMem, (UINT32)(UINTPTR)opsNode->restoreTask);
    }
    LOS_IntRestore(intSave);
}
#endif

VOID OsLmkInit(VOID)
{
    g_losLmkOps.isMemFreed = FALSE;
    LOS_ListInit(&g_losLmkOps.lmkOpsList);
}
#endif
