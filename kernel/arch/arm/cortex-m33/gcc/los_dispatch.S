/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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
 .thumb
 .fpu fpv5-d16
//;.arch_extension sec



.equ    OS_NVIC_INT_CTRL,            0xE000ED04
.equ    OS_NVIC_SYSPRI2,             0xE000ED20
.equ    OS_NVIC_PENDSV_PRI,          0xF0F00000
.equ    OS_NVIC_PENDSVSET,           0x10000000
.equ    OS_TASK_STATUS_RUNNING,      0x0010

    .section .text
    .thumb

    .type HalStartToRun, %function
    .global HalStartToRun
HalStartToRun:
    .fnstart
    .cantunwind

    ldr     r4, =OS_NVIC_SYSPRI2
    ldr     r5, =OS_NVIC_PENDSV_PRI
    str     r5, [r4]

    mov     r0, #2
    msr     CONTROL, r0

    ldr     r1, =g_losTask
    ldr     r0, [r1, #4]
    ldr     r12, [r0]
#if ((defined(__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined(__FPU_USED) && (__FPU_USED == 1U)))
    add     r12, r12, #100

    ldmfd   r12!, {r0-r7}
    add     r12, r12, #72
    msr     psp, r12
    vpush   {S0}
    vpop    {S0}
#else
    add     r12, r12, #36

    ldmfd   r12!, {r0-r7}
    msr     psp, r12
#endif
    mov     lr, r5
    //MSR     xPSR, R7

    cpsie   I
    bx      r6

    .fnend


    .type HalIntLock, %function
    .global HalIntLock
HalIntLock:
    .fnstart
    .cantunwind

    MRS R0, PRIMASK
    CPSID I
    BX LR
    .fnend

    .type HalIntUnLock, %function
    .global HalIntUnLock
HalIntUnLock:
    .fnstart
    .cantunwind

    MRS R0, PRIMASK
    CPSIE I
    BX LR
    .fnend

    .type HalIntRestore, %function
    .global HalIntRestore
HalIntRestore:
    .fnstart
    .cantunwind

    MSR PRIMASK, R0
    BX LR
    .fnend

    .type HalTaskSchedule, %function
    .global HalTaskSchedule
HalTaskSchedule:
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
    push    {r12, lr}
    blx     OsSchedTaskSwitch
    pop     {r12, lr}
    cmp     r0, #0
    mov     r0, lr
    bne     TaskContextSwitch
    msr     PRIMASK, r12
    bx      lr

TaskContextSwitch:
    mov     lr, r0
    mrs     r0, psp

    stmfd   r0!, {r4-r12}

#if ((defined(__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined(__FPU_USED) && (__FPU_USED == 1U)))
    vstmdb   r0!, {d8-d15}
#endif
    ldr     r5, =g_losTask
    ldr     r6, [r5]
    str     r0, [r6]

    ldr     r0, [r5, #4]
    str     r0, [r5]

    ldr     r1,   [r0]

#if ((defined(__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined(__FPU_USED) && (__FPU_USED == 1U)))
    vldmia   r1!, {d8-d15}
#endif
    ldmfd   r1!, {r4-r12}
    msr     psp,  r1

    msr     PRIMASK, r12


    bx      lr
    .fnend
