/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
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

#include "los_timer.h"
#include "los_config.h"
#include "los_tick.h"
#include "los_reg.h"
#include "los_arch_interrupt.h"
#include "los_arch_timer.h"
#include "riscv_hal.h"
#include "los_debug.h"

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler);
STATIC VOID SysTickReload(UINT64 nextResponseTime);
STATIC UINT64 SysTickCycleGet(UINT32 *period);
STATIC VOID SysTickLock(VOID);
STATIC VOID SysTickUnlock(VOID);

STATIC ArchTickTimer g_archTickTimer = {
    .freq = 0,
    .irqNum = RISCV_MACH_TIMER_IRQ,
    .init = SysTickStart,
    .getCycle = SysTickCycleGet,
    .reload = SysTickReload,
    .lock = SysTickLock,
    .unlock = SysTickUnlock,
    .tickHandler = NULL,
};

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler)
{
    ArchTickTimer *tick = &g_archTickTimer;

    UINT32 period = (UINT32)LOSCFG_BASE_CORE_TICK_RESPONSE_MAX;
    UINT32 ret = LOS_HwiCreate(RISCV_MACH_TIMER_IRQ, 0x1, 0, handler, period);
    if (ret != LOS_OK) {
        return ret;
    }

    tick->freq = OS_SYS_CLOCK;

    WRITE_UINT32(0xffffffff, MTIMERCMP + 4); /* The high 4 bits of mtimer */
    WRITE_UINT32(period, MTIMERCMP);
    WRITE_UINT32(0x0, MTIMERCMP + 4); /* The high 4 bits of mtimer */

    HalIrqEnable(RISCV_MACH_TIMER_IRQ);
    return LOS_OK;
}

STATIC VOID SysTickReload(UINT64 nextResponseTime)
{
    UINT64 timeMax = (UINT64)LOSCFG_BASE_CORE_TICK_RESPONSE_MAX - 1;
    UINT64 timer;
    UINT32 timerL, timerH;
    READ_UINT32(timerL, MTIMER);
    READ_UINT32(timerH, MTIMER + MTIMER_HI_OFFSET);
    timer = OS_COMBINED_64(timerH, timerL);
    if ((timeMax - nextResponseTime) > timer) {
        timer += nextResponseTime;
    } else {
        timer = timeMax;
    }

    HalIrqDisable(RISCV_MACH_TIMER_IRQ);
    WRITE_UINT32(0xffffffff, MTIMERCMP + MTIMER_HI_OFFSET);
    WRITE_UINT32((UINT32)timer, MTIMERCMP);
    WRITE_UINT32((UINT32)(timer >> SHIFT_32_BIT), MTIMERCMP + MTIMER_HI_OFFSET);
    HalIrqEnable(RISCV_MACH_TIMER_IRQ);
}

STATIC UINT64 SysTickCycleGet(UINT32 *period)
{
    (VOID)period;
    UINT32 timerL, timerH;

    READ_UINT32(timerL, MTIMER);
    READ_UINT32(timerH, MTIMER + MTIMER_HI_OFFSET);
    return OS_COMBINED_64(timerH, timerL);
}

STATIC VOID SysTickLock(VOID)
{
    HalIrqDisable(RISCV_MACH_TIMER_IRQ);
}

STATIC VOID SysTickUnlock(VOID)
{
    HalIrqEnable(RISCV_MACH_TIMER_IRQ);
}

ArchTickTimer *ArchSysTickTimerGet(VOID)
{
    return &g_archTickTimer;
}

UINT32 ArchEnterSleep(VOID)
{
    wfi();

    return LOS_OK;
}

