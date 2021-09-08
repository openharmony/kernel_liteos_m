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
#include "los_context.h"
#include "los_sched.h"
#include "los_debug.h"

#define OVERFLOW_MAX      0xFFFFFFFF

UINT32 GetCcount(VOID)
{
    UINT32 intSave;

    __asm__ __volatile__("rsr %0, ccount" : "=a"(intSave) :);

    return intSave;
}

VOID ResetCcount(VOID)
{
    __asm__ __volatile__("wsr %0, ccount; rsync" : :"a"(0));
}

UINT32 GetCcompare(VOID)
{
    UINT32 intSave;

    __asm__ __volatile__("rsr %0, ccompare0" : "=a"(intSave) :);

    return intSave;
}

VOID SetCcompare(UINT32 newCompareVal)
{
    __asm__ __volatile__("wsr %0, ccompare0; rsync" : : "a"(newCompareVal));
}

VOID HalUpdateTimerCmpVal(UINT32 newCompareVal)
{
    SetCcompare(newCompareVal);
    ResetCcount();
}

/* ****************************************************************************
Function    : HalTickStart
Description : Configure Tick Interrupt Start
Input       : none
output      : none
return      : LOS_OK - Success , or LOS_ERRNO_TICK_CFG_INVALID - failed
**************************************************************************** */
WEAK UINT32 HalTickStart(OS_TICK_HANDLER handler)
{
    UINT32 ret;
    UINT32 ccount;
    UINT32 nextTickCycles;

    if ((OS_SYS_CLOCK == 0) ||
        (LOSCFG_BASE_CORE_TICK_PER_SECOND == 0) ||
        (LOSCFG_BASE_CORE_TICK_PER_SECOND > OS_SYS_CLOCK)) {
        return LOS_ERRNO_TICK_CFG_INVALID;
    }

#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
#if (OS_HWI_WITH_ARG == 1)
    OsSetVector(OS_TICK_INT_NUM, (HWI_PROC_FUNC)handler, NULL);
#else
    OsSetVector(OS_TICK_INT_NUM, (HWI_PROC_FUNC)handler);
#endif
#endif

    g_sysClock = OS_SYS_CLOCK;
    g_cyclesPerTick = OS_SYS_CLOCK / LOSCFG_BASE_CORE_TICK_PER_SECOND;

    SetCcompare(g_cyclesPerTick);
    ResetCcount();

    __asm__ __volatile__("wsr %0, ccompare1; rsync" : : "a"(0));
    __asm__ __volatile__("wsr %0, ccompare2; rsync" : : "a"(0));

    HalIrqUnmask(OS_TICK_INT_NUM);
    return LOS_OK;
}

WEAK VOID HalSysTickReload(UINT64 nextResponseTime)
{
    HalUpdateTimerCmpVal(nextResponseTime);
}

WEAK UINT64 HalGetTickCycle(UINT32 *period)
{
    UINT32 ccount;
    UINT32 intSave = LOS_IntLock();

    ccount = GetCcount();
    *period = g_cyclesPerTick;

    LOS_IntRestore(intSave);

    return ccount;
}

WEAK VOID HalTickLock(VOID)
{
    HalIrqMask(OS_TICK_INT_NUM);
}

WEAK VOID HalTickUnlock(VOID)
{
    HalIrqUnmask(OS_TICK_INT_NUM);
}

VOID Wfi(VOID)
{
    __asm__ volatile("waiti 0" : : : "memory");
}

VOID Dsb(VOID)
{
    __asm__ volatile("dsync" : : : "memory");
}

UINT32 HalEnterSleep(VOID)
{
    Dsb();
    Wfi();

    return LOS_OK;
}
