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

#include "mtimer.h"
#include "los_reg.h"
#include "los_interrupt.h"
#include "los_tick.h"
#include "riscv_hal.h"
#include "soc.h"
#include "plic.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

VOID MTimerCpuCycle(UINT32 *contHi, UINT32 *contLo)
{
    READ_UINT32(*contLo, MTIMER);
    READ_UINT32(*contHi, MTIMER + 4);
    return;
}

STATIC INLINE VOID UpdateMtimerCmp(UINT32 tick)
{
    unsigned long long timer;
    unsigned timerL, timerH;
    READ_UINT32(timerL, MTIMER);
    READ_UINT32(timerH, MTIMER + 4);
    timer = (UINT64)(((UINT64)timerH << 32) + timerL);
    timer += tick;
    WRITE_UINT32(0xffffffff, MTIMERCMP + 4);
    WRITE_UINT32((UINT32)timer, MTIMERCMP);
    WRITE_UINT32((UINT32)(timer >> 32), MTIMERCMP + 4);
}

STATIC VOID OsMachineTimerInterrupt(UINT32 sysCycle)
{
    OsTickHandler();
    UpdateMtimerCmp(sysCycle);
}

UINT32 MTimerTickInit(UINT32 period)
{
    unsigned int ret;
    ret = LOS_HwiCreate(RISCV_MACH_TIMER_IRQ, 0x1, 0, OsMachineTimerInterrupt, period);
    if (ret != LOS_OK) {
        return ret;
    }

    WRITE_UINT32(0xffffffff, MTIMERCMP + 4);
    WRITE_UINT32(period, MTIMERCMP);
    WRITE_UINT32(0x0, MTIMERCMP + 4);

    OsIrqEnable(RISCV_MACH_TIMER_IRQ);
    return LOS_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
