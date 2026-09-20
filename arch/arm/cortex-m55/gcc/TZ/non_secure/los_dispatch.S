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
 .arch armv8.1-m.main
 .fpu vfpv3-d16-fp16
 .thumb

.equ    OS_FPU_CPACR,                0xE000ED88
.equ    OS_FPU_CPACR_ENABLE,         0x00F00000
.equ    OS_NVIC_INT_CTRL,            0xE000ED04
.equ    OS_NVIC_SYSPRI2,             0xE000ED20
.equ    OS_NVIC_PENDSV_PRI,          0xF0F00000
.equ    OS_NVIC_PENDSVSET,           0x10000000
.equ    OS_TASK_STATUS_RUNNING,      0x0010

    .section .text
    .thumb

.macro SIGNAL_CONTEXT_RESTORE
    PUSH    {R12, LR}
    BLX     OsSignalTaskContextRestore
    POP     {R12, LR}
    CMP     R0, #0
    MOV     R1, R0
    BNE     SignalContextRestore
.endm

    .type HalStartFirstTask, %function
    .global HalStartFirstTask
HalStartFirstTask:
    MOV     R0, #2
    MSR     CONTROL, R0

    LDR     R1, =g_losTask
    LDR     R0, [R1, #4]

    LDR     R12, [R0]                  /* Get the stack pointer of the current task. */
    LDMFD   R12!, {R1-R3}              /* Read from stack: R1 = secureContext, R2 = stackLmit and R3 = excReturn.*/
    LDR     R4, =g_secureContext
    STR     R1, [R4]                   /* Set the secureContext to g_secureContext handler. */
    MSR     PSPLIM, R2                 /* Set the stackLmit for the PSPLIM about current task. */
    ISB

    LDR.W   R1, =OS_FPU_CPACR
    LDR     R1, [R1]
    AND     R1, R1, #OS_FPU_CPACR_ENABLE
    CMP     R1, #OS_FPU_CPACR_ENABLE
    BNE     __DisabledFPU1
    ADD     R12, R12, #64
    VPUSH   {S0}
    VPOP    {S0}

__DisabledFPU1:
    ADD     R12, R12, #36
    MSR     PSP, R12
    CPSIE   I
    BX      R3

    .type ArchIntLock, %function
    .global ArchIntLock
ArchIntLock:
    .fnstart
    .cantunwind

    MRS R0, PRIMASK
    CPSID I
    BX LR
    .fnend

    .type ArchIntUnLock, %function
    .global ArchIntUnLock
ArchIntUnLock:
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
    mov     lr, r0
    mrs     r0, psp

    LDR     R2, =g_secureContext
    LDR     R1, [R2]
    CBZ     R1, __SaveNSContext               /* If the g_secureContext is NULL, so no secure context to save. */

    PUSH    {R0-R1, R12, R14}                 /* Store registers, include LR, PRIMASK. */
    BL      HalSecureContextSave              /* Store the secure context to g_secureContext->curStackPointer. */
    POP     {R0-R3}
    MOV     LR, R3
    MOV     R12, R2                           /* R2 = PRIMASK. */

__SaveNSContext:
    STMFD   R0!, {R4-R12}
    LDR.W   R3, =OS_FPU_CPACR
    LDR     R3, [R3]
    AND     R3, R3, #OS_FPU_CPACR_ENABLE
    CMP     R3, #OS_FPU_CPACR_ENABLE
    BNE     __DisabledFPU2
    VSTMDB  R0!, {D8-D15}

__DisabledFPU2:
    LDR     R5, =g_losTask
    LDR     R6, [R5]                          /* Get the stackPointer handler of the current task. */
    SUBS    R0, R0, #12
    STR     R0, [R6]                          /* Save the new top of stack in TCB. */
    MRS     R2, PSPLIM
    MOV     R3, LR
    STMIA   R0!, {R1, R2-R3}                  /* Store g_secureContext, PSPLIM and LR on the stack of current task. */

    LDR     R0, [R5, #4]
    STR     R0, [R5]
    LDR     R1, [R0]

SignalContextRestore:
    LDMIA   R1!, {R0, R2-R3}                  /* Restore secureContext, PSPLIM and LR from the current task stack. */
    MSR     PSPLIM, R2
    MOV     LR, R3
    LDR     R2, =g_secureContext
    STR     R0, [R2]                          /* Set the secureContext of the new task to g_secureContext. */
    CBZ     R0, __RestoreNSContext            /* If there is no secure context for the new task, so restore from the non-secure context. */
    PUSH    {R1, R3}
    BL      HalSecureContextLoad              /* Restore the secure context. */
    POP     {R1, R3}
    MOV     LR, R3

__RestoreNSContext:
    LDR.W   R3, =OS_FPU_CPACR
    LDR     R3, [R3]
    AND     R3, R3, #OS_FPU_CPACR_ENABLE
    CMP     R3, #OS_FPU_CPACR_ENABLE
    BNE     __DisabledFPU3
    VLDMIA  R1!, {D8-D15}

__DisabledFPU3:
    LDMFD   R1!, {R4-R12}
    MSR     PSP,  R1

    MSR     PRIMASK, R12
    BX      LR
    .fnend

    .type HalSVCStartSchedule, %function
    .global HalSVCStartSchedule
HalSVCStartSchedule:
    .fnstart
    .cantunwind
    LDR     R4, =OS_NVIC_SYSPRI2
    LDR     R5, =OS_NVIC_PENDSV_PRI
    STR     R5, [R4]
    CPSIE   I
    DSB
    ISB
    SVC     2
    .fnend

    .type HalSVCSecureContextAlloc, %function
    .global HalSVCSecureContextAlloc
HalSVCSecureContextAlloc:
    .fnstart
    .cantunwind
    SVC     0
    BX      LR
    .fnend

    .type HalSVCSecureContextFree, %function
    .global HalSVCSecureContextFree
HalSVCSecureContextFree:
    .fnstart
    .cantunwind
    SVC     1
    BX      LR
    .fnend

    .type HalSVCHandler, %function
    .global HalSVCHandler
HalSVCHandler:
    .fnstart
    .cantunwind
    TST     LR, #0x04
    ITE     EQ
    MRSEQ   R1, MSP
    MRSNE   R1, PSP
    LDR     R0, [R1, #24]
    LDRB    R0, [R0, #-2]                     /* Get the SVC number. */

    PUSH    {LR}
    MOV     R2, R1                            /* Get the stack for R2. */
    LDMFD   R2!, {R1}                         /* Get the input arg for HalSecureSVCHandler. */
    STMFD   R2!, {R1}
    BL      HalSecureSVCHandler
    POP     {LR}
    BX      LR
    .fnend
