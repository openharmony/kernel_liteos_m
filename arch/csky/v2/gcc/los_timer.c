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
#include "los_context.h"
#include "los_sched.h"
#include "los_debug.h"

typedef struct {
    UINT32 CTRL;
    UINT32 LOAD;
    UINT32 VAL;
    UINT32 CALIB;
} CORE_TIM_TYPE;

#define OS_CYCLE_PER_TICK    (OS_SYS_CLOCK / LOSCFG_BASE_CORE_TICK_PER_SECOND)

#define CORE_TIM_BASE        (0xE000E010UL)
#define SysTick              ((CORE_TIM_TYPE *)CORE_TIM_BASE)

#define CORETIM_ENABLE       (1UL << 0)
#define CORETIM_INTMASK      (1UL << 1)
#define CORETIM_SOURCE       (1UL << 2)
#define CORETIM_MODE         (1UL << 16)

#define TIM_INT_NUM          1

/* ****************************************************************************
Function    : HalTickStart
Description : Configure Tick Interrupt Start
Input       : none
output      : none
return      : LOS_OK - Success , or LOS_ERRNO_TICK_CFG_INVALID - failed
**************************************************************************** */
WEAK UINT32 HalTickStart(OS_TICK_HANDLER handler)
{
    if ((OS_SYS_CLOCK == 0) || (LOSCFG_BASE_CORE_TICK_PER_SECOND == 0) ||
        (LOSCFG_BASE_CORE_TICK_PER_SECOND > OS_SYS_CLOCK)) {
        return LOS_ERRNO_TICK_CFG_INVALID;
    }

    g_sysClock = OS_SYS_CLOCK;
    g_cyclesPerTick = OS_CYCLE_PER_TICK;
    SysTick->LOAD = (OS_CYCLE_PER_TICK - 1);
    SysTick->VAL = 0;
    SysTick->CTRL |= (CORETIM_SOURCE | CORETIM_ENABLE | CORETIM_INTMASK);

    VIC_REG->IWER[0] = 0x1 << TIM_INT_NUM;

#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
    OsSetVector(TIM_INT_NUM, (HWI_PROC_FUNC)handler, NULL);
#else
    OsSetVector(TIM_INT_NUM, (HWI_PROC_FUNC)handler);
#endif
#endif
    return LOS_OK;
}

WEAK VOID ArchSysTickReload(UINT64 nextResponseTime)
{
    SysTick->CTRL &= ~CORETIM_ENABLE;
    SysTick->LOAD = (UINT32)(nextResponseTime - 1UL); /* set reload register */
    SysTick->VAL = 0UL; /* Load the SysTick Counter Value */
    SysTick->CTRL |= CORETIM_ENABLE;
}

WEAK UINT64 ArchGetTickCycle(UINT32 *period)
{
    UINT32 hwCycle;
    UINT32 intSave = LOS_IntLock();
    *period = SysTick->LOAD;
    hwCycle = *period - SysTick->VAL;
    LOS_IntRestore(intSave);
    return (UINT64)hwCycle;
}

WEAK VOID ArchTickLock(VOID)
{
    SysTick->CTRL &= ~CORETIM_ENABLE;
}

WEAK VOID ArchTickUnlock(VOID)
{
    SysTick->CTRL |= CORETIM_ENABLE;
}

VOID Wfi(VOID)
{
    __asm__ volatile("wait");
}

VOID Dsb(VOID)
{
    __asm__ volatile("sync" : : : "memory");
}

UINT32 ArchEnterSleep(VOID)
{
    Dsb();
    Wfi();
    return LOS_OK;
}
