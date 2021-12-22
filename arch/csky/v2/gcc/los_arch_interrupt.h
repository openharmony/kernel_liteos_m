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

#ifndef _LOS_ARCH_INTERRUPT_H
#define _LOS_ARCH_INTERRUPT_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_interrupt.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* *
 * @ingroup los_arch_interrupt
 * Maximum number of used hardware interrupts.
 */
#ifndef OS_HWI_MAX_NUM
#define OS_HWI_MAX_NUM                        LOSCFG_PLATFORM_HWI_LIMIT
#endif

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
#define OS_HWI_PRIO_LOWEST                    3
#endif

/* *
 * @ingroup  los_arch_interrupt
 * Check the interrupt priority.
 */
#define HWI_PRI_VALID(pri)              (((pri) >= OS_HWI_PRIO_HIGHEST) && ((pri) <= OS_HWI_PRIO_LOWEST))

/* *
 * @ingroup  los_arch_interrupt
 * Define the type of a hardware interrupt vector table function.
 */
typedef VOID (**HWI_VECTOR_FUNC)(VOID);

/* *
 * @ingroup los_arch_interrupt
 * Count of interrupts.
 */
extern UINT32 g_intCount;

/* *
 * @ingroup los_arch_interrupt
 * Count of C-sky system interrupt vector.
 */
#define OS_SYS_VECTOR_CNT                     32

/* *
 * @ingroup los_arch_interrupt
 * Count of C-sky interrupt vector.
 */
#define OS_VECTOR_CNT                         (OS_SYS_VECTOR_CNT + OS_HWI_MAX_NUM)

#define PSR_VEC_OFFSET                         16U
#define VIC_REG_BASE                           0xE000E100UL

typedef struct {
    UINT32 ISER[4U];
    UINT32 RESERVED0[12U];
    UINT32 IWER[4U];
    UINT32 RESERVED1[12U];
    UINT32 ICER[4U];
    UINT32 RESERVED2[12U];
    UINT32 IWDR[4U];
    UINT32 RESERVED3[12U];
    UINT32 ISPR[4U];
    UINT32 RESERVED4[12U];
    UINT32 ISSR[4U];
    UINT32 RESERVED5[12U];
    UINT32 ICPR[4U];
    UINT32 RESERVED6[12U];
    UINT32 ICSR[4U];
    UINT32 RESERVED7[12U];
    UINT32 IABR[4U];
    UINT32 RESERVED8[60U];
    UINT32 IPR[32U];
    UINT32 RESERVED9[480U];
    UINT32 ISR;
    UINT32 IPTR;
    UINT32 TSPEND;
    UINT32 TSABR;
    UINT32 TSPR;
} VIC_TYPE;

extern VIC_TYPE *VIC_REG;
/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Invalid interrupt number.
 *
 * Value: 0x02000900
 *
 * Solution: Ensure that the interrupt number is valid.
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
 */
#define OS_ERRNO_HWI_PRIO_INVALID             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x05)

/* *
 * @ingroup los_arch_interrupt
 * Hardware interrupt error code: Incorrect interrupt creation mode.
 *
 * Value: 0x02000906
 *
 * Solution: The interrupt creation mode can be only set to OS_HWI_MODE_COMM or OS_HWI_MODE_FAST.
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
 * Hardware interrupt error code: Invalid interrupt number.
 *
 * Value: 0x02000900
 *
 * Solution: Ensure that the interrupt number is valid.
 */
#define LOS_ERRNO_HWI_NUM_INVALID             OS_ERRNO_HWI_NUM_INVALID

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
/* *
 * @ingroup los_arch_interrupt
 * Set interrupt vector table.
 */
extern VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector, VOID *arg);
#else
/* *
 * @ingroup los_arch_interrupt
 * Set interrupt vector table.
 */
extern VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector);
#endif

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
 * @brief: Get an interrupt number.
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
 * <ul><li>los_arch_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 HalIntNumGet(VOID);

/* *
 * @ingroup  los_arch_interrupt
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
 * <ul><li>los_arch_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern VOID HalHwiDefaultHandler(VOID);

#define OS_EXC_IN_INIT                      0
#define OS_EXC_IN_TASK                      1
#define OS_EXC_IN_HWI                       2

#define OS_VIC_INT_ENABLE_SIZE             0x4
#define OS_VIC_INT_WAKER_SIZE              0x4
#define OS_VIC_INT_ICER_SIZE               0x4
#define OS_VIC_INT_ISPR_SIZE               0x4
#define OS_VIC_INT_IABR_SIZE               0x4
#define OS_VIC_INT_IPR_SIZE                0x4
#define OS_VIC_INT_ISR_SIZE                0x4
#define OS_VIC_INT_IPTR_SIZE               0x4

#define OS_EXC_FLAG_FAULTADDR_VALID         0x01

#define OS_EXC_IMPRECISE_ACCESS_ADDR        0xABABABAB

/**
 * @ingroup los_exc
 * the struct of register files
 *
 * description: the register files that saved when exception triggered
 *
 * notes:the following register with prefix 'uw'  correspond to the registers in the cpu  data sheet.
 */
typedef struct TagExcContext {
    UINT32 R0;
    UINT32 R1;
    UINT32 R2;
    UINT32 R3;
    UINT32 R4;
    UINT32 R5;
    UINT32 R6;
    UINT32 R7;
    UINT32 R8;
    UINT32 R9;
    UINT32 R10;
    UINT32 R11;
    UINT32 R12;
    UINT32 R13;
    UINT32 R14;
    UINT32 R15;
    UINT32 EPSR;
    UINT32 EPC;
} EXC_CONTEXT_S;

/* *
 * @ingroup  los_arch_interrupt
 * @brief: Exception handler function.
 *
 * @par Description:
 * This API is used to handle Exception.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 * @param  excBufAddr     [IN] The address of stack pointer at which the error occurred.
 * @param  faultAddr      [IN] The address at which the error occurred.
 *
 * @retval:None.
 * @par Dependency:
 * <ul><li>los_arch_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
LITE_OS_SEC_TEXT_INIT VOID HalExcHandleEntry(EXC_CONTEXT_S *excBufAddr, UINT32 faultAddr);

VOID IrqEntry(VOID);

VOID HandleEntry(VOID);

VOID HalHwiInit(VOID);

/**
 * @ingroup los_exc
 * Exception information structure
 *
 * Description: Exception information saved when an exception is triggered on the Csky platform.
 *
 */
typedef struct TagExcInfo {
    UINT16 phase;
    UINT16 type;
    UINT32 faultAddr;
    UINT32 thrdPid;
    UINT16 nestCnt;
    UINT16 reserved;
    EXC_CONTEXT_S *context;
} ExcInfo;

extern ExcInfo g_excInfo;

#define MAX_INT_INFO_SIZE       (8 + 0x164)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_ARCH_INTERRUPT_H */
