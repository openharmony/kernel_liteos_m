/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Device Co., Ltd. 2020-2026. All rights reserved.
 * Description: Los_printf HeadFile
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

/**
 * @defgroup los_printf Printf
 * @ingroup kernel
 */

#ifndef _LOS_PRINTF_H
#define _LOS_PRINTF_H

#include "stdarg.h"
#include "los_config.h"
#include "los_typedef.h"
#ifdef LOSCFG_KERNEL_HPL
#include "los_hpl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define UART_WITH_LOCK        1
#define UART_WITHOUT_LOCK     0

extern VOID UartPuts(const CHAR *s, UINT32 len, BOOL isLock);

#ifdef LOSCFG_SHELL_LK
extern void LOS_LkPrint(int level, const char *func, int line, const char *fmt, ...);
#endif

/**
 * @ingroup los_printf
 * log print level definition, LOS_EMG_LEVEL is set to 0, it means the log is emergency.
 */
#define LOS_EMG_LEVEL    0

/**
 * @ingroup los_printf
 * log print level definition, LOS_COMMON_LEVEL is set to 1, it means the log is common.
 */
#define LOS_COMMON_LEVEL (LOS_EMG_LEVEL + 1)

/**
 * @ingroup los_printf
 * log print level definition, LOS_ERR_LEVEL is set to 2, it means it is a error log.
 */
#define LOS_ERR_LEVEL    (LOS_COMMON_LEVEL + 1)

/**
 * @ingroup los_printf
 * log print level definition, LOS_WARN_LEVEL is set to 3, it means it is a warning log.
 */
#define LOS_WARN_LEVEL   (LOS_ERR_LEVEL + 1)

/**
 * @ingroup los_printf
 * log print level definition, LOS_INFO_LEVEL is set to 4, it means the log is an information.
 */
#define LOS_INFO_LEVEL   (LOS_WARN_LEVEL + 1)

/**
 * @ingroup los_printf
 * log print level definition, LOS_DEBUG_LEVEL is set to 5, it means it is a debug log.
 */
#define LOS_DEBUG_LEVEL  (LOS_INFO_LEVEL + 1)

/**
 * @ingroup los_printf
 * The default log print level. PRINT_LEVEL is set to debug log level if
 * LOSCFG_SHELL_LK is defined, otherwise PRINT_LEVEL is set to error log level.
 * The default log print level means only print the log which its level value
 * is lower than or equal to the PRINT_LEVEL.
 * NOTE: PRINT_LEVEL is defined in los_debug.h, not here.
 */

typedef VOID (*pf_OUTPUT)(const CHAR *fmt, ...);

/**
 * @ingroup los_printf
 * @brief Format and print data.
 *
 * @par Description:
 * Print argument(s) according to fmt.
 *
 * @attention
 * None.
 *
 * @param fmt [IN] Type char*. It controls the output format as in C printf.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_debug.h: the header file that contains the LOS_Printf declaration.</li></ul>
 * @see printf
 */
/* LOS_Printf is declared in los_debug.h with the original signature:
 *   VOID LOS_Printf(LogModuleType type, INT32 level, const CHAR *fmt, ...);
 */
#define diag_printf(fmt, ...)   LOS_Printf(LOG_MODULE_KERNEL, LOG_COMMON_LEVEL, fmt, ##__VA_ARGS__)

/**
 * @ingroup los_printf
 * @brief Format and print log.
 *
 * @par Description:
 * Define function macros PRINT_RELEASE. The Function can print argument(s) according to fmt.
 * It is same with LOS_Printf function.
 *
 * @attention
 * None.
 *
 * @param fmt  [IN] Type: const CHAR *. It controls the output format as in C printf.
 * @param args [IN] It point to the variable parameters.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_printf.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_Printf
 */
#ifndef PRINT_RELEASE
#ifdef LOSCFG_KERNEL_HPL
#define PRINT_RELEASE(fmt, ...)   HP_LOG(HPL_NOTICE, fmt, ##__VA_ARGS__)
#else
#define PRINT_RELEASE(fmt, ...)   LOS_Printf(LOG_MODULE_KERNEL, LOG_COMMON_LEVEL, fmt, ##__VA_ARGS__)
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_PRINTF_H */
