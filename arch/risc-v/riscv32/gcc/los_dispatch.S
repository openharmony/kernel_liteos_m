/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
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

#include "soc_common.h"

.global  HalEnableIRQ
.global  HalDisableIRQ
.global  ArchIntLock
.global  ArchIntUnlock
.global  ArchIntRestore
.global  ArchStartToRun
.global  ArchTaskSchedule
.extern  __irq_stack_top
.section .interrupt.text, "ax"

.macro PUSH_ALL_REG
    addi sp, sp, -(32 * REGBYTES + FPU_SAVE_SIZE)
    SREG  t6, 2 * REGBYTES(sp)
    SREG  t5, 3 * REGBYTES(sp)
    SREG  t4, 4 * REGBYTES(sp)
    SREG  t3, 5 * REGBYTES(sp)
    SREG  t2, 6 * REGBYTES(sp)
    SREG  t1, 7 * REGBYTES(sp)
    SREG  t0, 8 * REGBYTES(sp)
    SREG  s11, 9 * REGBYTES(sp)
    SREG  s10, 10 * REGBYTES(sp)
    SREG  s9, 11 * REGBYTES(sp)
    SREG  s8, 12 * REGBYTES(sp)
    SREG  s7, 13 * REGBYTES(sp)
    SREG  s6, 14 * REGBYTES(sp)
    SREG  s5, 15 * REGBYTES(sp)
    SREG  a7, 18 * REGBYTES(sp)
    SREG  a6, 19 * REGBYTES(sp)
    SREG  a5, 20 * REGBYTES(sp)
    SREG  a4, 21 * REGBYTES(sp)
    SREG  a3, 22 * REGBYTES(sp)
    SREG  a2, 23 * REGBYTES(sp)
    SREG  a1, 24 * REGBYTES(sp)
    SREG  a0, 25 * REGBYTES(sp)
    SREG  s4, 26 * REGBYTES(sp)
    SREG  s3, 27 * REGBYTES(sp)
    SREG  s2, 28 * REGBYTES(sp)
    SREG  s1, 29 * REGBYTES(sp)
    SREG  s0, 30 * REGBYTES(sp)
    SREG  ra, 31 * REGBYTES(sp)
#ifndef LOSCFG_ARCH_FPU_DISABLE
    /* Save FPU caller-saved registers (ft0-ft11, fa0-fa7) */
    FSREG  ft11, (32 * REGBYTES + 0  * FPUREGBYTES)(sp)
    FSREG  ft10, (32 * REGBYTES + 1  * FPUREGBYTES)(sp)
    FSREG  ft9,  (32 * REGBYTES + 2  * FPUREGBYTES)(sp)
    FSREG  ft8,  (32 * REGBYTES + 3  * FPUREGBYTES)(sp)
    FSREG  fa7,  (32 * REGBYTES + 4  * FPUREGBYTES)(sp)
    FSREG  fa6,  (32 * REGBYTES + 5  * FPUREGBYTES)(sp)
    FSREG  fa5,  (32 * REGBYTES + 6  * FPUREGBYTES)(sp)
    FSREG  fa4,  (32 * REGBYTES + 7  * FPUREGBYTES)(sp)
    FSREG  fa3,  (32 * REGBYTES + 8  * FPUREGBYTES)(sp)
    FSREG  fa2,  (32 * REGBYTES + 9  * FPUREGBYTES)(sp)
    FSREG  fa1,  (32 * REGBYTES + 10 * FPUREGBYTES)(sp)
    FSREG  fa0,  (32 * REGBYTES + 11 * FPUREGBYTES)(sp)
    FSREG  ft7,  (32 * REGBYTES + 12 * FPUREGBYTES)(sp)
    FSREG  ft6,  (32 * REGBYTES + 13 * FPUREGBYTES)(sp)
    FSREG  ft5,  (32 * REGBYTES + 14 * FPUREGBYTES)(sp)
    FSREG  ft4,  (32 * REGBYTES + 15 * FPUREGBYTES)(sp)
    FSREG  ft3,  (32 * REGBYTES + 16 * FPUREGBYTES)(sp)
    FSREG  ft2,  (32 * REGBYTES + 17 * FPUREGBYTES)(sp)
    FSREG  ft1,  (32 * REGBYTES + 18 * FPUREGBYTES)(sp)
    FSREG  ft0,  (32 * REGBYTES + 19 * FPUREGBYTES)(sp)
    /* Save FPU callee-saved registers (fs0-fs11) */
    FSREG  fs11, (32 * REGBYTES + FPU_CALLER_REG_SIZE + 0  * FPUREGBYTES)(sp)
    FSREG  fs10, (32 * REGBYTES + FPU_CALLER_REG_SIZE + 1  * FPUREGBYTES)(sp)
    FSREG  fs9,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 2  * FPUREGBYTES)(sp)
    FSREG  fs8,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 3  * FPUREGBYTES)(sp)
    FSREG  fs7,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 4  * FPUREGBYTES)(sp)
    FSREG  fs6,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 5  * FPUREGBYTES)(sp)
    FSREG  fs5,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 6  * FPUREGBYTES)(sp)
    FSREG  fs4,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 7  * FPUREGBYTES)(sp)
    FSREG  fs3,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 8  * FPUREGBYTES)(sp)
    FSREG  fs2,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 9  * FPUREGBYTES)(sp)
    FSREG  fs1,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 10 * FPUREGBYTES)(sp)
    FSREG  fs0,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 11 * FPUREGBYTES)(sp)
    /* Save FCSR (t0 already saved above, safe to clobber) */
    frcsr  t0
    SREG   t0, FCSR_STACK_OFFSET(sp)
