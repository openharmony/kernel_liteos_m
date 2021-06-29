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

    .equ OS_PSR_INT_DIS,    0xC0U
    .equ OS_PSR_FIQ_DIS,    0x40U
    .equ OS_PSR_IRQ_DIS,    0x80U
    .equ OS_PSR_MODE_MASK,  0x1FU
    .equ OS_PSR_MODE_USR,   0x10U
    .equ OS_PSR_MODE_FIQ,   0x11U
    .equ OS_PSR_MODE_IRQ,   0x12U
    .equ OS_PSR_MODE_SVC,   0x13U
    .equ OS_PSR_MODE_ABT,   0x17U
    .equ OS_PSR_MODE_UND,   0x1BU
    .equ OS_PSR_MODE_SYS,   0x1FU

    .equ OS_EXCEPT_RESET,           0x00
    .equ OS_EXCEPT_UNDEF_INSTR,     0x01
    .equ OS_EXCEPT_SWI,             0x02
    .equ OS_EXCEPT_PREFETCH_ABORT,  0x03
    .equ OS_EXCEPT_DATA_ABORT,      0x04
    .equ OS_EXCEPT_FIQ,             0x05
    .equ OS_EXCEPT_ADDR_ABORT,      0x06
    .equ OS_EXCEPT_IRQ,             0x07


    .global HalExceptFiqHdl
    .global HalExceptAddrAbortHdl
    .global HalExceptDataAbortHdl
    .global HalExceptPrefetchAbortHdl
    .global HalExceptSwiHdl
    .global HalExceptUndefInstrHdl
    .global HalExceptIrqHdl

    .extern g_sysNeedSched
    .extern g_losTaskLock
    .extern HalExcHandleEntry
    .extern HalInterrupt
    .extern OsSchedTaskSwitch
    .extern __svc_stack_top
    .extern __exc_stack_top
    .extern __irq_stack_top

    .code 32
    .text

HalExceptUndefInstrHdl:
    STMFD   SP!, {R0-R5}
    MOV     R0, #OS_EXCEPT_UNDEF_INSTR
    B       _osExceptDispatch

HalExceptSwiHdl:
    STMFD   SP!, {LR}
    STMFD   SP!, {LR}
    STMFD   SP!, {SP}
    STMFD   SP!, {R0-R12}

    MRS     R1, SPSR
    STMFD   SP!, {R1}

    MOV     R0, #OS_EXCEPT_SWI

    B       _osExceptionSwi

HalExceptPrefetchAbortHdl:
    SUB     LR, LR, #4
    STMFD   SP!, {R0-R5}

    MOV     R0, #OS_EXCEPT_PREFETCH_ABORT

    B       _osExceptDispatch

HalExceptDataAbortHdl:
    SUB     LR, LR, #4
    STMFD   SP!, {R0-R5}

    MOV     R0, #OS_EXCEPT_DATA_ABORT

    B       _osExceptDispatch

HalExceptAddrAbortHdl:
    SUB     LR, LR, #8
    STMFD   SP!, {R0-R5}

    MOV     R0, #OS_EXCEPT_ADDR_ABORT

    B       _osExceptDispatch

HalExceptFiqHdl:
    SUB     LR, LR, #4
    STMFD   SP!, {R0-R5}

    MOV     R0, #OS_EXCEPT_FIQ

    B       _osExceptDispatch

HalExceptIrqHdl:
    SUB     LR, LR, #4
    STMFD   SP!, {R0-R2}

    MOV     R0, SP
    MRS     R1, SPSR
    MOV     R2, LR

    MSR     CPSR_c, #(OS_PSR_INT_DIS | OS_PSR_FIQ_DIS | OS_PSR_MODE_SVC)
    STMFD   SP!, {R2}
    STMFD   SP!, {LR}
    STMFD   SP!, {R3-R12}
    LDMFD   R0!, {R5-R7}
    STMFD   SP!, {R5-R7}
    STMFD   SP!, {R1}

    LDR     R2, =g_losTask
    LDR     R3, [R2]
    STR     SP, [R3]

    LDR     SP, =__svc_stack_top

    BLX     HalInterrupt

    LDR     R0, =g_losTaskLock
    LDR     R0, [R0]
    CMP     R0, #0
    BNE     NoTaskContextSwitch

    LDR     R0, =g_sysNeedSched
    LDR     R0, [R0]
    CMP     R0, #0
    BNE     NeedTaskSwitch
    B       NoTaskContextSwitch

NeedTaskSwitch:
    LDR     R0, =g_sysNeedSched
    MOV     R1, #0
    STR     R1, [R0]
    BLX     OsSchedTaskSwitch
    CMP     R0, #0
    BEQ     NoTaskContextSwitch
    LDR     R0, =g_losTask
    LDR     R1, [R0, #4]
    STR     R1, [R0]
    LDR     SP, [R1]
    B       TaskContextLoad

NoTaskContextSwitch:
    LDR     R1, =g_losTask
    LDR     R1, [R1]
    LDR     SP, [R1]

TaskContextLoad:
    LDMFD   SP!, {R0}
    MSR     SPSR_cxsf, R0
    LDMFD   SP!, {R0-R12, LR, PC}^

_osExceptDispatch:
    MRS     R1, SPSR
    MOV     R2, LR
    MOV     R4, SP
    ADD     SP, SP, #(6 * 4)

    MSR     CPSR_c, #(OS_PSR_INT_DIS | OS_PSR_MODE_SVC)
    MOV     R3, SP
    LDR     SP, =__exc_stack_top

    STMFD   SP!, {R2}
    STMFD   SP!, {LR}
    STMFD   SP!, {R3}
    STMFD   SP!, {R6-R12}
    LDMFD   R4!, {R6-R12}
    STMFD   SP!, {R6-R11}
    STMFD   SP!, {R1}

_osExceptionSwi:
    MOV     R3, SP

_osExceptionGetSP:
    STMFD   SP!, {R1}
    LDR     R2, =HalExcHandleEntry

    MOV     LR, PC
    BX      R2

    LDMFD   SP!, {R1}
    MOV     SP, R1

    LDMFD   SP!, {R1}
    MSR     CPSR, R1
    LDMFD   SP!, {R0-R12}
    ADD     SP, SP, #(4 * 2)
    LDMFD   SP!, {LR}
    SUB     SP, SP, #(4 * 3)
    LDMFD   SP, {SP}
    ADD     LR, LR, #4
    MOV     PC, LR

    .end
