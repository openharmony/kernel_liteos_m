/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: LiteOS libc lock adapt for morpheus liteos_m
 * Author: Huawei LiteOS Team
 * Create: 2021-06-10
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

#ifndef _LIBC_LOCK_ADAPT_H
#define _LIBC_LOCK_ADAPT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* recursive libc lock, adapted from fbb lib/liteos_libc/lock/libc_lock_adapt.h,
 * implemented with lazy-init LOS recursive mux (LosMuxCB muxCount). */
#define LIBC_LOCK_INITIALIZER ((unsigned int)-1)
#define LIBC_LOCK_INIT(x) volatile unsigned int x = LIBC_LOCK_INITIALIZER
#define LIBC_LOCK(x) LibcLock(&(x))
#define LIBC_UNLOCK(x) LibcUnlock(&(x))

extern int LibcLock(volatile unsigned int *lock);
extern int LibcUnlock(const volatile unsigned int *lock);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LIBC_LOCK_ADAPT_H */
