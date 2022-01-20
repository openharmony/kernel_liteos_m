/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_signal.h"
#include "securec.h"
#include "los_config.h"
#include "los_interrupt.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_debug.h"
#include "los_memory.h"
#include "los_context.h"
#include "los_arch_context.h"

UINTPTR OsSignalTaskContextRestore(VOID)
{
#if (LOSCFG_KERNEL_SIGNAL == 1)
    LosTaskCB *task = OsCurrTaskGet();
    OsSigCB *sigCB = (OsSigCB *)task->sig;
    UINTPTR sp;

    if ((sigCB == NULL) || (sigCB->sigRestoreSP == NULL)) {
        return 0;
    }

    sp = (UINTPTR)sigCB->sigRestoreSP;
    sigCB->sigRestoreSP = NULL;

    return sp;
#else
    return 0;
#endif
}

#if (LOSCFG_KERNEL_SIGNAL == 1)
STATIC LOS_DL_LIST g_waitSignalList;

STATIC VOID SignalDefaultHandler(INT32 signo)
{
    PRINTK("signal default handler, signo = %d\n", signo);
}

STATIC UINT32 AddSigInfoToList(OsSigCB *sigCB, siginfo_t *info)
{
    OsSigInfoNode *tmpInfo = NULL;
    BOOL findFlag = FALSE;

    LOS_DL_LIST_FOR_EACH_ENTRY(tmpInfo, &sigCB->sigInfoList, OsSigInfoNode, node) {
        if (tmpInfo->info.si_signo == info->si_signo) {
            findFlag = TRUE;
            break;
        }
    }

    if (findFlag == FALSE) {
        tmpInfo = (OsSigInfoNode *)LOS_MemAlloc(OS_SYS_MEM_ADDR, sizeof(OsSigInfoNode));
        if (tmpInfo == NULL) {
            return LOS_NOK;
        }
        LOS_ListAdd(&sigCB->sigInfoList, &tmpInfo->node);
    }
    (VOID)memcpy_s(&tmpInfo->info, sizeof(siginfo_t), info, sizeof(siginfo_t));
    return LOS_OK;
}

STATIC VOID DeleteSigInfoFromList(OsSigCB *sigCB, INT32 sigNo)
{
    OsSigInfoNode *tmpInfo = NULL;
    BOOL findFlag = FALSE;

    LOS_DL_LIST_FOR_EACH_ENTRY(tmpInfo, &sigCB->sigInfoList, OsSigInfoNode, node) {
        if (tmpInfo->info.si_signo == sigNo) {
            LOS_ListDelete(&tmpInfo->node);
            findFlag = TRUE;
            break;
        }
    }

    if (findFlag == TRUE) {
        (VOID)memcpy_s(&sigCB->sigInfo, sizeof(siginfo_t), &tmpInfo->info, sizeof(siginfo_t));
        (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, tmpInfo);
    }
}

STATIC VOID SignalHandle(LosTaskCB *task, BOOL cleanStatus)
{
    UINT32 intSave;
    OsSigCB *sigCB = NULL;

    intSave = LOS_IntLock();
    sigCB = task->sig;
    if (sigCB == NULL) {
        LOS_IntRestore(intSave);
        return;
    }

    while (sigCB->sigPendFlag & sigCB->sigSetFlag) {
        UINT32 sigFlag = sigCB->sigPendFlag & sigCB->sigSetFlag;
        INT32 sigNo = LOS_SIGNAL_SUPPORT_MAX - CLZ(sigFlag) + 1;
        DeleteSigInfoFromList(sigCB, sigNo);

        SIG_HANDLER handler = sigCB->sigHandlers[sigNo - 1];
        sigCB->sigPendFlag &= ~LOS_SIGNAL_MASK(sigNo);
        LOS_IntRestore(intSave);

        if (handler != NULL) {
            handler(sigNo);
        }
        intSave = LOS_IntLock();

        if (cleanStatus == TRUE) {
            task->taskStatus &= ~OS_TASK_FLAG_SIGNAL;
        }
    }
    LOS_IntRestore(intSave);
}

STATIC VOID SignalEntry(INT32 sigNo)
{
    LosTaskCB *task = OsCurrTaskGet();
    OsSigCB *sigCB = (OsSigCB *)task->sig;

    SignalHandle(task, FALSE);

    task->stackPointer = sigCB->sigSaveSP;
    sigCB->sigSaveSP = NULL;
    sigCB->sigRestoreSP = task->stackPointer;
    task->taskStatus &= ~OS_TASK_FLAG_SIGNAL;

    LOS_Schedule();
}

