/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Bridge HAL timer drivers (HalClockInit/Start/GetCycles, hal_timer.h) to
 * the liteos_m ArchTickTimer interface, so a generic drivers/timer HAL
 * driver can be the system tick source without a board-specific adapter.
 * By design: init()'s tickHandler is ignored — drivers own their ISR and a
 * non-OsTickHandler handler logs a warning; a driver without one-shot
 * reload degrades tickless to periodic; init() fails unless HalClockIsRunning()
 * confirms the counter is up; lock/unlock are the PM tick-lock pair.
 */

#include "los_timer.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_tick.h"
#include "los_interrupt.h"
#include "asm/interrupt_config.h"
#include "hal_timer.h"

#ifdef LOSCFG_TIMER_ARCH_BRIDGE

STATIC UINT64 HalBridgeReload(UINT64 nextResponseTime);

STATIC UINT32 HalBridgeTickStart(HWI_PROC_FUNC handler)
{
    if ((handler != NULL) && (handler != (HWI_PROC_FUNC)OsTickHandler)) {
        PRINT_WARN("tick handler %p ignored: HAL drivers own their ISR\n", handler);
    }

    if ((LOSCFG_BASE_CORE_TICK_PER_SECOND == 0) ||
        (LOSCFG_BASE_CORE_TICK_PER_SECOND > OS_SYS_CLOCK)) {
        return LOS_ERRNO_TICK_PER_SEC_TOO_SMALL;   /* arch SysTickStart parity */
    }

    /* Publish g_sysClock before HalClockStart consumes OS_CYCLE_PER_TICK. */
    g_sysClock = OS_SYS_CLOCK;

    HalClockInit();
    HalClockStart();

    /* HAL start() is void; fail the boot loudly if the counter never came
     * up (e.g. reload out of the driver's range) — arch parity. */
    if (!HalClockIsRunning()) {
        return LOS_ERRNO_TICK_NO_HWTIMER;
    }
    return LOS_OK;
}

STATIC UINT64 HalBridgeGetCycle(UINT32 *period)
{
    if (period != NULL) {
        *period = HalClockGetReloadValue();
    }
    return HalClockGetCycles();
}

/* Periodic tick is the correct quantum for drivers without one-shot reload. */
__attribute__((weak)) UINT32 HalClockGetReloadValue(VOID)
{
    return (UINT32)OS_CYCLE_PER_TICK;
}

/* No-op default: drivers with one-shot reload provide a strong definition. */
__attribute__((weak)) VOID HalClockTickTimerReload(UINT32 cycles)
{
    (VOID)cycles;
}

/* PM tick-lock pair: counter must be stopped across sleep (arch parity). */
__attribute__((weak)) VOID HalClockTickTimerLock(VOID)
{
}

__attribute__((weak)) VOID HalClockTickTimerUnlock(VOID)
{
}

/* No status bit? Claim running; drivers with a control reg provide a strong check. */
__attribute__((weak)) BOOL HalClockIsRunning(VOID)
{
    return TRUE;
}

/* No ArchEnterSleep here: each HAL driver provides its own. */

STATIC VOID HalBridgeLock(VOID)
{
    HalClockTickTimerLock();
}

STATIC VOID HalBridgeUnlock(VOID)
{
    HalClockTickTimerUnlock();
}

STATIC ArchTickTimer g_halBridgeTickTimer = {
    .freq = OS_SYS_CLOCK,
    .irqNum = OS_TICK_INT_NUM,
    .periodMax = LOSCFG_BASE_CORE_TICK_RESPONSE_MAX,
    .init = HalBridgeTickStart,
    .getCycle = HalBridgeGetCycle,
    .reload = HalBridgeReload,
    .lock = HalBridgeLock,
    .unlock = HalBridgeUnlock,
    .tickHandler = NULL,
};

STATIC UINT64 HalBridgeReload(UINT64 nextResponseTime)
{
    if (nextResponseTime > g_halBridgeTickTimer.periodMax) {
        nextResponseTime = g_halBridgeTickTimer.periodMax;
    }
    HalClockTickTimerReload((UINT32)nextResponseTime);
    return nextResponseTime;
}

/* Weak: a board-level strong provider wins the link. */
__attribute__((weak)) ArchTickTimer *ArchSysTickTimerGet(VOID)
{
    return &g_halBridgeTickTimer;
}

#endif /* LOSCFG_TIMER_ARCH_BRIDGE */
