/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
 * Description: HiSilicon himideerv200 PLIC interrupt controller driver
 *              for WS63 (riscv31 core, CSR-based — no memory map).
 *
 *              Replaces the prebuilt libinterrupt.a with a source
 *              implementation behaviorally equivalent to the
 *              disassembled original for all valid inputs (0..95
 *              routing, priority math, error codes, fence placement):
 *                - vectors 0..31 enable/pend through mie/mip bits
 *                - vectors 32..63 / 64..95 through LOCIEN0/1 + LOCIPD0/1
 *                - clear-pending = write vector number to LOCIPCLR + fence
 *                - priority (vectors 26..95): LOCIPRI[(n-26)/8] 4-bit
 *                  field at ((n-26)%8)*4, hardware value 7 - logical prio
 *                - software trigger is not supported (returns LOS_NOK,
 *                  which is why ws63 tests trigger IRQs via timer1)
 *                - vectors < 26 ignore priority setting (silent LOS_OK)
 *
 *              Two deliberate deviations (both improvements, verified
 *              safe by review):
 *                - HalIrqSetPrio uses read-modify-write with the
 *                  correct 0xF field mask. (The SDK's int_set_priority
 *                  used csrs — set-only, cannot lower priority — and
 *                  int_get_priority used a buggy 0x3 mask.)
 *                - HalIrqInit does not zero the library-internal
 *                  g_hwiForm[26..95]: the kernel owns g_hwiForm now
 *                  (arch HalHwiInit initializes it), and dispatch uses
 *                  g_hwiHandleForm. Keeping HalHwiDefaultHandler in
 *                  place unblocks LOS_HwiCreate for vectors 26..95.
 *
 *              Beyond the original: the driver statically initializes the
 *              kernel HwiControllerOps (arm_nvic.c pattern — g_hwiControllerOps
 *              designated initializer, complete from the first
 *              instruction), so the generic kernel paths (LOS_HwiEnable/
 *              Disable/Clear/Trigger/SetPriority/CurIrqNum) work on ws63
 *              — previously those ops were NULL and only direct
 *              board-HAL calls worked. Note this changes LOS_HwiTrigger's
 *              error return on ws63 from OS_ERRNO_HWI_OPS_FUNC_NULL
 *              (NULL op) to LOS_NOK (HwiTrigger); no in-tree caller
 *              distinguishes the two.
 *
 *              Hardware bring-up (mstatus.MIE, LOCIPRI defaults,
 *              PRITHD) is done by the SDK boot code; the original
 *              HalIrqInit did not touch it and neither does this one.
 * Reference: ws63v100 SDK arch/riscv/riscv31/{arch_encoding.h,
 *            interrupt.c} + libinterrupt.a disassembly.
 */

#include "los_interrupt.h"
#include "los_hwi_pri.h"
#include "los_arch_interrupt.h"
#include "riscv_himideerv200_plic.h"

/* ---- Hardware operations (libinterrupt.a equivalents) ---- */

UINT32 HalIrqUnmask(UINT32 hwiNum)
{
    if (hwiNum >= PLIC_VECTOR_MAX) {
        return LOS_ERRNO_HWI_NUM_INVALID;
    }
    if (hwiNum < PLIC_LOCAL_VECTOR_CNT) {
        PLIC_CSR_SET(PLIC_CSR_MIE, 1U << hwiNum);
    } else if (hwiNum < (PLIC_LOCAL_VECTOR_CNT + 32)) {
        PLIC_CSR_SET(PLIC_CSR_LOCIEN0, 1U << (hwiNum - PLIC_LOCAL_VECTOR_CNT));
    } else {
        PLIC_CSR_SET(PLIC_CSR_LOCIEN1, 1U << (hwiNum - PLIC_LOCAL_VECTOR_CNT - 32));
    }
    return LOS_OK;
}

UINT32 HalIrqMask(UINT32 hwiNum)
{
    if (hwiNum >= PLIC_VECTOR_MAX) {
        return LOS_ERRNO_HWI_NUM_INVALID;
    }
    if (hwiNum < PLIC_LOCAL_VECTOR_CNT) {
        PLIC_CSR_CLEAR(PLIC_CSR_MIE, 1U << hwiNum);
    } else if (hwiNum < (PLIC_LOCAL_VECTOR_CNT + 32)) {
        PLIC_CSR_CLEAR(PLIC_CSR_LOCIEN0, 1U << (hwiNum - PLIC_LOCAL_VECTOR_CNT));
    } else {
        PLIC_CSR_CLEAR(PLIC_CSR_LOCIEN1, 1U << (hwiNum - PLIC_LOCAL_VECTOR_CNT - 32));
    }
    return LOS_OK;
}

UINT32 HalIrqClear(UINT32 hwiNum)
{
    if (hwiNum >= PLIC_VECTOR_MAX) {
        return LOS_ERRNO_HWI_NUM_INVALID;
    }
    PLIC_CSR_WRITE(PLIC_CSR_LOCIPCLR, hwiNum);
    PLIC_FENCE();
    return LOS_OK;
}

/* Logical priority (0..6) -> hardware 4-bit field value (7 - prio).
 * Same inversion as the CLIC driver's SET_SYS_PRIOR. */
#define PLIC_PRIO_TO_HW(prio)         ((UINT32)(PLIC_PRIO_LEVELS - (prio)))