#endif
.endm

.macro POP_ALL_REG
#ifndef LOSCFG_ARCH_FPU_DISABLE
    /* Restore FCSR first so subsequent FLREG use the right rounding mode */
    LREG   t0, FCSR_STACK_OFFSET(sp)
    fscsr  t0
    /* Restore FPU callee-saved registers (fs0-fs11) */
    FLREG  fs11, (32 * REGBYTES + FPU_CALLER_REG_SIZE + 0  * FPUREGBYTES)(sp)
    FLREG  fs10, (32 * REGBYTES + FPU_CALLER_REG_SIZE + 1  * FPUREGBYTES)(sp)
    FLREG  fs9,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 2  * FPUREGBYTES)(sp)
    FLREG  fs8,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 3  * FPUREGBYTES)(sp)
    FLREG  fs7,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 4  * FPUREGBYTES)(sp)
    FLREG  fs6,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 5  * FPUREGBYTES)(sp)
    FLREG  fs5,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 6  * FPUREGBYTES)(sp)
    FLREG  fs4,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 7  * FPUREGBYTES)(sp)
    FLREG  fs3,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 8  * FPUREGBYTES)(sp)
    FLREG  fs2,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 9  * FPUREGBYTES)(sp)
    FLREG  fs1,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 10 * FPUREGBYTES)(sp)
    FLREG  fs0,  (32 * REGBYTES + FPU_CALLER_REG_SIZE + 11 * FPUREGBYTES)(sp)
    /* Restore FPU caller-saved registers (ft0-ft11, fa0-fa7) */
    FLREG  ft11, (32 * REGBYTES + 0  * FPUREGBYTES)(sp)
    FLREG  ft10, (32 * REGBYTES + 1  * FPUREGBYTES)(sp)
    FLREG  ft9,  (32 * REGBYTES + 2  * FPUREGBYTES)(sp)
    FLREG  ft8,  (32 * REGBYTES + 3  * FPUREGBYTES)(sp)
    FLREG  fa7,  (32 * REGBYTES + 4  * FPUREGBYTES)(sp)
    FLREG  fa6,  (32 * REGBYTES + 5  * FPUREGBYTES)(sp)
    FLREG  fa5,  (32 * REGBYTES + 6  * FPUREGBYTES)(sp)
    FLREG  fa4,  (32 * REGBYTES + 7  * FPUREGBYTES)(sp)
    FLREG  fa3,  (32 * REGBYTES + 8  * FPUREGBYTES)(sp)
    FLREG  fa2,  (32 * REGBYTES + 9  * FPUREGBYTES)(sp)
    FLREG  fa1,  (32 * REGBYTES + 10 * FPUREGBYTES)(sp)
    FLREG  fa0,  (32 * REGBYTES + 11 * FPUREGBYTES)(sp)
    FLREG  ft7,  (32 * REGBYTES + 12 * FPUREGBYTES)(sp)
    FLREG  ft6,  (32 * REGBYTES + 13 * FPUREGBYTES)(sp)
    FLREG  ft5,  (32 * REGBYTES + 14 * FPUREGBYTES)(sp)
    FLREG  ft4,  (32 * REGBYTES + 15 * FPUREGBYTES)(sp)
    FLREG  ft3,  (32 * REGBYTES + 16 * FPUREGBYTES)(sp)
    FLREG  ft2,  (32 * REGBYTES + 17 * FPUREGBYTES)(sp)
    FLREG  ft1,  (32 * REGBYTES + 18 * FPUREGBYTES)(sp)
    FLREG  ft0,  (32 * REGBYTES + 19 * FPUREGBYTES)(sp)
