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

#ifndef _LOS_EXC_H
#define _LOS_EXC_H

#include "los_config.h"
#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/* *
 * @ingroup los_hwi
 * Maximum number of used hardware interrupts.
 */
#ifndef OS_HWI_MAX_NUM
#define OS_HWI_MAX_NUM                        LOSCFG_PLATFORM_HWI_LIMIT
#endif

/* *
 * @ingroup los_hwi
 * Highest priority of a hardware interrupt.
 */
#ifndef OS_HWI_PRIO_HIGHEST
#define OS_HWI_PRIO_HIGHEST                   0
#endif

/* *
 * @ingroup los_hwi
 * Lowest priority of a hardware interrupt.
 */
#ifndef OS_HWI_PRIO_LOWEST
#define OS_HWI_PRIO_LOWEST                    7
#endif

/* *
 * @ingroup los_config
 * Configuration item for interrupt with argument
 */
#ifndef OS_HWI_WITH_ARG
#define OS_HWI_WITH_ARG                       NO
#endif

/* *
 * @ingroup los_hwi
 * Define the type of a hardware interrupt number.
 */
typedef UINT32 HWI_HANDLE_T;

/* *
 * @ingroup los_hwi
 * Define the type of a hardware interrupt priority.
 */
typedef UINT16 HWI_PRIOR_T;

/* *
 * @ingroup los_hwi
 * Define the type of hardware interrupt mode configurations.
 */
typedef UINT16 HWI_MODE_T;

/* *
 * @ingroup los_hwi
 * Define the type of the parameter used for the hardware interrupt creation function. The function of this parameter varies among platforms.
 */
typedef UINT32 HWI_ARG_T;

/* *
 * @ingroup  los_hwi
 * Define the type of a hardware interrupt handling function.
 */
#if (OS_HWI_WITH_ARG == YES)

typedef VOID (*HWI_PROC_FUNC)(VOID *parm);
typedef struct {
    HWI_PROC_FUNC pfnHandler;
    VOID *pParm;
} HWI_SLAVE_FUNC;

#else

typedef VOID (*HWI_PROC_FUNC)(void);

#endif

/* *
 * @ingroup  los_hwi
 * Define the type of a hardware interrupt vector table function.
 */
typedef VOID (**HWI_VECTOR_FUNC)(void);

/* *
 * @ingroup los_hwi
 * Count of interrupts.
 */
extern UINT32 g_vuwIntCount;

/* *
 * @ingroup los_hwi
 * An interrupt is active.
 */
#define OS_INT_ACTIVE                         (g_vuwIntCount > 0)

/* *
 * @ingroup los_hwi
 * An interrupt is inactive.
 */
#define OS_INT_INACTIVE                       (!(OS_INT_ACTIVE))

/* *
 * @ingroup los_hwi
 * Count of M-Core system interrupt vector.
 */
#define OS_SYS_VECTOR_CNT                     16

/* *
 * @ingroup los_hwi
 * Count of M-Core interrupt vector.
 */
#define OS_VECTOR_CNT                         (OS_SYS_VECTOR_CNT + OS_HWI_MAX_NUM)

/* *
 * @ingroup los_hwi
 * AIRCR register priority group parameter .
 */
#define OS_NVIC_AIRCR_PRIGROUP                7

/* *
 * @ingroup los_hwi
 * Boot interrupt vector table.
 */
extern UINT32 _BootVectors[];

/* *
 * @ingroup los_hwi
 * Hardware interrupt error code: Invalid interrupt number.
 *
 * Value: 0x02000900
 *
 * Solution: Ensure that the interrupt number is valid. The value range of the interrupt number applicable for a Cortex-A7 platform is [OS_USER_HWI_MIN,OS_USER_HWI_MAX].
 */
#define OS_ERRNO_HWI_NUM_INVALID              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x00)

/* *
 * @ingroup los_hwi
 * Hardware interrupt error code: Null hardware interrupt handling function.
 *
 * Value: 0x02000901
 *
 * Solution: Pass in a valid non-null hardware interrupt handling function.
 */
