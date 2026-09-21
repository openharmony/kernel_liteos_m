/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
 * Description: NVIC (Nested Vectored Interrupt Controller) and SCB
 *              (System Control Block) register layouts as C structs,
 *              for ARM Cortex-M (ARMv7-M memory map).
 *              Access via the NVIC_REG / SCB_REG struct pointers; CMSIS
 *              intrinsics are only used for non-memory-mapped core state
 *              (IPSR) and barriers (__DSB/__ISB).
 * Reference: ARMv7-M Architecture Reference Manual (DDI 0403).
 */
#ifndef _NVIC_H
#define _NVIC_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* ---- NVIC register bank (base 0xE000E100) ----
 * Reserved gaps are explicit padding so every field matches its
 * hardware offset. Array registers (ISER/ICER/...) hold 32 interrupts
 * per word; IPR is byte-wide (one priority byte per interrupt).
 */
struct NvicRegs {
    volatile UINT32 iser[8];     /* 0x000 Interrupt Set-Enable Registers */
    UINT32 reserved0[24];        /* 0x020 */
    volatile UINT32 icer[8];     /* 0x080 Interrupt Clear-Enable Registers */
    UINT32 reserved1[24];        /* 0x0A0 */
    volatile UINT32 ispr[8];     /* 0x100 Interrupt Set-Pending Registers */
    UINT32 reserved2[24];        /* 0x120 */
    volatile UINT32 icpr[8];     /* 0x180 Interrupt Clear-Pending Registers */
    UINT32 reserved3[24];        /* 0x1A0 */
    volatile UINT32 iabr[8];     /* 0x200 Interrupt Active Bit Registers */
    UINT32 reserved4[56];        /* 0x220 */
    volatile UINT8  ipr[240];    /* 0x300 Interrupt Priority Registers (byte-wide) */
};

/* ---- SCB register bank (base 0xE000ED00) ---- */
struct ScbRegs {
    volatile UINT32 cpuid;       /* 0x00 CPUID Base Register */
    volatile UINT32 icsr;        /* 0x04 Interrupt Control and State Register */
    volatile UINT32 vtor;        /* 0x08 Vector Table Offset Register */
    volatile UINT32 aircr;       /* 0x0C Application Interrupt and Reset Control Register */
    volatile UINT32 scr;         /* 0x10 System Control Register */
    volatile UINT32 ccr;         /* 0x14 Configuration and Control Register */
    volatile UINT8  shpr[12];    /* 0x18 System Handler Priority Registers (byte-wide) */
    volatile UINT32 shcsr;       /* 0x24 System Handler Control and State Register */
    volatile UINT32 cfsr;        /* 0x28 Configurable Fault Status Register */
    volatile UINT32 hfsr;        /* 0x2C Hard Fault Status Register */
};

#define NVIC_REG_BASE               0xE000E100U
#define SCB_REG_BASE                0xE000ED00U

#define NVIC_REG    ((volatile struct NvicRegs *)(UINTPTR)NVIC_REG_BASE)
#define SCB_REG     ((volatile struct ScbRegs *)(UINTPTR)SCB_REG_BASE)

/* ---- SCB register bits (used by the driver) ---- */
#define SCB_SCR_SLEEPDEEP           (1U << 2)   /* SCR: sleep deep */
#define SCB_AIRCR_VECTKEY_MASK      (0xFFFFU << 16)
#define SCB_AIRCR_PRIGROUP_MASK     (0x7U << 8)
#define SCB_AIRCR_VECTKEY_KEY       0x5FAU
#define SCB_SHCSR_USGFAULT          (1U << 18)
#define SCB_SHCSR_BUSFAULT          (1U << 17)
#define SCB_SHCSR_MEMFAULT          (1U << 16)
#define SCB_CCR_DIV0FAULT           (1U << 4)
#define SCB_CCR_UNALIGNFAULT        (1U << 3)

/* ---- Shared constants ---- */
#define NVIC_IRQS_PER_REG           32U  /* interrupts per enable/pending register */

#ifndef __NVIC_PRIO_BITS
#define __NVIC_PRIO_BITS            4U
#endif

/* Logical priority (0..7) -> hardware 8-bit IPR field (same as CMSIS
 * NVIC_SetPriority semantics). */
#define NVIC_PRIO_VALUE(prio)       ((((UINT32)(prio)) << (8U - __NVIC_PRIO_BITS)) & 0xFFU)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _NVIC_H */
