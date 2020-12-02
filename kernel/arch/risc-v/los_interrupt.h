/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
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
#ifndef _LOS_HWI_H
#define _LOS_HWI_H

#include "los_compiler.h"
#include "los_config.h"
#include "los_context.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_hwi
 * Define the type of a hardware interrupt number.
 */
typedef UINT32  HWI_HANDLE_T;

/**
 * @ingroup los_hwi
 * Define the type of a hardware interrupt priority.
 */
typedef UINT16  HWI_PRIOR_T;

/**
 * @ingroup los_hwi
 * Define the type of hardware interrupt mode configurations.
 */
typedef UINT16  HWI_MODE_T;

/**
 * @ingroup los_hwi
 * Define the type of the parameter used for the hardware interrupt creation function.
 * The function of this parameter varies among platforms.
 */
typedef UINT32  HWI_ARG_T;

/**
 * @ingroup  los_hwi
 * Define the type of a hardware interrupt handling function.
 */
typedef VOID (*HWI_PROC_FUNC)(UINTPTR);

/**
 * @ingroup  los_hwi
 * Define the type of a hardware interrupt vector table function.
 */
typedef VOID (**HWI_VECTOR_FUNC)(VOID);

/*
 * These flags used only by the kernel as part of the
 * irq handling routines.
 *
 * IRQF_SHARED - allow sharing the irq among several devices
 */
#define IRQF_SHARED 0x8000

/**
 * @ingroup  los_hwi
 * Define the type of a hardware interrupt vector table function.
 */
typedef struct tagHwiHandleForm {
    HWI_PROC_FUNC pfnHook;
    HWI_ARG_T     uwParam;
    UINTPTR uwreserved;
} HWI_HANDLE_FORM_S;

typedef UINTPTR HWI_IRQ_PARAM_S;

typedef struct {
    UINT32      mcause;
    UINT32      mtval;
    UINT32      medeleg;
    UINT32      gp;
    TaskContext taskContext;
} LosExcContext;

typedef struct {
    UINT16 nestCnt;
    UINT16 type;
    UINT32 thrID;
    LosExcContext *context;
} LosExcInfo;

typedef enum {
    OS_EXC_TYPE_CONTEXT     = 0,
    OS_EXC_TYPE_TSK         = 1,
    OS_EXC_TYPE_QUE         = 2,
    OS_EXC_TYPE_NVIC        = 3,
    OS_EXC_TYPE_TSK_SWITCH  = 4,
    OS_EXC_TYPE_MEM         = 5,
    OS_EXC_TYPE_MAX         = 6
} ExcInfoType;

typedef UINT32 (*EXC_INFO_SAVE_CALLBACK)(UINT32, VOID*);

typedef struct {
    ExcInfoType           uwType;
    UINT32                  uwValid;
    EXC_INFO_SAVE_CALLBACK  pFnExcInfoCb;
    VOID*                   pArg;
} ExcInfoArray;

#define MAX_EXC_MEM_SIZE           0

/**
 * @ingroup los_hwi
 * Highest priority of a hardware interrupt.
 */
#define OS_HWI_PRIO_HIGHEST        7

/**
 * @ingroup los_hwi
 * Lowest priority of a hardware interrupt.
 */
#define OS_HWI_PRIO_LOWEST         1

/**
 * @ingroup los_hwi
 * Count of HimiDeer system interrupt vector.
 */
#define OS_RISCV_SYS_VECTOR_CNT   (RISCV_SYS_MAX_IRQ + 1)

/**
 * @ingroup los_hwi
 * Count of HimiDeer local interrupt vector 0 - 5, enabled by CSR mie 26 -31 bit.
 */
#define OS_RISCV_MIE_IRQ_VECTOR_CNT  6

/**
 * @ingroup los_hwi
 * Count of HimiDeer local interrupt vector 6 - 31, enabled by custom CSR locie0 0 - 25 bit.
 */
#define OS_RISCV_CUSTOM_IRQ_VECTOR_CNT  RISCV_PLIC_VECTOR_CNT

/**
 * @ingroup los_hwi
 * Count of HimiDeer local IRQ interrupt vector.
 */
#define OS_RISCV_LOCAL_IRQ_VECTOR_CNT        (OS_RISCV_MIE_IRQ_VECTOR_CNT + OS_RISCV_SYS_VECTOR_CNT)

/**
 * @ingroup los_hwi
 * Count of himideer interrupt vector.
 */
#define OS_RISCV_VECTOR_CNT                  (OS_RISCV_SYS_VECTOR_CNT + OS_RISCV_CUSTOM_IRQ_VECTOR_CNT)

/**
 * Maximum number of supported hardware devices that generate hardware interrupts.
 * The maximum number of hardware devices that generate hardware interrupts supported by hi3518ev200 is 32.
 */