#define OS_ERRNO_HWI_PROC_FUNC_NULL           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x01)

/* *
 * @ingroup los_hwi
 * Hardware interrupt error code: Insufficient interrupt resources for hardware interrupt creation.
 *
 * Value: 0x02000902
 *
 * Solution: Increase the configured maximum number of supported hardware interrupts.
 */
#define OS_ERRNO_HWI_CB_UNAVAILABLE           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x02)

/* *
 * @ingroup los_hwi
 * Hardware interrupt error code: Insufficient memory for hardware interrupt initialization.
 *
 * Value: 0x02000903
 *
 * Solution: Expand the configured memory.
 */
#define OS_ERRNO_HWI_NO_MEMORY                LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x03)

/* *
 * @ingroup los_hwi
 * Hardware interrupt error code: The interrupt has already been created.
 *
 * Value: 0x02000904
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_ALREADY_CREATED          LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x04)

/* *
 * @ingroup los_hwi
 * Hardware interrupt error code: Invalid interrupt priority.
 *
 * Value: 0x02000905
 *
 * Solution: Ensure that the interrupt priority is valid. The value range of the interrupt priority applicable for a Cortex-A7 platform is [0,15].
 */
#define OS_ERRNO_HWI_PRIO_INVALID             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x05)

/* *
 * @ingroup los_hwi
 * Hardware interrupt error code: Incorrect interrupt creation mode.
 *
 * Value: 0x02000906
 *
 * Solution: The interrupt creation mode can be only set to OS_HWI_MODE_COMM or OS_HWI_MODE_FAST of which the value can be 0 or 1.
 */
#define OS_ERRNO_HWI_MODE_INVALID             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x06)

/* *
 * @ingroup los_hwi
 * Hardware interrupt error code: The interrupt has already been created as a fast interrupt.
 *
 * Value: 0x02000907
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_FASTMODE_ALREADY_CREATED LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x07)

/* *
 * @ingroup los_hwi
 * SysTick control and status register.
 */
#define OS_SYSTICK_CONTROL_REG                0xE000E010

/* *
 * @ingroup los_hw
 * SysTick current value register.
 */
#define OS_SYSTICK_CURRENT_REG                0xE000E018

/* *
 * @ingroup los_hwi
 * Interrupt Priority-Level Registers.
 */
#define OS_NVIC_PRI_BASE                      0xE000E400

/* *
 * @ingroup los_hwi
 * Interrupt enable register for 0-31.
 */
#define OS_NVIC_SETENA_BASE                   0xE000E100

/* *
 * @ingroup los_hwi
 * interrupt pending register.
 */
#define OS_NVIC_SETPEND_BASE                  0xE000E200

/* *
 * @ingroup los_hwi
 * Interrupt active register.
 */
#define OS_NVIC_INT_ACT_BASE                  0xE000E300

/* *
 * @ingroup los_hwi
 * Interrupt disable register for 0-31.
 */
#define OS_NVIC_CLRENA_BASE                   0xE000E180

/* *
 * @ingroup los_hwi
 * Interrupt control and status register.
 */
#define OS_NVIC_INT_CTRL                      0xE000ED04

/* *
 * @ingroup los_hwi
 * Vector table offset register.
 */
#define OS_NVIC_VTOR                          0xE000ED08

/* *
 * @ingroup los_hwi
 * Application interrupt and reset control register
 */
#define OS_NVIC_AIRCR                         0xE000ED0C

/* *
 * @ingroup los_hwi
 * System exception priority register.
 */
#define OS_NVIC_EXCPRI_BASE                   0xE000ED18

/* *
 * @ingroup los_hwi
 * Interrupt No. 1 :reset.
 */
#define OS_EXC_RESET                          1

/* *
 * @ingroup los_hwi
 * Interrupt No. 2 :Non-Maskable Interrupt.
 */
#define OS_EXC_NMI                            2

