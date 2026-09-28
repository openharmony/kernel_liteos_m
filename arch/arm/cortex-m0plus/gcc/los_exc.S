/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2024 Huawei Device Co., Ltd. All rights reserved.
 *
 * Cortex-M0+ (ARMv6-M) exception handling.
 *
 * Simplified for Thumb-1 / HardFault-only, but fixes two critical issues
 * compared to a naive port:
 *
 * 1. Context detection via EXC_RETURN (LR) instead of scanning only NVIC
 *    active registers.  On Cortex-M0+ the NVIC active array does NOT cover
 *    system exceptions (PendSV, SysTick, SVC), so a HardFault that occurs
 *    inside PendSV would be misidentified as a task exception.  Checking
 *    bit 2 of EXC_RETURN reliably distinguishes PSP (task) from MSP
 *    (handler / init).
 *
 * 2. Hardware frame copy for task exceptions.  When the fault originates
 *    in Thread mode (PSP), the 8-word hardware frame lives on PSP while
 *    the software-saved R4-R11 live on MSP.  We copy the hardware frame
 *    from PSP to MSP so that EXC_CONTEXT_S is contiguous and the display
 *    code reads correct register values.
 */

.syntax unified
.cpu cortex-m0plus
.thumb
.section .text

    .global  HalExcNMI
    .global  HalExcHardFault
    .global  HalExcMemFault
    .global  HalExcBusFault
    .global  HalExcUsageFault
    .global  HalExcSvcCall

    .extern HalExcHandleEntry
    .extern g_taskScheduled

.equ OS_FLG_BGD_ACTIVE,   0x0001

.equ OS_EXC_CAUSE_NMI,        16
.equ OS_EXC_CAUSE_HARDFAULT,  17

.equ FLAG_HWI_ACTIVE,     0x20000
.equ FLAG_NO_FLOAT,       0x10000000

.equ OS_NVIC_ACT_BASE,    0xE000E300

    .type HalExcNMI, %function
    .global HalExcNMI
HalExcNMI:
    .fnstart
    .cantunwind
    movs    r0, #OS_EXC_CAUSE_NMI
    movs    r1, #0
    b       osExcDispatch
    .fnend

    .type HalExcHardFault, %function
    .global HalExcHardFault
HalExcHardFault:
    .fnstart
    .cantunwind
    movs    r0, #OS_EXC_CAUSE_HARDFAULT
    movs    r1, #0
    b       osExcDispatch
    .fnend

    /*
     * ARMv6-M has no separate MemManage/Bus/Usage fault exceptions
     * (all faults escalate to HardFault) and no CFSR/HFSR/BFAR/MMAR
     * registers.  These stubs exist only because the shared NVIC driver
     * (drivers/interrupt/arm_nvic.c HalHwiInit) installs them into the
     * system vector slots unconditionally; on M0+ hardware the slots
     * are Reserved and never taken.
     */
    .type HalExcMemFault, %function
    .global HalExcMemFault
HalExcMemFault:
    .fnstart
    .cantunwind
    b       HalExcHardFault
    .fnend

    .type HalExcBusFault, %function
    .global HalExcBusFault
HalExcBusFault:
    .fnstart
    .cantunwind
    b       HalExcHardFault
    .fnend

    .type HalExcUsageFault, %function
    .global HalExcUsageFault
HalExcUsageFault:
    .fnstart
    .cantunwind
    b       HalExcHardFault
    .fnend

    .type HalExcSvcCall, %function
    .global HalExcSvcCall
