/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
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
 * "AS IS, AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
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

#ifndef _LOS_PERCPU_PRI_H
#define _LOS_PERCPU_PRI_H

#include "los_compiler.h"
#include "los_interrupt.h"
#include "los_sortlink_pri.h"
#include "los_list.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef LOSCFG_KERNEL_CORE_NUM
#ifdef LOSCFG_KERNEL_SMP
#define LOSCFG_KERNEL_CORE_NUM  LOSCFG_KERNEL_SMP_CORE_NUM
#else
#define LOSCFG_KERNEL_CORE_NUM  1
#endif
#endif

#ifndef LOSCFG_KERNEL_CPU_MASK
#define LOSCFG_KERNEL_CPU_MASK  ((1U << LOSCFG_KERNEL_CORE_NUM) - 1)
#endif

#define CPUID_TO_AFFI_MASK(cpuid)  (0x1u << (cpuid))

STATIC INLINE UINT32 ArchCurrCpuid(VOID)
{
    return 0;
}

typedef struct {
    UINT16 head;
    UINT16 tail;
    UINT32 free;
    UINTPTR *buf;
} ExpiryList;

typedef struct {
    SortLinkAttribute taskSortLink;
#ifdef LOSCFG_BASE_CORE_SWTMR
    SortLinkAttribute swtmrSortLink;
    ExpiryList expiryList;
#endif
    UINT32 idleTaskId;
    UINT32 taskLockCnt;
    UINT32 swtmrHandlerQueue;
#if (LOSCFG_BASE_CORE_SWTMR == 1)
    UINT32 swtmrTaskId;
#endif
#ifndef LOSCFG_SCHED_LATENCY
    UINT32 schedFlag;
#endif
#ifdef LOSCFG_KERNEL_SMP
    UINT32 excFlag;
#ifdef LOSCFG_KERNEL_SMP_CALL
    LOS_DL_LIST funcLink;
#endif
#endif
} Percpu;

extern Percpu g_percpu[LOSCFG_KERNEL_CORE_NUM];

STATIC INLINE Percpu *OsPercpuGet(VOID)
{
    return &g_percpu[ArchCurrCpuid()];
}

STATIC INLINE Percpu *OsPercpuGetByID(UINT32 cpuid)
{
    return &g_percpu[cpuid];
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_PERCPU_PRI_H */
