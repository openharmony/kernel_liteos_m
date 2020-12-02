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
#ifndef _SOC_H
#define _SOC_H
#include "soc_common.h"

#define MSIP                                          0x2000000
#define MTIMERCMP                                     0x2004000
#define MTIMER                                        0x200BFF8
#define CLOCK_CONTRAL_REG                             0x10008000

/* interrupt base addr : 0xc000000 + 4 * interrupt ID
 * [2:0]   priority
 * [31:3]  reserved
 */
#define PLIC_PRIO_BASE                                 0xC000000
#define PLIC_PEND_BASE                                 0xC001000 //interrupt 0-31
#define PLIC_PEND_REG2                                 0xC001004 //interrupt 32-52
#define PLIC_ENABLE_BASE                               0xC002000 //interrupt 0-31
#define PLIC_ENABLE_REG2                               0xC002004 //interrupt 32-52
#define PLIC_REG_BASE                                  0xC200000

#define UART0_BASE                                     0x10013000
#define UART1_BASE                                     0x10023000

#define UART0_CLK_FREQ                                 0x32000000
#define UART1_CLK_FREQ                                 0x32000000
#define UART0_BAUDRAT                                  115200

#define RISCV_SYS_MAX_IRQ                              11
#define RISCV_WDOGCMP_IRQ                              (RISCV_SYS_MAX_IRQ + 1)
#define RISCV_RTCCMP_IRQ                               (RISCV_SYS_MAX_IRQ + 2)
#define RISCV_UART0_IRQ                                (RISCV_SYS_MAX_IRQ + 3)
#define RISCV_UART1_IRQ                                (RISCV_SYS_MAX_IRQ + 4)

#define RISCV_PLIC_VECTOR_CNT                          53

#endif
