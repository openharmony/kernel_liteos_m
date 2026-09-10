/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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
.syntax unified
.arch armv7e-m
.fpu fpv4-sp-d16
.thumb

.equ    OS_FPU_CPACR,                0xE000ED88
.equ    OS_FPU_CPACR_ENABLE,         0x00F00000
.equ    OS_NVIC_INT_CTRL,            0xE000ED04
.equ    OS_NVIC_SYSPRI2,             0xE000ED20
.equ    OS_NVIC_PENDSV_PRI,          0xF0F00000
.equ    OS_NVIC_PENDSVSET,           0x10000000
.equ    OS_TASK_STATUS_RUNNING,      0x0010
.equ    OS_CONTROL_FPCA_ENABLE,      0x4
.equ    OS_CONTROL_KERNEL_MODE,      0x2
#if (LOSCFG_SECURE == 1)
.equ    OS_CONTROL_USER_MODE,        0x3
.equ    OS_TASK_FLAG_USER_TASK,      0x0200
#endif

    .section .text
    .thumb

.macro SIGNAL_CONTEXT_RESTORE
    push    {r12, lr}
    blx     OsSignalTaskContextRestore
    pop     {r12, lr}
    cmp     r0, #0
    mov     r1, r0
    bne     SignalContextRestore
.endm

    .type ArchStartToRun, %function
    .global ArchStartToRun
ArchStartToRun:
    .fnstart
    .cantunwind

    ldr     r4, =OS_NVIC_SYSPRI2
    ldr     r5, =OS_NVIC_PENDSV_PRI
    str     r5, [r4]

    mov     r2, #OS_CONTROL_KERNEL_MODE
    msr     CONTROL, r2

    ldr     r1, =g_oldTask
    str     r0, [r1]

    ldr     r12, [r0]

    ldr.w   r1, =OS_FPU_CPACR
    ldr     r1, [r1]
    and     r1, r1, #OS_FPU_CPACR_ENABLE
    cmp     r1, #OS_FPU_CPACR_ENABLE
    bne     __DisabledFPU
    add     r12, r12, #100

    ldmfd   r12!, {R0-R7}
    add     r12, r12, #72
    msr     psp, r12
    vpush   {s0}
    vpop    {s0}
    mov     lr, r5
    cpsie   i
    bx      r6

__DisabledFPU:
    add     r12, r12, #36

    ldmfd   r12!, {r0-r7}
    msr     psp, r12
    mov     lr, r5
    cpsie   I
    bx      r6

    .fnend

    .type ArchIntLock, %function
    .global ArchIntLock
ArchIntLock:
    .fnstart
    .cantunwind

    MRS R0, PRIMASK
    CPSID I
    BX LR
    .fnend

    .type ArchIntUnlock, %function
    .global ArchIntUnlock
ArchIntUnlock:
    .fnstart
    .cantunwind

    MRS R0, PRIMASK
    CPSIE I
    BX LR
    .fnend

    .type ArchIntRestore, %function
    .global ArchIntRestore
ArchIntRestore:
    .fnstart
    .cantunwind

    MSR PRIMASK, R0
    BX LR
    .fnend

    .type ArchTaskSchedule, %function
    .global ArchTaskSchedule
ArchTaskSchedule:
    .fnstart
    .cantunwind

    ldr     r0, =OS_NVIC_INT_CTRL
    ldr     r1, =OS_NVIC_PENDSVSET
    str     r1, [r0]
    mrs     r2, IPSR
    cbnz    r2, ArchTaskScheduleExit
    mrs     r0, PRIMASK
    cpsie   i
    msr     PRIMASK, r0
ArchTaskScheduleExit:
    bx      lr
    .fnend

    .type HalPendSV, %function
    .global HalPendSV
HalPendSV:
    .fnstart
    .cantunwind

    mrs     r12, PRIMASK
    cpsid   I

HalTaskSwitch:
    SIGNAL_CONTEXT_RESTORE

HalPendSVSwitch:
    /* Check if actual switch needed: g_runTask != g_oldTask */
    ldr     r0, =g_runTask
    ldr     r1, [r0]
    ldr     r2, =g_oldTask
    ldr     r3, [r2]
    cmp     r1, r3
    beq     HalPendSVNoSwitch

    /* Context switch: save old, load new */
    mrs     r0, psp

    stmfd   r0!, {r4-r12}
    ldr.w   r3, =OS_FPU_CPACR
    ldr     r3, [r3]
    and     r3, r3, #OS_FPU_CPACR_ENABLE
    cmp     r3, #OS_FPU_CPACR_ENABLE
    bne     __DisabledFPU1
    vstmdb  r0!, {d8-d15}

__DisabledFPU1:
    ldr     r5, =g_oldTask
    ldr     r6, [r5]
    str     r0, [r6]            /* save SP to oldTask */

    ldr     r0, =g_runTask
    ldr     r0, [r0]
    str     r0, [r5]             /* g_oldTask = g_runTask */
    ldr     r1, [r0]             /* newTask stackPointer */

SignalContextRestore:
    ldr.w   r3, =OS_FPU_CPACR
    ldr     r3, [r3]
    and     r3, r3, #OS_FPU_CPACR_ENABLE
    cmp     r3, #OS_FPU_CPACR_ENABLE
    bne     __DisabledFPU2
    vldmia  r1!, {d8-d15}

__DisabledFPU2:
#if (LOSCFG_SECURE == 1)
    ldrh    r7, [r0, #4]
    tst     r7, #OS_TASK_FLAG_USER_TASK
    ite     eq
    moveq   r3, #OS_CONTROL_KERNEL_MODE
    movne   r3, #OS_CONTROL_USER_MODE
    msr     CONTROL, r3
#endif
    ldmfd   r1!, {r4-r12}
    msr     psp,  r1

    msr     PRIMASK, r12

    bx      lr

HalPendSVNoSwitch:
    msr     PRIMASK, r12
    bx      lr
    .fnend