/* *
 * @ingroup los_hwi
 * Interrupt No. 3 :(hard)fault.
 */
#define OS_EXC_HARD_FAULT                     3

/* *
 * @ingroup los_hwi
 * Interrupt No. 4 :MemManage fault.
 */
#define OS_EXC_MPU_FAULT                      4

/* *
 * @ingroup los_hwi
 * Interrupt No. 5 :Bus fault.
 */
#define OS_EXC_BUS_FAULT                      5

/* *
 * @ingroup los_hwi
 * Interrupt No. 6 :Usage fault.
 */
#define OS_EXC_USAGE_FAULT                    6

/* *
 * @ingroup los_hwi
 * Interrupt No. 11 :SVCall.
 */
#define OS_EXC_SVC_CALL                       11

/* *
 * @ingroup los_hwi
 * Interrupt No. 12 :Debug monitor.
 */
#define OS_EXC_DBG_MONITOR                    12

/* *
 * @ingroup los_hwi
 * Interrupt No. 14 :PendSV.
 */
#define OS_EXC_PEND_SV                        14

/* *
 * @ingroup los_hwi
 * Interrupt No. 15 :SysTick.
 */
#define OS_EXC_SYS_TICK                       15

/* *
 * @ingroup los_hwi
 * hardware interrupt form mapping handling function array.
 */
extern HWI_PROC_FUNC g_hwiForm[OS_VECTOR_CNT];

#if (OS_HWI_WITH_ARG == YES)
/* *
 * @ingroup los_hwi
 * hardware interrupt Slave form mapping handling function array.
 */
extern HWI_SLAVE_FUNC g_hwiSlaveForm[OS_VECTOR_CNT];

/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
#define OsSetVectonr(num, vector, arg)                                    \
    do {                                                                      \
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)OsInterrupt; \
        g_hwiSlaveForm[num + OS_SYS_VECTOR_CNT].pfnHandler = vector;  \
        g_hwiSlaveForm[num + OS_SYS_VECTOR_CNT].pParm = (VOID *)arg;     \
    } while(0)
#else
/* *
 * @ingroup los_hwi
 * hardware interrupt Slave form mapping handling function array.
 */
extern HWI_PROC_FUNC g_hwiSlaveForm[OS_VECTOR_CNT];

/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
#define OsSetVector(num, vector)                             \
    do {                                                          \
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = OsInterrupt;    \
        g_hwiSlaveForm[num + OS_SYS_VECTOR_CNT] = vector; \
    } while(0)
#endif

/* *
 * @ingroup  los_hwi
 * @brief Create a hardware interrupt.
 *
 * @par Description:
 * This API is used to configure a hardware interrupt and register a hardware interrupt handling function.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for hardware interrupt tailoring is enabled.</li>
 * <li>Hardware interrupt number value range: [OS_USER_HWI_MIN,OS_USER_HWI_MAX]. The value range applicable for a Cortex-A7 platform is [32,95].</li>
 * <li>OS_HWI_MAX_NUM specifies the maximum number of interrupts that can be created.</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * </ul>
 *
 * @param  hwiNum   [IN] Type#HWI_HANDLE_T: hardware interrupt number. The value range applicable for a Cortex-A7 platform is [32,95].
 * @param  hwiPrio  [IN] Type#HWI_PRIOR_T: hardware interrupt priority. Ignore this parameter temporarily.
 * @param  mode     [IN] Type#HWI_MODE_T: hardware interrupt mode. Ignore this parameter temporarily.
 * @param  handler  [IN] Type#HWI_PROC_FUNC: interrupt handler used when a hardware interrupt is triggered.
 * @param  arg      [IN] Type#HWI_ARG_T: input parameter of the interrupt handler used when a hardware interrupt is triggered.
 *
 * @retval #OS_ERRNO_HWI_PROC_FUNC_NULL               0x02000901: Null hardware interrupt handling function.
 * @retval #OS_ERRNO_HWI_NUM_INVALID                  0x02000900: Invalid interrupt number.
 * @retval #OS_ERRNO_HWI_NO_MEMORY                    0x02000903: Insufficient memory for hardware interrupt creation.
 * @retval #OS_ERRNO_HWI_ALREADY_CREATED              0x02000904: The interrupt handler being created has already been created.
 * @retval #LOS_OK                                    0         : The interrupt is successfully created.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_HwiCreate(HWI_HANDLE_T hwiNum,
                            HWI_PRIOR_T hwiPrio,
                            HWI_MODE_T mode,
                            HWI_PROC_FUNC handler,
                            HWI_ARG_T arg);

/* *
 * @ingroup  los_hwi
 * @brief: Hardware interrupt entry function.
 *
 * @par Description:
 * This API is used as all hardware interrupt handling function entry.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 * @param:None.
 *
 * @retval:None.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern VOID OsInterrupt(VOID);

/* *
 * @ingroup  los_hwi
 * @brief: Get a interrupt number.
 *
 * @par Description:
 * This API is used to get the current interrupt number.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 * @param: None.
 *
 * @retval: Interrupt Indexes number.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 OsIntNumGet(VOID);

/* *
 * @ingroup  los_hwi
 * @brief: Default vector handling function.
 *
 * @par Description:
 * This API is used to configure interrupt for null function.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 * @param:None.
 *
 * @retval:None.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li
></ul>
 * @see None.
 */
