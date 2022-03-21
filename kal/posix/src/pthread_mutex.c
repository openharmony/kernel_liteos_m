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

#include <pthread.h>
#include <time.h>
#include <securec.h>
#include "los_compiler.h"
#include "los_mux.h"
#include "errno.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_sched.h"

#define MUTEXATTR_TYPE_MASK   0x0FU
#define OS_SYS_NS_PER_MSECOND 1000000
#define OS_SYS_NS_PER_SECOND  1000000000

static inline int MapError(UINT32 err)
{
    switch (err) {
        case LOS_OK:
            return 0;
        case LOS_ERRNO_MUX_IN_INTERR:
            return EPERM;
        case LOS_ERRNO_MUX_PEND_IN_LOCK:
            return EDEADLK;
        case LOS_ERRNO_MUX_PENDED:
        case LOS_ERRNO_MUX_UNAVAILABLE:
            return EBUSY;
        case LOS_ERRNO_MUX_TIMEOUT:
            return ETIMEDOUT;
        case LOS_ERRNO_MUX_ALL_BUSY:
            return EAGAIN;
        case LOS_ERRNO_MUX_INVALID:
        default:
            return EINVAL;
    }
}

int pthread_mutexattr_init(pthread_mutexattr_t *mutexAttr)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    mutexAttr->type = PTHREAD_MUTEX_DEFAULT;

    return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *outType)
{
    INT32 type;

    if ((attr == NULL) || (outType == NULL)) {
        return EINVAL;
    }

    type = (INT32)(attr->type & MUTEXATTR_TYPE_MASK);
    if ((type != PTHREAD_MUTEX_NORMAL) &&
        (type != PTHREAD_MUTEX_RECURSIVE) &&
        (type != PTHREAD_MUTEX_ERRORCHECK)) {
        return EINVAL;
    }

    *outType = type;

    return 0;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *mutexAttr, int type)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    if (((unsigned)type != PTHREAD_MUTEX_NORMAL) &&
        ((unsigned)type != PTHREAD_MUTEX_RECURSIVE) &&
        ((unsigned)type != PTHREAD_MUTEX_ERRORCHECK)) {
        return EINVAL;
    }
    mutexAttr->type = (UINT8)((mutexAttr->type & ~MUTEXATTR_TYPE_MASK) | (UINT32)type);

    return 0;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *mutexAttr)
{
    if (mutexAttr == NULL) {
        return EINVAL;
    }

    (VOID)memset_s(mutexAttr, sizeof(pthread_mutexattr_t), 0, sizeof(pthread_mutexattr_t));

    return 0;
}

/* Initialize mutex. If mutexAttr is NULL, use default attributes. */
int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexAttr)
{
    pthread_mutexattr_t useAttr;
    UINT32 muxHandle;
    UINT32 ret;

    if (mutex == NULL) {
        return EINVAL;
    }

    if (mutexAttr == NULL) {
        (VOID)pthread_mutexattr_init(&useAttr);
    } else {
        useAttr = *mutexAttr;
    }

    ret = LOS_MuxCreate(&muxHandle);
    if (ret != LOS_OK) {
        return MapError(ret);
    }

    mutex->stAttr = useAttr;
    mutex->magic = _MUX_MAGIC;
    mutex->handle = muxHandle;

    return 0;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    UINT32 ret;
    if ((mutex == NULL) || (mutex->magic != _MUX_MAGIC)) {
        return EINVAL;
    }
    ret = LOS_MuxDelete(mutex->handle);
    if (ret != LOS_OK) {
        return MapError(ret);
    }
    mutex->handle = _MUX_INVALID_HANDLE;
    mutex->magic = 0;

    return 0;
}

STATIC UINT32 CheckMutexAttr(const pthread_mutexattr_t *attr)
{
    if ((attr->type != PTHREAD_MUTEX_NORMAL) &&
        (attr->type != PTHREAD_MUTEX_RECURSIVE) &&
        (attr->type != PTHREAD_MUTEX_ERRORCHECK)) {
        return LOS_NOK;
    }

    return LOS_OK;
}

