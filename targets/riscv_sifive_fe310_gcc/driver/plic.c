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

#include "plic.h"
#include "soc.h"
#include "los_reg.h"
#include "los_interrupt.h"
#include "los_debug.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static VOID OsMachineExternalInterrupt(UINT32 arg)
{
    volatile UINT32 *plicReg = (volatile UINT32 *)(PLIC_REG_BASE + 0x4);
    UINT32 irqNum, saveIrqNum;

    READ_UINT32(irqNum, plicReg);
    saveIrqNum = irqNum;

    if ((irqNum >= OS_RISCV_CUSTOM_IRQ_VECTOR_CNT) || (irqNum == 0)) {
        OsHwiDefaultHandler(irqNum);
    }

    irqNum += RISCV_SYS_MAX_IRQ;

    g_hwiForm[irqNum].pfnHook(g_hwiForm[irqNum].uwParam);

    WRITE_UINT32(saveIrqNum, plicReg);
}

VOID PlicIrqInit()
{
    volatile UINT32 *plicPrioReg = (volatile UINT32 *)PLIC_PRIO_BASE;
    volatile UINT32 *plicEnReg = (volatile UINT32 *)PLIC_ENABLE_BASE;
    volatile UINT32 *plicReg = (volatile UINT32 *)PLIC_REG_BASE;
    INT32 i;
    UINT32 ret;

    for (i = 0; i < ((OS_RISCV_CUSTOM_IRQ_VECTOR_CNT >> 5) + 1); i++) {
        WRITE_UINT32(0x0, plicEnReg);
        plicEnReg++;
    }

    for (i = 0; i < OS_RISCV_CUSTOM_IRQ_VECTOR_CNT; i++) {
        WRITE_UINT32(0x0, plicPrioReg);
        plicPrioReg++;
    }

    WRITE_UINT32(0, plicReg);

    ret = LOS_HwiCreate(RISCV_MACH_EXT_IRQ, 0x1, 0, OsMachineExternalInterrupt, 0);
    if (ret != LOS_OK) {
        PRINT_ERR("Creat machine external failed! ret : 0x%x\n", ret);
    }
}

VOID PlicIrqSetPrio(UINT32 vector, UINT32 pri)
{
    volatile UINT32 *plicReg = (volatile UINT32 *)PLIC_PRIO_BASE;

    plicReg += (vector - RISCV_SYS_MAX_IRQ);
    WRITE_UINT32(pri, plicReg);
}

VOID PlicIrqEnable(UINT32 vector)
{
    UINT32 irqValue;
    UINT32 locIrq = vector - RISCV_SYS_MAX_IRQ;
    volatile UINT32 *plicReg = (volatile UINT32 *)PLIC_ENABLE_BASE;

    plicReg += (locIrq >> 5);
    READ_UINT32(irqValue, plicReg);
    irqValue |= (1 << (locIrq & 31));
    WRITE_UINT32(irqValue, plicReg);
}

VOID PlicIrqDisable(UINT32 vector)
{
    UINT32 irqValue;
    UINT32 locIrq = vector - RISCV_SYS_MAX_IRQ;
    volatile UINT32 *plicReg = (volatile UINT32 *)PLIC_ENABLE_BASE;

    plicReg += (locIrq >> 5);
    READ_UINT32(irqValue, plicReg);
    irqValue &= ~(1 << (locIrq & 31));
    WRITE_UINT32(irqValue, plicReg);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

