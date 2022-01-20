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

#include <signal.h>
#include <errno.h>
#include <pthread.h>
#include "los_config.h"
#include "los_signal.h"
#include "los_task.h"
#include "los_tick.h"

#if (LOSCFG_KERNEL_SIGNAL == 1)

static inline unsigned int TimeSpec2Tick(const struct timespec *tp)
{
    unsigned long long tick, ns;

    ns = (unsigned long long)tp->tv_sec * OS_SYS_NS_PER_SECOND + tp->tv_nsec;
    /* round up for ticks */
    tick = (ns * LOSCFG_BASE_CORE_TICK_PER_SECOND + (OS_SYS_NS_PER_SECOND - 1)) / OS_SYS_NS_PER_SECOND;
    if (tick > LOS_WAIT_FOREVER) {
        tick = LOS_WAIT_FOREVER;
    }
    return (unsigned int)tick;
}

int raise(int sig)
{
    unsigned int ret = LOS_SignalSend(LOS_CurTaskIDGet(), sig);
    if (ret != LOS_OK) {
        return -1;
    }

    return 0;
}

void (*signal(int sig, void (*func)(int)))(int)
{
    SIG_HANDLER h = NULL;

    if (!OS_SIGNAL_VALID(sig)) {
        errno = EINVAL;
        return SIG_ERR;
    }

    h = LOS_SignalSet(sig, func);
    if (h == SIG_ERR) {
        errno = EINVAL;
    }
    return h;
}

int sigprocmask(int how, const sigset_t *set, sigset_t *oset)
{
    unsigned int ret = LOS_SignalMask(how, set, oset);
    if (ret != LOS_OK) {
        errno = EINVAL;
        return -1;
    }
    return 0;
}

int sigaction(int sig, const struct sigaction *act, struct sigaction *oldact)
{
    SIG_HANDLER old = NULL;

    if (!OS_SIGNAL_VALID(sig)) {
        errno = EINVAL;
        return -1;
    }

    if (act != NULL) {
        old = LOS_SignalSet(sig, act->sa_handler);
    } else {
        old = LOS_SignalSet(sig, NULL);
        (void)LOS_SignalSet(sig, old);
    }

    if (oldact != NULL) {
        oldact->sa_handler = old;
    }

    return 0;
}

int sigwait(const sigset_t *set, int *sig)
{
    siginfo_t info = {0};

    int ret = LOS_SignalWait(set, &info, 0);
    if (ret < 0) {
        errno = EINVAL;
        return -1;
    }

    *sig = info.si_signo;
    return 0;
}

int sigtimedwait(const sigset_t *set, siginfo_t *info, const struct timespec *timeout)
{
    int tick = LOS_WAIT_FOREVER;
    int ret;

    if (timeout != NULL) {
        tick = TimeSpec2Tick(timeout);
    }

    ret = LOS_SignalWait(set, info, tick);
    if (ret == LOS_OK) {
        return 0;
    } else {
        if (ret == LOS_ERRNO_SIGNAL_PEND_INTERR) {
            errno = EINTR;
        } else if (ret == LOS_ERRNO_SIGNAL_TIMEOUT) {
            errno = EAGAIN;
        } else {
            errno = EINVAL;
        }
        return -1;
    }
}

int sigwaitinfo(const sigset_t *set, siginfo_t *info)
{
    return sigtimedwait(set, info, NULL);
}

int pthread_kill(pthread_t pid, int sig)
{
    unsigned int ret = LOS_SignalSend((unsigned int)pid, sig);
    if (ret != LOS_OK) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}

int kill(pid_t pid, int sig)
{
    unsigned int ret = LOS_SignalSend((unsigned int)pid, sig);
    if (ret != LOS_OK) {
        errno = EINVAL;
        return -1;
    }

    return 0;
}
#endif
