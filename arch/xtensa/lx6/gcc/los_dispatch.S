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

.section .text
.literal_position
.align      4
.type HalTaskContextSwitch, %function
.global HalTaskContextSwitch
HalTaskContextSwitch:

    entry   sp, 16
    addi    a2, sp, -CONTEXT_SIZE
    PUSH_ALL_REG a2
    call0   SaveRetAddr
    beqi    a3, 1, .switchdone
    movi    a4, g_losTask
    l32i    a5, a4, 0   /* get run task */
    s32i    a2, a5, 0   /* store sp */
    l32i    a5, a4, 4   /* get new task */
    s32i    a5, a4, 0   /* run task = new task */
    l32i    a4, a5, 0   /* get new sp */
    rsr     a5, PS
    movi    a3, ~(WOE_ENABLE | LEVEL_MASK)
    and     a2, a5, a3
    addi    a2, a2, 3
    wsr     a2, PS
    rsync
    call0   SpillWindow
    mov     a2, a4
    POP_ALL_REG a2 EPC5 EPS5
    rfi     5

.switchdone:
    retw

.type HakSpillWindow, %function
.global HakSpillWindow
HakSpillWindow:

    entry   sp, 32

    addi    a2, sp, -CONTEXT_SIZE
    PUSH_ALL_REG a2
    
    rsr     a5, PS
    movi    a3, ~(WOE_ENABLE | LEVEL_MASK)  /* disable woe and int */
    and     a3, a5, a3
    addi    a3, a3, LEVEL3
    wsr     a3, PS
    rsync

    mov     a4, a2
    call0   SpillWindow

    l32i    a5, a4, CONTEXT_OFF_PS /* restroe PS */
    wsr     a5, PS
    rsync
    l32i    a0, a4, CONTEXT_OFF_A0
    l32i    a1, a4, CONTEXT_OFF_A1

    retw

.type OsStartToRun, %function
.global OsStartToRun
OsStartToRun:

    movi    a2, g_losTask
    l32i    a3, a2, 4   /* get new task */
    l32i    sp, a3, 0   /* get sp */

    rsr     a4, PS
    movi    a3, ~(WOE_ENABLE | LEVEL_MASK)
    and     a2, a4, a3
    addi    a2, a2, LEVEL3
    wsr     a2, PS
    rsync

    mov     a5, a1
    l32i    a1, a1, CONTEXT_OFF_A1
    call0   SpillWindow

    mov     a2, a5
    POP_ALL_REG a2 EPC5 EPS5
    rfi     5

.global SaveRetAddr
.type   SaveRetAddr, @function
.literal_position
.align  4

SaveRetAddr:

    movi    a3, 1
    s32i    a3, a2, CONTEXT_OFF_A3
    s32i    a0, a2, CONTEXT_OFF_PC    /* save pc */
    movi    a3, 0
    ret
