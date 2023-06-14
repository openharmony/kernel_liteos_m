/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_ARCH_INTERRUPT_H
#define _LOS_ARCH_INTERRUPT_H

#include "los_common_interrupt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* *
 * @ingroup los_arch_interrupt
 * Highest priority of a hardware interrupt.
 */
#ifndef OS_HWI_PRIO_HIGHEST
#define OS_HWI_PRIO_HIGHEST                   0
#endif

/* *
 * @ingroup los_arch_interrupt
 * Lowest priority of a hardware interrupt.
 */
#ifndef OS_HWI_PRIO_LOWEST
#define OS_HWI_PRIO_LOWEST                    7
#endif

/* *
 * @ingroup los_arch_interrupt
 * Count of M-Core system interrupt vector.
 */
#define OS_SYS_VECTOR_CNT                     16

/* *
 * @ingroup los_arch_interrupt
 * Count of M-Core interrupt vector.
 */
#define OS_VECTOR_CNT                         (OS_SYS_VECTOR_CNT + OS_HWI_MAX_NUM)

/* *
 * @ingroup los_arch_interrupt
 * AIRCR register priority group parameter .
 */
#define OS_NVIC_AIRCR_PRIGROUP                7

/* *
 * @ingroup los_arch_interrupt
 * Boot interrupt vector table.
 */
extern UINT32 _BootVectors[];

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Invalid interrupt number.
 *
 * Value: 0x02000900
 *
 * Solution: Ensure that the interrupt number is valid.
 * The value range of the interrupt number applicable for a Cortex-M7  platformis [OS_USER_HWI_MIN,OS_USER_HWI_MAX].
 */
#define OS_ERRNO_HWI_NUM_INVALID              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x00)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Null hardware interrupt handling function.
 *
 * Value: 0x02000901
 *
 * Solution: Pass in a valid non-null hardware interrupt handling function.
 */
#define OS_ERRNO_HWI_PROC_FUNC_NULL           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x01)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Insufficient interrupt resources for hardware interrupt creation.
 *
 * Value: 0x02000902
 *
 * Solution: Increase the configured maximum number of supported hardware interrupts.
 */
#define OS_ERRNO_HWI_CB_UNAVAILABLE           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x02)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Insufficient memory for hardware interrupt initialization.
 *
 * Value: 0x02000903
 *
 * Solution: Expand the configured memory.
 */
#define OS_ERRNO_HWI_NO_MEMORY                LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x03)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: The interrupt has already been created.
 *
 * Value: 0x02000904
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_ALREADY_CREATED          LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x04)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Invalid interrupt priority.
 *
 * Value: 0x02000905
 *
 * Solution: Ensure that the interrupt priority is valid.
 * The value range of the interrupt priority applicable for a Cortex-M7 platform is [0,15].
 */
#define OS_ERRNO_HWI_PRIO_INVALID             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x05)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Incorrect interrupt creation mode.
 *
 * Value: 0x02000906
 *
 * Solution: The interrupt creation mode can be only set to OS_HWI_MODE_COMM or
 * OS_HWI_MODE_FAST of which the value can be 0 or 1.
 */
#define OS_ERRNO_HWI_MODE_INVALID             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x06)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: The interrupt has already been created as a fast interrupt.
 *
 * Value: 0x02000907
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_FASTMODE_ALREADY_CREATED LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x07)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Invalid interrupt operation function.
 *
 * Value: 0x02000908
 *
 * Solution: Set a valid interrupt operation function
 */
#define OS_ERRNO_HWI_OPS_FUNC_NULL            LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x08)

/* *
 * @ingroup los_arch_interrupt
 * SysTick control and status register.
 */
#define OS_SYSTICK_CONTROL_REG                0xE000E010

/* *
 * @ingroup los_hw
 * SysTick current value register.
 */
#define OS_SYSTICK_CURRENT_REG                0xE000E018

/* *
 * @ingroup los_arch_interrupt
 * Interrupt Priority-Level Registers.
 */
#define OS_NVIC_PRI_BASE                      0xE000E400

/* *
 * @ingroup los_arch_interrupt
 * Interrupt enable register for 0-31.
 */
#define OS_NVIC_SETENA_BASE                   0xE000E100

/* *
 * @ingroup los_arch_interrupt
 * interrupt pending register.
 */
#define OS_NVIC_SETPEND_BASE                  0xE000E200

/* *
 * @ingroup los_arch_interrupt
 * Interrupt active register.
 */
