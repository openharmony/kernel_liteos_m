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

#include "los_rwsem_pri.h"
#include "los_task_pri.h"
#include "los_task_base.h"
#include "los_spinlock.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_interrupt.h"
#include "los_memory.h"
#include "los_sched.h"
#include "los_sched_pri.h"

#if (LOSCFG_BASE_IPC_RWSEM == 1)

LITE_OS_SEC_BSS OsRwsemCB *g_osAllRwsem = NULL;
LITE_OS_SEC_BSS STATIC LOS_DL_LIST g_unusedRwsemList;

LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_rwsemSpin);
#define RWSEM_LOCK(state)       LOS_SpinLockSave(&g_rwsemSpin, &(state))
#define RWSEM_UNLOCK(state)     LOS_SpinUnlockRestore(&g_rwsemSpin, (state))

#define RWSEM_NOT_WAIT 0

LITE_OS_SEC_TEXT_INIT UINT32 OsRwsemInit(VOID)
{
    OsRwsemCB *rwsemNode = NULL;
    UINT32 index;

    if (LOSCFG_BASE_IPC_RWSEM_LIMIT == 0) {
        return LOS_ERRNO_RWSEM_ALL_BUSY;
    }

    g_osAllRwsem = (OsRwsemCB *)LOS_MemAlloc(m_aucSysMem0, (LOSCFG_BASE_IPC_RWSEM_LIMIT * sizeof(OsRwsemCB)));
    if (g_osAllRwsem == NULL) {
        return LOS_ERRNO_RWSEM_ALL_BUSY;
    }

    LOS_ListInit(&g_unusedRwsemList);

    for (index = 0; index < LOSCFG_BASE_IPC_RWSEM_LIMIT; index++) {
        rwsemNode = g_osAllRwsem + index;
        rwsemNode->rwsemId = index;
        rwsemNode->rwsemState = OS_RWSEM_UNUSED;
        LOS_ListTailInsert(&g_unusedRwsemList, &rwsemNode->waitList);
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_RwsemCreate(UINT32 *semHandle)
{
    UINT32 intSave;
    OsRwsemCB *rwsemCreated = NULL;
    LOS_DL_LIST *unusedRwsem = NULL;

    if (semHandle == NULL) {
        return LOS_ERRNO_RWSEM_PTR_NULL;
    }

    RWSEM_LOCK(intSave);
    if (LOS_ListEmpty(&g_unusedRwsemList)) {
        RWSEM_UNLOCK(intSave);
        return LOS_ERRNO_RWSEM_ALL_BUSY;
    }

    unusedRwsem = LOS_DL_LIST_FIRST(&g_unusedRwsemList);
    LOS_ListDelete(unusedRwsem);
    rwsemCreated = GET_RWSEM_LIST(unusedRwsem);
    rwsemCreated->rwsemCount = RWSEM_INITIAL;
    rwsemCreated->rwsemState = OS_RWSEM_USED;
    LOS_ListInit(&rwsemCreated->waitList);
    *semHandle = rwsemCreated->rwsemId;

    RWSEM_UNLOCK(intSave);
    return LOS_OK;
}

STATIC INLINE UINT32 RwsemStateVerify(UINT32 rwsemId, const OsRwsemCB *rwsemNode)
{
    (VOID)rwsemId;

    if (rwsemNode->rwsemState == OS_RWSEM_UNUSED) {
        return LOS_ERRNO_RWSEM_INVALID;
    }
    return LOS_OK;
}

STATIC INLINE VOID RwsemIdUpdate(OsRwsemCB *rwsemDeleted)
{
    (VOID)rwsemDeleted;
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_RwsemDelete(UINT32 semHandle)
{
    UINT32 intSave;
    OsRwsemCB *rwsemDeleted = NULL;
    UINT32 ret = LOS_OK;

    if (GET_RWSEM_INDEX(semHandle) >= (UINT32)LOSCFG_BASE_IPC_RWSEM_LIMIT) {
        return LOS_ERRNO_RWSEM_INVALID;
    }

    rwsemDeleted = GET_RWSEM(semHandle);

    RWSEM_LOCK(intSave);
    ret = RwsemStateVerify(semHandle, rwsemDeleted);
    if (ret != LOS_OK) {
        goto OUT;
    }

    if (!LOS_ListEmpty(&rwsemDeleted->waitList)) {
        ret = LOS_ERRNO_RWSEM_PENDED;
        goto OUT;
    }

    RwsemIdUpdate(rwsemDeleted);
    rwsemDeleted->rwsemState = OS_RWSEM_UNUSED;
    LOS_ListTailInsert(&g_unusedRwsemList, &rwsemDeleted->waitList);

OUT:
    RWSEM_UNLOCK(intSave);

    return ret;
}

STATIC INLINE UINT32 RwsemPendCheck(UINT32 semHandle, UINT32 timeout, LosTaskCB *runTask)
{
    if (GET_RWSEM_INDEX(semHandle) >= (UINT32)LOSCFG_BASE_IPC_RWSEM_LIMIT) {
        return LOS_ERRNO_RWSEM_INVALID;
    }

    if ((timeout > 0) && (OS_INT_ACTIVE)) {
        return LOS_ERRNO_RWSEM_PEND_INTERR;
    }

    if (runTask->taskFlags & OS_TASK_FLAG_SYSTEM) {
        PRINT_ERR("Warning: DO NOT recommend to use rwsem in system tasks.\n");
    }

    if (OsPercpuGet()->taskLockCnt) {
        return LOS_ERRNO_RWSEM_PEND_IN_LOCK;
    }
    return LOS_OK;
}

STATIC VOID RwsemTaskWait(OsRwsemCB *rwsem, LosTaskCB *runTask, UINT32 timeout, UINT32 *ret)
{
    OsSchedWait(OsCurrTaskGet(), &rwsem->waitList, timeout);

    if (runTask->taskStatus & OS_TASK_STATUS_TIMEOUT) {
        runTask->taskStatus &= ~OS_TASK_STATUS_TIMEOUT;
        *ret = LOS_ERRNO_RWSEM_TIMEOUT;
    }
}

LITE_OS_SEC_TEXT UINT32 LOS_RwsemPendRead(UINT32 semHandle, UINT32 timeout)
{
    UINT32 intSave;
    UINT32 ret;
    OsRwsemCB *rwsemPended = NULL;
    LosTaskCB *runTask = OsCurrTaskGet();

    ret = RwsemPendCheck(semHandle, timeout, runTask);
    if (ret != LOS_OK) {
        return ret;
    }

    rwsemPended = GET_RWSEM(semHandle);
    RWSEM_LOCK(intSave);
    ret = RwsemStateVerify(semHandle, rwsemPended);
    if (ret != LOS_OK) {
        goto OUT;
    }

    if ((rwsemPended->rwsemCount >= RWSEM_INITIAL) && LOS_ListEmpty(&rwsemPended->waitList)) {
        rwsemPended->rwsemCount++;
        goto OUT;
    } else if (timeout == RWSEM_NOT_WAIT) {
        ret = LOS_ERRNO_RWSEM_UNAVAILABLE;
        goto OUT;
    }

    RWSEM_UNLOCK(intSave);
    SCHEDULER_LOCK(intSave);
    if ((rwsemPended->rwsemCount >= RWSEM_INITIAL) && LOS_ListEmpty(&rwsemPended->waitList)) {
        rwsemPended->rwsemCount++;
    } else {
        runTask->waitType = RWSEM_READ;
        RwsemTaskWait(rwsemPended, runTask, timeout, &ret);
    }
    SCHEDULER_UNLOCK(intSave);
    return ret;

OUT:
    RWSEM_UNLOCK(intSave);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_RwsemPendWrite(UINT32 semHandle, UINT32 timeout)
{
    UINT32 intSave;
    UINT32 ret = LOS_OK;
    OsRwsemCB *rwsemPended = NULL;
    LosTaskCB *runTask = OsCurrTaskGet();

    ret = RwsemPendCheck(semHandle, timeout, runTask);
    if (ret != LOS_OK) {
        return ret;
    }

    rwsemPended = GET_RWSEM(semHandle);
    RWSEM_LOCK(intSave);
    ret = RwsemStateVerify(semHandle, rwsemPended);
    if (ret != LOS_OK) {
        goto OUT;
    }

    if (rwsemPended->rwsemCount == RWSEM_INITIAL) {
        rwsemPended->rwsemCount--;
        goto OUT;
    } else if (timeout == RWSEM_NOT_WAIT) {
        ret = LOS_ERRNO_RWSEM_UNAVAILABLE;
        goto OUT;
    }

    RWSEM_UNLOCK(intSave);
    SCHEDULER_LOCK(intSave);
    if (rwsemPended->rwsemCount == RWSEM_INITIAL) {
        rwsemPended->rwsemCount--;
    } else {
        runTask->waitType = RWSEM_WRITE;
        RwsemTaskWait(rwsemPended, runTask, timeout, &ret);
    }
    SCHEDULER_UNLOCK(intSave);

    return ret;

OUT:
    RWSEM_UNLOCK(intSave);
    return ret;
}

STATIC INLINE VOID RwsemTaskWakeup(OsRwsemCB *rwsem, enum RwsemWaitType type)
{
    LosTaskCB *resumedTask = NULL;

    while (!LOS_ListEmpty(&(rwsem->waitList))) {
        resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&rwsem->waitList));
        if (resumedTask->waitType != type) {
            break;
        }

        OsSchedWake(resumedTask);

        if (type == RWSEM_WRITE) {
            rwsem->rwsemCount--;
            break;
        }
        rwsem->rwsemCount++;
    }
}

LITE_OS_SEC_TEXT UINT32 LOS_RwsemPostRead(UINT32 semHandle)
{
    UINT32 intSave;
    UINT32 ret = LOS_OK;
    OsRwsemCB *rwsemPosted = NULL;

    if (GET_RWSEM_INDEX(semHandle) >= (UINT32)LOSCFG_BASE_IPC_RWSEM_LIMIT) {
        return LOS_ERRNO_RWSEM_INVALID;
    }

    rwsemPosted = GET_RWSEM(semHandle);
    RWSEM_LOCK(intSave);
    ret = RwsemStateVerify(semHandle, rwsemPosted);
    if (ret != LOS_OK) {
        goto OUT;
    }

    if (rwsemPosted->rwsemCount <= RWSEM_INITIAL) {
        ret = LOS_ERRNO_RWSEM_INVALID_STATUS;
        goto OUT;
    }

    rwsemPosted->rwsemCount--;
    if ((rwsemPosted->rwsemCount > RWSEM_INITIAL) || LOS_ListEmpty(&rwsemPosted->waitList)) {
        goto OUT;
    }

    RWSEM_UNLOCK(intSave);
    SCHEDULER_LOCK(intSave);
    RwsemTaskWakeup(rwsemPosted, RWSEM_WRITE);
    SCHEDULER_UNLOCK(intSave);
    LOS_Schedule();
    return ret;

OUT:
    RWSEM_UNLOCK(intSave);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_RwsemPostWrite(UINT32 semHandle)
{
    UINT32 intSave;
    UINT32 ret = LOS_OK;
    OsRwsemCB *rwsemPosted = NULL;
    LosTaskCB *resumedTask = NULL;

    if (GET_RWSEM_INDEX(semHandle) >= (UINT32)LOSCFG_BASE_IPC_RWSEM_LIMIT) {
        return LOS_ERRNO_RWSEM_INVALID;
    }

    rwsemPosted = GET_RWSEM(semHandle);
    RWSEM_LOCK(intSave);
    ret = RwsemStateVerify(semHandle, rwsemPosted);
    if (ret != LOS_OK) {
        goto OUT;
    }

    if (rwsemPosted->rwsemCount >= RWSEM_INITIAL) {
        ret = LOS_ERRNO_RWSEM_INVALID_STATUS;
        goto OUT;
    }

    rwsemPosted->rwsemCount++;
    if (LOS_ListEmpty(&rwsemPosted->waitList)) {
        goto OUT;
    }

    RWSEM_UNLOCK(intSave);
    SCHEDULER_LOCK(intSave);
    resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&rwsemPosted->waitList));
    RwsemTaskWakeup(rwsemPosted, resumedTask->waitType);
    SCHEDULER_UNLOCK(intSave);
    LOS_Schedule();

    return ret;

OUT:
    RWSEM_UNLOCK(intSave);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_RwsemDowngradeWrite(UINT32 semHandle)
{
    UINT32 intSave;
    UINT32 ret = LOS_OK;
    OsRwsemCB *rwsemDowngaded = NULL;
    LosTaskCB *resumedTask = NULL;
    BOOL wakeup = FALSE;

    if (GET_RWSEM_INDEX(semHandle) >= (UINT32)LOSCFG_BASE_IPC_RWSEM_LIMIT) {
        return LOS_ERRNO_RWSEM_INVALID;
    }

    rwsemDowngaded = GET_RWSEM(semHandle);
    RWSEM_LOCK(intSave);
    ret = RwsemStateVerify(semHandle, rwsemDowngaded);
    if (ret != LOS_OK) {
        goto OUT;
    }

    if (rwsemDowngaded->rwsemCount > RWSEM_INITIAL) {
        ret = LOS_ERRNO_RWSEM_INVALID_STATUS;
        goto OUT;
    }

    rwsemDowngaded->rwsemCount = RWSEM_READING;
    if (LOS_ListEmpty(&rwsemDowngaded->waitList)) {
        goto OUT;
    }

    RWSEM_UNLOCK(intSave);
    SCHEDULER_LOCK(intSave);
    resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&rwsemDowngaded->waitList));
    if (resumedTask->waitType == RWSEM_READ) {
        RwsemTaskWakeup(rwsemDowngaded, resumedTask->waitType);
        wakeup = TRUE;
    }
    SCHEDULER_UNLOCK(intSave);

    if (wakeup == TRUE) {
        LOS_Schedule();
    }

    return ret;

OUT:
    RWSEM_UNLOCK(intSave);
    return ret;
}

#endif /* LOSCFG_BASE_IPC_RWSEM == 1 */