extern VOID OsHwiDefaultHandler(VOID);

/* *
 * @ingroup  los_hwi
 * @brief: Reset the vector table.
 *
 * @par Description:
 * This API is used to reset the vector table.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 * @param:None.
 *
 * @retval:None.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern VOID Reset_Handler(VOID);

/* *
 * @ingroup  los_hwi
 * @brief: Pended System Call.
 *
 * @par Description:
 * PendSV can be pended and is useful for an OS to pend an exception
 * so that an action can be performed after other important tasks are completed.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 * @param:None.
 *
 * @retval:None.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern VOID osPendSV(VOID);

/* *
 * @ingroup los_hwi
 * @brief Enable all interrupts.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to enable all IRQ and FIQ interrupts in the CPSR.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param None.
 *
 * @retval CPSR value obtained after all interrupts are enabled.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_IntRestore
 */
extern UINTPTR LOS_IntUnLock(VOID);

/* *
 * @ingroup los_hwi
 * @brief Disable all interrupts.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to disable all IRQ and FIQ interrupts in the CPSR.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>None.</li>
 * </ul>
 *
 * @param None.
 *
 * @retval CPSR value obtained before all interrupts are disabled.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_IntRestore
 */
extern UINTPTR LOS_IntLock(VOID);

/* *
 * @ingroup los_hwi
 * @brief Restore interrupts.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to restore the CPSR value obtained before all interrupts are disabled.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>This API can be called only after all interrupts are disabled, and the input parameter value should be the value returned by calling the all interrupt disabling API.</li>
 * </ul>
 *
 * @param intSave [IN] CPSR value obtained before all interrupts are disabled.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_IntLock
 */
extern VOID LOS_IntRestore(UINTPTR intSave);

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
extern VOID LOS_GetCpuCycle(UINT32 *cntHi, UINT32 *cntLo);

#if (LOSCFG_KERNEL_TICKLESS == YES)
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
 * @see LOS_IntRestore
 */
extern VOID LOS_SysTickReload(UINT32 cyclesPerTick);
#endif

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
extern UINT32 LOS_SysTickCurrCycleGet(VOID);

extern VOID LOS_GetSystickCycle(UINT32 *puwCntHi, UINT32 *puwCntLo);