/* LOCIPRI registers are distinct CSRs (no dynamic CSR addressing), so
 * access goes through a switch — mirroring the SDK's int_set_priority
 * and the original libinterrupt.a jump table. */
STATIC VOID PlicLocipriWrite(UINT32 reg, UINT32 val)
{
    switch (reg) {
        case 0:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(0),  val); break;
        case 1:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(1),  val); break;
        case 2:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(2),  val); break;
        case 3:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(3),  val); break;
        case 4:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(4),  val); break;
        case 5:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(5),  val); break;
        case 6:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(6),  val); break;
        case 7:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(7),  val); break;
        case 8:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(8),  val); break;
        case 9:  PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(9),  val); break;
        case 10: PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(10), val); break;
        case 11: PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(11), val); break;
        case 12: PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(12), val); break;
        case 13: PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(13), val); break;
        case 14: PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(14), val); break;
        default: PLIC_CSR_WRITE(PLIC_CSR_LOCIPRI(15), val); break;
    }
}

STATIC UINT32 PlicLocipriRead(UINT32 reg)
{
    switch (reg) {
        case 0:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(0));
        case 1:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(1));
        case 2:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(2));
        case 3:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(3));
        case 4:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(4));
        case 5:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(5));
        case 6:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(6));
        case 7:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(7));
        case 8:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(8));
        case 9:  return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(9));
        case 10: return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(10));
        case 11: return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(11));
        case 12: return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(12));
        case 13: return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(13));
        case 14: return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(14));
        default: return PLIC_CSR_READ(PLIC_CSR_LOCIPRI(15));
    }
}

UINT32 HalIrqSetPrio(UINT32 hwiNum, UINT16 priority)
{
    if (hwiNum >= PLIC_VECTOR_MAX) {
        return LOS_ERRNO_HWI_NUM_INVALID;
    }
    if (priority >= PLIC_PRIO_LEVELS) {
        return LOS_ERRNO_HWI_PRIO_INVALID;
    }
    /* Vectors below PLIC_PRIO_VECTOR_BASE have no LOCIPRI field;
     * the original silently succeeds for them. */
    if (hwiNum < PLIC_PRIO_VECTOR_BASE) {
        return LOS_OK;
    }

    UINT32 idx = hwiNum - PLIC_PRIO_VECTOR_BASE;
    UINT32 reg = idx / PLIC_LOCIPRI_PER_REG;               /* LOCIPRI0..8 */
    UINT32 shift = (idx % PLIC_LOCIPRI_PER_REG) * PLIC_LOCIPRI_FIELD_BITS;
    UINT32 val = PlicLocipriRead(reg);

    val &= ~(PLIC_LOCIPRI_FIELD_MASK << shift);
    val |= PLIC_PRIO_TO_HW(priority) << shift;
    PlicLocipriWrite(reg, val);
    return LOS_OK;
}

UINT32 HalCurIrqGet(VOID)
{
    UINT32 mcause;

    __asm__ volatile("csrr %0, %1" : "=r"(mcause) : "i"(PLIC_CSR_MCAUSE));
    return mcause & PLIC_MCAUSE_ID_MASK;
}

/* ---- HwiControllerOps adapters ----
 * The ops signatures differ from the board-HAL entry points
 * (setIrqPriority takes UINT8, triggerIrq returns not-supported),
 * so wrap instead of aliasing.
 */

STATIC UINT32 HwiTrigger(HWI_HANDLE_T hwiNum)
{
    (VOID)hwiNum;
    /* himideerv200 has no software-pending CSR; the original returns
     * LOS_NOK. ws63 tests trigger IRQs through timer1 hardware. */
    return LOS_NOK;
}

STATIC UINT32 HwiSetPriority(HWI_HANDLE_T hwiNum, HWI_PRIOR_T priority)
{
    return HalIrqSetPrio(hwiNum, (UINT16)priority);
}

/* Driver-owned ops table (arm_nvic.c pattern): the selected driver owns
 * g_hwiControllerOps + HwiControllerOpsGet; the arch layer only consumes them via
 * HwiControllerOpsGet() (e.g. HalHwiInterruptDone). Statically initialized —
 * complete from the first instruction, no init-order window. The UINT8
 * setIrqPriority / no-op triggerIrq adapters above bridge the ops
 * signatures to the board-facing HAL entries. */
STATIC HwiControllerOps g_hwiControllerOps = {
    .triggerIrq     = HwiTrigger,
    .clearIrq       = HalIrqClear,
    .enableIrq      = HalIrqUnmask,
    .disableIrq     = HalIrqMask,
    .setIrqPriority = HwiSetPriority,
    .getCurIrqNum   = HalCurIrqGet,
    .getHandleForm  = HalGetHandleForm,
};

HwiControllerOps *HwiControllerOpsGet(VOID)
{
    return &g_hwiControllerOps;
}

/* Kept for board-HAL API compatibility (ws63 riscv_hal.c HalPlicInit
 * calls it). The original libinterrupt.a HalIrqInit zeroed the
 * library-internal g_hwiForm[26..95] and set g_hwiOps — both obsolete
 * here: the kernel owns the dispatch tables, and the ops table above is
 * statically initialized. Hardware bring-up (mstatus.MIE, LOCIPRI
 * defaults, PRITHD) is done by the SDK boot code; the original did not
 * touch it and neither does this one. */
LITE_OS_SEC_TEXT_INIT VOID HalIrqInit(VOID)
{
    return;
}
