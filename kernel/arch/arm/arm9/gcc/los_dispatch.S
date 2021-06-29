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

    .equ    OS_PSR_THUMB,       0x20U
    .equ    OS_PSR_INT_DIS,     0xC0U
    .equ    OS_PSR_FIQ_DIS,     0x40U
    .equ    OS_PSR_IRQ_DIS,     0x80U
    .equ    OS_PSR_MODE_MASK,   0x1FU
    .equ    OS_PSR_MODE_USR,    0x10U
    .equ    OS_PSR_MODE_FIQ,    0x11U
    .equ    OS_PSR_MODE_IRQ,    0x12U
    .equ    OS_PSR_MODE_SVC,    0x13U
    .equ    OS_PSR_MODE_ABT,    0x17U
    .equ    OS_PSR_MODE_UND,    0x1BU
    .equ    OS_PSR_MODE_SYS,    0x1FU

    .global HalExceptIrqHdl
    .global HalTaskSchedule
    .global HalStartToRun
    .global OsTaskEntryArm
    .global OsTaskEntryThumb

    .extern OsTaskEntry
    .extern OsSchedTaskSwitch
    .extern g_sysNeedSched

    .code 32
    .text

HalStartToRun:
    MSR     CPSR_c, #(OS_PSR_INT_DIS | OS_PSR_FIQ_DIS | OS_PSR_MODE_SVC)

    LDR     R1, =g_losTask
    LDR     R0, [R1, #4]
    LDR     SP, [R0]

    LDMFD   SP!, {R0}
    MSR     SPSR_cxsf, R0

    LDMFD   SP!, {R0-R12, LR, PC}^

HalTaskSchedule:
    STMFD   SP!, {LR}
    STMFD   SP!, {LR}
    STMFD   SP!, {R0-R12}

    MRS     R0, CPSR
    MRS     R1, CPSR
    ORR     R1, #OS_PSR_IRQ_DIS
    MSR     CPSR, R1

    TST     LR, #1
    ORRNE   R0, R0, #OS_PSR_THUMB
    STMFD   SP!, {R0}

    LDR     R1, =g_sysNeedSched
    MOV     R2, #1
    STR     R2, [R1]

    LDR     R0, =g_intCount
    LDR     R0, [R0]
    CMP     R0, #0
    BNE     TaskContextLoad
    STR     R0, [R1]

    BLX     OsSchedTaskSwitch
    CMP     R0, #0
    BNE     NewTaskContextSwitch
    B       TaskContextLoad

NewTaskContextSwitch:
    LDR     R0, =g_losTask
    LDR     R1, [R0]
    STR     SP, [R1]

    LDR     R1, [R0, #4]
    STR     R1, [R0]
    LDR     SP, [R1]

TaskContextLoad:
    LDMFD   SP!, {R0}
    MSR     SPSR_cxsf, R0
    LDMFD   SP!, {R0-R12, LR, PC}^

OsTaskEntryArm:
    STMFD   SP!, {LR}
    BL      OsTaskEntry
    LDMFD   SP!, {LR}
    BX      LR

    .code 16
    .text
OsTaskEntryThumb:
    PUSH    {LR}
    BL      OsTaskEntry
    POP     {R0}
    MOV     LR, R0
    BX      LR

