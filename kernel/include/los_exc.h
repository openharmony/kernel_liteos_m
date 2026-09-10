/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided the following conditions are met:
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

#include "los_typedef.h"
#include "los_backtrace.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define OsBackTrace LOS_BackTrace

#ifndef LOS_PANIC
#define LOS_PANIC(fmt, ...) LOS_Panic(fmt, ##__VA_ARGS__)
#endif

/*
 * LOS_ExcRegHook and ArchSetExcHook are arch-specific. Each arch's
 * los_arch_interrupt.h declares:    typedef VOID (*EXC_PROC_FUNC)(UINT32, <arch-context-type> *);
 *                                   UINT32 ArchSetExcHook(EXC_PROC_FUNC excHook);
 * LOS_ExcRegHook is provided as a thin macro alias so user code can call
 * the public API name without depending on a specific arch header.
 */
#ifndef LOS_ExcRegHook
#define LOS_ExcRegHook ArchSetExcHook
#endif

#ifdef LOSCFG_SHELL_EXCINFO_DUMP
/**
 * @ingroup los_exc
 * @brief  Define the type of functions for reading or writing exception information.
 *
 * @par Description:
 * This definition is used to declare the type of functions for reading or writing exception information.
 * @attention
 * The first parameter "startAddr" must be left to save the exception information.
 *
 * @param startAddr    [IN] The storage space address, it uses to save exception information.
 * @param space        [IN] The storage space size, it is also the size of the last parameter "buf".
 * @param rwFlag       [IN] The write-read flag, 0 for writing,1 for reading, other number is to do nothing.
 * @param buf          [IN] The buffer of storing data.
 *
 * @retval none.
 * @par Dependency:
 * <ul><li>los_exc.h: the header file that contains the type definition.</li></ul>
 */
typedef VOID (*LogReadWriteFunc)(UINTPTR startAddr, UINT32 space, UINT32 rwFlag, CHAR *buf);

 /**
 * @ingroup los_exc
 * @brief Register the function of recording exception information .
 *
 * @par Description:
 * This API is used to register the function of recording exception information, and specify the
 * location, size and buffer of the exception information recording.
 * @attention
 * The first parameter "startAddr" must be left to save the exception information.
 *
 * @param startAddr    [IN] The storage space address, it uses to save exception information.
 * @param space        [IN] The storage space size, it is also the size of the third parameter "buf".
 * @param buf          [IN] The buffer of storing exception information. The buffer is allocated or
 *                          free in user's code.
 * @param hook         [IN] the function for reading or writing exception information.
 *
 * @retval none.
 * @par Dependency:
 * <ul><li>los_exc.h: the header file that contains the API declaration.</li></ul>
 */
VOID LOS_ExcInfoRegHook(UINTPTR startAddr, UINT32 space, CHAR *buf, LogReadWriteFunc hook);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_EXC_H */
