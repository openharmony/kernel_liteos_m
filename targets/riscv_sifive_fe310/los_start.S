/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#include "soc.h"

.macro PUSH_CALLER_REG
    addi  sp, sp, -(INT_SIZE_ON_STACK)
    SREG  t6, 16 * REGBYTES(sp)
    SREG  t5, 17 * REGBYTES(sp)
    SREG  t4, 18 * REGBYTES(sp)
    SREG  t3, 19 * REGBYTES(sp)
    SREG  a7, 20 * REGBYTES(sp)
    SREG  a6, 21 * REGBYTES(sp)
    SREG  a5, 22 * REGBYTES(sp)
    SREG  a4, 23 * REGBYTES(sp)
    SREG  a3, 24 * REGBYTES(sp)
    SREG  a2, 25 * REGBYTES(sp)
    SREG  a1, 26 * REGBYTES(sp)
    SREG  a0, 27 * REGBYTES(sp)
    SREG  t2, 28 * REGBYTES(sp)
    SREG  t1, 29 * REGBYTES(sp)
    SREG  t0, 30 * REGBYTES(sp)
    SREG  ra, 31 * REGBYTES(sp)
.endm

.extern memset
.global _start
.section .start.text
.align 4
_start:
    li    t0, RISCV_MSTATUS_MPP
    csrw  mstatus, t0
    csrw  mie, zero
    la    t0, TrapVector
    csrw  mtvec, t0      # direct mode

    .option push
    .option norelax
    la    gp, __global_pointer$
    .option pop

    la    t0, __bss_start
    la    t1, __bss_end
2:
    sw    zero, 0x0(t0)
    addi  t0, t0, 0x4
    bgtu  t1, t0, 2b

    la    sp, __start_and_irq_stack_top

    tail  main
