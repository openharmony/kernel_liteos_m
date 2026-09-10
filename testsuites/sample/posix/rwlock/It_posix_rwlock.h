/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _IT_POSIX_RWLOCK_H
#define _IT_POSIX_RWLOCK_H

#include "osTest.h"
#include "pthread.h"
#include "errno.h"
#include "sched.h"
#include "unistd.h"

#define RWLOCK_TEST_DEFAULT_PRIO LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO
#define RWLOCK_TEST_HIGH_PRIO    (RWLOCK_TEST_DEFAULT_PRIO - 6)
#define RWLOCK_TEST_LOW_PRIO     (RWLOCK_TEST_DEFAULT_PRIO + 5)

UINT32 PosixPthreadDestroy(pthread_attr_t *attr, pthread_t thread);
UINT32 PosixPthreadInit(pthread_attr_t *attr, int pri);
void TestExtraTaskDelay(UINT32 tick);

VOID ItPosixRwlock001(void);
VOID ItPosixRwlock003(void);
VOID ItPosixRwlock004(void);
VOID ItPosixRwlock006(void);
VOID ItPosixRwlock007(void);
VOID ItPosixRwlock008(void);
VOID ItPosixRwlock009(void);
VOID ItPosixRwlock059(void);
VOID ItPosixRwlock060(void);
VOID ItPosixRwlock061(void);
VOID ItPosixRwlock062(void);
VOID ItPosixRwlock063(void);
VOID ItPosixRwlock064(void);
VOID ItPosixRwlock065(void);
VOID ItPosixRwlock066(void);
VOID ItPosixRwlock067(void);
VOID ItPosixRwlock068(void);
VOID ItPosixRwlock069(void);
VOID ItPosixRwlock070(void);
VOID ItPosixRwlock071(void);
VOID ItPosixRwlock072(void);
VOID ItPosixRwlock073(void);
VOID ItPosixRwlock074(void);
VOID ItPosixRwlock075(void);

VOID ItSuitePosixRwlock(VOID);

#endif
