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

#ifndef _LOS_INIT_LEVEL_H
#define _LOS_INIT_LEVEL_H

/*
 * This header file only used for board.ld.S.
 * Do not include other header file here, otherwise compiler may can not find the file.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SYSINIT_CALLS_LEVEL(level)                  \
    __sysinitcall##level##_start = .;                   \
    KEEP(*(SORT(.sysinitcall_level##level##_*)))

#define SYSINIT_CALLS                            \
    __sysinitcall_start = .;                \
    SYSINIT_CALLS_LEVEL(0)                  \
    SYSINIT_CALLS_LEVEL(1)                  \
    SYSINIT_CALLS_LEVEL(2)                  \
    SYSINIT_CALLS_LEVEL(3)                  \
    SYSINIT_CALLS_LEVEL(4)                  \
    SYSINIT_CALLS_LEVEL(5)                  \
    SYSINIT_CALLS_LEVEL(6)                  \
    SYSINIT_CALLS_LEVEL(7)                  \
    __sysinitcall_end = .;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_INIT_LEVEL_H */
