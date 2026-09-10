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

#ifndef _SOC_COMMON_H
#define _SOC_COMMON_H

#define LREG lw
#define SREG sw
#define REGBYTES 4

#define EXC_SIZE_ON_STACK  (36 * REGBYTES)
#define INT_SIZE_ON_STACK  (32 * REGBYTES)

/* ---- FPU support (enabled when LOSCFG_ARCH_FPU_DISABLE is not defined) ----
 * The ws63 chip (rv32imfc) has the single-precision F extension. FPU register
 * state (f0-f31 + fcsr) is saved/restored on top of the 32 integer slots in
 * the task/interrupt stack frame so all existing integer-reg offsets stay
 * unchanged.
 * Stack layout (sp-relative, growing down):  slots 0..31 = integer regs,
 * slots 32..51 = FPU caller regs (ft0-ft11, fa0-fa7),
 * slots 52..63 = FPU callee regs (fs0-fs11),
 * slots 64..67 = fcsr + 3 reserved.
 */
#ifndef LOSCFG_ARCH_FPU_DISABLE
#define FPUREGBYTES             4
#define FSREG                   fsw
#define FLREG                   flw
#define FPU_CALLER_REG_NUM      20  /* ft0-ft11, fa0-fa7 */
#define FPU_CALLEE_REG_NUM      12  /* fs0-fs11 */
#define FPU_STATUS_REG_NUM      4   /* fcsr, res[3] */
#define FPU_CALLER_REG_SIZE     (FPU_CALLER_REG_NUM * FPUREGBYTES)
#define FPU_CALLEE_REG_SIZE     (FPU_CALLEE_REG_NUM * FPUREGBYTES)
#define FPU_EXTENSION_REG_SIZE  (FPU_CALLER_REG_SIZE + FPU_CALLEE_REG_SIZE)
#define FPU_STATUS_REG_SIZE     (FPU_STATUS_REG_NUM * REGBYTES)
#define FPU_SAVE_SIZE           (FPU_EXTENSION_REG_SIZE + FPU_STATUS_REG_SIZE)
#define FCSR_STACK_OFFSET       (32 * REGBYTES + FPU_EXTENSION_REG_SIZE)
#define RISCV_MSTATUS_FS        0x00006000  /* FS=Dirty so FPU state is saved/restored */
#else
#define FPU_SAVE_SIZE           0
#endif

/* task TCB offset */
#define TASK_CB_KERNEL_SP       0x0
#define TASK_CB_STATUS          0x4

#define UINT32_CUT_MASK         0xFFFFFFFF
#define UINT8_CUT_MASK          0xFF
#define OS_MV_32_BIT            32

/************************ mstatus ************************/
#define RISCV_MSTATUS_UIE                   0x00000001
#define RISCV_MSTATUS_MIE                   0x00000008
#define RISCV_MSTATUS_UPIE                  0x00000010
#define RISCV_MSTATUS_MPIE                  0x00000080
#define RISCV_MSTATUS_MPP                   0x00001800

/************************ mie ***************************/
#define RISCV_MIE_USIE                      0x000000001
#define RISCV_MIE_MSIE                      0x000000008
#define RISCV_MIE_UTIE                      0x000000010
#define RISCV_MIE_MTIE                      0x000000080
#define RISCV_MIE_UEIE                      0x000000100
#define RISCV_MIE_MEIE                      0x000000800

/************************** mcause ***********************/
#ifndef MCAUSE_INT_ID_MASK
#define MCAUSE_INT_ID_MASK                  0x7FFFFFF
#endif
#define RISCV_MCAUSE_ECALL_U                8

#define RISCV_USER_SOFT_IRQ                 0
#define RISCV_MACH_SOFT_IRQ                 3
#define RISCV_USER_TIMER_IRQ                4
#define RISCV_MACH_TIMER_IRQ                7
#define RISCV_USER_EXT_IRQ                  8
#define RISCV_MACH_EXT_IRQ                  11

#define READ_CSR(reg) ({                                          \
    UINT32 _tmp;                                                  \
    __asm__ volatile("csrr %0, " #reg : "=r"(_tmp) : : "memory"); \
    _tmp;                                                         \
})

#define WRITE_CSR(reg, val) ({                                    \
    __asm__ volatile("csrw " #reg ", %0" : : "r"(val) : "memory"); \
})

#define SET_CSR(reg, val) ({                                       \
    __asm__ volatile("csrs " #reg ", %0" : : "r"(val) : "memory"); \
})

#define CLEAR_CSR(reg, val) ({                                     \
    __asm__ volatile("csrc " #reg ", %0" : : "r"(val) : "memory"); \
})

#define READ_CUSTOM_CSR(reg) ({                                         \
    UINT32 _tmp;                                                        \
    __asm__ volatile("csrr %0, %1" : "=r"(_tmp) : "i"(reg) : "memory"); \
    _tmp;                                                               \
})

#define WRITE_CUSTOM_CSR(reg, val) ({                             \
    __asm__ volatile("csrw %0, %1" : : "i"(reg), "r"(val) : "memory"); \
})

#define SET_CUSTOM_CSR(reg, val) ({                                \
    __asm__ volatile("csrs " #reg ", %0" : : "r"(val) : "memory"); \
})

#define CLEAR_CUSTOM_CSR(reg, val) ({                              \
    __asm__ volatile("csrc " #reg ", %0" : : "r"(val) : "memory"); \
})

#endif
