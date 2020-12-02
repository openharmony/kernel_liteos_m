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

#ifndef _RISCV_HAL_H
#define _RISCV_HAL_H

#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * backtrace
 */
extern CHAR *__except_stack_top;
extern CHAR *__start_and_irq_stack_top;
extern CHAR *__text_start;
extern CHAR *__text_end;
extern CHAR *__bss_end;

extern BOOL OsBackTraceRaCheck(UINT32 value);
extern BOOL OsBackTraceFpCheck(UINT32 value);

extern VOID OsIrqDisable(UINT32 vector);
extern VOID OsIrqEnable(UINT32 vector);
extern VOID OsSetLocalInterPri(UINT32 vector, UINT16 prior);

extern VOID OsGetCpuCycle(UINT32 *cntHi, UINT32 *cntLo);
extern VOID SysClockInit(UINT32 period);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* _RISCV_HAL_H */
