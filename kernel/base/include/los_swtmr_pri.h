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
 * "AS IS, AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
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

#ifndef _LOS_SWTMR_PRI_H
#define _LOS_SWTMR_PRI_H

#include "los_swtmr.h"
#include "los_spinlock.h"
#include "los_percpu_pri.h"

#ifndef OS_ALL_SWTMR_MASK
#define OS_ALL_SWTMR_MASK 0xFFFFFFFF
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#ifdef LOSCFG_BASE_CORE_SWTMR

/**
 * @ingroup los_swtmr
 * Software timer state
 */
enum SwtmrState {
    OS_SWTMR_STATUS_UNUSED,             /**< The software timer is not used. */
    OS_SWTMR_STATUS_CREATED,            /**< The software timer is created. */
    OS_SWTMR_STATUS_TICKING,            /**< The software timer is timing. */
    OS_SWTMR_STATUS_DELETING            /**< The software timer is pending deletion (handler running). */
};

/**
 * @ingroup los_swtmr
 * Structure of the callback function that handles software timer timeout
 */
typedef struct {
    SWTMR_PROC_FUNC     handler;        /**< Callback function that handles software timer timeout */
    UINT32              arg;            /**< Parameter passed in when the callback function
                                             that handles software timer timeout is called */
    UINT32              swtmrID;        /**< The id used to obtain the software timer handle */
} SwtmrHandlerItem;

/**
 * @ingroup los_swtmr
 * Software timer control structure
 */
typedef struct tagSwTmrCtrl {
    struct tagSwTmrCtrl *pstNext;       /* Pointer to the next software timer                    */
    UINT8               ucState;        /* Software timer state                                  */
    UINT8               ucMode;         /* Software timer mode                                   */
    UINT8               ucOverrun;      /* Times that a software timer repeats timing            */
    UINT8               inProcess;      /* Handler-in-progress counter (deferred delete support)  */
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    UINT8               ucRouses;       /* wake up enable                                        */
    UINT8               ucSensitive;    /* align enable                                          */
#endif
    UINT32              usTimerID;      /* Software timer ID                                     */
    UINT32              uwInterval;     /* Timeout interval of a periodic software timer         */
    UINT32              expiry;        /* Timeout interval of a one-off software timer         */
    UINT32              uwArg;          /* Parameter passed in when the callback function
                                           that handles software timer timeout is called         */
    SWTMR_PROC_FUNC     pfnHandler;     /* Callback function that handles software timer timeout */
    SortLinkList        stSortList;
    UINT64              startTime;
#ifdef LOSCFG_KERNEL_SMP
    UINT32              cpuid;          /* The cpu where the timer running on                  */
#endif
} SWTMR_CTRL_S;

extern SWTMR_CTRL_S *g_swtmrCBArray;
extern LOS_DL_LIST g_swtmrFreeList;
extern SPIN_LOCK_S g_swtmrSpin;

#define SWTMR_LOCK(state)       LOS_SpinLockSave(&g_swtmrSpin, &(state))
#define SWTMR_UNLOCK(state)     LOS_SpinUnlockRestore(&g_swtmrSpin, (state))

#define OS_SWT_FROM_SID(swtmrId)    ((SWTMR_CTRL_S *)g_swtmrCBArray + ((swtmrId) % LOSCFG_BASE_CORE_SWTMR_LIMIT))

#if defined(LOSCFG_TASK_STACK_STATIC_ALLOCATION) && !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
extern UINT8 *g_osSwtmrTaskStack[LOSCFG_KERNEL_CORE_NUM];
#endif

extern BOOL OsSwtmrScan(VOID);
extern UINT32 OsSwtmrInit(VOID);
#if !defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR)
extern VOID OsSwtmrTask(VOID);
#endif
extern BOOL OsSwtmrIdVerify(UINT32 swtmrId);
extern SWTMR_CTRL_S *OsSwtmrIdGet(UINT32 swtmrId);
/* OsSwtmrCheckSelfDelete is STATIC INLINE in los_swtmr.c */
#ifdef LOSCFG_EXC_INTERACTION
extern BOOL IsSwtmrTask(UINT32 taskId);
#endif
#ifdef LOSCFG_DEBUG_SWTMR
extern UINT32 OsDbgSwtmrInfoGet(UINT32 timerId);
#endif
extern UINT32 OsSwtmrStartTimer(UINT32 swtmrId, UINT32 interval, UINT32 expiry);

#endif /* LOSCFG_BASE_CORE_SWTMR */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_SWTMR_PRI_H */
