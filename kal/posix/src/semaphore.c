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

#include <stdint.h>
#include <semaphore.h>
#include <errno.h>
#include "los_sem.h"
#include "time_internal.h"

#define _SEM_MAGIC 0xEBCFDEA1

#define s_magic __val[0]
#define s_handle __val[1]

static inline int MapError(UINT32 err)
{
    switch (err) {
        case LOS_OK:
            return 0;
        case LOS_ERRNO_SEM_INVALID:
        case LOS_ERRNO_SEM_UNAVAILABLE:
            return EINVAL;
        case LOS_ERRNO_SEM_ALL_BUSY:
            return ENOSPC;
        case LOS_ERRNO_SEM_OVERFLOW:
            return ENOMEM;
        case LOS_ERRNO_SEM_PENDED:
            return EBUSY;
        case LOS_ERRNO_SEM_PEND_IN_LOCK:
            return EPERM;
        case LOS_ERRNO_SEM_PEND_INTERR:
            return EINTR;
        case LOS_ERRNO_SEM_TIMEOUT:
            return ETIMEDOUT;
        default:
            return EINVAL;
    }
}

int sem_init(sem_t *sem, int shared, unsigned int value)
{
    UINT32 semHandle = 0;
    UINT32 ret;

    (VOID)shared;
    if ((sem == NULL) || (value >= OS_SEM_COUNTING_MAX_COUNT)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemCreate(value, &semHandle);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    sem->s_magic = (INT32)_SEM_MAGIC;
    sem->s_handle = (INT32)semHandle;

    return 0;
}

int sem_destroy(sem_t *sem)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemDelete((UINT32)sem->s_handle);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_wait(sem_t *sem)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemPend((UINT32)sem->s_handle, LOS_WAIT_FOREVER);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_post(sem_t *sem)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemPost((UINT32)sem->s_handle);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_trywait(sem_t *sem)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemPend((UINT32)sem->s_handle, LOS_NO_WAIT);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_timedwait(sem_t *sem, const struct timespec *timeout)
{
    UINT32 ret;
    UINT64 tickCnt;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)) {
        errno = EINVAL;
        return -1;
    }

    ret = OsGetTickTimeFromNow(timeout, CLOCK_REALTIME, &tickCnt);
    if (ret != 0) {
        errno = (INT32)ret;
        return -1;
    }

    if (tickCnt > LOS_WAIT_FOREVER) {
        tickCnt = LOS_WAIT_FOREVER;
    }

    ret = LOS_SemPend((UINT32)sem->s_handle, (UINT32)tickCnt);
    if (ret != LOS_OK) {
        errno = MapError(ret);
        return -1;
    }

    return 0;
}

int sem_getvalue(sem_t *sem, int *currVal)
{
    UINT32 ret;

    if ((sem == NULL) || (sem->s_magic != (INT32)_SEM_MAGIC)|| (currVal == NULL)) {
        errno = EINVAL;
        return -1;
    }

    ret = LOS_SemGetValue(sem->s_handle, currVal);
    if (ret) {
        errno = EINVAL;
        return -1;
    }

    return LOS_OK;
}