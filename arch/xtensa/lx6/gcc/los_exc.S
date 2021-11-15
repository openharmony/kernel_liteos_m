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

#include "los_arch_regs.h"
#include "los_arch_macro.h"

.extern HalInterrupt
.extern UserException

.section    .iram,"ax"
.literal_position
.type       HandleEntry,@function
.align      4
HandleEntry:

    mov     a0, sp
    addi    sp, sp, -CONTEXT_SIZE

    PUSH_ALL_REG a1
    s32i    a0, sp, CONTEXT_OFF_A1
    rsr     a0, EPC1
    s32i    a0, sp, CONTEXT_OFF_PC
    rsr     a0, EXCSAVE1
    s32i    a0, sp, CONTEXT_OFF_A0

    wsr     a1, EXCSAVE1
    l32i    a1, a1, CONTEXT_OFF_A1
    call0   SpillWindow
    rsr     a1, EXCSAVE1
    rsr     a0, EXCCAUSE
    s32i    a0, sp, CONTEXT_OFF_EXCCAUSE
    rsr     a0, EXCVADDR
    s32i    a0, sp, CONTEXT_OFF_EXCVADDR
    movi    a0, INT_MASK | USER_VECTOR_MODE | WOE_ENABLE
    wsr     a0, PS

    rsr     a6, EPC1
    mov     a7, sp
    rsr     a8, EXCCAUSE
    call4   HalExcHandleEntry
1:
    j       1b

.begin      literal_prefix .DebugExceptionHandler
.section    .DebugExceptionHandler.text, "ax"
.global     DebugExceptionHandler
.align      4
.literal_position
DebugExceptionHandler:

    call0   UserException

.end        literal_prefix

.begin      literal_prefix .NMIExceptionHandler
.section    .NMIExceptionHandler.text, "ax"
.global     NMIExceptionHandler
.align      4
.literal_position
NMIExceptionHandler:

    call0   UserException

.end        literal_prefix

.begin      literal_prefix .DoubleExceptionHandler
.section    .DoubleExceptionHandler.text, "ax"
.global     DoubleExceptionHandler
.align      4
.literal_position
DoubleExceptionHandler:

    movi    a0, INT_MASK | USER_VECTOR_MODE | WOE_ENABLE
    wsr     a0, PS
    rsr     a2, EXCCAUSE
    call0   UserException

.end        literal_prefix

.begin      literal_prefix .KernelExceptionHandler
.section    .KernelExceptionHandler.text, "ax"
.global     KernelExceptionHandler
.align      4
.literal_position
KernelExceptionHandler:

    wsr     a0, EXCSAVE1
    call0   KernelException

.end        literal_prefix

.section .iram,"ax"
.align      4
KernelException:

    movi    a0,5
    wsr     a0,EXCCAUSE
    call0   UserException
    rfe

.begin      literal_prefix .UserExceptionHandler
.section    .UserExceptionHandler.text, "ax"
.global     UserExceptionHandler
.type       UserExceptionHandler,@function
.align      4
.literal_position
UserExceptionHandler:

    wsr     a0, EXCSAVE1
    call0   UserException

.end        literal_prefix

.section    .iram,"ax"
.type       UserException,@function
.align      4
UserException:

    rsr     a0, EXCCAUSE
    beqi    a0, 4, InterruptEntry1
    call0   HandleEntry
1:
    j 1b

.section    .iram,"ax"
.type       InterruptEntry1,@function
.align      4
InterruptEntry1:

    mov     a0, sp
    addi    sp, sp, -CONTEXT_SIZE

    PUSH_ALL_REG a1

    s32i    a0, sp, CONTEXT_OFF_A1
    rsr     a0, EPC1
    s32i    a0, sp, CONTEXT_OFF_PC
    rsr     a0, EXCSAVE1
    s32i    a0, sp, CONTEXT_OFF_A0

    wsr     a1, EXCSAVE1
    l32i    a1, a1, CONTEXT_OFF_A1
    call0   SpillWindow
    rsr     a1, EXCSAVE1
    
    movi    a0, LEVEL1_INT_MASK | USER_VECTOR_MODE | WOE_ENABLE
    wsr     a0, PS
    rsync

    call4   HalInterrupt
    mov     a2, a1
    POP_ALL_REG a2 EPC1 PS
    rfe    

.begin      literal_prefix .InterruptEntry2
.section    .InterruptEntry2.text, "ax"
.global     InterruptEntry2
.type       InterruptEntry2,@function
.align      4
.literal_position
InterruptEntry2:

    wsr     a0, EXCSAVE2
    movi    a4, LEVEL2
    call0   HandleEntry

