/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_INTERRUPT_H
#define _LOS_INTERRUPT_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_error.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * An interrupt is active.
 */
extern size_t IntActive(VOID);

/**
 * @ingroup los_hwi
 * It is used to check whether there are active interrupts or not.
 *
 * @see OS_INT_INACTIVE
 */
#define OS_INT_ACTIVE (IntActive() != 0)

/**
 * @ingroup los_hwi
 * Check whether there are active interrupts or not.
 * The API returns a boolean value. True means no active interrupts on the current CPU.
 * False means that there are active interrupts on the current CPU.
 *
 * @see OS_INT_ACTIVE
 */
#define OS_INT_INACTIVE (!(OS_INT_ACTIVE))

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Invalid interrupt number.
 *
 * Value: 0x02000900.
 *
 * Solution: Ensure that the interrupt number is valid.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_NUM_INVALID               LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x00)
#define OS_ERRNO_HWI_NUM_INVALID                LOS_ERRNO_HWI_NUM_INVALID

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Null hardware interrupt handling function.
 *
 * Value: 0x02000901.
 *
 * Solution: Pass in a valid non-null hardware interrupt handling function.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_PROC_FUNC_NULL            LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x01)
#define OS_ERRNO_HWI_PROC_FUNC_NULL             LOS_ERRNO_HWI_PROC_FUNC_NULL

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Insufficient interrupt resources for hardware interrupt creation.
 *
 * Value: 0x02000902.
 *
 * Solution: Increase the configured maximum number of supported hardware interrupts.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_CB_UNAVAILABLE            LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x02)
#define OS_ERRNO_HWI_CB_UNAVAILABLE             LOS_ERRNO_HWI_CB_UNAVAILABLE

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Insufficient memory for hardware interrupt initialization.
 *
 * Value: 0x02000903.
 *
 * Solution: Expand the configured memory.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_NO_MEMORY                 LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x03)
#define OS_ERRNO_HWI_NO_MEMORY                  LOS_ERRNO_HWI_NO_MEMORY

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: The interrupt has already been created.
 *
 * Value: 0x02000904.
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_ALREADY_CREATED           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x04)
#define OS_ERRNO_HWI_ALREADY_CREATED            LOS_ERRNO_HWI_ALREADY_CREATED

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Invalid interrupt priority.
 *
 * Value: 0x02000905.
 *
 * Solution: Ensure that the interrupt priority is valid.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_PRIO_INVALID              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x05)
#define OS_ERRNO_HWI_PRIO_INVALID               LOS_ERRNO_HWI_PRIO_INVALID

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Incorrect interrupt creation mode.
 *
 * Value: 0x02000906.
 *
 * Solution: The interrupt creation mode can be only set to OS_HWI_MODE_COMM or OS_HWI_MODE_FAST.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_MODE_INVALID              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x06)
#define OS_ERRNO_HWI_MODE_INVALID               LOS_ERRNO_HWI_MODE_INVALID

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: The interrupt has already been created as a fast interrupt.
 *
 * Value: 0x02000907.
 *
 * Solution: Check whether the interrupt specified by the passed-in interrupt number has already been created.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_FASTMODE_ALREADY_CREATED  LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x07)
#define OS_ERRNO_HWI_FASTMODE_ALREADY_CREATED   LOS_ERRNO_HWI_FASTMODE_ALREADY_CREATED

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Invalid interrupt operation.
 *
 * Value: 0x02000908.
 *
 * Solution: Ensure the interrupt operation is valid.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_INTERR                    LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x08)
#define OS_ERRNO_HWI_INTERR                     LOS_ERRNO_HWI_INTERR

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: the hardware interrupt supports SHARED error.
 *
 * Value: 0x02000909.
 *
 * Solution: Check the input params hwiMode and irqParam of LOS_HwiCreate or
 * LOS_HwiDelete whether adapt the current hardware interrupt.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_SHARED_ERROR              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x09)
#define OS_ERRNO_HWI_SHARED_ERROR               LOS_ERRNO_HWI_SHARED_ERROR

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Invalid interrupt argument.
 *
 * Value: 0x0200090a.
 *
 * Solution: Ensure that the interrupt argument is valid.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_ARG_INVALID               LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x0a)
#define OS_ERRNO_HWI_ARG_INVALID                LOS_ERRNO_HWI_ARG_INVALID

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: The interrupt corresponded to the hardware interrupt number
 * or devid has not been created.
 *
 * Value: 0x0200090b.
 *
 * Solution: Check the irqParam->pDevId of LOS_HwiDelete, make sure the devid need to delete.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_HWINUM_UNCREATE           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x0b)