/* *
 * @ingroup  los_hwi
 * @brief Delete hardware interrupt.
 *
 * @par Description:
 * This API is used to delete hardware interrupt.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for hardware interrupt tailoring is enabled.</li>
 * <li>Hardware interrupt number value range: [OS_USER_HWI_MIN,OS_USER_HWI_MAX]. The value range applicable for a Cortex-A7 platform is [32,95].</li>
 * <li>OS_HWI_MAX_NUM specifies the maximum number of interrupts that can be created.</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * </ul>
 *
 * @param  hwiNum   [IN] Type#HWI_HANDLE_T: hardware interrupt number. The value range applicable for a Cortex-A7 platform is [32,95].
 *
 * @retval #OS_ERRNO_HWI_NUM_INVALID              0x02000900: Invalid interrupt number.
 * @retval #LOS_OK                                0         : The interrupt is successfully delete.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_HwiDelete(HWI_HANDLE_T hwiNum);

typedef enum {
	OS_SYS_NORMAL_SLEEP = 0,
	OS_SYS_DEEP_SLEEP,
} LOS_SysSleepEnum;

VOID LOS_TickLock(VOID);

VOID LOS_TickUnlock(VOID);

BOOL LOS_GetSysSleepFlag(VOID);

VOID LOS_ClearSysSleepFlag(VOID);

VOID LOS_EnterSleep(LOS_SysSleepEnum sleep);

VOID LOS_SystemWakeup(UINT32 hwiIndex);

#define OS_EXC_IN_INIT                      0
#define OS_EXC_IN_TASK                      1
#define OS_EXC_IN_HWI                       2

#define OS_EXC_MAX_BUF_LEN                  25
#define OS_EXC_MAX_NEST_DEPTH               1

#define OS_NVIC_SHCSR                       0xE000ED24
#define OS_NVIC_CCR                         0xE000ED14

#define OS_NVIC_INT_ENABLE_SIZE             0x20
#define OS_NVIC_INT_PRI_SIZE                0xF0
#define OS_NVIC_EXCPRI_SIZE                 0xC

#define OS_NVIC_INT_PEND_SIZE               OS_NVIC_INT_ACT_SIZE
#define OS_NVIC_INT_ACT_SIZE                OS_NVIC_INT_ENABLE_SIZE

#define OS_EXC_FLAG_NO_FLOAT                0x10000000
#define OS_EXC_FLAG_FAULTADDR_VALID         0x01
#define OS_EXC_FLAG_IN_HWI                  0x02

#define OS_EXC_IMPRECISE_ACCESS_ADDR        0xABABABAB

#define OS_EXC_EVENT                        0x00000001

/**
 *@ingroup los_exc
 * the struct of register files
 *
 * description: the register files that saved when exception triggered
 *
 * notes:the following register with prefix 'uw'  correspond to the registers in the cpu  data sheet.
 */
typedef struct tagExcContext {
    //handler save
#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED   ) && (__FPU_USED    == 1U))     )
    UINT32 S16;
    UINT32 S17;
    UINT32 S18;
    UINT32 S19;
    UINT32 S20;
    UINT32 S21;
    UINT32 S22;
    UINT32 S23;
    UINT32 S24;
    UINT32 S25;
    UINT32 S26;
    UINT32 S27;
    UINT32 S28;
    UINT32 S29;
    UINT32 S30;
    UINT32 S31;
#endif
    UINT32 uwR4;
    UINT32 uwR5;
    UINT32 uwR6;
    UINT32 uwR7;
    UINT32 uwR8;
    UINT32 uwR9;
    UINT32 uwR10;
    UINT32 uwR11;
    UINT32 uwPriMask;
    //auto save
    UINT32 uwSP;
    UINT32 uwR0;
    UINT32 uwR1;
    UINT32 uwR2;
    UINT32 uwR3;
    UINT32 uwR12;
    UINT32 uwLR;
    UINT32 uwPC;
    UINT32 uwxPSR;
#if ((defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined (__FPU_USED) && (__FPU_USED== 1U)))
    UINT32 S0;
    UINT32 S1;
    UINT32 S2;
    UINT32 S3;
    UINT32 S4;
    UINT32 S5;
    UINT32 S6;
    UINT32 S7;
    UINT32 S8;
    UINT32 S9;
    UINT32 S10;
    UINT32 S11;
    UINT32 S12;
    UINT32 S13;
    UINT32 S14;
    UINT32 S15;
    UINT32 FPSCR;
    UINT32 NO_NAME;
#endif
}EXC_CONTEXT_S;

