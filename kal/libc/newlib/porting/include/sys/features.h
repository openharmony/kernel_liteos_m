/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _ADAPT_SYS_FEATURES_H
#define _ADAPT_SYS_FEATURES_H

#define _GNU_SOURCE
#define __USE_NEWLIB__

/* adapt time.h */
#define _POSIX_TIMERS 1
#define _POSIX_CPUTIME 1
#define _POSIX_THREAD_CPUTIME 1
#define _POSIX_MONOTONIC_CLOCK 1
#define _POSIX_CLOCK_SELECTION 1

/* adapt sys/signal.h */
#define _POSIX_REALTIME_SIGNALS 1

/* adapt pthread */
#define _POSIX_THREADS 1
#define _POSIX_TIMEOUTS
#define _POSIX_THREAD_PRIORITY_SCHEDULING
#define _UNIX98_THREAD_MUTEX_ATTRIBUTES
#define _POSIX_THREAD_PROCESS_SHARED

#include_next <sys/features.h>

#endif /* !_ADAPT_SYS_FEATURES_H */