#define OS_ERRNO_HWI_HWINUM_UNCREATE            LOS_ERRNO_HWI_HWINUM_UNCREATE

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Invalid interrupt operation function.
 *
 * Value: 0x0200090c.
 *
 * Solution: Set a valid interrupt operation function.
 * @attention
 * <ul><li>Please use macros starting with LOS, and macros starting with OS will not be supported.</li></ul>
 */
#define LOS_ERRNO_HWI_OPS_FUNC_NULL             LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x0c)
#define OS_ERRNO_HWI_OPS_FUNC_NULL              LOS_ERRNO_HWI_OPS_FUNC_NULL

/**
 * @ingroup los_interrupt
 * interrupt bottom half error code: The caller is not in interrupt context.
 *
 * Value: 0x0200090d.
 *
 * Solution: Check the caller of LOS_HwiBhworkAdd, make sure the caller in interrupt context, rather than task context.
 * @attention
 * <ul><li>None.</li></ul>
 */
#define LOS_ERRNO_HWI_NOT_INTERRUPT_CONTEXT     LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x0d)

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: Null pointer.
 *
 * Value: 0x0200090e.
 *
 * Solution: Change the passed-in null pointer to a valid non-null pointer.
 * @attention
 * <ul><li>None.</li></ul>
 */
#define LOS_ERRNO_HWI_PTR_NULL                  LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x0e)

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: create hwi with arg when LOSCFG_PLATFORM_HWI_WITH_ARG not enabled.
 *
 * Value: 0x0200090f.
 *
 * Solution: Pass NULL to the last parameter of LOS_HwiCreate or enable LOSCFG_PLATFORM_HWI_WITH_ARG.
 * @attention
 * <ul><li>None.</li></ul>
 */
#define LOS_ERRNO_HWI_ARG_NOT_ENABLED           LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x0f)

/**
 * @ingroup los_interrupt
 * Hardware interrupt error code: set hwi affinity with invalid cpu mask arguments.
 *
 * Value: 0x02000910.
 *
 * Solution: Pass valid cpuMask arg to the LOS_HwiSetAffinity.
 * @attention
 * <ul><li>The range of available cpuMask may vary accroding to different interrupt controller.</li></ul>
 */
#define LOS_ERRNO_HWI_AFFI_INVALID              LOS_ERRNO_OS_ERROR(LOS_MOD_HWI, 0x10)

/**
 * @ingroup los_interrupt
 * Define the type HWI_HANDLE_T for a hardware interrupt number, the type is an unsigned int.
 */
typedef UINT32 HWI_HANDLE_T;

/**
 * @ingroup los_interrupt
 * Define the type HWI_PRIOR_T for a hardware interrupt priority, the type is an unsigned short.
 */
typedef UINT16 HWI_PRIOR_T;

/**
 * @ingroup los_interrupt
 * Define the type HWI_MODE_T for hardware interrupt mode configurations, the type is an unsigned short.
 */
typedef UINT16 HWI_MODE_T;

/**
 * @ingroup los_interrupt
 * Define the type HWI_ARG_T for the parameter used for the hardware interrupt creation function.
 * The function of this parameter varies among platforms.
 */
typedef UINTPTR HWI_ARG_T;

/**
 * @ingroup  los_interrupt
 * @brief Define the type of a hardware interrupt handling function.
 *
 * @par Description:
 * This definition is used to declare the type of a hardware interrupt handling function.
 * It is used as the handler parameter type of ArchHwiCreate.
 * @attention
 * None.
 *
 * @param parm [IN] Type #VOID *. Parameter passed to the interrupt handler
 *                  (only available when LOSCFG_PLATFORM_HWI_WITH_ARG is 1).
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see ArchHwiCreate
 */
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
typedef VOID (*HWI_PROC_FUNC)(VOID *parm);
#else
typedef VOID (*HWI_PROC_FUNC)(void);
#endif

/**
 * @ingroup  los_interrupt
 * @brief Define the type of an interrupt bottom half function.
 *
 * @par Description:
 * This definition is used to declare the type of an interrupt bottom half handling function.
 * It will be used when calling LOS_HwiBhworkAdd.
 * @attention
 * None.
 *
 * @param data [IN] Type #VOID *. Input parameter of the bottom half handler.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_HwiBhworkAdd
 */
