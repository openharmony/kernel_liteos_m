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
 * @defgroup los_init Kernel init
 * @ingroup kernel
 */

#ifndef _LOS_INIT_H
#define _LOS_INIT_H

#include "los_typedef.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef UINT32 (*SysInitcallFunc)(VOID);
#ifndef LOSCFG_COMPILER_ICCARM
extern SysInitcallFunc __sysinitcall_start[];
extern SysInitcallFunc __sysinitcall0_start[];
extern SysInitcallFunc __sysinitcall1_start[];
extern SysInitcallFunc __sysinitcall2_start[];
extern SysInitcallFunc __sysinitcall3_start[];
extern SysInitcallFunc __sysinitcall4_start[];
extern SysInitcallFunc __sysinitcall5_start[];
extern SysInitcallFunc __sysinitcall6_start[];
extern SysInitcallFunc __sysinitcall7_start[];
extern SysInitcallFunc __sysinitcall_end[];
#endif

#define ATTRIBUTE_USED  __attribute__((__used__))
#define DO_INITCALL(fn, level) \
    SysInitcallFunc initFn_##fn ATTRIBUTE_USED __attribute__((section(".sysinitcall"#level".init"))) = fn

#define CREATE_INITCALL(fn, level, sync)    DO_INITCALL(fn, _level##level##_##sync)
#define SYS_INITCALL(fn, level, sync)       CREATE_INITCALL(fn, level, sync)

/**
 * @ingroup  los_init
 * Kernel module init level, used to describe the startup stage.
 * Smaller level number, execute earlier.
 * SYS_INIT_LEVEL_EARLY and SYS_INIT_LEVEL_RESERVE reserve for user and other level used for LiteOS.
 */
#define SYS_INIT_LEVEL_EARLY                0 /* reserved before kernel init */
#define SYS_INIT_LEVEL_AHEAD                1 /* prepare for initialization and debug tool init */
#define SYS_INIT_LEVEL_ARCH                 2 /* arch and platform initialization */
#define SYS_INIT_LEVEL_KERNEL               3 /* system basic module need init earlier */
#define SYS_INIT_LEVEL_KERNEL_ADDITION      4 /* system additional module init, rely on kernel */
#define SYS_INIT_LEVEL_COMPONENT            5 /* component level used for fs drivers compat and so on */
#define SYS_INIT_LEVEL_APP                  6 /* application init */
#define SYS_INIT_LEVEL_RESERVE              7 /* reserved after kernel init */

/**
 * @ingroup  los_init
 * Kernel Module Init sync, used to set up the priority in same level.
 * Smaller sync number, execute earlier.
 */
#define SYS_INIT_SYNC_0     s0
#define SYS_INIT_SYNC_1     s1
#define SYS_INIT_SYNC_2     s2
#define SYS_INIT_SYNC_3     s3
#define SYS_INIT_SYNC_4     s4

/**
 * @ingroup  los_init
 * @brief Register a startup module to the startup process.
 *
 * @par Description:
 * This API is used to register a startup module to the startup process.
 *
 * @attention
 * <ul>
 * <li>Register a new module in the boot process of the kernel as part of the kernel capability component.</li>
 * <li>In the startup framework, within the same level, the startup sequence is sorted by
 * the sync number </li>
 * <li>Using -u option can avoid linker to optimize symbol, otherwise the symbol may register failed! </li>
 * <li>The parameter func is not be NULL, otherwise the system maybe crash! </li>
 * </ul>
 *
 * @param  func  [IN] Type  #UINT32 (*)(VOID)    Register function.
 * @param  level [IN] Type  #UINT32              Init level in the kernel.
 * @param  sync  [IN] Type  #UINT32              Init sync in the kernel.
 *
 * @retval None
 * @par Dependency:
 * <ul><li>los_init.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
#define LOS_SYS_INIT(func, level, sync) SYS_INITCALL(func, level, sync)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_INIT_H */
