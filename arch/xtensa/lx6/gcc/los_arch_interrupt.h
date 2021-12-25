/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_config.h"
#include "los_compiler.h"
#include "los_interrupt.h"

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
 * Maximum number of used hardware interrupts.
 */
#ifndef OS_HWI_MAX_NUM
#define OS_HWI_MAX_NUM                        LOSCFG_PLATFORM_HWI_LIMIT
#endif

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

#define OS_EXC_IN_INIT                        0
#define OS_EXC_IN_TASK                        1
#define OS_EXC_IN_HWI                         2

/* *
 * @ingroup  los_arch_interrupt
 * Define the type of a hardware interrupt vector table function.
 */
typedef VOID (**HWI_VECTOR_FUNC)(VOID);

/* *
 * @ingroup los_arch_interrupt
 * Count of interrupts.
 */
extern UINT32 g_intCount;

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

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Invalid interrupt number.
 *
 * Value: 0x02000900
 *
 * Solution: Ensure that the interrupt number is valid.
 */
#define OS_ERRNO_HWI_NUM_INVALID              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x00)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Null hardware interrupt handling function.
 *
 * Value: 0x02000901
 *
 * Solution: Pass in a valid non-null hardware interrupt handling function.
 */
#define OS_ERRNO_HWI_PROC_FUNC_NULL           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x01)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Insufficient interrupt resources for hardware interrupt creation.
 *
 * Value: 0x02000902
 *
 * Solution: Increase the configured maximum number of supported hardware interrupts.
 */
#define OS_ERRNO_HWI_CB_UNAVAILABLE           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x02)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Insufficient memory for hardware interrupt initialization.
 *
 * Value: 0x02000903
 *
 * Solution: Expand the configured memory.
 */
#define OS_ERRNO_HWI_NO_MEMORY                LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x03)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: The interrupt has already been created.
 *
 * Value: 0x02000904
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_ALREADY_CREATED          LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x04)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Invalid interrupt priority.
 *
 * Value: 0x02000905
 *
 * Solution: Ensure that the interrupt priority is valid.
 */
#define OS_ERRNO_HWI_PRIO_INVALID             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x05)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Incorrect interrupt creation mode.
 *
 * Value: 0x02000906
 *
 * Solution: The interrupt creation mode can be only set to OS_HWI_MODE_COMM or OS_HWI_MODE_FAST.
 */
#define OS_ERRNO_HWI_MODE_INVALID             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x06)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: The interrupt has already been created as a fast interrupt.
 *
 * Value: 0x02000907
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_FASTMODE_ALREADY_CREATED LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x07)

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
/* *
 * @ingroup los_arch_interrupt
 * Set interrupt vector table.
 */
extern VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector, VOID *arg);
#else
/* *
 * @ingroup los_arch_interrupt
 * Set interrupt vector table.
 */
extern VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector);
#endif

VOID HalInterrupt(VOID);
UINT32 HalIntNumGet(VOID);
VOID HalHwiDefaultHandler(VOID);
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