typedef VOID (*HWI_BOTTOM_HALF_FUNC)(VOID *);

/**
 * @ingroup  los_interrupt
 * The flag means the IRQ is allowed to share among several devices.
 *
 * The flag only used by the kernel as part of the IRQ handling routines.
 */
#define IRQF_SHARED 0x8000U

/**
 * @ingroup  los_interrupt
 * The hardware interrupt parameter for #LOS_HwiDelete and interrupt handler in #LOS_HwiCreate.
 */
typedef struct tagIrqParam {
    int swIrq;         /**< The interrupt number */
    VOID *pDevId;      /**< The pointer to the device ID that launches the interrupt */
    const CHAR *pName; /**< The interrupt name */
} HwiIrqParam;

/* Alias: HWI_IRQ_PARAM_S is the canonical name for the interrupt parameter struct. */
typedef HwiIrqParam HWI_IRQ_PARAM_S;

typedef struct {
    UINT32 (*triggerIrq)(HWI_HANDLE_T hwiNum);
    UINT32 (*clearIrq)(HWI_HANDLE_T hwiNum);
    UINT32 (*enableIrq)(HWI_HANDLE_T hwiNum);
    UINT32 (*disableIrq)(HWI_HANDLE_T hwiNum);
    UINT32 (*setIrqPriority)(HWI_HANDLE_T hwiNum, UINT8 priority);
    UINT32 (*getCurIrqNum)(VOID);
    UINT32 (*createIrq)(HWI_HANDLE_T hwiNum, HWI_PRIOR_T hwiPrio);
    VOID *(*getHandleForm)(HWI_HANDLE_T hwiNum);
} HwiControllerOps;

#if (LOSCFG_HWI_PRE_POST_PROCESS == 1)
typedef enum {
    HWI_PRE_PROC_HOOK,
    HWI_POST_PROC_HOOK,
    HWI_PROC_HOOK_BUTT
} HwiProcHookType;

typedef VOID (*HWI_PROC_HOOK)(HWI_HANDLE_T hwiNum);

/**
 * @ingroup  los_interrupt
 * @brief Register a pre-processing interrupt hook.
 *
 * @par Description:
 * This API is used to register a hook function that is called before
 * the interrupt handler is dispatched.
 *
 * @param intPreHook [IN] Type #HWI_PROC_HOOK. The pre-processing hook function.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 */
extern VOID LOS_HwiPreHookReg(HWI_PROC_HOOK intPreHook);

/**
 * @ingroup  los_interrupt
 * @brief Register a post-processing interrupt hook.
 *
 * @par Description:
 * This API is used to register a hook function that is called after
 * the interrupt handler is dispatched.
 *
 * @param intPostHook [IN] Type #HWI_PROC_HOOK. The post-processing hook function.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 */
extern VOID LOS_HwiPostHookReg(HWI_PROC_HOOK intPostHook);
#endif

/* stack protector */
extern UINT32 __stack_chk_guard;
extern VOID __stack_chk_fail(VOID);
#if (LOSCFG_DEBUG_TOOLS == 1)
extern UINT32 OsGetHwiFormCnt(UINT32 index);
extern CHAR *OsGetHwiFormName(UINT32 index);
extern BOOL OsHwiIsCreated(UINT32 index);
#endif