STATIC VOID SignalSend(LosTaskCB *task, INT32 sigNo)
{
    UINT32 intSave;
    OsSigCB *sigCB = NULL;
    sigset_t mask = LOS_SIGNAL_MASK(sigNo);

    intSave = LOS_IntLock();
    sigCB = task->sig;
    if (sigCB == NULL) {
        LOS_IntRestore(intSave);
        return;
    }

    if (!(sigCB->sigPendFlag & mask)) {
        sigCB->sigPendFlag |= mask;
    }

    if (task == OsCurrTaskGet()) {
        task->taskStatus |= OS_TASK_FLAG_SIGNAL;
        LOS_IntRestore(intSave);

        if (!OS_INT_ACTIVE) {
            SignalHandle(task, TRUE);
        }
    } else {
        if (sigCB->sigStatus & OS_SIGNAL_STATUS_WAIT) {
            if (sigCB->sigWaitFlag & LOS_SIGNAL_MASK(sigNo)) {
                DeleteSigInfoFromList(sigCB, sigNo);
                OsSchedTaskWake(task);
                task->taskStatus |= OS_TASK_FLAG_SIGNAL;
            }
        } else if (!(task->taskStatus & OS_TASK_FLAG_SIGNAL)) {
            task->taskStatus |= OS_TASK_FLAG_SIGNAL;
            sigCB->sigSaveSP = task->stackPointer;
            sigCB->sigRestoreSP = NULL;
            task->stackPointer = ArchSignalContextInit(task->stackPointer, (VOID *)task->topOfStack,
                                                       (UINTPTR)SignalEntry, sigNo);
        }
        LOS_IntRestore(intSave);
        LOS_Schedule();
    }
}

STATIC OsSigCB *SignalCBInit(LosTaskCB *task)
{
    OsSigCB *sigCB = NULL;
    UINT32 i;

    if (task->sig == NULL) {
        sigCB = (OsSigCB *)LOS_MemAlloc(OS_SYS_MEM_ADDR, sizeof(OsSigCB));
        if (sigCB == NULL) {
            return NULL;
        }
        (VOID)memset_s(sigCB, sizeof(OsSigCB), 0, sizeof(OsSigCB));
        LOS_ListInit(&sigCB->sigInfoList);

        for (i = 0; i <= LOS_SIGNAL_SUPPORT_MAX; i++) {
            sigCB->sigHandlers[i] = SignalDefaultHandler;
        }

        task->sig = (VOID *)sigCB;
    } else {
        sigCB = (OsSigCB *)task->sig;
    }

    return sigCB;
}

SIG_HANDLER LOS_SignalSet(INT32 sigNo, SIG_HANDLER handler)
{
    UINT32 intSave;
    SIG_HANDLER old = NULL;
    LosTaskCB *task = OsCurrTaskGet();
    OsSigCB *sigCB = NULL;

    if (task == NULL) {
        return SIG_ERR;
    }

    if (!OS_SIGNAL_VALID(sigNo)) {
        return SIG_ERR;
    }

    intSave = LOS_IntLock();
    sigCB = SignalCBInit(task);
    if (sigCB == NULL) {
        LOS_IntRestore(intSave);
        return SIG_ERR;
    }

    old = sigCB->sigHandlers[sigNo - 1]; /* signal number from 1, but index from 0 */
    if (handler == SIG_IGN) {
        sigCB->sigHandlers[sigNo - 1] = NULL;
        sigCB->sigSetFlag &= ~LOS_SIGNAL_MASK(sigNo);
    } else if (handler == SIG_DFL) {
        sigCB->sigHandlers[sigNo - 1] = SignalDefaultHandler;
        sigCB->sigSetFlag |= LOS_SIGNAL_MASK(sigNo);
    } else {
        sigCB->sigHandlers[sigNo - 1] = handler;
        sigCB->sigSetFlag |= LOS_SIGNAL_MASK(sigNo);
    }
    LOS_IntRestore(intSave);

    return old;
}

UINT32 LOS_SignalMask(INT32 how, const sigset_t *set, sigset_t *oldSet)
{
    UINT32 intSave;
    LosTaskCB *task = OsCurrTaskGet();
    OsSigCB *sigCB = NULL;

    if (task == NULL) {
        return LOS_ERRNO_SIGNAL_CAN_NOT_CALL;
    }

    intSave = LOS_IntLock();
    sigCB = SignalCBInit(task);
    if (sigCB == NULL) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_SIGNAL_NO_MEMORY;
    }

    if (oldSet != NULL) {
        *oldSet = sigCB->sigSetFlag;
    }

    if (set == NULL) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_SIGNAL_INVALID;
    }

    switch (how) {
        case SIG_BLOCK:
            sigCB->sigSetFlag &= ~*set;
            break;
        case SIG_SETMASK:
            sigCB->sigSetFlag = *set;
            break;
        case SIG_UNBLOCK:
            sigCB->sigSetFlag |= *set;
            break;
        default:
            PRINT_ERR("The error parameter how = %d\n", how);
            break;
    }
    LOS_IntRestore(intSave);

    return LOS_OK;
}

