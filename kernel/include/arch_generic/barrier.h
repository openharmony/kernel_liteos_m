/* ----------------------------------------------------------------------------
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd. All rights reserved.
 * Description: Barrier HeadFile
 * Create: 2013-01-01
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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
 * --------------------------------------------------------------------------- */

#ifndef _ARCH_GENERIC_BARRIER_H
#define _ARCH_GENERIC_BARRIER_H

#include "los_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* TODO： 暂时将多个架构实现放这里，后续会按照架构依次重构到arch目录下 */

#if defined(__thumb__) || (defined(__ARM_ARCH) && (__ARM_ARCH >= 7))
/*
 * Cortex-M (Thumb-2) or ARMv7+:
 *   dsb / dmb / isb are valid instructions, use inline asm.
 *   Lowercase aliases are safe (no extern function conflict).
 */
#define DSB()       __asm__ volatile("dsb" : : : "memory")
#define DMB()       __asm__ volatile("dmb" : : : "memory")
#define ISB()       __asm__ volatile("isb" : : : "memory")
#define dsb DSB
#define dmb DMB
#define isb ISB

#elif defined(__arm__)
/*
 * ARM9 / older ARM (ARMv5/v6):
 *   dsb / dmb / isb are provided as extern C functions in los_arch_context.h
 *   (CP15 based). Do NOT define lowercase macros -- they would clash with
 *   the real function names declared by the arch layer.
 */
extern VOID dsb(VOID);
extern VOID dmb(VOID);
extern VOID isb(VOID);
#define DSB()       dsb()
#define DMB()       dmb()
#define ISB()       isb()

#elif defined(__riscv)
/*
 * RISC-V:
 *   dsb is provided as an extern C function in los_arch_context.h
 *   (fence based). Do NOT define the 'dsb' lowercase macro.
 *   dmb / isb lowercase aliases are safe (not declared by arch).
 */
extern VOID dsb(VOID);
#define DSB()       dsb()
#define DMB()       dsb()
#define ISB()       dsb()
#define dmb DMB
#define isb ISB

#else
/*
 * Fallback: compiler barrier only.
 */
#define DSB()       __asm__ volatile("" : : : "memory")
#define DMB()       __asm__ volatile("" : : : "memory")
#define ISB()       __asm__ volatile("" : : : "memory")
#define dsb DSB
#define dmb DMB
#define isb ISB
#endif

#if defined(__thumb__)
#define SEV()       __asm__ volatile("sev" : : : "memory")
#define WFE()       __asm__ volatile("wfe" : : : "memory")
#define WFI()       __asm__ volatile("wfi" : : : "memory")
#else
#define SEV()
#define WFE()
extern VOID wfi(VOID);
#define WFI() wfi()
#endif

#define BARRIER()   __asm__ volatile("" ::: "memory")

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ARCH_GENERIC_BARRIER_H */