/**
 * @ingroup  los_interrupt
 * @brief Create a hardware interrupt.
 *
 * @par Description:
 * This API is used to configure a hardware interrupt and register a hardware interrupt handling function.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for
 * hardware interrupt tailoring is enabled.</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * <li>The parameter handler of this interface is a interrupt handler, it should be correct, otherwise,
 * the system may be abnormal.</li>
 * <li>The input irqParam could be NULL, if not, it should be address which point to a struct HWI_IRQ_PARAM_S,
 * the parameter pDevId and pName should be constant.</li>
 * <li>A smaller value indicates a higher interrupt priority, the interrupt processor is modified uniformly.</li>
 * </ul>
 *
 * @param  hwiNum     [IN] Type #HWI_HANDLE_T. The hardware interrupt number. The value range is
 *                                               [OS_USER_HWI_MIN, OS_USER_HWI_MAX].
 * @param  hwiPrio    [IN] Type #HWI_PRIOR_T. The hardware interrupt priority. The value range is
 *                                            [OS_HWI_PRIO_HIGHEST, OS_HWI_PRIO_LOWEST].
 * @param  hwiMode    [IN] Type #HWI_MODE_T. The hardware interrupt mode.
 * @param  handler    [IN] Type #HWI_PROC_FUNC. The interrupt handler used when a hardware interrupt is triggered.
 * @param  irqParam   [IN] Type #HWI_IRQ_PARAM_S. The input parameter of the interrupt handler used when
 *                                                a hardware interrupt is triggered.
 *
 * @retval #LOS_ERRNO_HWI_PROC_FUNC_NULL      Null hardware interrupt handling function.
 * @retval #LOS_ERRNO_HWI_NUM_INVALID         Invalid interrupt number.
 * @retval #LOS_ERRNO_HWI_NO_MEMORY           Insufficient memory for hardware interrupt creation.
 * @retval #LOS_ERRNO_HWI_ALREADY_CREATED     The interrupt handler being created has already been created.
 * @retval #LOS_ERRNO_HWI_SHARED_ERROR        The interrupt can not be shared. The interrupt number has been
 *                                            registered as a non-shared interrupt, or a shared interrupt is
 *                                            specified to be created, but the device ID is empty.
 * @retval #LOS_ERRNO_HWI_ARG_NOT_ENABLED     Passed non-NULL to irqParam and LOSCFG_PLATFORM_HWI_WITH_ARG not enabled.
 * @retval #LOS_OK                            The interrupt is successfully created.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_HwiDelete
 */
extern UINT32 LOS_HwiCreate(HWI_HANDLE_T hwiNum,
					        HWI_PRIOR_T hwiPrio,
					        HWI_MODE_T hwiMode,
					        HWI_PROC_FUNC handler,
					        HWI_IRQ_PARAM_S *irqParam);
/**
 * @ingroup  los_interrupt
 * @brief delete a hardware interrupt.
 *
 * @par Description:
 * This API is used to delete a hardware interrupt.
 *
 * @attention
 * <ul>
 * <li>The hardware interrupt module is usable only when the configuration item for
 * hardware interrupt tailoring is enabled.</li>
 * <li>Hardware interrupt number value range: [OS_USER_HWI_MIN, OS_USER_HWI_MAX].</li>
 * <li>Before executing an interrupt on a platform, refer to the chip manual of the platform.</li>
 * <li>Do not call this API in interrupt handler,
 * otherwise, invalid memory may be accessed in interrupt share mode.</li>
 * </ul>
 *
 * @param  hwiNum   [IN] Type #HWI_HANDLE_T. The hardware interrupt number.
 * @param  irqParam [IN] Type #HWI_IRQ_PARAM_S *. ID of hardware interrupt which will base on
 *                                                when delete the hardware interrupt.
 *
 * @retval #LOS_ERRNO_HWI_NUM_INVALID         Invalid interrupt number.
 * @retval #LOS_ERRNO_HWI_SHARED_ERROR        The interrupt number is a shared interrupt, but the device ID of the
 *                                            shared interrupt to be deleted is not specified.
 * @retval #LOS_ERRNO_HWI_HWINUM_UNCREATE     The interrupt corresponded to the hwiNum(
 *                                            the hardware interrupt number) or
 *                                            irqParam->pDevId(the interrupt device id)
 *                                            has not been created.
 * @retval #LOS_ERRNO_HWI_PROC_FUNC_NULL      Not supported disable interrupt.
 * @retval #LOS_OK                            The interrupt is successfully deleted.
 *
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_HwiCreate
 */
