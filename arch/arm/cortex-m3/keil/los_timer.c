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
#include "ARMCM3.h"
#include "los_tick.h"
#include "los_arch_interrupt.h"
#include "los_context.h"
#include "los_sched.h"
#include "los_debug.h"

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

    if ((OS_SYS_CLOCK == 0) ||
        (LOSCFG_BASE_CORE_TICK_PER_SECOND == 0) ||
        (LOSCFG_BASE_CORE_TICK_PER_SECOND > OS_SYS_CLOCK)) {
        return LOS_ERRNO_TICK_CFG_INVALID;
    }

#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
    OsSetVector(SysTick_IRQn, (HWI_PROC_FUNC)handler, NULL);
#else
    OsSetVector(SysTick_IRQn, (HWI_PROC_FUNC)handler);
#endif
#endif

    g_sysClock = OS_SYS_CLOCK;
    g_cyclesPerTick = OS_SYS_CLOCK / LOSCFG_BASE_CORE_TICK_PER_SECOND;

    ret = SysTick_Config(LOSCFG_BASE_CORE_TICK_RESPONSE_MAX);
    if (ret == 1) {
        return LOS_ERRNO_TICK_PER_SEC_TOO_SMALL;
    }

    return LOS_OK;
}

WEAK VOID ArchSysTickReload(UINT64 nextResponseTime)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    SysTick->LOAD = (UINT32)(nextResponseTime - 1UL); /* set reload register */
    SysTick->VAL = 0UL; /* Load the SysTick Counter Value */
    NVIC_ClearPendingIRQ(SysTick_IRQn);
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

WEAK UINT64 ArchGetTickCycle(UINT32 *period)
{
    UINT32 hwCycle = 0;
    UINT32 intSave = LOS_IntLock();
    UINT32 val = SysTick->VAL;
    *period = SysTick->LOAD;
    if (val != 0) {
        hwCycle = *period - val;
    }
    LOS_IntRestore(intSave);
    return (UINT64)hwCycle;
}

WEAK VOID ArchTickLock(VOID)
{
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

WEAK VOID ArchTickUnlock(VOID)
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
}

UINT32 ArchEnterSleep(VOID)
{
    __DSB();
    __WFI();
    __ISB();

    return LOS_OK;
}
