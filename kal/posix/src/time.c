/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#include <errno.h>
#include <time.h>
#include <signal.h>
#include <unistd.h>
#include <los_swtmr.h>
#include <los_swtmr_pri.h>

#ifndef STATIC
#define STATIC static
#endif

#define OS_SYS_NS_PER_US 1000
#define OS_SYS_NS_PER_SECOND 1000000000
#define OS_SYS_US_PER_SECOND 1000000
#define OS_SYS_MS_PER_SECOND 1000

STATIC INLINE BOOL ValidTimerID(UINT16 swtmrID)
{
    /* check timer id */
    return (swtmrID < LOSCFG_BASE_CORE_SWTMR_LIMIT);
}

/* internal functions */
STATIC INLINE BOOL ValidTimeSpec(const struct timespec *tp)
{
    /* Fail a NULL pointer */
    if (tp == NULL) {
        return FALSE;
    }

    /* Fail illegal nanosecond values */
    if ((tp->tv_nsec < 0) || (tp->tv_nsec >= OS_SYS_NS_PER_SECOND) || (tp->tv_sec < 0)) {
        return FALSE;
    }

    return TRUE;
}

STATIC INLINE UINT32 OsTimeSpec2Tick(const struct timespec *tp)
{
    UINT64 tick, ns;

    ns = (UINT64)tp->tv_sec * OS_SYS_NS_PER_SECOND + tp->tv_nsec;
    /* Round up for ticks */
    tick = (ns * LOSCFG_BASE_CORE_TICK_PER_SECOND + (OS_SYS_NS_PER_SECOND - 1)) / OS_SYS_NS_PER_SECOND;
    if (tick > LOS_WAIT_FOREVER) {
        tick = LOS_WAIT_FOREVER;
    }
    return (UINT32)tick;
}

STATIC INLINE VOID OsTick2TimeSpec(struct timespec *tp, UINT32 tick)
{
    UINT64 ns = ((UINT64)tick * OS_SYS_NS_PER_SECOND) / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    tp->tv_sec = (time_t)(ns / OS_SYS_NS_PER_SECOND);
    tp->tv_nsec = (long)(ns % OS_SYS_NS_PER_SECOND);
}

int nanosleep(const struct timespec *req, struct timespec *rem)
{
    UINT64 us = (UINT64)req->tv_sec * OS_SYS_US_PER_SECOND + (req->tv_nsec + OS_SYS_NS_PER_US - 1) / OS_SYS_NS_PER_US;
    if (us > 0xFFFFFFFFU) {
        errno = EINVAL;
        return -1;
    }
    if (usleep(us) == 0) {
        if (rem) {
            rem->tv_sec = rem->tv_nsec = 0;
        }
        return 0;
    }
    return -1;
}

int timer_create(clockid_t clockID, struct sigevent *restrict evp, timer_t *restrict timerID)
{
    UINT32 ret;
    UINT16 swtmrID;

    if (!timerID || (clockID != CLOCK_REALTIME)) {
        errno = EINVAL;
        return -1;
    }

    if (!evp || evp->sigev_notify != SIGEV_THREAD || evp->sigev_notify_attributes) {
        errno = ENOTSUP;
        return -1;
    }

    ret = LOS_SwtmrCreate(1, LOS_SWTMR_MODE_ONCE, (SWTMR_PROC_FUNC)evp->sigev_notify_function,
                          &swtmrID, (UINT32)(UINTPTR)evp->sigev_value.sival_ptr);
    if (ret != LOS_OK) {
        errno = (ret == LOS_ERRNO_SWTMR_MAXSIZE) ? EAGAIN : EINVAL;
        return -1;
    }

    *timerID = (timer_t)(UINTPTR)swtmrID;
    return 0;
}

int timer_delete(timer_t timerID)
{
    UINT16 swtmrID = (UINT16)(UINTPTR)timerID;

    if (!ValidTimerID(swtmrID)) {
        errno = EINVAL;
        return -1;
    }

    if (LOS_SwtmrDelete(swtmrID) != LOS_OK) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int timer_settime(timer_t timerID, int flags,
                  const struct itimerspec *restrict value,
                  struct itimerspec *restrict oldValue)
{
    UINT16 swtmrID = (UINT16)(UINTPTR)timerID;
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 interval, expiry, ret;

    if (flags != 0) {
        /* flags not supported currently */
        errno = ENOSYS;
        return -1;
    }

    if (value == NULL || !ValidTimerID(swtmrID)) {
        errno = EINVAL;
        return -1;
    }

    if (!ValidTimeSpec(&value->it_value) || !ValidTimeSpec(&value->it_interval)) {
        errno = EINVAL;
        return -1;
    }

    expiry = OsTimeSpec2Tick(&value->it_value);
    interval = OsTimeSpec2Tick(&value->it_interval);

    /* if specified interval, it must be same with expiry due to the limitation of liteos-m */
    if (interval && interval != expiry) {
        errno = ENOTSUP;
        return -1;
    }

    if (oldValue) {
        (VOID)timer_gettime(timerID, oldValue);
    }

    ret = LOS_SwtmrStop(swtmrID);
    if ((ret != LOS_OK) && (ret != LOS_ERRNO_SWTMR_NOT_STARTED)) {
        errno = EINVAL;
        return -1;
    }

    swtmr = OS_SWT_FROM_SID(swtmrID);
    ret = LOS_SwtmrModify(swtmrID, expiry, (interval ? LOS_SWTMR_MODE_PERIOD : LOS_SWTMR_MODE_NO_SELFDELETE),
                          swtmr->pfnHandler, swtmr->uwArg);
    if (ret != LOS_OK) {
        errno = EINVAL;
        return -1;
    }

    if ((value->it_value.tv_sec == 0) && (value->it_value.tv_nsec == 0)) {
        /*
         * 1) when expiry is 0, means timer should be stopped.
         * 2) If timer is ticking, stopping timer is already done before.
         * 3) If timer is created but not ticking, return 0 as well.
         */
        return 0;
    }

    if (LOS_SwtmrStart(swtmr->usTimerID) != LOS_OK) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int timer_gettime(timer_t timerID, struct itimerspec *value)
{
    UINT32 tick = 0;
    SWTMR_CTRL_S *swtmr = NULL;
    UINT16 swtmrID = (UINT16)(UINTPTR)timerID;
    UINT32 ret;

    if ((value == NULL) || !ValidTimerID(swtmrID)) {
        errno = EINVAL;
        return -1;
    }

    swtmr = OS_SWT_FROM_SID(swtmrID);

    /* get expire time */
    ret = LOS_SwtmrTimeGet(swtmr->usTimerID, &tick);
    if ((ret != LOS_OK) && (ret != LOS_ERRNO_SWTMR_NOT_STARTED)) {
        errno = EINVAL;
        return -1;
    }

    OsTick2TimeSpec(&value->it_value, tick);
    OsTick2TimeSpec(&value->it_interval, (swtmr->ucMode == LOS_SWTMR_MODE_ONCE) ? 0 : swtmr->uwInterval);
    return 0;
}

int timer_getoverrun(timer_t timerID)
{
    UINT16 swtmrID = (UINT16)(UINTPTR)timerID;

    if (!ValidTimerID(swtmrID)) {
        errno = EINVAL;
        return -1;
    }

    errno = ENOSYS;
    return -1;
}
