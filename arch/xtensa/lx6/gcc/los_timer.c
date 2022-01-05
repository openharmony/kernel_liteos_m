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

#include "los_timer.h"
#include "los_config.h"
#include "los_tick.h"
#include "los_arch_interrupt.h"
#include "los_arch_timer.h"
#include "los_debug.h"

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler);
STATIC VOID SysTickReload(UINT64 nextResponseTime);
STATIC UINT64 SysTickCycleGet(UINT32 *period);
STATIC VOID SysTickLock(VOID);
STATIC VOID SysTickUnlock(VOID);

STATIC ArchTickTimer g_archTickTimer = {
    .freq = 0,
    .irqNum = OS_TICK_INT_NUM,
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

    tick->freq = OS_SYS_CLOCK;

#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
    OsSetVector(tick->irqNum, handler, NULL);
#else
    OsSetVector(tick->irqNum, handler);
#endif
#endif

    ResetCcount();
    SetCcompare(LOSCFG_BASE_CORE_TICK_RESPONSE_MAX);

    __asm__ __volatile__("wsr %0, ccompare1; rsync" : : "a"(0));
    __asm__ __volatile__("wsr %0, ccompare2; rsync" : : "a"(0));

    HalIrqUnmask(tick->irqNum);
    return LOS_OK;
}

STATIC VOID SysTickReload(UINT64 nextResponseTime)
{
    UINT32 timerL;
    timerL = GetCcount();
    timerL += nextResponseTime;
    SetCcompare(timerL);
}

STATIC UINT64 SysTickCycleGet(UINT32 *period)
{
    UINT32 tickCycleH;
    UINT32 tickCycleL;
    UINT32 temp;
    static UINT64 tickCycle = 0;

    (VOID)period;
    UINT32 intSave = LOS_IntLock();
    temp = tickCycle & 0xFFFFFFFF;
    tickCycleH = tickCycle >> SHIFT_32_BIT;
    tickCycleL = GetCcount();
    if (tickCycleL < temp) {
        tickCycleH++;
    }
    tickCycle = (((UINT64)tickCycleH) << SHIFT_32_BIT) | tickCycleL;

    LOS_IntRestore(intSave);

    return tickCycle;
}

STATIC VOID SysTickLock(VOID)
{
    HalIrqMask(OS_TICK_INT_NUM);
}

STATIC VOID SysTickUnlock(VOID)
{
    HalIrqUnmask(OS_TICK_INT_NUM);
}

ArchTickTimer *ArchSysTickTimerGet(VOID)
{
    return &g_archTickTimer;
}

VOID Wfi(VOID)
{
    __asm__ volatile("waiti 0" : : : "memory");
}

VOID Dsb(VOID)
{
    __asm__ volatile("dsync" : : : "memory");
}

UINT32 ArchEnterSleep(VOID)
{
    Dsb();
    Wfi();

    return LOS_OK;
}
