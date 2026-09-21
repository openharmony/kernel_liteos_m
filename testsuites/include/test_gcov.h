/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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
 *
 * Description: gcov dump glue for bare-metal targets (test infrastructure).
 *
 *   Design (standalone runtime, the first proven implementation): the
 *   kernel is compiled with -fprofile-arcs -ftest-coverage but does NOT
 *   link libgcov. test_gcov.c supplies the runtime symbols the instrumented
 *   code references (__gcov_init / __gcov_merge_add), reads the
 *   compiler-emitted gcov_info / gcov_fn_info structs through fixed byte
 *   offsets (verified for riscv32-linux-musl-gcc 7.3 and
 *   arm-none-eabi-gcc 10.3.1), and at dump time streams the counters over
 *   the console UART as base64 GCOV_BEGIN/GCOV_END frames for host-side
 *   .gcda reconstruction (no on-target filesystem needed).
 */

#ifndef _TEST_GCOV_H
#define _TEST_GCOV_H

#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* Run all .init_array constructors so libgcov's __gcov_init registers every
 * instrumented translation unit's gcov_info. Call once after kernel init,
 * before tests run. Only effective when LOSCFG_TEST_KERNEL_COVERAGE is
 * defined.
 */
VOID TEST_GcovRunConstructors(VOID);

/* Dump accumulated coverage counters over the console UART as base64-framed
 * text. Call once when testing is complete (before "--- Test End ---").
 * Idempotent: subsequent calls are no-ops.
 */
VOID TEST_GcovDump(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _TEST_GCOV_H */
