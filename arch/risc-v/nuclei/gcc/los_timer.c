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
#include "los_arch_timer.h"
#include "nuclei_sdk_hal.h"

#define configKERNEL_INTERRUPT_PRIORITY         0

#define SYSTICK_TICK_CONST  (SOC_TIMER_FREQ / LOSCFG_BASE_CORE_TICK_PER_SECOND)

STATIC HWI_PROC_FUNC g_sysTickHandler = (HWI_PROC_FUNC)NULL;

extern UINT32 g_intCount;

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler);
STATIC VOID SysTickReload(UINT64 nextResponseTime);
STATIC UINT64 SysTickCycleGet(UINT32 *period);
STATIC VOID SysTickLock(VOID);
STATIC VOID SysTickUnlock(VOID);

STATIC ArchTickTimer g_archTickTimer = {
    .freq = OS_SYS_CLOCK,
    .irqNum = SysTimer_IRQn,
    .init = SysTickStart,
    .getCycle = SysTickCycleGet,
    .reload = SysTickReload,
    .lock = SysTickLock,
    .unlock = SysTickUnlock,
    .tickHandler = NULL,
};

STATIC UINT32 SysTickStart(HWI_PROC_FUNC handler)
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
    g_intCount = 0;

    g_sysTickHandler = handler;

    return LOS_OK; /* never return */
}

#define ArchTickSysTickHandler eclic_mtip_handler

void ArchTickSysTickHandler(void)
{
    /* Do systick handler registered in HalTickStart. */
    if ((void *)g_sysTickHandler != NULL) {
        g_sysTickHandler();
    }
}

STATIC VOID SysTickReload(UINT64 nextResponseTime)
{
    SysTick_Reload(nextResponseTime);
}

STATIC UINT64 SysTickCycleGet(UINT32 *period)
{
    UINT64 ticks;
    UINT32 intSave = LOS_IntLock();
    ticks = SysTimer_GetLoadValue();
    *period = (UINT32)ticks;
    LOS_IntRestore(intSave);
    return ticks;
}

STATIC VOID SysTickLock(VOID)
{
    SysTimer_Stop();
}

STATIC VOID SysTickUnlock(VOID)
{
    SysTimer_Start();
}

ArchTickTimer *ArchSysTickTimerGet(VOID)
{
    return &g_archTickTimer;
}

UINT32 ArchEnterSleep(VOID)
{
    __WFI();

    return LOS_OK;
}