extern UINT32 LOS_HwiDelete(HWI_HANDLE_T hwiNum, HWI_IRQ_PARAM_S *irqParam);
/**
 * @ingroup los_interrupt
 * @brief Create an interrupt bottom half work.
 *
 * @par Description:
 * This API is used to configure a bottom half work and register an interrupt bottom half handling function.
 * @attention
 * <ul>
 * <li>The input data could be NULL if it's not necessary for bhHandler.</li>
 * <li>This function is defined only when LOSCFG_HWI_BOTTOM_HALF is defined.</li>
 * </ul>
 *
 * @param bhHandler [IN] Type #HWI_BOTTOM_HALF_FUNC. Bottom half interrupt handler.
 * @param data      [IN] Type #VOID *. The input parameter of the interrupt bottom half handler.
 *
 * @retval #LOS_ERRNO_HWI_PROC_FUNC_NULL         Null hardware interrupt bottom half handling function.
 * @retval #LOS_ERRNO_HWI_NOT_INTERRUPT_CONTEXT  Not in interrupt context.
 * @retval #LOS_ERRNO_HWI_NO_MEMORY              Insufficient memory for interrupt bottom half.
 * @retval #LOS_OK                                The interrupt bottom half work is successfully registered.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_HwiBhworkAdd(HWI_BOTTOM_HALF_FUNC bhHandler, VOID *data);

#ifdef LOSCFG_KERNEL_SMP
/**
 * @ingroup los_interrupt
 * @brief Send inter-core interrupts to designated cores.
 *
 * @par Description:
 * Send inter-core interrupts to designated cores.
 * @attention
 * <ul>
 * <li>This function depends on the hardware implementation of the interrupt
 * controller and CPU architecture. Only used in SMP architecture.</li>
 * <li>Current implementation is a stub and always returns
 * LOS_ERRNO_HWI_ARG_NOT_ENABLED.</li>
 * </ul>
 *
 * @param hwiNum   [IN] Type #HWI_HANDLE_T. Hardware interrupt number.
 * @param cpuMask  [IN] Type #UINT32. CPU number.
 *
 * @retval #LOS_ERRNO_HWI_ARG_NOT_ENABLED  Inter-core interrupt is not supported.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_HwiSendIpi(HWI_HANDLE_T hwiNum, UINT32 cpuMask);

/**
 * @ingroup los_interrupt
 * @brief Interrupt response specified CPU processing.
 *
 * @par Description:
 * Interrupt response specified CPU processing.
 * @attention
 * <ul>
 * <li>This function depends on the hardware implementation of the interrupt
 * controller and CPU architecture. Only used in SMP architecture.</li>
 * <li>Current implementation is a stub and always returns
 * LOS_ERRNO_HWI_AFFI_INVALID.</li>
 * </ul>
 *
 * @param hwiNum   [IN] Type #HWI_HANDLE_T. The hardware interrupt number.
 * @param cpuMask  [IN] Type #UINT32. The CPU number.
 *
 * @retval #LOS_ERRNO_HWI_AFFI_INVALID  Invalid CPU affinity.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_HwiSetAffinity(HWI_HANDLE_T hwiNum, UINT32 cpuMask);
#endif

/**
 * @ingroup los_interrupt
 * @brief Get the number of interrupt responses.
 *
 * @par Description:
 * Get the number of responses to a specified interrupt.
 * @attention
 * <ul>
 * <li>The number of interrupt responses is always changed, so this API just provides a reference value.</li>
 * <li>The type of interrupt response is UINT32, which may cause data overflow.</li>
 * </ul>
 *
 * @param hwiNum     [IN] Type #HWI_HANDLE_T. Hardware interrupt number.
 * @param respCount  [OUT] Type #UINT32 *. A pointer is used to store the number of interrupt responses.
 *
 * @retval #LOS_ERRNO_HWI_PTR_NULL       The passed-in respCount value is NULL.
 * @retval #LOS_ERRNO_HWI_NUM_INVALID    Invalid interrupt number.
 * @retval #LOS_OK                       Number of times that interrupt responses are successfully obtained.
 * @par Dependency:
 * <ul><li>los_interrupt.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 */
extern UINT32 LOS_HwiRespCntGet(HWI_HANDLE_T hwiNum, UINT32 *respCount);

UINT32 ArchIntLock(VOID);
UINT32 ArchIntUnlock(VOID);
VOID ArchIntRestore(UINT32 intSave);
UINT32 ArchIntTrigger(HWI_HANDLE_T hwiNum);
UINT32 ArchIntEnable(HWI_HANDLE_T hwiNum);
UINT32 ArchIntDisable(HWI_HANDLE_T hwiNum);
UINT32 ArchIntClear(HWI_HANDLE_T hwiNum);
UINT32 ArchIntSetPriority(HWI_HANDLE_T hwiNum, HWI_PRIOR_T priority);
UINT32 ArchIntCurIrqNum(VOID);
HwiControllerOps *ArchIntOpsGet(VOID);

#define LOS_IntLock             ArchIntLock
#define LOS_IntRestore          ArchIntRestore
#define LOS_IntUnLock           ArchIntUnlock
#define LOS_HwiTrigger          ArchIntTrigger
#define LOS_HwiClear            ArchIntClear
#define LOS_HwiEnable           ArchIntEnable
#define LOS_HwiDisable          ArchIntDisable
#define LOS_HwiSetPriority      ArchIntSetPriority
#define LOS_HwiCurIrqNum        ArchIntCurIrqNum
#define LOS_HwiOpsGet           ArchIntOpsGet

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_INTERRUPT_H */