STATIC UINT32 MuxPreCheck(const pthread_mutex_t *mutex, const LosTaskCB *runTask)
{
    if ((mutex == NULL) || (mutex->magic != _MUX_MAGIC) ||
        ((mutex->handle != _MUX_INVALID_HANDLE) && (mutex->handle >= (UINT32)LOSCFG_BASE_IPC_MUX_LIMIT))) {
        return EINVAL;
    }

    if (OS_INT_ACTIVE) {
        return EPERM;
    }
    /* DO NOT recommend to use blocking API in system tasks */
    if ((runTask != NULL) && (runTask->taskStatus & OS_TASK_FLAG_SYSTEM_TASK)) {
        PRINT_DEBUG("Warning: DO NOT recommend to use %s in system tasks.\n", __FUNCTION__);
    }

    if (CheckMutexAttr(&mutex->stAttr) != LOS_OK) {
        return EINVAL;
    }

    return 0;
}

STATIC UINT32 MuxPendForPosix(pthread_mutex_t *mutex, UINT32 timeout)
{
    UINT32 intSave;
    LosMuxCB *muxPended = NULL;
    UINT32 retErr;
    LosTaskCB *runningTask = NULL;
    UINT32 muxHandle = mutex->handle;

    muxPended = GET_MUX(muxHandle);
    intSave = LOS_IntLock();

    if (muxPended->muxStat == OS_MUX_UNUSED) {
        LOS_IntRestore(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    runningTask = (LosTaskCB *)g_losTask.runTask;
    if (muxPended->muxCount == 0) {
        muxPended->muxCount++;
        muxPended->owner = runningTask;
        muxPended->priority = runningTask->priority;
        LOS_IntRestore(intSave);
        OsHookCall(LOS_HOOK_TYPE_MUX_PEND, muxPended, timeout);
        return LOS_OK;
    }

    if ((muxPended->owner == runningTask) && (mutex->stAttr.type == PTHREAD_MUTEX_RECURSIVE)) {
        muxPended->muxCount++;
        LOS_IntRestore(intSave);
        OsHookCall(LOS_HOOK_TYPE_MUX_PEND, muxPended, timeout);
        return LOS_OK;
    }

    if (!timeout) {
        LOS_IntRestore(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_MUX_UNAVAILABLE);
    }

    runningTask->taskMux = (VOID *)muxPended;

    if (muxPended->owner->priority > runningTask->priority) {
        (VOID)OsSchedModifyTaskSchedParam(muxPended->owner, runningTask->priority);
    }

    OsSchedTaskWait(&muxPended->muxList, timeout);

    LOS_IntRestore(intSave);
    OsHookCall(LOS_HOOK_TYPE_MUX_PEND, muxPended, timeout);
    LOS_Schedule();

    intSave = LOS_IntLock();
    if (runningTask->taskStatus & OS_TASK_STATUS_TIMEOUT) {
        runningTask->taskStatus &= (~OS_TASK_STATUS_TIMEOUT);
        retErr = LOS_ERRNO_MUX_TIMEOUT;
        LOS_IntRestore(intSave);
        OS_RETURN_ERROR(retErr);
    }

    LOS_IntRestore(intSave);
    return LOS_OK;
}

STATIC UINT32 MuxPostForPosix(pthread_mutex_t *mutex)
{
    UINT32 intSave;
    LosMuxCB *muxPosted = NULL;
    LosTaskCB *resumedTask = NULL;
    LosTaskCB *runningTask = NULL;
    UINT32 muxHandle = mutex->handle;

    muxPosted = GET_MUX(muxHandle);
    intSave = LOS_IntLock();

    if (muxPosted->muxStat == OS_MUX_UNUSED) {
        LOS_IntRestore(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    runningTask = (LosTaskCB *)g_losTask.runTask;
    if ((muxPosted->muxCount == 0) || (muxPosted->owner != runningTask)) {
        LOS_IntRestore(intSave);
        OS_RETURN_ERROR(LOS_ERRNO_MUX_INVALID);
    }

    if ((--(muxPosted->muxCount) != 0) && (mutex->stAttr.type == PTHREAD_MUTEX_RECURSIVE)) {
        LOS_IntRestore(intSave);
        OsHookCall(LOS_HOOK_TYPE_MUX_POST, muxPosted);
        return LOS_OK;
    }

    if ((muxPosted->owner->priority) != muxPosted->priority) {
        (VOID)OsSchedModifyTaskSchedParam(muxPosted->owner, muxPosted->priority);
    }

    if (!LOS_ListEmpty(&muxPosted->muxList)) {
        resumedTask = OS_TCB_FROM_PENDLIST(LOS_DL_LIST_FIRST(&(muxPosted->muxList)));

        muxPosted->muxCount = 1;
        muxPosted->owner = resumedTask;
        muxPosted->priority = resumedTask->priority;
        resumedTask->taskMux = NULL;

        OsSchedTaskWake(resumedTask);

        LOS_IntRestore(intSave);
        OsHookCall(LOS_HOOK_TYPE_MUX_POST, muxPosted);
        LOS_Schedule();
    } else {
        muxPosted->owner = NULL;
        LOS_IntRestore(intSave);
    }

    return LOS_OK;
}

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *absTimeout)
{
    UINT32 ret;
    UINT32 timeout;
    UINT64 timeoutNs;
    struct timespec curTime = {0};
    LosMuxCB *muxPended = NULL;

    ret = MuxPreCheck(mutex, OS_TCB_FROM_TID(LOS_CurTaskIDGet()));
    if (ret != 0) {
        return (INT32)ret;
    }

    if ((absTimeout == NULL) || (absTimeout->tv_nsec < 0) || (absTimeout->tv_nsec >= OS_SYS_NS_PER_SECOND)) {
        return EINVAL;
    }
    if (mutex->handle == _MUX_INVALID_HANDLE) {
        ret = LOS_MuxCreate(&mutex->handle);
        if (ret != LOS_OK) {
            return MapError(ret);
        }
    } else {
        muxPended = GET_MUX(mutex->handle);
        if ((mutex->stAttr.type == PTHREAD_MUTEX_ERRORCHECK) &&
            (muxPended->muxCount != 0) &&
            (muxPended->owner == OS_TCB_FROM_TID(LOS_CurTaskIDGet()))) {
            return EDEADLK;
        }
    }
    ret = clock_gettime(CLOCK_REALTIME, &curTime);
    if (ret != LOS_OK) {
        return EINVAL;
    }
    timeoutNs = (absTimeout->tv_sec - curTime.tv_sec) * OS_SYS_NS_PER_SECOND + (absTimeout->tv_nsec - curTime.tv_nsec);
    if (timeoutNs <= 0) {
        return ETIMEDOUT;
    }
    timeout = (timeoutNs + (OS_SYS_NS_PER_MSECOND - 1)) / OS_SYS_NS_PER_MSECOND;
    ret = MuxPendForPosix(mutex, timeout);

    return MapError(ret);
}

/* Lock mutex, waiting for it if necessary. */
int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    UINT32 ret;
    LosMuxCB *muxPended = NULL;
    LosTaskCB *runTask = OS_TCB_FROM_TID(LOS_CurTaskIDGet());

    ret = MuxPreCheck(mutex, runTask);
    if (ret != 0) {
        return (INT32)ret;
    }

    if (mutex->handle == _MUX_INVALID_HANDLE) {
        ret = LOS_MuxCreate(&mutex->handle);
        if (ret != LOS_OK) {
            return MapError(ret);
        }
    } else {
        muxPended = GET_MUX(mutex->handle);
        if ((mutex->stAttr.type == PTHREAD_MUTEX_ERRORCHECK) &&
            (muxPended->muxCount != 0) &&
            (muxPended->owner == runTask)) {
            return EDEADLK;
        }
    }
    ret = MuxPendForPosix(mutex, LOS_WAIT_FOREVER);

    return MapError(ret);
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    UINT32 ret;
    LosMuxCB *muxPended = NULL;

    ret = MuxPreCheck(mutex, OS_TCB_FROM_TID(LOS_CurTaskIDGet()));
    if (ret != 0) {
        return (INT32)ret;
    }

    if (mutex->handle == _MUX_INVALID_HANDLE) {
        ret = LOS_MuxCreate(&mutex->handle);
        if (ret != LOS_OK) {
            return MapError(ret);
        }
    } else {
        muxPended = GET_MUX(mutex->handle);
        if ((mutex->stAttr.type != PTHREAD_MUTEX_RECURSIVE) && (muxPended->muxCount != 0)) {
            return EBUSY;
        }
    }
    ret = MuxPendForPosix(mutex, 0);

    return MapError(ret);
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    UINT32 ret;
    ret = MuxPreCheck(mutex, OS_TCB_FROM_TID(LOS_CurTaskIDGet()));
    if (ret != 0) {
        return (INT32)ret;
    }

    if (mutex->handle == _MUX_INVALID_HANDLE) {
        return EINVAL;
    }

    ret = MuxPostForPosix(mutex);

    return MapError(ret);
}