typedef UINT32 (*EXC_INFO_SAVE_CALLBACK)(UINT32, VOID*);
typedef VOID (*EXC_PROC_FUNC)(UINT32, EXC_CONTEXT_S *);
VOID OsExcHandleEntry(UINT32 excType, UINT32 faultAddr, UINT32 pid, EXC_CONTEXT_S *excBufAddr);

/**
 * @ingroup  los_hwi
 * @brief: Exception initialization.
 *
 * @par Description:
 * This API is used to configure the exception function vector table.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 *@param uwArraySize [IN] Memory size of exception.
 *
 * @retval: None
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
VOID OsExcInit(VOID);

extern VOID OsExcNMI(VOID);
extern VOID OsExcHardFault(VOID);
extern VOID OsExcMemFault(VOID);
extern VOID OsExcBusFault(VOID);
extern VOID OsExcUsageFault(VOID);
extern VOID OsExcSvcCall(VOID);
extern UINT8 g_aucTaskArray[];

extern void OsBackTrace();

/**
 *@ingroup los_exc
 *@brief Kernel panic function.
 *
 *@par Description:
 *Stack function that prints kernel panics.
 *@attention After this function is called and stack information is printed, the system will fail to respond.
 *@attention The input parameter can be NULL.
 *@param fmt [IN] Type #char* : variadic argument.
 *
 *@retval #None.
 *
 *@par Dependency:
 *los_interrupt.h: the header file that contains the API declaration.
 *@see None.
*/
VOID LOS_Panic(const CHAR *fmt, ...);

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:总线状态寄存器入栈时发生错误
 */
#define OS_EXC_BF_STKERR           1

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:总线状态寄存器出栈时发生错误
 */
#define OS_EXC_BF_UNSTKERR         2

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:总线状态寄存器不精确的数据访问违例
 */
#define OS_EXC_BF_IMPRECISERR      3

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:总线状态寄存器精确的数据访问违例
 */
#define OS_EXC_BF_PRECISERR        4

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:总线状态寄存器取指时的访问违例
 */
#define OS_EXC_BF_IBUSERR          5

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:存储器管理状态寄存器入栈时发生错误
 */
#define OS_EXC_MF_MSTKERR          6

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:存储器管理状态寄存器出栈时发生错误
 */
#define OS_EXC_MF_MUNSTKERR        7

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:存储器管理状态寄存器数据访问违例
 */
#define OS_EXC_MF_DACCVIOL         8

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:存储器管理状态寄存器取指访问违例
 */
#define OS_EXC_MF_IACCVIOL         9


/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:用法错误，表示除法运算时除数为零
 */
#define OS_EXC_UF_DIVBYZERO        10

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:用法错误，未对齐访问导致的错误
 */
#define OS_EXC_UF_UNALIGNED        11

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:用法错误，试图执行协处理器相关指令
 */
#define OS_EXC_UF_NOCP             12

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:用法错误，在异常返回时试图非法地加载EXC_RETURN到PC
 */
#define OS_EXC_UF_INVPC            13

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:用法错误，试图切入ARM状态
 */
#define OS_EXC_UF_INVSTATE         14

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:用法错误，执行的指令其编码是未定义的——解码不能
 */
#define OS_EXC_UF_UNDEFINSTR       15

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:NMI中断
 */

#define OS_EXC_CAUSE_NMI           16

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:硬fault
 */
#define OS_EXC_CAUSE_HARDFAULT     17

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:任务处理函数退出
 */
#define OS_EXC_CAUSE_TASK_EXIT     18

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:致命错误
 */
#define OS_EXC_CAUSE_FATAL_ERR     19

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:调试事件导致的硬fault
 */
#define OS_EXC_CAUSE_DEBUGEVT      20

/**
 *@ingroup los_exc
 *Cortex-M3异常具体类型:取向量时发生的硬fault
 */
