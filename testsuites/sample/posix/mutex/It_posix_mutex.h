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

#ifndef _IT_POSIX_MUTEX_H
#define _IT_POSIX_MUTEX_H

#include "osTest.h"
#include "pthread.h"
#include "errno.h"
#include "sched.h"
#include "semaphore.h"
#include "unistd.h"

#ifdef LOSCFG_DEBUG_DEADLOCK
#define TEST_MUTEX_INIT              \
    {                                \
        { 0, 0, 0, 0 },              \
        {                            \
            { 0, 0 }, { 0, 0 }, 0, 0 \
        }                            \
    }
#else
#define TEST_MUTEX_INIT    \
    {                      \
        { 0, 0, 0, 0 },    \
        {                  \
            { 0, 0 }, 0, 0 \
        }                  \
    }
#endif

#define MUTEX_TEST_DEFAULT_PRIO LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO
#define MUTEX_TEST_HIGH_PRIO    (MUTEX_TEST_DEFAULT_PRIO - 6)
#define MUTEX_TEST_LOW_PRIO     (MUTEX_TEST_DEFAULT_PRIO + 5)

UINT32 PosixPthreadDestroy(pthread_attr_t *attr, pthread_t thread);
UINT32 PosixPthreadInit(pthread_attr_t *attr, int pri);
void TestExtraTaskDelay(UINT32 tick);

VOID ItPosixMux001(void);
VOID ItPosixMux002(void);
VOID ItPosixMux003(void);
VOID ItPosixMux004(void);
VOID ItPosixMux005(void);
VOID ItPosixMux006(void);
VOID ItPosixMux007(void);
VOID ItPosixMux008(void);
VOID ItPosixMux009(void);
VOID ItPosixMux010(void);
VOID ItPosixMux011(void);
VOID ItPosixMux012(void);
VOID ItPosixMux013(void);
VOID ItPosixMux014(void);
VOID ItPosixMux015(void);
VOID ItPosixMux016(void);
VOID ItPosixMux017(void);
VOID ItPosixMux018(void);
VOID ItPosixMux019(void);
VOID ItPosixMux020(void);
VOID ItPosixMux021(void);
VOID ItPosixMux022(void);
VOID ItPosixMux023(void);
VOID ItPosixMux024(void);
VOID ItPosixMux025(void);
VOID ItPosixMux026(void);
VOID ItPosixMux027(void);
VOID ItPosixMux028(void);
VOID ItPosixMux029(void);
VOID ItPosixMux030(void);
VOID ItPosixMux031(void);
VOID ItPosixMux032(void);
VOID ItPosixMux033(void);
VOID ItPosixMux034(void);
VOID ItPosixMux035(void);
VOID ItPosixMux036(void);
VOID ItPosixMux037(void);
VOID ItPosixMux038(void);
VOID ItPosixMux039(void);
VOID ItPosixMux040(void);
VOID ItPosixMux041(void);
VOID ItPosixMux042(void);
VOID ItPosixMux043(void);
VOID ItPosixMux044(void);
VOID ItPosixMux045(void);
VOID ItPosixMux046(void);
VOID ItPosixMux047(void);
VOID ItPosixMux048(void);
VOID ItPosixMux049(void);

VOID ItSuitePosixMutex(void);

#endif
