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

#include "riscv_hal.h"
#include "los_debug.h"
#include "soc.h"
#include "plic.h"
#include "mtimer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

VOID OsIrqDisable(UINT32 vector)
{
    if (vector <= RISCV_SYS_MAX_IRQ) {
        CLEAR_CSR(mie, 1 << vector);
    } else {
        PlicIrqDisable(vector);
    }
}

VOID OsIrqEnable(UINT32 vector)
{
    if (vector <= RISCV_SYS_MAX_IRQ) {
        SET_CSR(mie, 1 << vector);
    } else {
        PlicIrqEnable(vector);
    }
}

VOID OsSetLocalInterPri(UINT32 interPriNum, UINT16 prior)
{
    PlicIrqSetPrio(interPriNum, prior);
}

VOID OsGetCpuCycle(UINT32 *cntHi, UINT32 *cntLo)
{
    if ((cntHi == NULL) || (cntLo == NULL)) {
        return;
    }

    MTimerCpuCycle(cntHi, cntLo);
}

BOOL OsBackTraceFpCheck(UINT32 value)
{
    if (value >= (UINT32)(UINTPTR)(&__bss_end)) {
        return TRUE;
    }

    if ((value >= (UINT32)(UINTPTR)(&__start_and_irq_stack_top)) && (value < (UINT32)(UINTPTR)(&__except_stack_top))) {
        return TRUE;
    }

    return FALSE;
}

BOOL OsBackTraceRaCheck(UINT32 value)
{
    BOOL ret = FALSE;

    if ((value >= (UINT32)(UINTPTR)(&__text_start)) &&
        (value < (UINT32)(UINTPTR)(&__text_end))) {
        ret = TRUE;
    }

    return ret;
}

VOID SysClockInit(UINT32 period)
{
    UINT32 ret;
    ret = MTimerTickInit(period);
    if (ret != LOS_OK) {
        PRINT_ERR("Creat Mtimer failed! ret : 0x%x \n", ret);
        return;
    }

    PlicIrqInit();

    OsIrqEnable(RISCV_MACH_EXT_IRQ);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
