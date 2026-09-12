/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
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

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 0)
#error "LOSCFG_PLATFORM_HWI_WITH_ARG must be set to 1 !!!"
#endif

/**
 * @ingroup  los_arch_interrupt
 * Define the type of a hardware interrupt vector table function.
 */
typedef struct tagHwiHandleForm {
    HWI_PROC_FUNC pfnHook;
    VOID *uwParam;
    UINTPTR uwreserved;
} HWI_HANDLE_FORM_S;

typedef struct {
    UINT32      mcause;
    UINT32      mtval;
    UINT32      medeleg;
    UINT32      gp;
    TaskContext taskContext;
} LosExcContext;

typedef struct {
    UINT16 nestCnt;
    UINT16 type;
    UINT32 thrID;
    LosExcContext *context;
} LosExcInfo;

extern LosExcInfo g_excInfo;

typedef VOID (*EXC_PROC_FUNC)(UINT32, LosExcContext *);

extern EXC_PROC_FUNC g_excRegHook;

extern UINT32 ArchSetExcHook(EXC_PROC_FUNC excHook);

/**
 * @ingroup los_arch_interrupt
 * Highest priority of a hardware interrupt.
 */
#define OS_HWI_PRIO_HIGHEST        7

/**
 * @ingroup los_arch_interrupt
 * Lowest priority of a hardware interrupt.
 */
#define OS_HWI_PRIO_LOWEST         1

/**
 * @ingroup los_arch_interrupt
 * Count of HimiDeer system interrupt vector.
 */
#define OS_RISCV_SYS_VECTOR_CNT   (RISCV_SYS_MAX_IRQ + 1)

/**
 * @ingroup los_arch_interrupt
 * Count of HimiDeer local interrupt vector 0 - 5, enabled by CSR mie 26 -31 bit.
 */
#define OS_RISCV_MIE_IRQ_VECTOR_CNT  6

/**
 * @ingroup los_arch_interrupt
 * Count of HimiDeer local interrupt vector 6 - 31, enabled by custom CSR locie0 0 - 25 bit.
 */
#define OS_RISCV_CUSTOM_IRQ_VECTOR_CNT  RISCV_PLIC_VECTOR_CNT

/**
 * @ingroup los_arch_interrupt
 * Count of HimiDeer local IRQ interrupt vector.
 */
#define OS_RISCV_LOCAL_IRQ_VECTOR_CNT        (OS_RISCV_MIE_IRQ_VECTOR_CNT + OS_RISCV_SYS_VECTOR_CNT)

/**
 * @ingroup los_arch_interrupt
 * Count of himideer interrupt vector.
 */
#define OS_RISCV_VECTOR_CNT                  (OS_RISCV_SYS_VECTOR_CNT + OS_RISCV_CUSTOM_IRQ_VECTOR_CNT)

/**
 * @ingroup los_arch_interrupt
 * Count of risc-v system interrupt vector.
 */
#define OS_SYS_VECTOR_CNT                    0

/**
 * Maximum number of supported hardware devices that generate hardware interrupts.
 * The maximum number of hardware devices that generate hardware interrupts supported by hi3518ev200 is 32.
 */
#define OS_HWI_MAX_NUM        OS_RISCV_VECTOR_CNT

/**
 * Maximum interrupt number.
 */
#define OS_HWI_MAX            ((OS_HWI_MAX_NUM) - 1)

/**
 * Minimum interrupt number.
 */
#define OS_HWI_MIN            0

/**
 * Maximum usable interrupt number.
 */
#define OS_USER_HWI_MAX        OS_HWI_MAX

/**
 * Minimum usable interrupt number.
 */
#define OS_USER_HWI_MIN        OS_HWI_MIN

extern HWI_HANDLE_FORM_S g_hwiForm[OS_HWI_MAX_NUM];

extern VOID HalHwiInit(VOID);
extern UINT32 HalGetHwiFormCnt(HWI_HANDLE_T hwiNum);
extern HWI_HANDLE_FORM_S *HalGetHwiForm(VOID);
extern VOID HalHwiInterruptDone(HWI_HANDLE_T hwiNum);

extern UINT32 HalUnalignedAccessFix(UINTPTR mcause, UINTPTR mepc, UINTPTR mtval, VOID *sp);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_ARCH_INTERRUPT_H */
