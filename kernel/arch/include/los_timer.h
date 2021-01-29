/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#ifndef LOS_TIMER_H
#define LOS_TIMER_H

#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define TICK_CHECK                           0x4000000
#define CYCLE_CHECK                          0xFFFFFFFFU
#define SHIFT_32_BIT                          32
#define MAX_HOUR 24
#define MAX_MINUTES 60
#define MAX_SECONDS 60
#define MILSEC 1000
#define RTC_WAKEUPCLOCK_RTCCLK 32768
#define RTC_WAKEUPCLOCK_RTCCLK_DIV 16
#define RTC_CALIBRATE_SLEEP_TIME 8
#define MACHINE_CYCLE_DEALAY_TIMES (LOSCFG_BASE_CORE_TICK_PER_SECOND << 2)

typedef enum {
	OS_SYS_NORMAL_SLEEP = 0,
	OS_SYS_DEEP_SLEEP,
} LOS_SysSleepEnum;

VOID HalTickLock(VOID);

VOID HalTickUnlock(VOID);

BOOL HalGetSysSleepFlag(VOID);

VOID HalClearSysSleepFlag(VOID);

VOID HalEnterSleep(LOS_SysSleepEnum sleep);

VOID HalDelay(UINT32 ticks);

UINT64 HalGetExpandTick(VOID);

INT32 HalGetRtcTime(UINT64 *usec);

INT32 HalGetRtcTimeZone(INT32 *timeZone);

INT32 HalSetRtcTime(UINT64 utcTime, UINT64 *usec);

INT32 HalSetRtcTimeZone(INT32 timeZone);

 /**
 * @ingroup los_timer
 * @brief Configure Tick Interrupt Start.
 *
 * @par Description:
 * This API is used to configure Tick Interrupt Start while macro LOSCFG_BASE_CORE_TICK_HW_TIME is No.
 *
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param: None.
 *
 * @retval #LOS_OK                               0:configure Tick Interrupt success.
 * @retval #LOS_ERRNO_TICK_CFG_INVALID           0x02000400:configure Tick Interrupt failed.
 *
 * @par Dependency:
 * <ul><li>los_config.h: the header file that contains the API declaration.</li></ul>
 * @see
 */

/**
 * @ingroup los_hwi
 * @brief reconfig systick, and clear SysTick_IRQn.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to reconfig systick, and clear SysTick_IRQn.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  cyclesPerTick  [IN] Cycles Per Tick
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None
 */
extern VOID HalSysTickReload(UINT32 cyclesPerTick);

/**
 * @ingroup los_hwi
 * @brief Get System cycle count.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to Get System cycle count.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  None.
 *
 * @retval: The value of the system cycle count.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_IntRestore
 */
extern UINT32 HalSysTickCurrCycleGet(VOID);


/* *
 * @ingroup los_hwi
 * @brief Get value from xPSR register.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to Get value from xPSR register.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param  cntHi  [IN] CpuTick High 4 byte
 * @param  cntLo  [IN] CpuTick Low 4 byte
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_IntRestore
 */
extern VOID HalGetCpuCycle(UINT32 *cntHi, UINT32 *cntLo);



extern VOID HalGetSystickCycle(UINT32 *puwCntHi, UINT32 *puwCntLo);

/**
 * @ingroup  los_tickless
 * @brief enable the tickless mode.
 *
 * @par Description:
 * This API is used to enable the tickless mode. System can change from periodic clock mode to dynamic clock mode.
 *
 * @attention
 * <ul>
 * </ul>
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_tickless.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_TicklessDisable
 */
extern VOID HalTicklessEnable(VOID);
/**
 * @ingroup  los_tickless
 * @brief disable the tickless mode.
 *
 * @par Description:
 * This API is used to diable the tickless mode. System will not change from periodic clock mode to dynamic clock mode.
 *
 * @attention
 * <ul>
 * </ul>
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_tickless.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_TicklessEnable
 */
extern VOID HalTicklessDisable(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif