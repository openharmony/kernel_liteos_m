/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2024 Huawei Device Co., Ltd. All rights reserved.
 *
 * Cortex-M0+ (ARMv6-M) task dispatch and context switch.
 *
 * ARMv6-M Thumb-1 constraints:
 *   - movs for immediate moves (not mov)
 *   - ldm/stm/push/pop only access R0-R7 (plus LR for push, PC for pop)
 *   - ldr/str register-base only R0-R7 as destination
 *   - No IT blocks, no conditional execution
 *   - No FPU
 *
 * TaskContext (9 words = 36 bytes, saved on PSP):
 *   [0] R4  [1] R5  [2] R6  [3] R7
 *   [4] R8  [5] R9  [6] R10 [7] R11
 *   [8] PRIMASK
 *   Then hardware exception frame: R0-R3, R12, LR, PC, xPSR
 *
 * New modular-arch model (g_runTask / g_oldTask):
 *   - OsSchedStart() calls OsCurrTaskSet(newTask) -> g_runTask = newTask
 *   - ArchStartToRun(newTask) sets g_oldTask = newTask, loads first task
 *   - ArchTaskSchedule() triggers PendSV
 *   - PendSV saves old context to g_oldTask->stackPointer,
 *     loads new from g_runTask->stackPointer, g_oldTask = g_runTask
 */

.syntax unified
.cpu cortex-m0plus
.thumb

.equ    OS_NVIC_INT_CTRL,            0xE000ED04
.equ    OS_NVIC_SYSPRI2,             0xE000ED20
.equ    OS_NVIC_PENDSV_PRI,          0xC0C00000
.equ    OS_NVIC_PENDSVSET,           0x10000000
.equ    OS_CONTROL_KERNEL_MODE,      0x2

    .section .text
    .thumb

    .type ArchStartToRun, %function
    .global ArchStartToRun
ArchStartToRun:
    .fnstart
    .cantunwind

    /* r0 = newTask (set by caller via OsCurrTaskSet) */

    /* Set PendSV + SysTick priority to lowest */
    ldr     r4, =OS_NVIC_SYSPRI2
    ldr     r5, =OS_NVIC_PENDSV_PRI
    str     r5, [r4]

    /* Switch to privileged Thread mode using PSP */
    movs    r1, #OS_CONTROL_KERNEL_MODE
    msr     CONTROL, r1
    isb

    /* g_oldTask = newTask (r0) */
    ldr     r1, =g_oldTask
    str     r0, [r1]

    /* r0 = newTask->stackPointer (offset 0) */
    ldr     r1, [r0]

    /* Set PSP past the entire TaskContext (36 bytes).
     * Future stack pushes grow downward from here. */
    adds    r1, #36
    msr     psp, r1

    /* Load R0 (taskID), LR (ArchTaskExit), PC (OsTaskEntry) from context.
     *
     * We CANNOT use exception return (bx 0xFFFFFFFD) from Thread mode:
     * on ARMv6-M this is UNPREDICTABLE — the core treats it as a regular
     * branch to 0xFFFFFFFC, causing a HardFault.
     * Instead, we branch directly to OsTaskEntry, matching the M7 port.
     */
    ldr     r1, [r0]          /* r1 = context_base */
    ldr     r0, [r1, #36]     /* r0 = uwR0 = taskID (first arg) */
    ldr     r2, [r1, #56]     /* r2 = uwLR = ArchTaskExit */
    ldr     r3, [r1, #60]     /* r3 = uwPC = OsTaskEntry */
    mov     lr, r2

    cpsie   i
    bx      r3                /* direct branch — NOT exception return */

    .fnend

    .type ArchIntLock, %function
    .global ArchIntLock
ArchIntLock:
    .fnstart
    .cantunwind
    mrs     r0, primask
    cpsid   i
    bx      lr
    .fnend

    .type ArchIntUnLock, %function
    .global ArchIntUnLock
ArchIntUnLock:
    .fnstart
    .cantunwind
    mrs     r0, primask
    cpsie   i
    bx      lr
    .fnend

    .type ArchIntRestore, %function
    .global ArchIntRestore
ArchIntRestore:
    .fnstart
    .cantunwind
    msr     primask, r0
    bx      lr
    .fnend

    .type ArchTaskSchedule, %function
    .global ArchTaskSchedule
ArchTaskSchedule:
    .fnstart
    .cantunwind
    ldr     r0, =OS_NVIC_INT_CTRL
    ldr     r1, =OS_NVIC_PENDSVSET
    str     r1, [r0]
    dsb
    isb
    bx      lr
    .fnend

    .type HalPendSV, %function
    .global HalPendSV
HalPendSV:
    .fnstart
    .cantunwind

    mrs     r0, psp

    /* Reserve 36 bytes below PSP for R4-R11 + PriMask */
    subs    r0, #36          /* r0 = context_base */

    /* Save R4-R7 (offset 0-15) — must save BEFORE clobbering */
    stmia   r0!, {r4-r7}     /* r0 = context_base + 16 */

    /* Save R8-R11 (offset 16-31) via low register temps */
    mov     r4, r8
    mov     r5, r9
    mov     r6, r10
    mov     r7, r11
    stmia   r0!, {r4-r7}     /* r0 = context_base + 32 */

    /* Save PRIMASK (offset 32) — read BEFORE disabling interrupts */
    mrs     r1, primask
    str     r1, [r0]         /* PriMask at context_base + 32; r0 unchanged */

    /* Disable interrupts for the duration of the context switch */
    cpsid   i

    /* r0 = context_base + 32, compute context_base */
    subs    r0, #32          /* r0 = context_base */

    /* Store saved SP to old task: g_oldTask->stackPointer = context_base */
    ldr     r1, =g_oldTask
    ldr     r1, [r1]
    str     r0, [r1]

    /* Check if actual switch needed: g_runTask != g_oldTask */
    ldr     r1, =g_runTask
    ldr     r2, [r1]
    ldr     r3, =g_oldTask
    ldr     r3, [r3]
    cmp     r2, r3
    beq     HalPendSVNoSwitch

    /* Load new task SP: r0 = g_runTask->stackPointer */
    ldr     r0, [r2]

    /* g_oldTask = g_runTask */
    ldr     r3, =g_oldTask
    str     r2, [r3]

    /* Restore R8-R11 first (offset 16-28) */
    adds    r0, #16
    ldmia   r0!, {r4-r7}
    mov     r8, r4
    mov     r9, r5
    mov     r10, r6
    mov     r11, r7

    /* Restore R4-R7 (offset 0-12) */
    subs    r0, #32          /* r0 = context_base + 0 */
    ldmia   r0!, {r4-r7}     /* r0 = context_base + 16 */

    /* Load PRIMASK (offset 32) */
    adds    r0, #16          /* r0 = context_base + 32 */
    ldr     r1, [r0]

    /* PSP = context_base + 36 */
    adds    r0, #4           /* r0 = context_base + 36 */
    msr     psp, r0

    /* Restore PRIMASK */
    msr     primask, r1

    bx      lr

HalPendSVNoSwitch:
    /* No context switch needed — PSP still points to hardware frame.
     * Restore PRIMASK from saved context (PriMask at PSP - 4). */
    mrs     r0, psp
    subs    r0, #4
    ldr     r1, [r0]
    msr     primask, r1
    bx      lr

    .fnend