HalExcSvcCall:
    .fnstart
    .cantunwind
    mrs     r1, psp
    ldr     r0, [r1, #24]
    subs    r0, r0, #2
    ldrb    r0, [r0]
    movs    r1, #0
    b       osExcDispatch
    .fnend

    .type osExcDispatch, %function
    .global osExcDispatch
osExcDispatch:
    .fnstart
    .cantunwind

    /*
     * Use EXC_RETURN (LR) to determine the fault context.
     * Bit 2:  0 = MSP  (handler mode or Thread+MSP init)
     *         1 = PSP  (Thread mode — task)
     *
     * R2 and R3 are safe to clobber: their task values are preserved
     * in the hardware frame and will be read from there.
     */
    mov     r2, lr
    movs    r3, #4
    tst     r2, r3
    bne     osExcFromTask

    /* ===========================================================
     * Fault from MSP context (Handler mode or Thread+MSP init).
     * The hardware frame is already on MSP, contiguous after our
     * 40-byte save area — no copy needed.
     * =========================================================== */
    mov     r3, sp              /* r3 = MSP after hardware push */

    /* Save R4-R11 + PRIMASK + SP on MSP (10 words = 40 bytes) */
    push_save_and_scan:
    sub     sp, sp, #40
    str     r4, [sp, #0]
    str     r5, [sp, #4]
    str     r6, [sp, #8]
    str     r7, [sp, #12]
    mov     r4, r8
    str     r4, [sp, #16]
    mov     r4, r9
    str     r4, [sp, #20]
    mov     r4, r10
    str     r4, [sp, #24]
    mov     r4, r11
    str     r4, [sp, #28]
    mrs     r4, primask
    str     r4, [sp, #32]
    adds    r3, #32             /* original SP before hardware push */
    str     r3, [sp, #36]

    /* R4-R7 now free — check NVIC active registers for external IRQ */
    ldr     r2, =OS_NVIC_ACT_BASE
    movs    r4, #0
osExcIrqLoop:
    ldr     r5, [r2, r4]
    cmp     r5, #0
    bne     osExcInIrq
    adds    r4, r4, #4
    cmp     r4, #32
    bne     osExcIrqLoop

    /* Not in external IRQ — check if scheduler started */
    ldr     r2, =g_taskScheduled
    ldr     r2, [r2]
    movs    r5, #OS_FLG_BGD_ACTIVE
    tst     r2, r5
    beq     osExcCallHandler    /* Init phase — no HWI flag */

    /* In a system exception handler (PendSV / SysTick / HardFault) */
    ldr     r5, =FLAG_HWI_ACTIVE
    orrs    r0, r0, r5
    movs    r1, #0
    b       osExcCallHandler

osExcInIrq:
    /* r5 = active bits, r4 = byte offset */
    ldr     r6, =FLAG_HWI_ACTIVE
    orrs    r0, r0, r6
    /* Find IRQ number by scanning set bits */
    movs    r2, #0
osExcIrqFindBit:
    lsrs    r5, r5, #1
    bcc     osExcIrqNextBit
    b       osExcIrqFound
osExcIrqNextBit:
    adds    r2, r2, #1
    cmp     r2, #32
    bne     osExcIrqFindBit
osExcIrqFound:
    lsrs    r4, r4, #2          /* byte offset / 4 = word index */
    lsls    r4, r4, #5          /* word index * 32 */
    adds    r2, r2, r4          /* full IRQ number */
    mov     r1, r2              /* pid = IRQ number */
    b       osExcCallHandler

    /* ===========================================================
     * Fault from PSP context (Thread mode — task).
     * Hardware frame is on PSP — must copy to MSP for a
     * contiguous EXC_CONTEXT_S layout.
     * =========================================================== */
osExcFromTask:
    mrs     r3, psp             /* r3 = PSP (hardware frame start) */

    /* Save R4-R11 + PRIMASK + SP on MSP */
    sub     sp, sp, #40
    str     r4, [sp, #0]
    str     r5, [sp, #4]
    str     r6, [sp, #8]
    str     r7, [sp, #12]
    mov     r4, r8
    str     r4, [sp, #16]
    mov     r4, r9
    str     r4, [sp, #20]
    mov     r4, r10
    str     r4, [sp, #24]
    mov     r4, r11
    str     r4, [sp, #28]
    mrs     r4, primask
    str     r4, [sp, #32]
    /* Save task SP = PSP + 32 (the original SP before hardware push) */
    mov     r4, r3
    adds    r4, #32
    str     r4, [sp, #36]

    /*
     * Copy the 8-word hardware frame from PSP to sp+40 so that
     * EXC_CONTEXT_S is fully contiguous on MSP.
     * r3 still points to PSP.
     */
    ldmia   r3!, {r4-r7}        /* load R0, R1, R2, R3 from PSP */
    mov     r2, sp
    adds    r2, #40             /* destination: sp + 40 */
    stmia   r2!, {r4-r7}        /* store R0-R3 */
    ldmia   r3!, {r4-r7}        /* load R12, LR, PC, xPSR from PSP */
    stmia   r2!, {r4-r7}        /* store R12, LR, PC, xPSR */
    /* fall through */

osExcCallHandler:
    /* Set FLAG_NO_FLOAT (no FPU on Cortex-M0+) */
    ldr     r4, =FLAG_NO_FLOAT
    orrs    r0, r0, r4

    /* Call HalExcHandleEntry(excType, faultAddr, pid, excBufAddr) */
    mov     r3, sp              /* excBufAddr = saved context on MSP */
    cpsid   i
    ldr     r4, =HalExcHandleEntry
    bx      r4

    .fnend