#define OS_NVIC_INT_ACT_BASE                  0xE000E300

/* *
 * @ingroup los_arch_interrupt
 * Interrupt disable register for 0-31.
 */
#define OS_NVIC_CLRENA_BASE                   0xE000E180

/* *
 * @ingroup los_arch_interrupt
 * Interrupt control and status register.
 */
#define OS_NVIC_INT_CTRL                      0xE000ED04

/* *
 * @ingroup los_arch_interrupt
 * Vector table offset register.
 */
#define OS_NVIC_VTOR                          0xE000ED08

/* *
 * @ingroup los_arch_interrupt
 * Application interrupt and reset control register
 */
#define OS_NVIC_AIRCR                         0xE000ED0C

/* *
 * @ingroup los_arch_interrupt
 * System exception priority register.
 */
#define OS_NVIC_EXCPRI_BASE                   0xE000ED18

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 1 :reset.
 */
#define OS_EXC_RESET                          1

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 2 :Non-Maskable Interrupt.
 */
#define OS_EXC_NMI                            2

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 3 :(hard)fault.
 */
#define OS_EXC_HARD_FAULT                     3

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 4 :MemManage fault.
 */
#define OS_EXC_MPU_FAULT                      4

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 5 :Bus fault.
 */
#define OS_EXC_BUS_FAULT                      5

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 6 :Usage fault.
 */
#define OS_EXC_USAGE_FAULT                    6

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 11 :SVCall.
 */
#define OS_EXC_SVC_CALL                       11

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 12 :Debug monitor.
 */
#define OS_EXC_DBG_MONITOR                    12

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 14 :PendSV.
 */
#define OS_EXC_PEND_SV                        14

/* *
 * @ingroup los_arch_interrupt
 * Interrupt No. 15 :SysTick.
 */
#define OS_EXC_SYS_TICK                       15

/* *
 * @ingroup  los_arch_interrupt
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
 * <ul><li>los_arch_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern VOID HalInterrupt(VOID);

/* *
 * @ingroup  los_arch_interrupt
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
 * <ul><li>los_arch_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern VOID Reset_Handler(VOID);

/* *
 * @ingroup  los_arch_interrupt
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
 * <ul><li>los_arch_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern VOID HalPendSV(VOID);

#define OS_EXC_MAX_BUF_LEN                  25
#define OS_EXC_MAX_NEST_DEPTH               1
#define OS_EXC_FLAG_NO_FLOAT                0x10000000

#define OS_NVIC_SHCSR                       0xE000ED24
#define OS_NVIC_CCR                         0xE000ED14
#define OS_NVIC_INT_ENABLE_SIZE             0x20
#define OS_NVIC_INT_PRI_SIZE                0xF0
#define OS_NVIC_EXCPRI_SIZE                 0xC
#define OS_NVIC_INT_CTRL_SIZE               4
#define OS_NVIC_SHCSR_SIZE                  4
#define OS_NVIC_INT_PEND_SIZE               OS_NVIC_INT_ACT_SIZE
#define OS_NVIC_INT_ACT_SIZE                OS_NVIC_INT_ENABLE_SIZE

/**
 * @ingroup los_exc
 * the struct of register files
 *
 * description: the register files that saved when exception triggered
 *
 * notes:the following register with prefix 'uw'  correspond to the registers in the cpu  data sheet.
 */
typedef struct TagExcContext {
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
    /* auto save */
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
} EXC_CONTEXT_S;

typedef VOID (*EXC_PROC_FUNC)(UINT32, EXC_CONTEXT_S *);
VOID HalExcHandleEntry(UINT32 excType, UINT32 faultAddr, UINT32 pid, EXC_CONTEXT_S *excBufAddr);
VOID HalExcNMI(VOID);
VOID HalExcHardFault(VOID);
VOID HalExcMemFault(VOID);
VOID HalExcBusFault(VOID);
VOID HalExcUsageFault(VOID);
VOID HalExcSvcCall(VOID);
VOID HalHwiInit(VOID);

/**
 * @ingroup los_exc
 * Cortex-M exception types: An error occurred while the bus status register was being pushed.
 */
#define OS_EXC_BF_STKERR           1

/**
 * @ingroup los_exc
 * Cortex-M exception types: An error occurred while the bus status register was out of the stack.
 */
#define OS_EXC_BF_UNSTKERR         2

/**
 * @ingroup los_exc
 * Cortex-M exception types: Bus status register imprecise data access violation.
 */
#define OS_EXC_BF_IMPRECISERR      3