#define OS_HWI_MAX_NUM        OS_RISCV_VECTOR_CNT

/**
 * Maximum interrupt number.
 */
#define OS_HWI_MAX            ((OS_HWI_MAX_NUM) - 1)

/**
 * Minimum interrupt number.
 */
#define OS_HWI_MIN            0

/**
 * Maximum usable interrupt number.
 */
#define OS_USER_HWI_MAX        OS_HWI_MAX

/**
 * Minimum usable interrupt number.
 */
#define OS_USER_HWI_MIN        OS_HWI_MIN

extern HWI_HANDLE_FORM_S g_hwiForm[OS_HWI_MAX_NUM];

extern VOID OsHwiInit(VOID);
extern UINT32 OsGetHwiFormCnt(HWI_HANDLE_T hwiNum);
extern HWI_HANDLE_FORM_S *OsGetHwiForm(VOID);
extern VOID OsHwiInterruptDone(HWI_HANDLE_T hwiNum);
extern VOID OsHwiDefaultHandler(UINTPTR arg);

extern VOID BackTraceSub(UINT32 fp);
extern VOID OsDisableIRQ(VOID);
extern VOID OsEnableIRQ(VOID);

extern VOID LOS_Panic(const CHAR *fmt, ...);
extern VOID OsBackTrace(VOID);
extern VOID OsTaskBackTrace(UINT32 taskID);
extern VOID OsExcInit(VOID);
extern VOID OsExcRegister(ExcInfoType type, EXC_INFO_SAVE_CALLBACK func, VOID *arg);

STATIC INLINE UINTPTR GetFp(VOID)
{
    UINTPTR fpSave = 0;
    __asm__ __volatile__("mv %0, s0" : "=r"(fpSave));
    return fpSave;
}

extern UINT32 g_intCount;

/**
 * @ingroup los_hwi
 * An interrupt is active.
 */
#define OS_INT_ACTIVE               (g_intCount > 0)

/**
 * @ingroup los_hwi
 * An interrupt is inactive.
 */
#define OS_INT_INACTIVE             (!(OS_INT_ACTIVE))

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Invalid interrupt number.
 *
 * Value: 0x02000900
 *
 * Solution: Ensure that the interrupt number is valid. The value range of the interrupt number applicable
 * for a risc-v platform is [0, OS_RISCV_VECTOR_CNT].
 */
#define OS_ERRNO_HWI_NUM_INVALID                 LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x00)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Null hardware interrupt handling function.
 *
 * Value: 0x02000901
 *
 * Solution: Pass in a valid non-null hardware interrupt handling function.
 */
#define OS_ERRNO_HWI_PROC_FUNC_NULL              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x01)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Insufficient interrupt resources for hardware interrupt creation.
 *
 * Value: 0x02000902
 *
 * Solution: Increase the configured maximum number of supported hardware interrupts.
 */
#define OS_ERRNO_HWI_CB_UNAVAILABLE              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x02)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Insufficient memory for hardware interrupt initialization.
 *
 * Value: 0x02000903
 *
 * Solution: Expand the configured memory.
 */
#define OS_ERRNO_HWI_NO_MEMORY                   LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x03)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: The interrupt has already been created.
 *
 * Value: 0x02000904
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_ALREADY_CREATED             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x04)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Invalid interrupt priority.
 *
 * Value: 0x02000905
 *
 * Solution: Ensure that the interrupt priority is valid.
 */
#define OS_ERRNO_HWI_PRIO_INVALID                LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x05)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: Incorrect interrupt creation mode.
 *
 * Value: 0x02000906
 *
 * Solution: The interrupt creation mode can be only set to OS_HWI_MODE_COMM or OS_HWI_MODE_FAST of which the
 * value can be 0 or 1.
 */
#define OS_ERRNO_HWI_MODE_INVALID                LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x06)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: The interrupt has already been created as a fast interrupt.
 *
 * Value: 0x02000907
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 */
#define OS_ERRNO_HWI_FASTMODE_ALREADY_CREATED    LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x07)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code: The API is called during an interrupt, which is forbidden.
 *
 * Value: 0x02000908
 *
 * * Solution: Do not call the API during an interrupt.
 */
#define OS_ERRNO_HWI_INTERR LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x08)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code:the hwi support SHARED error.
 *
 * Value: 0x02000909
 *
 * * Solution:check the input params hwiMode and irqParam of LOS_HwiCreate or LOS_HwiDelete whether adapt the current
 * hwi.
 */
#define OS_ERRNO_HWI_SHARED_ERROR LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x09)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code:Invalid interrupt Arg when interrupt mode is IRQF_SHARED.
 *
 * Value: 0x0200090a
 *
 * * Solution:check the interrupt Arg, Arg should not be NULL and pDevId should not be NULL.
 */
#define OS_ERRNO_HWI_ARG_INVALID LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x0a)

