;
; Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
; Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
;
; Redistribution and use in source and binary forms, with or without modification,
; are permitted provided that the following conditions are met:
;
; 1. Redistributions of source code must retain the above copyright notice, this list of
;    conditions and the following disclaimer.
;
; 2. Redistributions in binary form must reproduce the above copyright notice, this list
;    of conditions and the following disclaimer in the documentation and/or other materials
;    provided with the distribution.
;
; 3. Neither the name of the copyright holder nor the names of its contributors may be used
;    to endorse or promote products derived from this software without specific prior written
;    permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
; "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
; THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
; PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
; WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
; OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
; ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;

    PRESERVE8

    EXPORT  ArchIntLock
    EXPORT  ArchIntUnLock
    EXPORT  ArchIntRestore
    EXPORT  ArchTaskSchedule
    EXPORT  HalPendSV
    EXPORT  HalSVCHandler
    EXPORT  HalStartFirstTask
    EXPORT  HalSVCStartSchedule
    EXPORT  HalSVCSecureContextAlloc
    EXPORT  HalSVCSecureContextFree

    IMPORT  OsSchedTaskSwitch
    IMPORT  g_losTask

    EXTERN  HalSecureSVCHandler
    EXTERN  HalSecureContextSave
    EXTERN  HalSecureContextLoad
    EXTERN  g_secureContext

OS_FPU_CPACR                EQU     0xE000ED88
OS_FPU_CPACR_ENABLE         EQU     0x00F00000
OS_NVIC_INT_CTRL            EQU     0xE000ED04
OS_NVIC_SYSPRI2             EQU     0xE000ED20
OS_NVIC_PENDSV_PRI          EQU     0xF0F00000
OS_NVIC_PENDSVSET           EQU     0x10000000
OS_TASK_STATUS_RUNNING      EQU     0x0010

    SECTION    .text:CODE(2)
    THUMB
    REQUIRE8

HalStartFirstTask
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
    VPUSH   S0;
    VPOP    S0;

__DisabledFPU1
    ADD     R12, R12, #36
    MSR     PSP, R12
    CPSIE   I
    BX      R3

ArchIntLock
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR

ArchIntUnLock
    MRS     R0, PRIMASK
    CPSIE   I
    BX      LR

ArchIntRestore
    MSR     PRIMASK, R0
    BX      LR

ArchTaskSchedule
    LDR     R0, =OS_NVIC_INT_CTRL
    LDR     R1, =OS_NVIC_PENDSVSET
    STR     R1, [R0]
    DSB
    ISB
    BX      LR

HalPendSV
    MRS     R12, PRIMASK
    CPSID   I

HalTaskSwitch
    PUSH    {R12, LR}
    BLX     OsSchedTaskSwitch
    POP     {R12, LR}
    CMP     R0, #0
    MOV     R0, LR
    BNE     TaskContextSwitch
    MSR     PRIMASK, R12
    BX      LR

TaskContextSwitch
    MOV     LR, R0
    MRS     R0, PSP

    LDR     R2, =g_secureContext
    LDR     R1, [R2]
    CBZ     R1, __SaveNSContext               /* If the g_secureContext is NULL, so no secure context to save. */

    PUSH    {R0-R1, R12, R14}                 /* Store registers, include LR, PRIMASK. */
    BL      HalSecureContextSave              /* Store the secure context to g_secureContext->curStackPointer. */
    POP     {R0-R3}
    MOV     LR, R3
    MOV     R12, R2                           /* R2 = PRIMASK. */

__SaveNSContext
    STMFD   R0!, {R4-R12}
    LDR.W   R3, =OS_FPU_CPACR
    LDR     R3, [R3]
    AND     R3, R3, #OS_FPU_CPACR_ENABLE
    CMP     R3, #OS_FPU_CPACR_ENABLE
    BNE     __DisabledFPU2
    VSTMDB  R0!, {D8-D15}

__DisabledFPU2
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

__RestoreNSContext
    LDR.W   R3, =OS_FPU_CPACR
    LDR     R3, [R3]
    AND     R3, R3, #OS_FPU_CPACR_ENABLE
    CMP     R3, #OS_FPU_CPACR_ENABLE
    BNE     __DisabledFPU3
    VLDMIA  R1!, {D8-D15}

__DisabledFPU3
    LDMFD   R1!, {R4-R12}
    MSR     PSP,  R1

    MSR     PRIMASK, R12
    BX      LR

HalSVCStartSchedule
    LDR     R4, =OS_NVIC_SYSPRI2
    LDR     R5, =OS_NVIC_PENDSV_PRI
    STR     R5, [R4]
    CPSIE   I
    DSB
    ISB
    SVC     2

HalSVCSecureContextAlloc
    SVC     0
    BX      LR

HalSVCSecureContextFree
    SVC     1
    BX      LR

HalSVCHandler
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

    END
