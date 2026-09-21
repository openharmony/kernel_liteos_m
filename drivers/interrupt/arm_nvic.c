/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
 * Description: ARM Cortex-M NVIC interrupt controller driver.
 *              Moved from arch/arm/cortex-m4/gcc/los_interrupt.c to the
 *              driver layer (matching arm_gic_v2.c / riscv_lcmp_clic.c).
 *              Memory-mapped registers (NVIC/SCB banks) are accessed via
 *              struct pointers from nvic.h; IPSR is read with the CMSIS
 *              intrinsic (not memory-mapped).
 * Reference: ARMv7-M Architecture Reference Manual (DDI 0403).
 *
 * Numbering convention (unchanged from the previous arch placement):
 *   - HwiControllerOps callbacks (enable/disable/prio/pending/clear)
 *     receive EXTERNAL irq numbers (0-based, excluding the 16 system
 *     vectors), which is what the NVIC register arrays take.
 *   - getCurIrqNum / HalInterrupt work in VECTOR numbers (IPSR value,
 *     16+ for external irqs), which is how g_hwiHandleForm is indexed.
 */

#include "los_arch_interrupt.h"
#include "nvic.h"
#include "los_hwi_pri.h"
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
#include "los_cpup_pri.h"
#endif

/* Cortex-M exception vector number for HardFault (CMSIS IRQn value). */
#define HARDFAULT_IRQN              (-13)

/* ---- HwiControllerOps callbacks ----
 * All callbacks receive EXTERNAL irq numbers (0-based). Range-check against
 * OS_HWI_MAX_NUM before touching NVIC registers (FBB pattern: HWI_NUM_VALID).
 */

STATIC UINT32 HwiNumGet(VOID)
{
    return __get_IPSR();
}

STATIC UINT32 HwiUnmask(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    NVIC_REG->iser[hwiNum / NVIC_IRQS_PER_REG] = 1U << (hwiNum % NVIC_IRQS_PER_REG);
    return LOS_OK;
}

STATIC UINT32 HwiMask(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    NVIC_REG->icer[hwiNum / NVIC_IRQS_PER_REG] = 1U << (hwiNum % NVIC_IRQS_PER_REG);
    __DSB();
    __ISB();
    return LOS_OK;
}

STATIC UINT32 HwiSetPriority(HWI_HANDLE_T hwiNum, HWI_PRIOR_T priority)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    NVIC_REG->ipr[hwiNum] = (UINT8)NVIC_PRIO_VALUE(priority);
    return LOS_OK;
}

STATIC UINT32 HwiPending(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    NVIC_REG->ispr[hwiNum / NVIC_IRQS_PER_REG] = 1U << (hwiNum % NVIC_IRQS_PER_REG);
    return LOS_OK;
}

STATIC UINT32 HwiClear(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    NVIC_REG->icpr[hwiNum / NVIC_IRQS_PER_REG] = 1U << (hwiNum % NVIC_IRQS_PER_REG);
    return LOS_OK;
}

STATIC HwiControllerOps g_hwiControllerOps = {
    .enableIrq      = HwiUnmask,
    .disableIrq     = HwiMask,
    .setIrqPriority = HwiSetPriority,
    .getCurIrqNum   = HwiNumGet,
    .triggerIrq     = HwiPending,
    .clearIrq       = HwiClear,
    .getHandleForm  = HalGetHandleForm,
};

HwiControllerOps *HwiControllerOpsGet(VOID)
{
    return &g_hwiControllerOps;
}

/* ****************************************************************************
 Function    : HalInterrupt
 Description : Hardware interrupt entry function (vector table entry,
               installed via OsSetVector / HalHwiInit).
 **************************************************************************** */
LITE_OS_SEC_TEXT VOID HalInterrupt(VOID)
{
    UINT32 hwiIndex;

#if (LOSCFG_KERNEL_RUNSTOP == 1)
    SCB_REG->scr &= (UINT32)~SCB_SCR_SLEEPDEEP;
#endif

    hwiIndex = HwiNumGet();

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    if (hwiIndex >= OS_SYS_VECTOR_CNT) {
        OsCpupIrqStart(OS_HWI_INVALID_IRQ, hwiIndex);
    }
#endif
    OsIntHandle(hwiIndex, &g_hwiHandleForm[hwiIndex]);
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    if (hwiIndex >= OS_SYS_VECTOR_CNT) {
        OsCpupIrqEnd(OS_HWI_INVALID_IRQ, hwiIndex);
    }
#endif
}

