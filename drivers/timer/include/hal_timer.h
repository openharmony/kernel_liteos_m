/* ----------------------------------------------------------------------------
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2013-2022. All rights reserved.
 * Description: LiteOS Timer Header File
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
/* Migrated from hs-fbb LiteOS drivers/timer; OS_CYCLE_PER_TICK / g_sysClock
 * come from liteos_m los_tick.h. */

#ifndef HAL_TIMER_H
#define HAL_TIMER_H

#include "los_typedef.h"
#include "los_tick.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Runtime system clock (Hz), set by OsTickTimerInit in liteos_m. */
#ifndef GET_SYS_CLOCK
#define GET_SYS_CLOCK()        (g_sysClock)
#endif

/* OS_CYCLE_PER_TICK is defined in los_tick.h as (g_sysClock / LOSCFG_BASE_CORE_TICK_PER_SECOND). */

#ifdef LOSCFG_KERNEL_LOWPOWER
#if defined(LOSCFG_CORTEX_M_SYSTICK)
#define TIMER_MAX_CYCLE     0x00FFFFFF
#elif defined(LOSCFG_DW_APB_TIMER)
#define TIMER_MAX_CYCLE     0xFFFFFFFF
#endif

#define TIMER_MAX_TICK      (TIMER_MAX_CYCLE / OS_CYCLE_PER_TICK)
#endif

#ifndef NOP
#define NOP()       __asm__ volatile ("nop")
#endif


extern UINT64 hi_sched_clock(VOID);
extern VOID HalClockStart(VOID);
extern VOID HalClockInit(VOID);

extern UINT64 HalClockGetCycles(VOID);
extern VOID HalDelayUs(UINT32 usecs);
extern UINT32 HalClockGetTickTimerCycles(VOID);
extern VOID HalClockTickTimerReload(UINT32 cycles);

/* Current one-shot reload window (SysTick->LOAD); weak default = periodic tick. */
extern UINT32 HalClockGetReloadValue(VOID);

/* TRUE when the counter is confirmed running; bridge init() fails loudly
 * otherwise. Weak default = TRUE (drivers without a status register). */
extern BOOL HalClockIsRunning(VOID);

/* PM tick-lock pair: stop/restart the counter around sleep (weak defaults). */
extern VOID HalClockTickTimerLock(VOID);
extern VOID HalClockTickTimerUnlock(VOID);
extern UINT32 HalGetRemainingCycles(UINT32 *ticks);
extern UINT32 HalTickAdjust(UINT32 ticks);
extern VOID HalTickReload(VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* HAL_TIMER_H */