#endif
    LREG  t6, 2 * REGBYTES(sp)
    LREG  t5, 3 * REGBYTES(sp)
    LREG  t4, 4 * REGBYTES(sp)
    LREG  t3, 5 * REGBYTES(sp)
    LREG  t2, 6 * REGBYTES(sp)
    LREG  t1, 7 * REGBYTES(sp)
    LREG  t0, 8 * REGBYTES(sp)
    LREG  s11, 9 * REGBYTES(sp)
    LREG  s10, 10 * REGBYTES(sp)
    LREG  s9, 11 * REGBYTES(sp)
    LREG  s8, 12 * REGBYTES(sp)
    LREG  s7, 13 * REGBYTES(sp)
    LREG  s6, 14 * REGBYTES(sp)
    LREG  s5, 15 * REGBYTES(sp)
    LREG  a7, 18 * REGBYTES(sp)
    LREG  a6, 19 * REGBYTES(sp)
    LREG  a5, 20 * REGBYTES(sp)
    LREG  a4, 21 * REGBYTES(sp)
    LREG  a3, 22 * REGBYTES(sp)
    LREG  a2, 23 * REGBYTES(sp)
    LREG  a1, 24 * REGBYTES(sp)
    LREG  a0, 25 * REGBYTES(sp)
    LREG  s4, 26 * REGBYTES(sp)
    LREG  s3, 27 * REGBYTES(sp)
    LREG  s2, 28 * REGBYTES(sp)
    LREG  s1, 29 * REGBYTES(sp)
    LREG  s0, 30 * REGBYTES(sp)
    LREG  ra, 31 * REGBYTES(sp)
    addi  sp, sp, 32 * REGBYTES + FPU_SAVE_SIZE
.endm

ArchTaskSchedule:
    PUSH_ALL_REG

    // clear mpie
    csrr t0, mstatus
    li   t2, RISCV_MSTATUS_MPIE
    not  t2, t2
    and  t0, t0, t2

    // get mie
    andi t1, t0, RISCV_MSTATUS_MIE

    // must be in machine mode
    ori  t1, t1, 0x180
    slli t1, t1, 0x4
    or   t0, t0, t1

    // clear mie
    li   t2, RISCV_MSTATUS_MIE
    not  t2, t2
    and  t0, t0, t2

#ifndef LOSCFG_ARCH_FPU_DISABLE
    // keep FPU enabled (mstatus.FS = Dirty) so the task can use FP on resume
    li   t2, RISCV_MSTATUS_FS
    or   t0, t0, t2
#endif

    SREG t0, 16 * REGBYTES(sp)

    SREG ra, 17 * REGBYTES(sp)

    // a0=newTask, a1=runTask
    sw   sp, TASK_CB_KERNEL_SP(a1)

    .type ArchStartToRun, %function
    .global ArchStartToRun
ArchStartToRun:
    lw      sp, TASK_CB_KERNEL_SP(a0)

    lw      t0, 16 * REGBYTES(sp)
    csrw    mstatus, t0

    lw      t0, 17 * REGBYTES(sp)
    csrw    mepc, t0

    POP_ALL_REG

    mret

.section .text, "ax"
HalDisableIRQ:
    li      t0, (RISCV_MSTATUS_MPIE | RISCV_MSTATUS_MIE) // mpie | mie
    csrrc   zero, mstatus, t0
    ret

HalEnableIRQ:
    csrsi  mstatus, RISCV_MSTATUS_MIE
    ret

ArchIntLock:
    csrr    a0, mstatus           // return value
    li      t0, RISCV_MSTATUS_MIE   // mie
    csrrc   zero, mstatus, t0
    ret

ArchIntUnlock:
    csrr    a0, mstatus           // return value
    li      t0, RISCV_MSTATUS_MIE   // mie
    csrrs   zero, mstatus, t0
    ret

ArchIntRestore:
    csrw mstatus, a0
    ret
