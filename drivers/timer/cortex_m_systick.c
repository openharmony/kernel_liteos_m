/* ----------------------------------------------------------------------------
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2013-2022. All rights reserved.
 * Description: ARM cortex-m timer
 * Author: Huawei LiteOS Team
 * Create: 2013-01-01
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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
 * --------------------------------------------------------------------------- */

#include "hal_timer.h"
#include "asm/interrupt_config.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "los_tick.h"

#if defined(LOSCFG_ARCH_CORTEX_M3) || defined(LOSCFG_ARCH_CORTEX_M4) || \
    defined(LOSCFG_ARCH_CORTEX_M7) || defined(LOSCFG_ARCH_CORTEX_M55)
/* Vector-slot contract: handler installs at (num + OS_SYS_VECTOR_CNT);
 * SysTick_IRQn(-1) lands in slot 15. Drift = build failure. */
#if ((SysTick_IRQn) != -1) || ((OS_TICK_INT_NUM) != 15)
#error "SysTick vector contract broken: expect SysTick_IRQn == -1 and OS_TICK_INT_NUM == 15 (slot = SysTick_IRQn + OS_SYS_VECTOR_CNT)"
#endif

#define M_INT_NUM                   OS_TICK_INT_NUM

/* SysTick CSR | bit2 CLKSOURCE | bit1 TICKINT | bit0 ENABLE */
#define SYST_CSR                    0xE000E010U
#define SYST_CSR_ENABLE_MASK        1U
#define SYST_CSR_TICKINT_MASK       (1U << 1U)
#define SYST_CSR_CLKSOURCE_MASK     (1U << 2U)

#define SYST_RVR                    0xE000E014U
#define SYST_RVR_MAX                0xFFFFFFU
#define SYST_CVR                    0xE000E018U
#endif
#define TICK_INTR_CHECK             0x4000000U

#define TIMER_REG32_READ(addr)       (*(volatile UINT32 *)(UINTPTR)(addr))
#define TIMER_REG32_WRITE(addr, val) (*(volatile UINT32 *)(UINTPTR)(addr) = (val))
#define TIMER_REG8_WRITE(addr, val)  (*(volatile UINT8 *)(UINTPTR)(addr) = (val))

/* Minimal CMSIS-compatible definitions (no nvic.h in liteos_m). */
#ifndef __NVIC_PRIO_BITS
#define __NVIC_PRIO_BITS            3U
#endif
#define NVIC_PRIO_MASK              0xFFU
#define NVIC_PRIO_BITS              8U
#ifndef NVIC_CALC_PRIOVALUE
#define NVIC_CALC_PRIOVALUE(prio)   (((prio) << (NVIC_PRIO_BITS - (UINT8)__NVIC_PRIO_BITS)) & NVIC_PRIO_MASK)
#endif
#ifndef SCB_ICSR
#define SCB_ICSR                    0xE000ED04U
#endif
/* System Handler Priority Register. Begin Of Irq 4 */
#ifndef SCB_SHPRx
#define SCB_SHPRx(irqNum)           (0xE000ED18U + (irqNum) - 4U)
#endif

#ifdef LOSCFG_KERNEL_LOWPOWER
static UINT32 g_reloadSysTickFlag = 0;
#endif

/* g_cyclesPerTick is owned by the kernel; do not redefine it here. */

STATIC VOID ClockConfig(UINT32 reloadTick)
{
    /* RELOAD range 0x1-0xFFFFFF; 0 has no effect (no pulse from 1 to 0). */
    if (reloadTick > SYST_RVR_MAX) {
        PRINT_ERR("invalid reload tick\n");
        return;
    }
    TIMER_REG32_WRITE(SYST_RVR, reloadTick);
    TIMER_REG32_WRITE(SYST_CVR, 0);
    TIMER_REG32_WRITE(SYST_CSR, SYST_CSR_ENABLE_MASK | SYST_CSR_TICKINT_MASK | SYST_CSR_CLKSOURCE_MASK);
}

VOID HalClockInit(VOID)
{
    /* LOS_HwiCreate would shift SysTick_IRQn into the external-IRQ space. */
#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
    OsSetVector(SysTick_IRQn, (HWI_PROC_FUNC)OsTickHandler, NULL);
#else
    OsSetVector(SysTick_IRQn, (HWI_PROC_FUNC)OsTickHandler);
#endif
#endif
}

/* Strong: the bridge must not shadow this with its own fallback. */
UINT32 ArchEnterSleep(VOID)
{
    __asm__ volatile ("dsb" ::: "memory");
    __asm__ volatile ("wfi");
    __asm__ volatile ("isb" ::: "memory");
    return LOS_OK;
}

VOID HalClockStart(VOID)
{
    if ((LOSCFG_BASE_CORE_TICK_PER_SECOND == 0) || (LOSCFG_BASE_CORE_TICK_PER_SECOND > OS_SYS_CLOCK)) {
        return;
    }

    g_cyclesPerTick = (UINT32)OS_CYCLE_PER_TICK;

#if defined(LOSCFG_KERNEL_TICKLESS_GLOBAL) || defined(LOSCFG_KERNEL_TICKLESS)
    /* Tickless: arm the max response window; the scheduler reprograms per event. */
    ClockConfig(LOSCFG_BASE_CORE_TICK_RESPONSE_MAX - 1U);
#else
    /* reload action on the next clock edge, then counts down on subsequent clocks. */
    ClockConfig(OS_CYCLE_PER_TICK - 1U);
#endif

    /* Lowest SysTick priority, matching the arch timer's (1<<__NVIC_PRIO_BITS)-1. */
    TIMER_REG8_WRITE(SCB_SHPRx(M_INT_NUM), (UINT8)NVIC_CALC_PRIOVALUE((1U << __NVIC_PRIO_BITS) - 1U));
}

