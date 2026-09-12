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
 * "AS IS, AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LOS_TASK_BASE_H
#define _LOS_TASK_BASE_H

#include "los_task.h"
#include "los_spinlock.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task control block is unused.
 */
#define OS_TASK_STATUS_UNUSED                       0x0001

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is suspended.
 */
#define OS_TASK_STATUS_SUSPEND                      0x0002

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is ready.
 */
#define OS_TASK_STATUS_READY                        0x0004

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is blocked.
 */
#define OS_TASK_STATUS_PEND                         0x0008

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is running.
 */
#define OS_TASK_STATUS_RUNNING                      0x0010

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is delayed.
 */
#define OS_TASK_STATUS_DELAY                        0x0020

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The time for waiting for an event to occur expires.
 */
#define OS_TASK_STATUS_TIMEOUT                      0x0040

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task is blocked on a time.
 */
#define OS_TASK_STATUS_PEND_TIME                    0x0080

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The task exits and waits for the parent thread to reclaim the resource.
 */
#define OS_TASK_STATUS_EXIT                         0x0100

#if (LOSCFG_SECURE == 1)
/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * This task is a user mode task.
 */
#define OS_TASK_FLAG_USER_TASK                      0x0200
#endif


#define OS_TASK_FLAG_DETACHED                       0x0001U
#define OS_TASK_FLAG_JOINABLE                       0x0000U
#define OS_TASK_FLAG_SYSTEM                         0x0002U
#ifdef LOSCFG_TASK_JOINABLE
#define OS_TASK_IS_JOINABLE(taskCB)                 (((taskCB)->taskFlags & OS_TASK_FLAG_DETACHED) == 0)
#define OS_TASK_IS_DETACHED(taskCB)                 (((taskCB)->taskFlags & OS_TASK_FLAG_DETACHED) != 0)
#else
#define OS_TASK_IS_JOINABLE(taskCB)                 FALSE
#define OS_TASK_IS_DETACHED(taskCB)                 TRUE
#endif

/**
 * @ingroup los_task
 * Flag that indicates the task is processing signal.
 *
 */
#define OS_TASK_FLAG_SIGNAL                         0x2000

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status.
 *
 * The delayed operation of this task is frozen.
 */
#define OS_TASK_FLAG_FREEZE                         0x4000


/**
 * @ingroup los_task
 * Define the task control block structure.
 */

typedef struct LosTaskCB {
    VOID                        *stackPointer;            /**< Task stack pointer */
    UINT16                      taskStatus;
    UINT16                      priority;
    UINT32                      taskFlags : 31;          /**< Task attribute flags */
    UINT32                      usrStack : 1;            /**< Usr Stack uses the last bit */
    UINT32                      priBitMap;               /**< BitMap for priority inheritance */
    INT32                       timeSlice;
    UINT32                      waitTimes;
    SortLinkList                sortList;
    UINT64                      startTime;
    UINT32                      stackSize;                /**< Task stack size */
    UINTPTR                     topOfStack;               /**< Task stack top */
    UINT32                      taskId;                   /**< Task ID */
    TSK_ENTRY_FUNC              taskEntry;                /**< Task entrance function */
    VOID                        *taskSem;                 /**< Task-held semaphore */
    VOID                        *taskMux;                 /**< Task-held mutex */
#ifdef LOSCFG_KERNEL_TASK_ENTRY_VOID_PTR
    VOID                       *args;                     /**< Parameter (VOID*) */
#else
    UINT32                      arg;                      /**< Parameter */
#endif
    CHAR                        *taskName;                /**< Task name */
    LOS_DL_LIST                 pendList;
    LOS_DL_LIST                 timerList;
#ifdef LOSCFG_TASK_JOINABLE
    LOS_DL_LIST                 joinList;
    UINTPTR                     taskRetval;               /**< Return value of the end of the task, If the task does not exit by itself, the ID of the task that killed the task is recorded. */
#endif
#ifdef LOSCFG_BASE_IPC_EVENT
    EVENT_CB_S                  event;
    UINT32                      eventMask;                /**< Event mask */
    UINT32                      eventMode;                /**< Event mode */
#endif
#ifdef LOSCFG_BASE_IPC_RWSEM
    UINT16                      waitType;                 /**< The type of the rwsem that the task waits for */
#endif
    VOID                        *msg;                     /**< Memory allocated to queues */
    INT32                       errorNo;
#if (LOSCFG_KERNEL_SIGNAL == 1)
    VOID                        *sig;                     /**< Task signal */
#endif


#ifdef LOSCFG_TASK_STRUCT_EXTENSION
    LOSCFG_TASK_STRUCT_EXTENSION;                         /**< Task extension field */
#endif
    void *rwList;
} LosTaskCB;

extern SPIN_LOCK_S g_taskSpin;

#define SCHEDULER_LOCK(state)       LOS_SpinLockSave(&g_taskSpin, &(state))
#define SCHEDULER_UNLOCK(state)     LOS_SpinUnlockRestore(&g_taskSpin, (state))

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_TASK_BASE_H */
