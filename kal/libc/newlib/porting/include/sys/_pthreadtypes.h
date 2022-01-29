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

#ifndef _ADAPT_SYS_PTHREADTYPES_H
#define	_ADAPT_SYS_PTHREADTYPES_H

#define pthread_t __pthread_t_discard
#define pthread_attr_t __pthread_attr_t_discard
#define pthread_mutex_t __pthread_mutex_t_discard
#define pthread_mutexattr_t __pthread_mutexattr_t_discard
#define pthread_cond_t __pthread_cond_t_discard
#define pthread_condattr_t __pthread_condattr_t_discard
#define pthread_once_t __pthread_once_t_discard
#define pthread_barrierattr_t __pthread_barrierattr_t_discard
#define pthread_spinlock_t __pthread_spinlock_t_discard

#include_next <sys/_pthreadtypes.h>

#undef pthread_t
#undef pthread_attr_t
#undef pthread_mutex_t
#undef pthread_mutexattr_t
#undef pthread_cond_t
#undef pthread_condattr_t
#undef pthread_once_t
#undef pthread_barrierattr_t
#undef pthread_spinlock_t
#undef _PTHREAD_MUTEX_INITIALIZER
#undef _PTHREAD_COND_INITIALIZER
#undef _PTHREAD_ONCE_INIT

#undef PTHREAD_STACK_MIN

#include "los_config.h"
#define PTHREAD_STACK_MIN LOSCFG_BASE_CORE_TSK_MIN_STACK_SIZE

typedef unsigned long pthread_t;            /* identify a thread */

typedef struct {
    unsigned int detachstate;
    unsigned int schedpolicy;
    struct sched_param schedparam;
    unsigned int inheritsched;
    unsigned int scope;
    unsigned int stackaddr_set;
    void *stackaddr;
    unsigned int stacksize_set;
    size_t stacksize;
} pthread_attr_t;

#include "los_list.h"
typedef struct { unsigned type; } pthread_mutexattr_t;
typedef struct { unsigned int magic; unsigned int handle; pthread_mutexattr_t stAttr;} pthread_mutex_t;

#define _MUX_MAGIC 0xEBCFDEA0
#define _MUX_INVALID_HANDLE 0xEEEEEEEF
#define PTHREAD_MUTEXATTR_INITIALIZER { PTHREAD_MUTEX_DEFAULT }
#define _PTHREAD_MUTEX_INITIALIZER  { _MUX_MAGIC, _MUX_INVALID_HANDLE, PTHREAD_MUTEXATTR_INITIALIZER }

#define PTHREAD_PROCESS_PRIVATE 0
#define PTHREAD_PROCESS_SHARED 1

#ifdef _GNU_SOURCE
#define PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP { _MUX_MAGIC, _MUX_INVALID_HANDLE }
#endif

#include "los_event.h"

typedef struct pthread_cond {
  volatile int count;      /**< The number of tasks blocked by condition */
  EVENT_CB_S event;        /**< Event object*/
  pthread_mutex_t* mutex;  /**< Mutex locker for condition variable protection */
  volatile int value;      /**< Condition variable state value*/
  int clock;
} pthread_cond_t;

#define _PTHREAD_COND_INITIALIZER   { 0 }

typedef struct { int clock; } pthread_condattr_t;

typedef __uint32_t pthread_key_t;        /* thread-specific data keys */

typedef int pthread_once_t;

#define _PTHREAD_ONCE_INIT  { 0 }

typedef struct { unsigned __attr; } pthread_barrierattr_t;

typedef int pthread_spinlock_t;

#endif /* !_ADAPT_SYS_PTHREADTYPES_H */
