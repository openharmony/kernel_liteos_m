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

/**
 * @defgroup Special Register Fields and Values
 * @ingroup kernel
 */

#ifndef _ARCH_REGS_H
#define _ARCH_REGS_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* PS register -- imprecise exception */
#define SPREG_PS_DEPC_SHIFT                                  4
#define SPREG_PS_DEPC_MASK                                   0x00000004
#define SPREG_PS_DEPC                                        SPREG_PS_DEPC_MASK
/* PS register -- interrupt part */
#define SPREG_PS_DI_SHIFT                                    3
#define SPREG_PS_DI_MASK                                     0x0000000F
#define SPREG_PS_DI                                          SPREG_PS_DI_MASK
#define SPREG_PS_DI_DEPC                                     0x0000000C
/* PS register -- stack part */
#define SPREG_PS_STACK_SHIFT                                 5
#define SPREG_PS_STACK_MASK                                  0x000000E0
#define SPREG_PS_STACK_INTERRUPT                             0x00000000
#define SPREG_PS_STACK_CROSS                                 0x00000020
#define SPREG_PS_STACK_IDLE                                  0x00000040
#define SPREG_PS_STACK_KERNEL                                0x00000060
#define SPREG_PS_STACK_PAGE                                  0x000000E0
#define SPREG_PS_STACK_FIRSTINT                              0x00000080
#define SPREG_PS_STACK_FIRSTKER                              0x000000A0
/* PS register -- entry no rotate */
#define SPREG_PS_ENTRYNR_SHIFT                               22
#define SPREG_PS_ENTRYNR_MASK                                0x00400000
#define SPREG_PS_ENTRYNR                                     SPREG_PS_ENTRYNR_MASK

/* Exccause Register  -- cause */
#define SPREG_EXCCAUSE_CAUSE_SHIFT                           0
#define SPREG_EXCCAUSE_CAUSE_BITS                            4
#define SPREG_EXCCAUSE_CAUSE_MASK                            0x0000000F

/**
 * @ingroup Execute level of core
 */
#define EXEC_LEVEL_APPLICATION_CODE                          2
#define EXEC_LEVEL_EXCEPTION_HANDLER                         3
#define EXEC_LEVEL_INTERRUPT_HANDLER                         4
#define EXEC_LEVEL_NON_INTERRUPTIBLE                         5

/**
 * @ingroup Schedule Flag stored on Task Context
 */
#define OS_SCHED_FLAG_TASKPREEMT                             4 /* Task Preemted through LOS_Schedule */

/**
 * @ingroup Context Fields Define
 */
#define CONTEXT_OFF_EPC                                      0
#define CONTEXT_OFF_PC                                       0 /* reuse with EPC */
#define CONTEXT_OFF_PS                                       4
#define CONTEXT_OFF_A0                                       8
#define CONTEXT_OFF_A1                                       12
#define CONTEXT_OFF_A2                                       16
#define CONTEXT_OFF_A3                                       20
#define CONTEXT_OFF_A4                                       24
#define CONTEXT_OFF_A5                                       28
#define CONTEXT_OFF_A6                                       32
#define CONTEXT_OFF_A7                                       36
#define CONTEXT_OFF_A8                                       40
#define CONTEXT_OFF_A9                                       44
#define CONTEXT_OFF_A10                                      48
#define CONTEXT_OFF_A11                                      52
#define CONTEXT_OFF_A12                                      56
#define CONTEXT_OFF_A13                                      60
#define CONTEXT_OFF_A14                                      64
#define CONTEXT_OFF_A15                                      68
#define CONTEXT_OFF_RESERVED                                 72
#define CONTEXT_OFF_SAR                                      72
#define CONTEXT_OFF_SCHED_FLAG                               76 /* reuse with exccause */
#define CONTEXT_OFF_EXCCAUSE                                 76
#define CONTEXT_OFF_EXCVADDR                                 80
#define CONTEXT_OFF_LBEG                                     84
#define CONTEXT_OFF_LEND                                     88
#define CONTEXT_OFF_LCOUNT                                   92

#if (defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U))
#define CONTEXT_OFF_TMP0                                     96
#define CONTEXT_OFF_CPENABLE                                 100
#define CONTEXT_OFF_CPSTORED                                 102
#define CONTEXT_OFF_FCR                                      104
#define CONTEXT_OFF_FSR                                      108
#define CONTEXT_OFF_F0                                       112
#define CONTEXT_OFF_F1                                       116
#define CONTEXT_OFF_F2                                       120
#define CONTEXT_OFF_F3                                       124
#define CONTEXT_OFF_F4                                       128
#define CONTEXT_OFF_F5                                       132
#define CONTEXT_OFF_F6                                       136
#define CONTEXT_OFF_F7                                       140
#define CONTEXT_OFF_F8                                       144
#define CONTEXT_OFF_F9                                       148
#define CONTEXT_OFF_F10                                      152
#define CONTEXT_OFF_F11                                      156
#define CONTEXT_OFF_F12                                      160
#define CONTEXT_OFF_F13                                      164
#define CONTEXT_OFF_F14                                      168
#define CONTEXT_OFF_F15                                      172
#define CONTEXT_SIZE                                         192
#else
#define CONTEXT_SIZE                                         112
#endif
#define EXCCAUSE_LEVEL1INTERRUPT                             4
#define XTENSA_LOGREG_NUM                                    16
#define INDEX_OF_SP                                          1
#define INDEX_OF_ARGS0                                       6

#define WINDOWSTARTBITS                                      16
#define WINDOWBASEBITS                                       4
#define WINDOWSTARTMASK                                      ((1 << WINDOWSTARTBITS) - 1)

#define WOE_ENABLE                                           0x40000
#define BIT_CALLINC                                          16
#define LEVEL_MASK                                           0xf
#define INT_MASK                                             5
#define LEVEL1_INT_MASK                                      1
#define USER_VECTOR_MODE                                     0x20
#define LEVEL1                                               1
#define LEVEL2                                               2
#define LEVEL3                                               3
#define LEVEL4                                               4
#define LEVEL5                                               5

#ifdef YES
#undef YES
#endif
#define YES                                                  1

#ifdef NO
#undef NO
#endif
#define NO                                                   0

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _ARCH_REGS_H */