/* Strong: read-back for the bridge's start check (CSR.ENABLE). */
BOOL HalClockIsRunning(VOID)
{
    return (TIMER_REG32_READ(SYST_CSR) & SYST_CSR_ENABLE_MASK) != 0;
}

/*
 * Program the next one-shot deadline (cycles from now), mirroring the arch
 * SysTickReload: cap the load, stop, program, clear pending (PENDSTCLR),
 * re-enable. SysTick is auto-reload by nature; the tickless kernel
 * reprograms before each expiry, making it one-shot.
 */
VOID HalClockTickTimerReload(UINT32 cycles)
{
    UINT32 ctrl;
    UINT32 load;

    if (cycles == 0) {
        return;
    }
    load = (cycles > (SYST_RVR_MAX + 1U)) ? SYST_RVR_MAX : (cycles - 1U);

    /* Stop -> program -> PENDSTCLR -> start, mirroring the arch reload. */
    ctrl = TIMER_REG32_READ(SYST_CSR);
    TIMER_REG32_WRITE(SYST_CSR, ctrl & ~SYST_CSR_ENABLE_MASK);
    TIMER_REG32_WRITE(SYST_RVR, load);
    TIMER_REG32_WRITE(SYST_CVR, 0);
    TIMER_REG32_WRITE(SCB_ICSR, (1U << 25));   /* PENDSTCLR */
    TIMER_REG32_WRITE(SYST_CSR, ctrl | SYST_CSR_ENABLE_MASK);
}

UINT64 HalClockGetCycles(VOID)
{
    /* Count WITHIN the current window; the kernel owns the base. NEVER call
     * LOS_TickCountGet() here — tick count derives from this: recursion.
     * intLock'd, VAL then LOAD (arch read order): across a wrap this yields
     * the pre-wrap remainder (0 at VAL==LOAD), the kernel's base quantum. */
    UINT32 intSave = LOS_IntLock();
    UINT32 val = TIMER_REG32_READ(SYST_CVR);
    UINT32 load = TIMER_REG32_READ(SYST_RVR);
    LOS_IntRestore(intSave);
    return (UINT64)((val != 0) ? (load - val) : 0);
}

UINT32 HalClockGetReloadValue(VOID)
{
    /* Wrap-compensation quantum: must be the CURRENT window (SysTick LOAD). */
    return TIMER_REG32_READ(SYST_RVR);
}

/* PM tick-lock: counter stopped across sleep; resume restarts window at 0. */
VOID HalClockTickTimerLock(VOID)
{
    UINT32 ctrl = TIMER_REG32_READ(SYST_CSR);
    TIMER_REG32_WRITE(SYST_CSR, ctrl & ~SYST_CSR_ENABLE_MASK);
}

VOID HalClockTickTimerUnlock(VOID)
{
    UINT32 ctrl = TIMER_REG32_READ(SYST_CSR);
    TIMER_REG32_WRITE(SYST_CSR, ctrl | SYST_CSR_ENABLE_MASK);
}

VOID HalDelayUs(UINT32 usecs)
{
    UINT64 tmo = LOS_CurrNanosec() + ((UINT64)usecs * OS_SYS_NS_PER_US);

    while (LOS_CurrNanosec() < tmo) {
        NOP();
    }
}

UINT64 hi_sched_clock(VOID)
{
    return LOS_CurrNanosec();
}

#ifdef LOSCFG_KERNEL_LOWPOWER

/* just stub api for tickless */
UINT32 HalClockGetTickTimerCycles(VOID)
{
    return TIMER_REG32_READ(SYST_CVR);
}

VOID HalTickReload(VOID)
{
    if (g_reloadSysTickFlag == 1) {
        ClockConfig(g_cyclesPerTick);
        g_reloadSysTickFlag = 0;
    }
}

UINT32 HalGetRemainingCycles(UINT32 *ticks)
{
    UINT32 hwCycle;

    TIMER_REG32_WRITE(SYST_CSR, 0);

    hwCycle = TIMER_REG32_READ(SYST_CVR);

    /* If the tick interrupt has arrived, update tick count and remaining cycles */
    if ((TIMER_REG32_READ(SCB_ICSR) & TICK_INTR_CHECK) != 0) {
        (*ticks)--;
        hwCycle = TIMER_REG32_READ(SYST_CVR);
    }
    return hwCycle;
}

UINT32 HalTickAdjust(UINT32 ticks)
{
    UINT32 remainingCycles;
    UINT64 sleepCycles;

    if (ticks > TIMER_MAX_TICK) {
        return LOS_NOK;
    }

    remainingCycles = HalGetRemainingCycles(&ticks);
    sleepCycles = (ticks - 1) * g_cyclesPerTick;

    HalClockTickTimerReload((UINT32)(sleepCycles + remainingCycles));
    return LOS_OK;
}
#endif
