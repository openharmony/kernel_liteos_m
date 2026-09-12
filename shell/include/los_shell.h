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

/**
 * @defgroup Shell Shell
 * @defgroup shell shell
 * @ingroup Shell
 */

#ifndef _LOS_SHELL_H
#define _LOS_SHELL_H

#include "shell.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef LOSCFG_BOX

/**
 * @ingroup shell
 * Shell error code: Invalid shell command parameters.
 *
 * Value: 0x02003101.
 *
 * Solution: Check registered parameters.
 */
#define LOS_ERRNO_SHELL_CMDREG_PARA_ERROR        LOS_ERRNO_OS_ERROR(LOS_MOD_SHELL, 0x01)

/**
 * @ingroup shell
 * Shell error code: Command keyword error.
 *
 * Value: 0x02003102.
 *
 * Solution: Check the registered command keyword.
 */
#define LOS_ERRNO_SHELL_CMDREG_CMD_ERROR         LOS_ERRNO_OS_ERROR(LOS_MOD_SHELL, 0x02)

/**
 * @ingroup shell
 * Shell error code: Command is already existed.
 *
 * Value: 0x02003103.
 *
 * Solution: Use another keyword to register command.
 */
#define LOS_ERRNO_SHELL_CMDREG_CMD_EXIST         LOS_ERRNO_OS_ERROR(LOS_MOD_SHELL, 0x03)

/**
 * @ingroup shell
 * Shell error code: Not enough memory.
 *
 * Value: 0x02003104.
 *
 * Solution: Enlarge heap section.
 */
#define LOS_ERRNO_SHELL_CMDREG_MEMALLOC_ERROR    LOS_ERRNO_OS_ERROR(LOS_MOD_SHELL, 0x04)

/**
 * @ingroup shell
 * Shell error code: Operation denied for creating and registering.
 *
 * Value: 0x02003120.
 *
 * Solution: Call LOS_ShellCreate or LOS_ShellCmdReg in appropriate box and location.
 */
#define LOS_ERRNO_SHELL_PERM_ERROR               LOS_ERRNO_OS_ERROR(LOS_MOD_SHELL, 0x20)

/**
 * @ingroup shell
 * Shell error code: Shell creating error.
 *
 * Value: 0x02003121.
 *
 * Solution: Check specific errors in OsShellInit.
 */
#define LOS_ERRNO_SHELL_INIT_ERROR               LOS_ERRNO_OS_ERROR(LOS_MOD_SHELL, 0x21)

/**
 * @ingroup shell
 * @brief Create shell.
 *
 * @par Description:
 * This API is used to create a shell.
 * Return #LOS_OK when creating is successful, return specific error code otherwise.
 * @attention
 * The API can only be called in box, and can't be invoked in kernel or irq.
 *
 * @param VOID
 *
 * @retval #LOS_ERRNO_SHELL_PERM_ERROR        Registering command to current box is permission denied.
 * @retval #LOS_ERRNO_SHELL_INIT_ERROR        Failed to initialize shell component.
 * @retval #LOS_OK                            The shell is successfully created.
 * @par Dependency:
 * <ul><li>los_shell.h: the header file that contains the API declaration.</li></ul>
 * @see shell
 * @since Huawei LiteOS SIL Certified 207.1.0
 */
extern UINT32 LOS_ShellCreate(VOID);

/**
 * @ingroup shell
 * @brief Register shell command.
 *
 * @par Description:
 * This API is used to register shell command in runtime.
 * Return #LOS_OK when register is successful, return specific error code otherwise.
 * @attention
 * The API can only register command for current box, and can't be invoked in kernel or irq.
 *
 * @param cmdType  [IN] To specify command type.
 * @param cmdKey   [IN] To specify command key word for registry.
 * @param paraNum  [IN] Number of input parameters for registered command.
 * @param cmdProc  [IN] Call back function of registered command.
 *
 * @retval #LOS_ERRNO_SHELL_PERM_ERROR               Registering command to current box is permission denied.
 * @retval #LOS_ERRNO_SHELL_CMDREG_PARA_ERROR        The registered parameters are invalid.
 * @retval #LOS_ERRNO_SHELL_CMDREG_CMD_ERROR         Registering command error.
 * @retval #LOS_ERRNO_SHELL_CMDREG_CMD_EXIST         Command is already existed.
 * @retval #LOS_ERRNO_SHELL_CMDREG_MEMALLOC_ERROR    Fail to alloc enough memory for registered command.
 * @retval #LOS_OK                                   The command is registered successfully.
 * @par Dependency:
 * <ul><li>los_shell.h: the header file that contains the API declaration.</li></ul>
 * @see shell
 * @since Huawei LiteOS SIL Certified 207.1.0
 */
extern UINT32 LOS_ShellCmdReg(CmdType cmdType, CHAR *cmdKey, UINT32 paraNum, CmdCallBackFunc cmdProc);

#endif /* LOSCFG_BOX */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_SHELL_H */
