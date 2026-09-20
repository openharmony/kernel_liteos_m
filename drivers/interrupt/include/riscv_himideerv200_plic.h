/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
 * Description: HiSilicon himideerv200 PLIC (Platform Level Interrupt
 *              Controller) register description for WS63 (riscv31 core).
 *
 *              Unlike SiFive-style PLICs, himideerv200 is NOT memory
 *              mapped: all registers are RISC-V custom CSRs, so there is
 *              no struct layout to describe — registers are CSR numbers
 *              with inline accessors (matching the GIC/NVIC struct
 *              headers' role of "single place describing the hardware").
 *
 *              Vector space (96 vectors, 0..95 — per the vendor
 *              libinterrupt.a reference implementation):
 *                0..31   core local interrupts, mie/mip bit n
 *                32..63  external group 0, LOCIEN0/LOCIPD0 bit n-32
 *                64..95  external group 1, LOCIEN1/LOCIPD1 bit n-64
 *
 *              Priority (vectors 26..95 only): LOCIPRI[(n-26)/8] holds 8
 *              interrupts x 4 bits; hardware value = 7 - logical prio
 *              (logical 0..6), same inversion as the CLIC driver.
 * Reference: ws63v100 SDK arch/riscv/riscv31/arch_encoding.h and the
 *            libinterrupt.a disassembly this driver replaces.
 */
#ifndef _RISCV_HIMIDEERV200_PLIC_H
#define _RISCV_HIMIDEERV200_PLIC_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* ---- Standard RISC-V CSRs used by this driver ---- */
#define PLIC_CSR_MIE                    0x304   /* machine interrupt-enable */
#define PLIC_CSR_MCAUSE                 0x342   /* machine cause */

/* ---- Interrupt enable CSRs (external groups) ---- */
#define PLIC_CSR_LOCIEN0               0xBE0   /* ext group 0: vectors 32..63 */
#define PLIC_CSR_LOCIEN1               0xBE1   /* ext group 1: vectors 64..95 */
#define PLIC_CSR_LOCIEN2               0xBE2   /* ext group 2: vectors 96..127 (unused, limit is 96) */

/* ---- Interrupt pending CSRs (read-only; group 0 pending is mip) ---- */
#define PLIC_CSR_LOCIPD0               0xBE8   /* ext group 0 pending */
#define PLIC_CSR_LOCIPD1               0xBE9   /* ext group 1 pending */
#define PLIC_CSR_LOCIPD2               0xBEA
#define PLIC_CSR_LOCIPD3               0xBEB

/* ---- Interrupt clear-pending CSR (write vector number) ---- */
#define PLIC_CSR_LOCIPCLR              0xBF0

/* ---- Priority threshold CSR (set by SDK boot; driver does not touch) ---- */
#define PLIC_CSR_PRITHD                0xBFE

/* ---- Priority CSRs: 16 regs x 8 vectors x 4 bits, vectors 26..95 ---- */
#define PLIC_CSR_LOCIPRI(n)            (0xBC0 + (n))   /* n = 0..15 */
#define PLIC_LOCIPRI_PER_REG           8U      /* vectors per priority register */
#define PLIC_LOCIPRI_FIELD_BITS        4       /* bits per vector priority field */
#define PLIC_LOCIPRI_FIELD_MASK        0xFU

/* ---- Vector space ---- */
#define PLIC_VECTOR_MAX                96      /* valid vectors 0..95 */
#define PLIC_LOCAL_VECTOR_CNT          32      /* 0..31: mie/mip bits */
#define PLIC_PRIO_VECTOR_BASE          26      /* vectors >= 26 have a LOCIPRI field */
#define PLIC_PRIO_LEVELS               7       /* logical priorities 0..6 */

/* ---- mcause layout (claim path) ---- */
#define PLIC_MCAUSE_ID_MASK            0xFFU   /* interrupt id = mcause & 0xFF */
/* ---- CSR accessors (csr operand must be an immediate) ---- */
#define PLIC_CSR_READ(csr)             ({ UINT32 v_; __asm__ volatile("csrr %0, %1" : "=r"(v_) : "i"(csr)); v_; })
#define PLIC_CSR_SET(csr, mask)        ({ __asm__ volatile("csrs %0, %1" :: "i"(csr), "rK"(mask) : "memory"); })
#define PLIC_CSR_CLEAR(csr, mask)      ({ __asm__ volatile("csrc %0, %1" :: "i"(csr), "rK"(mask) : "memory"); })
#define PLIC_CSR_WRITE(csr, val)       ({ __asm__ volatile("csrw %0, %1" :: "i"(csr), "rK"(val) : "memory"); })
#define PLIC_FENCE()                   ({ __asm__ volatile("fence" ::: "memory"); })

/* ---- Hardware operations (exact libinterrupt.a replacements) ----
 * The board HAL (device/board/hihope/ws63_m/.../riscv_hal.c) links these
 * directly, so names and signatures are fixed.
 */
extern UINT32 HalIrqUnmask(UINT32 hwiNum);
extern UINT32 HalIrqMask(UINT32 hwiNum);
extern UINT32 HalIrqClear(UINT32 hwiNum);
extern UINT32 HalIrqSetPrio(UINT32 hwiNum, UINT16 priority);
extern UINT32 HalCurIrqGet(VOID);
extern VOID HalIrqInit(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _RISCV_HIMIDEERV200_PLIC_H */