/* Weak no-op tick vector; the tick driver installs the real handler. */
WEAK VOID SysTick_Handler(VOID)
{
    return;
}

/* AIRCR is write-protected by VECTKEY; PRIGROUP occupies bits [10:8]. */
STATIC VOID HwiSetPrioGroup(UINT32 prioGroup)
{
    UINT32 regData = SCB_REG->aircr;

    regData &= ~(SCB_AIRCR_VECTKEY_MASK | SCB_AIRCR_PRIGROUP_MASK);
    regData |= (SCB_AIRCR_VECTKEY_KEY << 16) | ((prioGroup << 8) & SCB_AIRCR_PRIGROUP_MASK);
    SCB_REG->aircr = regData;
}

/* ****************************************************************************
 Function    : HalHwiInit
 Description : initialization of the hardware interrupt:
               - Installs default handlers into the vector table (g_hwiForm)
               - Points VTOR at the vector table
               - Sets the NVIC priority grouping (AIRCR.PRIGROUP)
               - Enables usage/bus/mem faults + div0 (unaligned optional)
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID HalHwiInit(VOID)
{
#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
    UINT32 index;
    HWI_PROC_FUNC *hwiForm = (HWI_PROC_FUNC *)ArchGetHwiFrom();

    hwiForm[0] = 0; /* [0] Top of Stack */
    hwiForm[1] = (HWI_PROC_FUNC)Reset_Handler; /* [1] reset */
    for (index = 2; index < OS_SYS_VECTOR_CNT; index++) { /* 2: The starting position of the interrupt */
        hwiForm[index] = (HWI_PROC_FUNC)HalHwiDefaultHandler;
    }
    /* External interrupts: all routes through HalInterrupt (the driver-layer
     * entry), which dispatches via g_hwiHandleForm + OsIntHandle. */
    for (index = OS_SYS_VECTOR_CNT; index < OS_VECTOR_CNT; index++) {
        hwiForm[index] = (HWI_PROC_FUNC)HalInterrupt;
    }
    /* Exception handler register */
    hwiForm[NonMaskableInt_IRQn + OS_SYS_VECTOR_CNT]   = (HWI_PROC_FUNC)HalExcNMI;
    hwiForm[HARDFAULT_IRQN + OS_SYS_VECTOR_CNT]        = (HWI_PROC_FUNC)HalExcHardFault;
    hwiForm[MemoryManagement_IRQn + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalExcMemFault;
    hwiForm[BusFault_IRQn + OS_SYS_VECTOR_CNT]         = (HWI_PROC_FUNC)HalExcBusFault;
    hwiForm[UsageFault_IRQn + OS_SYS_VECTOR_CNT]       = (HWI_PROC_FUNC)HalExcUsageFault;
    hwiForm[SVCall_IRQn + OS_SYS_VECTOR_CNT]           = (HWI_PROC_FUNC)HalExcSvcCall;
    hwiForm[PendSV_IRQn + OS_SYS_VECTOR_CNT]           = (HWI_PROC_FUNC)HalPendSV;
    hwiForm[SysTick_IRQn + OS_SYS_VECTOR_CNT]          = (HWI_PROC_FUNC)SysTick_Handler;

    /* Interrupt vector table location */
    SCB_REG->vtor = (UINT32)(UINTPTR)hwiForm;
#endif
#if (__CORTEX_M >= 0x03U) /* only for Cortex-M3 and above */
    HwiSetPrioGroup(OS_NVIC_AIRCR_PRIGROUP);
#endif

    /* Enable USGFAULT, BUSFAULT, MEMFAULT */
    SCB_REG->shcsr |= (SCB_SHCSR_USGFAULT | SCB_SHCSR_BUSFAULT | SCB_SHCSR_MEMFAULT);

    /* Enable DIV 0 and unaligned exception */
#ifdef LOSCFG_ARCH_UNALIGNED_EXC
    SCB_REG->ccr |= (SCB_CCR_DIV0FAULT | SCB_CCR_UNALIGNFAULT);
#else
    SCB_REG->ccr |= SCB_CCR_DIV0FAULT;
#endif

    return;
}
