/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_ARCH_INTERRUPT_H
#define _LOS_ARCH_INTERRUPT_H

#include "los_common_interrupt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef struct {
    UINT32 pc;
    UINT32 ps;
    UINT32 regA[16];
    UINT32 sar;
    UINT32 excCause;
    UINT32 excVaddr;
    UINT32 lbeg;
    UINT32 lend;
    UINT32 lcount;
#if (defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U))
    UINT32 temp;
    UINT16 cpenable;
    UINT16 cpstored;
    UINT32 fcr;
    UINT32 fsr;
    UINT32 regF[16];
#endif
    UINT32 res[4];
} EXC_CONTEXT_S;

#define VECTOR_START                          _init_start
extern CHAR *VECTOR_START;
#define INIT_VECTOR_START                     ((UINTPTR)&VECTOR_START)

/* *
 * @ingroup los_arch_interrupt
 * Highest priority of a hardware interrupt.
 */
#ifndef OS_HWI_PRIO_HIGHEST
#define OS_HWI_PRIO_HIGHEST                   0
#endif

/* *
 * @ingroup los_arch_interrupt
 * Lowest priority of a hardware interrupt.
 */
#ifndef OS_HWI_PRIO_LOWEST
#define OS_HWI_PRIO_LOWEST                    7
#endif

/* *
 * @ingroup los_arch_interrupt
 * Count of Xtensa system interrupt vector.
 */
#define OS_SYS_VECTOR_CNT                     0

/* *
 * @ingroup los_arch_interrupt
 * Count of Xtensa interrupt vector.
 */
#define OS_VECTOR_CNT                         (OS_SYS_VECTOR_CNT + OS_HWI_MAX_NUM)

VOID HalInterrupt(VOID);
VOID HalExcHandleEntry(UINTPTR faultAddr, EXC_CONTEXT_S *excBufAddr, UINT32 type);
VOID HalHwiInit(VOID);

/**
 * @ingroup los_exc
 * Exception information structure
 *
 * Description: Exception information saved when an exception is triggered on the Xtensa platform.
 *
 */
typedef struct TagExcInfo {
    UINT16 phase;
    UINT16 type;
    UINT32 faultAddr;
    UINT32 thrdPid;
    UINT16 nestCnt;
    UINT16 reserved;
    EXC_CONTEXT_S *context;
} ExcInfo;

extern UINT32 g_curNestCount;
extern ExcInfo g_excInfo;

#define MAX_INT_INFO_SIZE       (8 + 0x164)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_ARCH_INTERRUPT_H */
