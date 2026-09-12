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

#include "los_mux.h"
#include "los_mux_debug_pri.h"
#include "los_task_pri.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_interrupt.h"
#include "los_memory.h"
#include "los_sched.h"
#include "los_sched_pri.h"
#include "los_percpu_pri.h"
#include "los_bitmap.h"

#if (LOSCFG_BASE_IPC_MUX == 1)

LITE_OS_SEC_BSS       LosMuxCB*   g_allMux = NULL;
LITE_OS_SEC_DATA_INIT LOS_DL_LIST g_unusedMuxList;

/*****************************************************************************
 Function      : OsMuxInit
 Description  : Initializes the mutex
 Input        : None
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 OsMuxInit(VOID)
{
    LosMuxCB *muxNode = NULL;
    UINT32 index;

    LOS_ListInit(&g_unusedMuxList);

    if (LOSCFG_BASE_IPC_MUX_LIMIT == 0) {
        return LOS_ERRNO_MUX_MAXNUM_ZERO;
    }

    g_allMux = (LosMuxCB *)LOS_MemAlloc(m_aucSysMem0, (LOSCFG_BASE_IPC_MUX_LIMIT * sizeof(LosMuxCB)));
    if (g_allMux == NULL) {
        return LOS_ERRNO_MUX_NO_MEMORY;
    }

    for (index = 0; index < LOSCFG_BASE_IPC_MUX_LIMIT; index++) {
        muxNode = ((LosMuxCB *)g_allMux) + index;
        muxNode->muxID = index;
        muxNode->owner = (LosTaskCB *)NULL;
        muxNode->muxStat = OS_MUX_UNUSED;
#if (LOSCFG_MUTEX_CREATE_TRACE == 1)
        muxNode->createInfo = 0;
#endif
        LOS_ListTailInsert(&g_unusedMuxList, &muxNode->muxList);
    }
    if (OsMuxDbgInitHook() != LOS_OK) {
        return LOS_ERRNO_MUX_NO_MEMORY;
    }
    return LOS_OK;
}

/*****************************************************************************
 Function     : LOS_MuxCreate
 Description  : Create a mutex
 Input        : None
 Output       : muxHandle ------ Mutex operation handle
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_MuxCreate(UINT32 *muxHandle)
{
    UINT32 intSave;
    LosMuxCB *muxCreated = NULL;
    LOS_DL_LIST *unusedMux = NULL;
    UINT32 errNo;
    UINT32 errLine;

    if (muxHandle == NULL) {
        return LOS_ERRNO_MUX_PTR_NULL;
    }

    SCHEDULER_LOCK(intSave);
    if (LOS_ListEmpty(&g_unusedMuxList)) {
        SCHEDULER_UNLOCK(intSave);
        OsMutexCheckHook();
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_MUX_ALL_BUSY);
    }

    unusedMux = LOS_DL_LIST_FIRST(&(g_unusedMuxList));
    LOS_ListDelete(unusedMux);
    muxCreated = (GET_MUX_LIST(unusedMux));
    muxCreated->muxCount = 0;
    muxCreated->muxStat = OS_MUX_USED;
    muxCreated->priority = 0;
    muxCreated->owner = (LosTaskCB *)NULL;
    LOS_ListInit(&muxCreated->muxList);
    *muxHandle = (UINT32)muxCreated->muxID;
    OsMuxDbgUpdateHook(muxCreated->muxID, OsCurrTaskGet()->taskEntry);
    SCHEDULER_UNLOCK(intSave);
    OsHookCall(LOS_HOOK_TYPE_MUX_CREATE, muxCreated);
    return LOS_OK;
ERR_HANDLER:
    OS_RETURN_ERROR_P2(errLine, errNo);
}

/*****************************************************************************
 Function     : LOS_MuxDelete
 Description  : Delete a mutex
 Input        : muxHandle ------Mutex operation handle
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT UINT32 LOS_MuxDelete(UINT32 muxHandle)
{
    UINT32 intSave;
    LosMuxCB *muxDeleted = NULL;
    UINT32 errNo;
    UINT32 errLine;

    if (muxHandle >= (UINT32)LOSCFG_BASE_IPC_MUX_LIMIT) {
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_MUX_INVALID);
    }

    muxDeleted = GET_MUX(muxHandle);
    SCHEDULER_LOCK(intSave);
    if (muxDeleted->muxStat == OS_MUX_UNUSED) {
        SCHEDULER_UNLOCK(intSave);
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_MUX_INVALID);
    }

    if ((!LOS_ListEmpty(&muxDeleted->muxList)) || muxDeleted->muxCount) {
        SCHEDULER_UNLOCK(intSave);
        OS_GOTO_ERR_HANDLER(LOS_ERRNO_MUX_PENDED);
    }

    LOS_ListAdd(&g_unusedMuxList, &muxDeleted->muxList);
    muxDeleted->muxStat = OS_MUX_UNUSED;
#if (LOSCFG_MUTEX_CREATE_TRACE == 1)
    muxDeleted->createInfo = 0;
#endif
    SCHEDULER_UNLOCK(intSave);

    OsMuxDbgUpdateHook(muxDeleted->muxID, NULL);
    OsHookCall(LOS_HOOK_TYPE_MUX_DELETE, muxDeleted);
    return LOS_OK;
ERR_HANDLER:
    OS_RETURN_ERROR_P2(errLine, errNo);
}

STATIC_INLINE UINT32 OsMuxValidCheck(LosMuxCB *muxPended)
{
    if (muxPended->muxStat == OS_MUX_UNUSED) {
        return LOS_ERRNO_MUX_INVALID;
    }

    if (OS_INT_ACTIVE) {
        return LOS_ERRNO_MUX_IN_INTERR;
    }

    if (OsPercpuGet()->taskLockCnt) {
        PRINT_ERR("!!!LOS_ERRNO_MUX_PEND_IN_LOCK!!!\n");
        return LOS_ERRNO_MUX_PEND_IN_LOCK;
    }

    if (OsCurrTaskGet()->taskFlags & OS_TASK_FLAG_SYSTEM) {
        return LOS_ERRNO_MUX_PEND_IN_SYSTEM_TASK;
    }

    return LOS_OK;
}

STATIC VOID OsMuxBitmapSet(const LosTaskCB *runTask, const LosMuxCB *muxPended)
{
    LosTaskCB *owner = muxPended->owner;
    if (owner->priority > runTask->priority) {
        LOS_BitmapSet(&owner->priBitMap, owner->priority);
        (VOID)OsSchedPrioModify(owner, runTask->priority);
    }
}

VOID OsMuxBitmapRestore(const LosTaskCB *runTask, LosTaskCB *owner)
{
    UINT16 bitMapPri;

    if (owner->priority >= runTask->priority) {
        bitMapPri = LOS_LowBitGet(owner->priBitMap);
        if (bitMapPri != LOS_INVALID_BIT_INDEX) {
            LOS_BitmapClr(&owner->priBitMap, bitMapPri);
            (VOID)OsSchedPrioModify(owner, bitMapPri);
        }
    } else {
        if (LOS_HighBitGet(owner->priBitMap) != runTask->priority) {
            LOS_BitmapClr(&owner->priBitMap, runTask->priority);
        }
    }
}

#ifdef LOSCFG_MUTEX_WAITMODE_PRIO
STATIC LOS_DL_LIST *OsMuxPendFindPosSub(const LosTaskCB *runTask, const LosMuxCB *muxPended)
{
    LosTaskCB *pendedTask = NULL;
    LOS_DL_LIST *node = NULL;

    LOS_DL_LIST_FOR_EACH_ENTRY(pendedTask, &muxPended->muxList, LosTaskCB, pendList) {
        if (pendedTask->priority < runTask->priority) {
            continue;
        } else if (pendedTask->priority > runTask->priority) {
            node = &pendedTask->pendList;
            break;
        } else {
            node = pendedTask->pendList.pstNext;
            break;
        }
    }
    return node;
}

STATIC LOS_DL_LIST *OsMuxPendFindPos(const LosTaskCB *runTask, LosMuxCB *muxPended)
{
    LOS_DL_LIST *node = NULL;
    LosTaskCB *pendedTask1 = NULL;
    LosTaskCB *pendedTask2 = NULL;

    if (LOS_ListEmpty(&muxPended->muxList)) {
        node = &muxPended->muxList;
    } else {
        pendedTask1 = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&muxPended->muxList));
        pendedTask2 = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_LAST(&muxPended->muxList));
        if ((pendedTask1 != NULL) && (pendedTask1->priority > runTask->priority)) {
            node = muxPended->muxList.pstNext;
        } else if ((pendedTask2 != NULL) && (pendedTask2->priority <= runTask->priority)) {
            node = &muxPended->muxList;
        } else {
            node = OsMuxPendFindPosSub(runTask, muxPended);
        }
    }
    return node;
}
#else
STATIC LOS_DL_LIST *OsMuxPendFindPos(const LosTaskCB *runTask, LosMuxCB *muxPended)
{
    (VOID)runTask;
    return &muxPended->muxList;
}
#endif

VOID OsMuxPostOpSub(LosTaskCB *runTask, LosMuxCB *muxPosted)
{
    LosTaskCB *pendedTask = NULL;
    UINT16 bitMapPri;

    if (!LOS_ListEmpty(&muxPosted->muxList)) {
        bitMapPri = LOS_HighBitGet(runTask->priBitMap);
        LOS_DL_LIST_FOR_EACH_ENTRY(pendedTask, &muxPosted->muxList, LosTaskCB, pendList) {
            if (bitMapPri != pendedTask->priority) {
                LOS_BitmapClr(&runTask->priBitMap, pendedTask->priority);
            }
        }
    }
    bitMapPri = LOS_LowBitGet(runTask->priBitMap);
    LOS_BitmapClr(&runTask->priBitMap, bitMapPri);
    (VOID)OsSchedPrioModify(muxPosted->owner, bitMapPri);
}

/*****************************************************************************
 Function     : LOS_MuxPend
 Description  : Specify the mutex P operation
 Input        : muxHandle ------ Mutex operation handleone
              : timeOut   ------- waiting time
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_MuxPend(UINT32 muxHandle, UINT32 timeout)
{
    UINT32 intSave;
    LosMuxCB *muxPended = NULL;
    UINT32 retErr;
    LosTaskCB *runningTask = NULL;
    LOS_DL_LIST *node = NULL;

    if (muxHandle >= (UINT32)LOSCFG_BASE_IPC_MUX_LIMIT) {
        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    muxPended = GET_MUX(muxHandle);
    SCHEDULER_LOCK(intSave);
    retErr = OsMuxValidCheck(muxPended);
    if (retErr) {
        goto ERROR_MUX_PEND;
    }

    OsMuxDbgTimeUpdateHook(muxHandle);
    runningTask = OsCurrTaskGet();
    if (muxPended->muxCount == 0) {
        muxPended->muxCount++;
        muxPended->owner = runningTask;
        SCHEDULER_UNLOCK(intSave);
        goto HOOK;
    }

    if (muxPended->owner == runningTask) {
        muxPended->muxCount++;
        SCHEDULER_UNLOCK(intSave);
        goto HOOK;
    }

    if (!timeout) {
        retErr = LOS_ERRNO_MUX_UNAVAILABLE;
        goto ERROR_MUX_PEND;
    }

    runningTask->taskMux = (VOID *)muxPended;

    OsMuxBitmapSet(runningTask, muxPended);

    node = OsMuxPendFindPos(runningTask, muxPended);
    OsHookCall(LOS_HOOK_TYPE_MUX_PEND, muxPended, timeout);

    OsSchedWait(OsCurrTaskGet(), node, timeout);
    SCHEDULER_UNLOCK(intSave);

    SCHEDULER_LOCK(intSave);
    if (runningTask->taskStatus & OS_TASK_STATUS_TIMEOUT) {
        runningTask->taskStatus &= (~OS_TASK_STATUS_TIMEOUT);
        OsMuxBitmapRestore(runningTask, muxPended->owner);
        retErr = LOS_ERRNO_MUX_TIMEOUT;
        goto ERROR_MUX_PEND;
    }

    SCHEDULER_UNLOCK(intSave);
    return LOS_OK;

HOOK:
    OsHookCall(LOS_HOOK_TYPE_MUX_PEND, muxPended, timeout);
    return LOS_OK;

ERROR_MUX_PEND:
    SCHEDULER_UNLOCK(intSave);
    OS_RETURN_ERROR(retErr);
}

/*****************************************************************************
 Function     : LOS_MuxPost
 Description  : Specify the mutex V operation,
 Input        : muxHandle ------ Mutex operation handle
 Output       : None
 Return       : LOS_OK on success, or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_MuxPost(UINT32 muxHandle)
{
    UINT32 intSave;
    LosMuxCB *muxPosted = GET_MUX(muxHandle);
    LosTaskCB *resumedTask = NULL;
    LosTaskCB *runningTask = NULL;

    SCHEDULER_LOCK(intSave);

    if ((muxHandle >= (UINT32)LOSCFG_BASE_IPC_MUX_LIMIT) ||
        (muxPosted->muxStat == OS_MUX_UNUSED)) {
        SCHEDULER_UNLOCK(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    if (OS_INT_ACTIVE) {
        SCHEDULER_UNLOCK(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_MUX_IN_INTERR);
    }

    runningTask = OsCurrTaskGet();
    if ((muxPosted->muxCount == 0) || (muxPosted->owner != runningTask)) {
        SCHEDULER_UNLOCK(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    if (--(muxPosted->muxCount) != 0) {
        SCHEDULER_UNLOCK(intSave);
        OsHookCall(LOS_HOOK_TYPE_MUX_POST, muxPosted);
        return LOS_OK;
    }

    if (!LOS_ListEmpty(&muxPosted->muxList)) {
        resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(muxPosted->muxList)));

#ifdef LOSCFG_MUTEX_WAITMODE_PRIO
        if (resumedTask->priority > runningTask->priority) {
            if (LOS_HighBitGet(runningTask->priBitMap) != resumedTask->priority) {
                LOS_BitmapClr(&runningTask->priBitMap, resumedTask->priority);
            }
        } else if (runningTask->priBitMap != 0) {
            OsMuxPostOpSub(runningTask, muxPosted);
        }
#else
        if (runningTask->priBitMap != 0) {
            OsMuxPostOpSub(runningTask, muxPosted);
        }
#endif

        muxPosted->muxCount = 1;
        muxPosted->owner = resumedTask;
        resumedTask->taskMux = NULL;

        OsSchedWake(resumedTask);

        SCHEDULER_UNLOCK(intSave);
        OsHookCall(LOS_HOOK_TYPE_MUX_POST, muxPosted);
        LOS_Schedule();
    } else {
        muxPosted->owner = NULL;
        SCHEDULER_UNLOCK(intSave);
    }

    return LOS_OK;
}

#endif /* (LOSCFG_BASE_IPC_MUX == 1) */
