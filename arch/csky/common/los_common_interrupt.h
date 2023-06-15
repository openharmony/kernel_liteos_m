/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_COMMON_INTERRUPT_H
#define _LOS_COMMON_INTERRUPT_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_interrupt.h"
#include "los_error.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* *
 * @ingroup  los_arch_interrupt
 * Define the type of a hardware interrupt vector table function.
 */
typedef VOID (**HWI_VECTOR_FUNC)(VOID);

/* *
 * @ingroup los_arch_interrupt
 * Count of interrupts.
 */
extern volatile UINT32 g_intCount;

/* *
 * @ingroup los_arch_interrupt
 * Maximum number of used hardware interrupts.
 */
#ifndef OS_HWI_MAX_NUM
#define OS_HWI_MAX_NUM                        LOSCFG_PLATFORM_HWI_LIMIT
#endif

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
/* *
 * @ingroup los_arch_interrupt
 * Set interrupt vector table.
 */
extern VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector, VOID *arg);
extern HWI_HANDLER_FUNC g_hwiHandlerForm[];
#else
/* *
 * @ingroup los_arch_interrupt
 * Set interrupt vector table.
 */
extern VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector);
extern HWI_PROC_FUNC g_hwiHandlerForm[];
#endif

#define OS_EXC_IN_INIT                      0
#define OS_EXC_IN_TASK                      1
#define OS_EXC_IN_HWI                       2
#define OS_EXC_FLAG_FAULTADDR_VALID         0x01
#define OS_EXC_IMPRECISE_ACCESS_ADDR        0xABABABAB

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

VOID HalPreInterruptHandler(UINT32 arg);
VOID HalAftInterruptHandler(UINT32 arg);
VOID *ArchGetHwiFrom(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_COMMON_INTERRUPT_H */