/**
 * @ingroup los_exc
 * Cortex-M exception types: Bus status register exact data access violation.
 */
#define OS_EXC_BF_PRECISERR        4

/**
 * @ingroup los_exc
 * Cortex-M exception types: Bus status register access violation while pointing.
 */
#define OS_EXC_BF_IBUSERR          5

/**
 * @ingroup los_exc
 * Cortex-M exception types: An error occurred while the memory management status register was being pushed.
 */
#define OS_EXC_MF_MSTKERR          6

/**
 * @ingroup los_exc
 * Cortex-M exception types: An error occurred while the memory management status register was out of the stack.
 */
#define OS_EXC_MF_MUNSTKERR        7

/**
 * @ingroup los_exc
 * Cortex-M exception types: Memory management status register data access violation.
 */
#define OS_EXC_MF_DACCVIOL         8

/**
 * @ingroup los_exc
 * Cortex-M exception types: Memory management status register access violation.
 */
#define OS_EXC_MF_IACCVIOL         9


/**
 * @ingroup los_exc
 * Cortex-M exception types: Incorrect usage indicating that the divisor is zero during the division operation.
 */
#define OS_EXC_UF_DIVBYZERO        10

/**
 * @ingroup los_exc
 * Cortex-M exception types: Usage error, error caused by unaligned access.
 */
#define OS_EXC_UF_UNALIGNED        11

/**
 * @ingroup los_exc
 * Cortex-M exception types: Incorrect usage attempting to execute coprocessor related instruction.
 */
#define OS_EXC_UF_NOCP             12

/**
 * @ingroup los_exc
 * Cortex-M exception types: Usage error attempting to load EXC_RETURN to PC illegally on exception return.
 */
#define OS_EXC_UF_INVPC            13

/**
 * @ingroup los_exc
 * Cortex-M exception types: Incorrect usage, attempting to cut to ARM state.
 */
#define OS_EXC_UF_INVSTATE         14

/**
 * @ingroup los_exc
 * Cortex-M exception types: Incorrect usage. Executed instruction whose code is undefined.
 */
#define OS_EXC_UF_UNDEFINSTR       15

/**
 * @ingroup los_exc
 * Cortex-M exception types: NMI
 */

#define OS_EXC_CAUSE_NMI           16

/**
 * @ingroup los_exc
 * Cortex-M exception types: hard fault
 */
#define OS_EXC_CAUSE_HARDFAULT     17

/**
 * @ingroup los_exc
 * Cortex-M exception types: The task handler exits.
 */
#define OS_EXC_CAUSE_TASK_EXIT     18

/**
 * @ingroup los_exc
 * Cortex-M exception types: A fatal error.
 */
#define OS_EXC_CAUSE_FATAL_ERR     19

/**
 * @ingroup los_exc
 * Cortex-M exception types: Hard Fault caused by a debug event.
 */
#define OS_EXC_CAUSE_DEBUGEVT      20

/**
 * @ingroup los_exc
 * Cortex-M exception types: A hard fault that occurs when a quantity is oriented.
 */
#define OS_EXC_CAUSE_VECTBL        21

/**
 * @ingroup los_exc
 * Exception information structure
 *
 * Description: Exception information saved when an exception is triggered on the Cortex-M7 platform.
 *
 */
typedef struct TagExcInfo {
    /**< Exception occurrence phase: 0 means that an exception occurs in initialization,
     * 1 means that an exception occurs in a task, and 2 means that an exception occurs in an interrupt */
    UINT16 phase;
    /**< Exception type. When exceptions occur, check the numbers 1 - 21 listed above */
    UINT16 type;
    /**< If the exact address access error indicates the wrong access address when the exception occurred */
    UINT32 faultAddr;
    /**< An exception occurs in an interrupt, indicating the interrupt number.
     * An exception occurs in the task, indicating the task ID, or 0xFFFFFFFF if it occurs during initialization */
    UINT32 thrdPid;
    /**< Number of nested exceptions. Currently only registered hook functions are supported
     * when an exception is entered for the first time */
    UINT16 nestCnt;
    /**< reserve */
    UINT16 reserved;
    /**< Hardware context at the time an exception to the automatic stack floating-point register occurs */
    EXC_CONTEXT_S *context;
} ExcInfo;

extern ExcInfo g_excInfo;
extern UINT32 g_curNestCount;
extern UINT8 g_uwExcTbl[32];

#define MAX_INT_INFO_SIZE       (8 + 0x164)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_ARCH_INTERRUPT_H */
