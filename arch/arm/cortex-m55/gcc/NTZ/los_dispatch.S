/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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
 .arch armv8.1-m.main
 .fpu vfpv3-d16-fp16
 .thumb

.equ    OS_NVIC_INT_CTRL,            0xE000ED04
.equ    OS_NVIC_SYSPRI3,             0xE000ED20
.equ    OS_NVIC_PENDSV_PRI,          0xF0F00000
.equ    OS_NVIC_PENDSVSET,           0x10000000

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

    .type HalStartToRun, %function
    .global HalStartToRun
HalStartToRun:
    .fnstart
    .cantunwind

    ldr     r4, =OS_NVIC_SYSPRI3
    ldr     r5, =OS_NVIC_PENDSV_PRI
    str     r5, [r4]

    mov     r0, #2
    msr     CONTROL, r0

    ldr     r1, =g_losTask
    ldr     r0, [r1, #4]
    ldr     r12, [r0]

    ldr     r2, [r12, #4]
    tst     r2, #0x10
    it      ne
    bne     __DisabledFPU

    add     r12, r12, #108
    ldmfd   r12!, {r0-r7}
    add     r12, r12, #72
    msr     psp, r12
    vpush   {s0}
    vpop    {s0}

    mov     lr, r5
    cpsie   I
    bx      r6

__DisabledFPU:
    add     r12, r12, #44

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

    mrs     r0, PRIMASK
    cpsid   I
    bx      lr
    .fnend

    .type ArchIntUnLock, %function
    .global ArchIntUnLock
ArchIntUnLock:
    .fnstart
    .cantunwind

    mrs     r0, PRIMASK
    cpsie   I
    bx      lr
    .fnend

    .type ArchIntRestore, %function
    .global ArchIntRestore
ArchIntRestore:
    .fnstart
    .cantunwind

    msr     PRIMASK, r0
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

    mrs     r12, PRIMASK
    cpsid   I

HalTaskSwitch:
    SIGNAL_CONTEXT_RESTORE

    push    {r12, lr}
    blx     OsSchedTaskSwitch
    pop     {r12, lr}
    cmp     r0, #0
    mov     r0, lr
    bne     TaskContextSwitch
    msr     PRIMASK, r12
    bx      lr

TaskContextSwitch:
    mov      lr, r0
    mrs      r0, psp
    tst      lr, #0x10
    it       eq
    vstmdbeq r0!, {d8-d15}
    mrs      r2, psplim
    mov      r3, lr
    stmfd    r0!, {r2-r12}

    ldr      r5, =g_losTask
    ldr      r6, [r5]
    str      r0, [r6]

    ldr      r0, [r5, #4]
    str      r0, [r5]
    ldr      r1, [r0]

SignalContextRestore:
    ldmfd    r1!, {r2-r12}
    msr      psplim, r2
    tst      r3, #0x10
    it       eq
    vldmiaeq r1!, {d8-d15}
    msr      psp, r1
    msr      PRIMASK, r12

    bx       lr
    .fnend