/**
 * @ingroup los_hwi
 * Hardware interrupt error code:The interrupt corresponded to the hwi number or devid  has not been created.
 *
 * Value: 0x0200090b
 *
 * * Solution:check the hwi number or devid, make sure the hwi number or devid need to delete.
 */
#define OS_ERRNO_HWI_HWINUM_UNCREATE LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x0b)

/**
 * @ingroup  los_hwi
 * @brief Create a hardware interrupt.
 *
 * @par Description:
 * This API is used to configure a hardware interrupt and register a hardware interrupt handling function.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for hardware interrupt tailoring
 * is enabled.</li>
 * <li>Hardware interrupt number value range: [OS_HWI_MIN,OS_HWI_MAX]. The value range applicable for a riscv
 * platform is [0, OS_HWI_MAX_NUM].</li>
 * <li>OS_HWI_MAX_NUM specifies the maximum number of interrupts that can be created.</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * </ul>
 *
 * @param  hwiNum     [IN] Type#HWI_HANDLE_T: hardware interrupt number. The value range applicable for a riscv
 * platform is [0, OS_HWI_MAX_NUM].
 * @param  hwiPrio    [IN] Type#HWI_PRIOR_T: hardware interrupt priority. Ignore this parameter temporarily.
 * @param  hwiMode    [IN] Type#HWI_MODE_T: hardware interrupt mode. Ignore this parameter temporarily.
 * @param  hwiHandler [IN] Type#HWI_PROC_FUNC: interrupt handler used when a hardware interrupt is triggered.
 * @param  irqParam   [IN] Type#HWI_IRQ_PARAM_S: input parameter of the interrupt handler used when a hardware
 * interrupt is triggered.
 *
 * @retval #OS_ERRNO_HWI_PROC_FUNC_NULL               0x02000901: Null hardware interrupt handling function.
 * @retval #OS_ERRNO_HWI_NUM_INVALID                  0x02000900: Invalid interrupt number.
 * @retval #OS_ERRNO_HWI_NO_MEMORY                    0x02000903: Insufficient memory for hardware interrupt creation.
 * @retval #OS_ERRNO_HWI_ALREADY_CREATED              0x02000904: The interrupt handler being created has already been
 * created.
 * @retval #LOS_OK                                    0         : The interrupt is successfully created.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V200R003C00
 */
extern UINT32 LOS_HwiCreate(HWI_HANDLE_T hwiNum,
                            HWI_PRIOR_T hwiPrio,
                            HWI_MODE_T hwiMode,
                            HWI_PROC_FUNC hwiHandler,
                            HWI_IRQ_PARAM_S irqParam);

/**
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
 * @since Huawei LiteOS V200R003C00
 */
extern UINT32 LOS_IntUnLock(VOID);

/**
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
 * @since Huawei LiteOS V200R003C00
 */
extern UINT32 LOS_IntLock(VOID);

/**
 * @ingroup los_hwi
 * @brief Restore interrupts.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to restore the CPSR value obtained before all interrupts are disabled.</li>
 * </ul>
 * @attention
 * <ul>
 * <li>This API can be called only after all interrupts are disabled, and the input parameter value should be the
 * value returned by calling the all interrupt disabling API.</li>
 * </ul>
 *
 * @param intSave [IN] CPSR value obtained before all interrupts are disabled.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_IntLock
 * @since Huawei LiteOS V200R003C00
 */
extern VOID LOS_IntRestore(UINT32 intSave);

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

/**
 * @ingroup  los_hwi
 * @brief Delete hardware interrupt.
 *
 * @par Description:
 * This API is used to delete hardware interrupt.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for hardware interrupt tailoring
 * is enabled.</li>
 * <li>Hardware interrupt number value range: [OS_HWI_MIN, OS_HWI_MAX]. The value range applicable for a riscv
 * platform is [0, OS_HWI_MAX_NUM].</li>
 * <li>OS_HWI_MAX_NUM specifies the maximum number of interrupts that can be created.</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * </ul>
 *
 * @param  hwiNum  [IN] Type#HWI_HANDLE_T: hardware interrupt number. The value range applicable for a riscv
 * platform is [0, OS_HWI_MAX_NUM].
 * @param irqParam [IN] Type#HWI_IRQ_PARAM_S: id of hardware interrupt which will base on when delete the hardware
 * interrupt.
 *
 * @retval #OS_ERRNO_HWI_NUM_INVALID              0x02000900: Invalid interrupt number.
 * @retval #LOS_OK                                0: The interrupt is successfully delete.
 * @par Dependency:
 * <ul><li>los_hwi.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V200R003C00
 */
extern UINT32 LOS_HwiDelete(HWI_HANDLE_T hwiNum, HWI_IRQ_PARAM_S irqParam);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_HWI_H */
