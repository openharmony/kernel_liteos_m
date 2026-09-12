/*
 * Copyright (c) 2013-2023 Huawei Technologies Co., Ltd. All rights reserved.
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

#ifndef _LOS_COMPILER_H
#define _LOS_COMPILER_H

#include "los_typedef.h"
#include "los_builddef.h"
#include "los_toolchain.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_base
 * Align the beginning of the object with the base address addr,
 * with boundary bytes being the smallest unit of alignment.
 */
#ifndef ALIGN
#define ALIGN(addr, boundary)        LOS_Align(addr, boundary)
#endif

/**
 * @ingroup los_base
 * Align the tail of the object with the base address addr, with size bytes being the smallest unit of alignment.
 */
#define TRUNCATE(addr, size)         ((addr) & ~((size) - 1))

/**
 * @ingroup los_base
 * @brief Align the value (addr) by some bytes (boundary) you specify.
 *
 * @par Description:
 * This API is used to align the value (addr) by some bytes (boundary) you specify.
 *
 * @attention
 * <ul>
 * <li>the value of boundary usually is 4,8,16,32.</li>
 * </ul>
 *
 * @param addr     [IN]  The variable what you want to align.
 * @param boundary [IN]  The align size what you want to align.
 *
 * @retval #UINT32 The variable what have been aligned.
 * @par Dependency:
 * <ul><li>los_base.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
static inline UINT32 LOS_Align(UINT32 addr, UINT32 boundary)
{
    return (addr + (((addr + (boundary - 1)) > addr) ? (boundary - 1) : 0)) & ~(boundary - 1);
}

#define OS_GOTO_ERREND() \
        do {                 \
            goto LOS_ERREND; \
        } while (0)

#if defined(__GNUC__)
#ifndef __XTENSA_LX6__
static inline void maybe_release_fence(int model)
{
    switch (model) {
        case __ATOMIC_RELEASE:
            __atomic_thread_fence (__ATOMIC_RELEASE);
            break;
        case __ATOMIC_ACQ_REL:
            __atomic_thread_fence (__ATOMIC_ACQ_REL);
            break;
        case __ATOMIC_SEQ_CST:
            __atomic_thread_fence (__ATOMIC_SEQ_CST);
            break;
        default:
            break;
    }
}

static inline void maybe_acquire_fence(int model)
{
    switch (model) {
        case __ATOMIC_ACQUIRE:
            __atomic_thread_fence (__ATOMIC_ACQUIRE);
            break;
        case __ATOMIC_ACQ_REL:
            __atomic_thread_fence (__ATOMIC_ACQ_REL);
            break;
        case __ATOMIC_SEQ_CST:
            __atomic_thread_fence (__ATOMIC_SEQ_CST);
            break;
        default:
            break;
    }
}

#define __LIBATOMIC_N_LOCKS	(1 << 4) /* 4, 1<<4 locks num */
static inline BOOL *__libatomic_flag_for_address(void *addr)
{
    static BOOL flag_table[__LIBATOMIC_N_LOCKS] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    UINTPTR p = (UINTPTR)(UINTPTR *)addr;
    p += (p >> 2) + (p << 4); /* 2, 4, hash data */
    p += (p >> 7) + (p << 5); /* 7, 5, hash data */
    p += (p >> 17) + (p << 13); /* 17, 13, hash data */

    if (sizeof(void *) > 4) { /* 4, sizeof int in 32bit system */
        p += (p >> 31); /* 31, for hash high bits data */
    }

    p &= (__LIBATOMIC_N_LOCKS - 1);
    return flag_table + p;
}

static inline void get_lock(void *addr, int model)
{
    BOOL *lock_ptr = __libatomic_flag_for_address (addr);

    maybe_release_fence (model);
    while (__atomic_test_and_set (lock_ptr, __ATOMIC_ACQUIRE) == 1) {
        ;
    }
}

static inline void free_lock(void *addr, int model)
{
    BOOL *lock_ptr = __libatomic_flag_for_address (addr);

    __atomic_clear (lock_ptr, __ATOMIC_RELEASE);
    maybe_acquire_fence (model);
}

static inline UINT64  __atomic_load_8(const volatile void *mem, int model)
{
    UINT64 ret;

    void *memP = (void *)mem;
    get_lock (memP, model);
    ret = *(UINT64 *)mem;
    free_lock (memP, model);
    return ret;
}

static inline void __atomic_store_8(volatile void *mem, UINT64 val, int model)
{
    void *memP = (void *)mem;
    get_lock (memP, model);
    *(UINT64 *)mem = val;
    free_lock (memP, model);
}

static inline UINT64 __atomic_exchange_8(volatile void *mem, UINT64 val, int model)
{
    UINT64 ret;

    void *memP = (void *)mem;
    get_lock (memP, model);
    ret = *(UINT64 *)mem;
    *(UINT64 *)mem = val;
    free_lock (memP, model);
    return ret;
}
#endif /* __XTENSA_LX6__ */

#define ALIAS_OF(of) __attribute__((alias(#of)))
#define FUNC_ALIAS(real_func, new_alias, args_list, return_type) \
    return_type new_alias args_list ALIAS_OF(real_func)

#else

#define FUNC_ALIAS(real_func, new_alias, args_list, return_type)

#endif /* __GNUC__ */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_COMPILER_H */