#define OS_EXC_CAUSE_VECTBL        21

/**
 *@ingroup los_exc
 * 异常信息结构体
 *
 * 描述:M4平台下的异常触发时保存的异常信息
 *
 */
typedef struct tagExcInfo {
    /**< 异常发生阶段： 0表示异常发生在初始化中，1表示异常发生在任务中，2表示异常发生在中断中 */
    UINT16 phase;
    /**< 异常类型,出异常时对照上面列出的1-19号 */
    UINT16 type;
    /**< 若为精确地址访问错误表示异常发生时的错误访问地址 */
    UINT32 faultAddr;
    /**< 在中断中发生异常，表示中断号。在任务中发生异常，表示任务id，如果发生在初始化中，则为0xffffffff */
    UINT32 thrdPid;
    /**< 异常嵌套个数，目前仅支持第一次进入异常时执行注册的钩子函数 */
    UINT16 nestCnt;
    /**< 保留 */
    UINT16 reserved;
    /**< 自动压栈浮点寄存器的异常发生时刻的硬件上下文 */
    EXC_CONTEXT_S * context;
} ExcInfo;

extern UINT32 g_curNestCount;
extern UINT32 g_vuwIntCount;

static VOID OsExcSave2DDR(VOID);
VOID OsExcInfoDisplay(ExcInfo *exc);

extern UINT8 g_uwExcTbl[32];

typedef enum {
    OS_EXC_TYPE_CONTEXT     = 0,
    OS_EXC_TYPE_TSK         = 1,
    OS_EXC_TYPE_QUE         = 2,
    OS_EXC_TYPE_NVIC        = 3,
    OS_EXC_TYPE_TSK_SWITCH  = 4,
    OS_EXC_TYPE_MEM         = 5,
    OS_EXC_TYPE_MAX         = 6
} ExcInfoType;

typedef struct tagExcInfoCallBackArray {
    ExcInfoType           uwType;
    UINT32                  uwValid;
    EXC_INFO_SAVE_CALLBACK  pFnExcInfoCb;
    VOID*                   pArg;
} ExcInfoArray;



#define MAX_SCENE_INFO_SIZE     (8 + sizeof(ExcInfo) - 4 + sizeof(EXC_CONTEXT_S))
#define MAX_TSK_INFO_SIZE       (8 + sizeof(TSK_INFO_S) * (LOSCFG_BASE_CORE_TSK_LIMIT + 1))
#define MAX_INT_INFO_SIZE       (8 + 0x164)

#if (LOSCFG_BASE_IPC_QUEUE == YES)
#define MAX_QUEUE_INFO_SIZE     (8 + sizeof(QUEUE_INFO_S) * LOSCFG_BASE_IPC_QUEUE_LIMIT)
#else
#define MAX_QUEUE_INFO_SIZE     (0)
#endif

#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == YES)
#define MAX_SWITCH_INFO_SIZE    (8 + (sizeof(UINT32) + sizeof(CHAR) * LOS_TASK_NAMELEN) * OS_TASK_SWITCH_INFO_COUNT)
#else
#define MAX_SWITCH_INFO_SIZE    (0)
#endif

#if (LOSCFG_BASE_MEM_NODE_INTEGRITY_CHECK == YES)
#define MAX_MEM_INFO_SIZE       (8 + sizeof(MEM_INFO_S) * OS_SYS_MEM_NUM)
#else
#define MAX_MEM_INFO_SIZE       (0)
#endif

#define MAX_EXC_MEM_SIZE ( 4 + MAX_SCENE_INFO_SIZE + MAX_TSK_INFO_SIZE + MAX_QUEUE_INFO_SIZE + MAX_INT_INFO_SIZE + MAX_SWITCH_INFO_SIZE + MAX_MEM_INFO_SIZE + 4)

VOID OsExcRegister(ExcInfoType type, EXC_INFO_SAVE_CALLBACK func, VOID *arg);
VOID LOS_Reboot(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* _LOS_EXC_H */