STATIC INLINE UINT32 SignalTimedWait(LosTaskCB *task, const sigset_t *set, UINT32 timeout, UINT32 *intSave)
{
    OsSigCB *sigCB = (OsSigCB *)task->sig;
    INT32 sigNo;

    if (timeout == 0) {
        LOS_IntRestore(*intSave);
        return LOS_ERRNO_SIGNAL_INVALID;
    }

    if (OS_INT_ACTIVE) {
        LOS_IntRestore(*intSave);
        return LOS_ERRNO_SIGNAL_PEND_INTERR;
    }

    sigCB->sigWaitFlag |= *set;
    sigCB->sigStatus |= OS_SIGNAL_STATUS_WAIT;

    OsSchedTaskWait(&g_waitSignalList, timeout);
    LOS_IntRestore(*intSave);
    LOS_Schedule();

    *intSave = LOS_IntLock();
    task->taskStatus &= ~OS_TASK_FLAG_SIGNAL;
    sigCB->sigStatus &= ~OS_SIGNAL_STATUS_WAIT;
    sigCB->sigWaitFlag = 0;
    if (task->taskStatus & OS_TASK_STATUS_TIMEOUT) {
        task->taskStatus &= ~OS_TASK_STATUS_TIMEOUT;
        LOS_IntRestore(*intSave);
        return LOS_ERRNO_SIGNAL_TIMEOUT;
    }
    sigNo = sigCB->sigInfo.si_signo;
    sigCB->sigPendFlag &= ~LOS_SIGNAL_MASK(sigNo);
    return sigNo;
}

UINT32 LOS_SignalWait(const sigset_t *set, siginfo_t *info, UINT32 timeout)
{
    UINT32 intSave;
    LosTaskCB *task = OsCurrTaskGet();
    OsSigCB *sigCB = NULL;
    sigset_t sigFlag;
    INT32 sigNo;

    if ((set == NULL) || (*set == 0)) {
        return LOS_ERRNO_SIGNAL_INVALID;
    }

    if (task == NULL) {
        return LOS_ERRNO_SIGNAL_CAN_NOT_CALL;
    }

    intSave = LOS_IntLock();
    sigCB = SignalCBInit(task);
    if (sigCB == NULL) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_SIGNAL_NO_MEMORY;
    }

    sigFlag = sigCB->sigPendFlag & *set;
    if (sigFlag) {
        sigCB->sigPendFlag ^= sigFlag;
        sigNo = LOS_SIGNAL_SUPPORT_MAX - CLZ(sigFlag) + 1;
        DeleteSigInfoFromList(sigCB, sigNo);
    } else {
        sigNo = SignalTimedWait(task, set, timeout, &intSave);
        if (sigNo > LOS_SIGNAL_SUPPORT_MAX) {
            LOS_IntRestore(intSave);
            return sigNo;
        }
    }

    if (info != NULL) {
        (VOID)memcpy_s(info, sizeof(siginfo_t), &sigCB->sigInfo, sizeof(siginfo_t));
    }
    LOS_IntRestore(intSave);

    return sigNo;
}

UINT32 LOS_SignalSend(UINT32 taskID, INT32 sigNo)
{
    siginfo_t info;
    UINT32 intSave;
    OsSigCB *sigCB = NULL;
    LosTaskCB *task = NULL;

    if (taskID > LOSCFG_BASE_CORE_TSK_LIMIT) {
        return LOS_ERRNO_SIGNAL_INVALID;
    }

    if (!OS_SIGNAL_VALID(sigNo)) {
        return LOS_ERRNO_SIGNAL_INVALID;
    }

    info.si_signo = sigNo;
    info.si_code = SI_USER;
    info.si_value.sival_ptr = NULL;

    intSave = LOS_IntLock();
    task = OS_TCB_FROM_TID(taskID);
    sigCB = SignalCBInit(task);
    if (sigCB == NULL) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_SIGNAL_NO_MEMORY;
    }

    if (!(sigCB->sigSetFlag & LOS_SIGNAL_MASK(sigNo))) { /* the signal has not been set */
        LOS_IntRestore(intSave);
        return LOS_ERRNO_SIGNAL_NO_SET;
    }

    UINT32 ret = AddSigInfoToList(sigCB, &info);
    if (ret != LOS_OK) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_SIGNAL_NO_MEMORY;
    }
    LOS_IntRestore(intSave);

    /* send signal to this thread */
    SignalSend(task, sigNo);

    return LOS_OK;
}

UINT32 OsSignalInit(VOID)
{
    LOS_ListInit(&g_waitSignalList);
    return LOS_OK;
}
#endif