.end        literal_prefix

.begin      literal_prefix .InterruptEntry3
.section    .InterruptEntry3.text, "ax"
.global     InterruptEntry3
.type       InterruptEntry3,@function
.align      4
.literal_position
InterruptEntry3:

    wsr     a0, EXCSAVE3
    movi    a4, LEVEL3
    call0   HandleEntry

.end        literal_prefix

.begin      literal_prefix .InterruptEntry4
.section    .InterruptEntry4.text, "ax"
.global     InterruptEntry4
.type       InterruptEntry4,@function
.align      4
.literal_position
InterruptEntry4:

    wsr     a0, EXCSAVE4
    movi    a4, LEVEL4
    call0   HandleEntry

.end        literal_prefix

.begin      literal_prefix .InterruptEntry5
.section    .InterruptEntry5.text, "ax"
.global     InterruptEntry5
.type       InterruptEntry5,@function
.align      4
.literal_position
InterruptEntry5:

    wsr     a0, EXCSAVE5
    movi    a4, LEVEL5
    call0   HandleEntry

.end        literal_prefix

.section .WindowVectors.text, "ax"

.org    0x0
.global OverFlowGroup1
OverFlowGroup1:

    addi    a5, a5, -16
    s32i    a3, a5, 12
    s32i    a2, a5, 8
    s32i    a1, a5, 4
    s32i    a0, a5, 0
    addi    a5, a5, 16
    rfwo

.org    0x40
.global UnderFlowGroup1
UnderFlowGroup1:

    addi    a5, a5, -16
    l32i    a3, a5, 12
    l32i    a2, a5, 8
    l32i    a1, a5, 4
    l32i    a0, a5, 0
    addi    a5, a5, 16
    rfwu

.org    0x80
.global OverFlowGroup2
OverFlowGroup2:

    addi    a9, a9, -16
    s32i    a3, a9, 12
    s32i    a2, a9, 8
    s32i    a1, a9, 4
    s32i    a0, a9, 0
    addi    a9, a9, 16

    addi    a1, a1, -12
    l32i    a0, a1, 0
    addi    a1, a1, 12

    addi    a0, a0, -32

    s32i    a7, a0, 12
    s32i    a6, a0, 8
    s32i    a5, a0, 4
    s32i    a4, a0, 0

    rfwo

.org    0xC0
.global UnderFlowGroup2
UnderFlowGroup2:

    addi  a9, a9, -16
    l32i  a3, a9, 12
    l32i  a2, a9, 8
    l32i  a1, a9, 4
    l32i  a0, a9, 0
    addi  a9, a9, 16

    addi  a1, a1, -12
    l32i  a4, a1, 0
    addi  a1, a1, 12

    addi    a4, a4, -32
    l32i    a7, a4, 12
    l32i    a6, a4, 8
    l32i    a5, a4, 4
    l32i    a4, a4, 0
    rfwu

.org    0x100
.global OverFlowGroup3
OverFlowGroup3:

    addi    a13, a13, -16
    s32i    a3, a13, 12
    s32i    a2, a13, 8
    s32i    a1, a13, 4
    s32i    a0, a13, 0
    addi    a13, a13, 16

    addi    a1, a1, -12
    l32i    a0, a1, 0
    addi    a1, a1, 12

    addi    a0, a0, -48

    s32i    a11, a0, 28
    s32i    a10, a0, 24
    s32i    a9, a0, 20
    s32i    a8, a0, 16
    s32i    a7, a0, 12
    s32i    a6, a0, 8
    s32i    a5, a0, 4
    s32i    a4, a0, 0
    rfwo

.org 0x140
.global UnderFlowGroup3
UnderFlowGroup3:

    addi  a13, a13, -16
    l32i  a3, a13, 12
    l32i  a2, a13, 8
    l32i  a1, a13, 4
    l32i  a0, a13, 0
    addi  a13, a13, 16

    addi  a1, a1, -12
    l32i  a4, a1, 0
    addi  a1, a1, 12

    addi    a4, a4, -48
    l32i    a11, a0, 28
    l32i    a10, a0, 24
    l32i    a9, a0, 20
    l32i    a8, a0, 16
    l32i    a7, a4, 12
    l32i    a6, a4, 8
    l32i    a5, a4, 4
    l32i    a4, a4, 0
    rfwu

.global EnableExceptionInterface
.section    .iram,"ax"
.type       EnableExceptionInterface,@function
.align      4
EnableExceptionInterface:
    entry   a1,16
    retw
