/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 * Copyright (c) 2021 Nuclei Limited. All rights reserved.
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
#include "los_sched.h"
#include "los_arch_timer.h"
#include "nuclei_sdk_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define configKERNEL_INTERRUPT_PRIORITY         0

#define SYSTICK_TICK_CONST  (SOC_TIMER_FREQ / LOSCFG_BASE_CORE_TICK_PER_SECOND)

static OS_TICK_HANDLER systick_handler = (OS_TICK_HANDLER)NULL;

extern UINT32 g_intCount;

WEAK UINT32 HalTickStart(OS_TICK_HANDLER handler)
{
    SysTick_Config(SYSTICK_TICK_CONST);
    ECLIC_DisableIRQ(SysTimer_IRQn);
    ECLIC_SetLevelIRQ(SysTimer_IRQn, configKERNEL_INTERRUPT_PRIORITY);
    ECLIC_SetShvIRQ(SysTimer_IRQn, ECLIC_NON_VECTOR_INTERRUPT);
    ECLIC_EnableIRQ(SysTimer_IRQn);

    /* Set SWI interrupt level to lowest level/priority, SysTimerSW as Vector Interrupt */
    ECLIC_SetShvIRQ(SysTimerSW_IRQn, ECLIC_VECTOR_INTERRUPT);
    ECLIC_SetLevelIRQ(SysTimerSW_IRQn, configKERNEL_INTERRUPT_PRIORITY);
    ECLIC_EnableIRQ(SysTimerSW_IRQn);
    g_sysClock = OS_SYS_CLOCK;
    g_cyclesPerTick = g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND;
    g_intCount = 0;

    systick_handler = handler;

    return LOS_OK; /* never return */
}

#define HalTickSysTickHandler eclic_mtip_handler

void HalTickSysTickHandler( void )
{
    /* Do systick handler registered in HalTickStart. */
    if ((void *)systick_handler != NULL) {
        systick_handler();
    }
}

WEAK VOID HalSysTickReload(UINT64 nextResponseTime)
{
    SysTick_Reload(nextResponseTime);
}

WEAK UINT64 HalGetTickCycle(UINT32 *period)
{
    UINT64 ticks;
    UINT32 intSave = LOS_IntLock();
    ticks = SysTimer_GetLoadValue();
    *period = (UINT32)ticks;
    LOS_IntRestore(intSave);
    return ticks;
}

WEAK VOID HalTickLock(VOID)
{
    SysTimer_Stop();
}

WEAK VOID HalTickUnlock(VOID)
{
    SysTimer_Start();
}


VOID HalEnterSleep(LOS_SysSleepEnum sleep)
{
#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
    if (sleep == OS_SYS_DEEP_SLEEP) {
        OsSchedToSleep();
    }
#endif

    __WFI();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
