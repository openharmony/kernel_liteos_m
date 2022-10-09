/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

#ifndef IT_POSIX_PTHREAD_H
#define IT_POSIX_PTHREAD_H

#include "sched.h"
#include "signal.h"
#include "semaphore.h"
#include "sched.h"
#include "osTest.h"
#include "pthread.h"
#include "limits.h"
#include "unistd.h"
#include "mqueue.h"
#include "signal.h"

/* Some routines are part of the XSI Extensions */
#ifndef WITHOUT_XOPEN
#define _XOPEN_SOURCE 600
#endif

#define PTHREAD_IS_ERROR (-1)
#define PTHREAD_PRIORITY_TEST 20
#define PTHREAD_DEFAULT_STACK_SIZE (LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE)
#define PTHREAD_KEY_NUM 10
#define THREAD_NUM 3
#define PTHREAD_TIMEOUT (THREAD_NUM * 2)
#define PTHREAD_INTHREAD_TEST 0 /* Control going to or is already for Thread */
#define PTHREAD_INMAIN_TEST 1   /* Control going to or is already for Main */
#define INVALID_PSHARED_VALUE (-100)
#define NUM_OF_CONDATTR 10
#define RUNTIME 5
#define PTHREAD_THREADS_NUM 3
#define TCOUNT 5      // Number of single-threaded polling
#define COUNT_LIMIT 7 // The number of times the signal is sent
#define HIGH_PRIORITY 5
#define LOW_PRIORITY 10
#define PTHREAD_EXIT_VALUE ((void *)100) /* The return code of the thread when using pthread_exit(). */

#define PTHREAD_EXISTED_NUM TASK_EXISTED_NUM
#define PTHREAD_EXISTED_SEM_NUM SEM_EXISTED_NUM

/* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
#define _POSIX_C_SOURCE 200112L

#define PTHREAD_MUTEX_RECURSIVE 0
#define PTHREAD_MUTEX_ERRORCHECK 0

#define PRIORITY_OTHER (-1)
#define PRIORITY_FIFO 20
#define PRIORITY_RR 20

#ifdef LOSCFG_AARCH64
#define PTHREAD_STACK_MIN_TEST (PTHREAD_STACK_MIN * 3)
#else
#define PTHREAD_STACK_MIN_TEST PTHREAD_STACK_MIN
#endif

pthread_t TestPthreadSelf(void);

VOID ItPosixPthread001(VOID);
VOID ItPosixPthread002(VOID);
VOID ItPosixPthread003(VOID);
VOID ItPosixPthread004(VOID);
VOID ItPosixPthread005(VOID);
VOID ItPosixPthread006(VOID);
VOID ItPosixPthread007(VOID);
VOID ItPosixPthread008(VOID);
VOID ItPosixPthread009(VOID);
VOID ItPosixPthread010(VOID);
VOID ItPosixPthread011(VOID);
VOID ItPosixPthread012(VOID);
VOID ItPosixPthread013(VOID);
VOID ItPosixPthread014(VOID);
VOID ItPosixPthread015(VOID);
VOID ItPosixPthread016(VOID);
VOID ItPosixPthread017(VOID);
VOID ItPosixPthread018(VOID);
VOID ItPosixPthread019(VOID);
VOID ItPosixPthread020(VOID);
VOID ItPosixPthread021(VOID);
VOID ItPosixPthread022(VOID);
VOID ItPosixPthread023(VOID);
VOID ItPosixPthread024(VOID);
VOID ItPosixPthread025(VOID);

#endif /* IT_POSIX_PTHREAD_H */
