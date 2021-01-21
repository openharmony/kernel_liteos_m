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

#include <time.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "los_debug.h"
#include "los_task.h"
#include "los_swtmr.h"
#include "los_timer.h"
#include "los_context.h"

#ifndef STATIC
#define STATIC static
#endif

#define OS_SYS_NS_PER_US 1000
#define OS_SYS_NS_PER_SECOND 1000000000
#define OS_SYS_US_PER_SECOND 1000000
#define OS_SYS_MS_PER_SECOND 1000

/* accumulative time delta from discontinuous modify */
STATIC struct timespec g_accDeltaFromSet;

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

int nanosleep(const struct timespec *rqtp, struct timespec *rmtp)
{
    UINT64 nseconds;
    UINT64 tick;
    UINT32 ret;
    const UINT32 nsPerTick = OS_SYS_NS_PER_SECOND / LOSCFG_BASE_CORE_TICK_PER_SECOND;

    if (!ValidTimeSpec(rqtp)) {
        errno = EINVAL;
        return -1;
    }

    nseconds = (UINT64)rqtp->tv_sec * OS_SYS_NS_PER_SECOND + rqtp->tv_nsec;

    tick = (nseconds + nsPerTick - 1) / nsPerTick; // Round up for ticks

    if (tick >= UINT32_MAX) {
        errno = EINVAL;
        return -1;
    }

    /* PS: skip the first tick because it is NOT a full tick. */
    ret = LOS_TaskDelay(tick ? (UINT32)(tick + 1) : 0);
    if (ret == LOS_OK || ret == LOS_ERRNO_TSK_YIELD_NOT_ENOUGH_TASK) {
        if (rmtp) {
            rmtp->tv_sec = rmtp->tv_nsec = 0;
        }
        return 0;
    }

    /* sleep in interrupt context or in task sched lock state */
    errno = EPERM;
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
                          &swtmrID, (UINT32)(UINTPTR)evp->sigev_value.sival_ptr
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
                          , OS_SWTMR_ROUSES_IGNORE, OS_SWTMR_ALIGN_INSENSITIVE
#endif
    );
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
    UINTPTR intSave;
    UINT16 swtmrID = (UINT16)(UINTPTR)timerID;
    SWTMR_CTRL_S *swtmr = NULL;
    UINT32 interval, expiry, ret;

    if (flags != 0) {
        /* flags not supported currently */
        errno = ENOSYS;
        return -1;
    }

    if (value == NULL) {
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

    intSave = LOS_IntLock();
    swtmr = OS_SWT_FROM_SID(swtmrID);
    swtmr->ucMode = (interval ? LOS_SWTMR_MODE_PERIOD : LOS_SWTMR_MODE_NO_SELFDELETE);
    swtmr->uwInterval = interval;
    LOS_IntRestore(intSave);

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

    if (value == NULL) {
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
    (void)timerID;

    errno = ENOSYS;
    return -1;
}

STATIC VOID OsGetHwTime(struct timespec *hwTime)
{
    UINT64 nowNsec;
    UINT32 countHigh = 0;
    UINT32 countLow = 0;
    HalGetCpuCycle(&countHigh, &countLow);
    nowNsec = (((UINT64)countHigh * OS_SYS_NS_PER_SECOND / OS_SYS_CLOCK) << 32) +
              ((((UINT64)countHigh * OS_SYS_NS_PER_SECOND % OS_SYS_CLOCK) << 32) / OS_SYS_CLOCK) +
              ((UINT64)countLow * OS_SYS_NS_PER_SECOND / OS_SYS_CLOCK);
    hwTime->tv_sec = nowNsec / OS_SYS_NS_PER_SECOND;
    hwTime->tv_nsec = nowNsec % OS_SYS_NS_PER_SECOND;
}

STATIC VOID OsGetRealTime(struct timespec *realTime)
{
    UINTPTR intSave;
    struct timespec hwTime = {0};
    OsGetHwTime(&hwTime);
    intSave = LOS_IntLock();
    realTime->tv_nsec = hwTime.tv_nsec + g_accDeltaFromSet.tv_nsec;
    realTime->tv_sec = hwTime.tv_sec + g_accDeltaFromSet.tv_sec + (realTime->tv_nsec >= OS_SYS_NS_PER_SECOND);
    realTime->tv_nsec %= OS_SYS_NS_PER_SECOND;
    LOS_IntRestore(intSave);
}

STATIC VOID OsSetRealTime(const struct timespec *realTime)
{
    UINTPTR intSave;
    struct timespec hwTime = {0};
    OsGetHwTime(&hwTime);
    intSave = LOS_IntLock();
    g_accDeltaFromSet.tv_nsec = realTime->tv_nsec - hwTime.tv_nsec;
    g_accDeltaFromSet.tv_sec = realTime->tv_sec - hwTime.tv_sec - (g_accDeltaFromSet.tv_nsec < 0);
    g_accDeltaFromSet.tv_nsec = (g_accDeltaFromSet.tv_nsec + OS_SYS_NS_PER_SECOND) % OS_SYS_NS_PER_SECOND;
    LOS_IntRestore(intSave);
}

int clock_settime(clockid_t clockID, const struct timespec *tp)
{
    if (!ValidTimeSpec(tp)) {
        errno = EINVAL;
        return -1;
    }

    switch (clockID) {
        case CLOCK_REALTIME:
            /* we only support the realtime clock currently */
            OsSetRealTime(tp);
            return 0;
        case CLOCK_MONOTONIC_COARSE:
        case CLOCK_REALTIME_COARSE:
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_BOOTTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_BOOTTIME_ALARM:
        case CLOCK_SGI_CYCLE:
        case CLOCK_TAI:
        case CLOCK_THREAD_CPUTIME_ID:
            errno = ENOTSUP;
            return -1;
        case CLOCK_MONOTONIC:
        default:
            errno = EINVAL;
            return -1;
    }
}

int clock_gettime(clockid_t clockID, struct timespec *tp)
{
    if (tp == NULL) {
        errno = EINVAL;
        return -1;
    }

    switch (clockID) {
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_MONOTONIC:
        case CLOCK_MONOTONIC_COARSE:
            OsGetHwTime(tp);
            return 0;
        case CLOCK_REALTIME:
        case CLOCK_REALTIME_COARSE:
            OsGetRealTime(tp);
            return 0;
        case CLOCK_THREAD_CPUTIME_ID:
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_BOOTTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_BOOTTIME_ALARM:
        case CLOCK_SGI_CYCLE:
        case CLOCK_TAI:
            errno = ENOTSUP;
            return -1;
        default:
            errno = EINVAL;
            return -1;
    }
}

int clock_getres(clockid_t clockID, struct timespec *tp)
{
    if (tp == NULL) {
        errno = EINVAL;
        return -1;
    }

    switch (clockID) {
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_MONOTONIC:
        case CLOCK_REALTIME:
        case CLOCK_MONOTONIC_COARSE:
        case CLOCK_REALTIME_COARSE:
            tp->tv_nsec = OS_SYS_NS_PER_SECOND / OS_SYS_CLOCK;
            tp->tv_sec = 0;
            return 0;
        case CLOCK_THREAD_CPUTIME_ID:
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_BOOTTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_BOOTTIME_ALARM:
        case CLOCK_SGI_CYCLE:
        case CLOCK_TAI:
            errno = ENOTSUP;
            return -1;
        default:
            errno = EINVAL;
            return -1;
    }
}

int clock_nanosleep(clockid_t clk, int flags, const struct timespec *req, struct timespec *rem)
{
    switch (clk) {
        case CLOCK_REALTIME:
            if (flags == 0) {
                /* we only support the realtime clock currently */
                return nanosleep(req, rem);
            }
            /* fallthrough */
        case CLOCK_MONOTONIC_COARSE:
        case CLOCK_REALTIME_COARSE:
        case CLOCK_MONOTONIC_RAW:
        case CLOCK_MONOTONIC:
        case CLOCK_PROCESS_CPUTIME_ID:
        case CLOCK_BOOTTIME:
        case CLOCK_REALTIME_ALARM:
        case CLOCK_BOOTTIME_ALARM:
        case CLOCK_SGI_CYCLE:
        case CLOCK_TAI:
            if (flags == 0 || flags == TIMER_ABSTIME) {
                return ENOTSUP;
            }
            /* fallthrough */
        case CLOCK_THREAD_CPUTIME_ID:
        default:
            return EINVAL;
    }
}
