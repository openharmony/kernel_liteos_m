/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_ARCH_ATOMIC_H
#define _LOS_ARCH_ATOMIC_H

#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

STATIC INLINE INT32 ArchAtomicRead(const Atomic *v)
{
    INT32 val;

    __asm__ __volatile__("ldrex %0, [%1]\n"
                         : "=&r"(val)
                         : "r"(v)
                         : "cc");

    return val;
}

STATIC INLINE VOID ArchAtomicSet(Atomic *v, INT32 setVal)
{
    UINT32 status;

    do {
        __asm__ __volatile__("ldrex %0, [%1]\n"
                             "strex %0, %2, [%1]\n"
                             : "=&r"(status)
                             : "r"(v), "r"(setVal)
                             : "cc");
    } while (__builtin_expect(status != 0, 0));
}

STATIC INLINE INT32 ArchAtomicAdd(Atomic *v, INT32 addVal)
{
    INT32 val;
    UINT32 status;

    do {
        __asm__ __volatile__("ldrex %1, [%2]\n"
                             "add %1, %1, %3\n"
                             "strex %0, %1, [%2]"
                             : "=&r"(status), "=&r"(val)
                             : "r"(v), "r"(addVal)
                             : "cc");
    } while (__builtin_expect(status != 0, 0));

    return val;
}

STATIC INLINE INT32 ArchAtomicSub(Atomic *v, INT32 subVal)
{
    INT32 val;
    UINT32 status;

    do {
        __asm__ __volatile__("ldrex %1, [%2]\n"
                             "sub %1, %1, %3\n"
                             "strex %0, %1, [%2]"
                             : "=&r"(status), "=&r"(val)
                             : "r"(v), "r"(subVal)
                             : "cc");
    } while (__builtin_expect(status != 0, 0));

    return val;
}

STATIC INLINE VOID ArchAtomicInc(Atomic *v)
{
    (VOID)ArchAtomicAdd(v, 1);
}

STATIC INLINE VOID ArchAtomicDec(Atomic *v)
{
    (VOID)ArchAtomicSub(v, 1);
}

STATIC INLINE INT32 ArchAtomicIncRet(Atomic *v)
{
    return ArchAtomicAdd(v, 1);
}

STATIC INLINE INT32 ArchAtomicDecRet(Atomic *v)
{
    return ArchAtomicSub(v, 1);
}

/**
 * @ingroup  los_arch_atomic
 * @brief Atomic exchange for 32-bit variable.
 *
 * @par Description:
 * This API is used to implement the atomic exchange for 32-bit variable
 * and return the previous value of the atomic variable.
 * @attention
 * <ul>The pointer v must not be NULL.</ul>
 *
 * @param  v       [IN] The variable pointer.
 * @param  val     [IN] The exchange value.
 *
 * @retval #INT32       The previous value of the atomic variable
 * @par Dependency:
 * <ul><li>los_arch_atomic.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
STATIC INLINE INT32 ArchAtomicXchg32bits(volatile INT32 *v, INT32 val)
{
    INT32 prevVal = 0;
    UINT32 status = 0;

    do {
        __asm__ __volatile__("ldrex %0, [%3]\n"
                             "strex %1, %4, [%3]"
                             : "=&r"(prevVal), "=&r"(status), "+m"(*v)
                             : "r"(v), "r"(val)
                             : "cc");
    } while (__builtin_expect(status != 0, 0));

    return prevVal;
}

/**
 * @ingroup  los_arch_atomic
 * @brief Atomic exchange for 32-bit variable with compare.
 *
 * @par Description:
 * This API is used to implement the atomic exchange for 32-bit variable, if the value of variable is equal to oldVal.
 * @attention
 * <ul>The pointer v must not be NULL.</ul>
 *
 * @param  v       [IN] The variable pointer.
 * @param  val     [IN] The new value.
 * @param  oldVal  [IN] The old value.
 *
 * @retval TRUE  The previous value of the atomic variable is not equal to oldVal.
 * @retval FALSE The previous value of the atomic variable is equal to oldVal.
 * @par Dependency:
 * <ul><li>los_arch_atomic.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
STATIC INLINE BOOL ArchAtomicCmpXchg32bits(volatile INT32 *v, INT32 val, INT32 oldVal)
{
    INT32 prevVal = 0;
    UINT32 status = 0;

    do {
        __asm__ __volatile__("ldrex %0, %2\n"
                             "mov %1, #0\n"
                             "cmp %0, %3\n"
                             "bne 1f\n"
                             "strex %1, %4, %2\n"
                             "1:"
                             : "=&r"(prevVal), "=&r"(status), "+Q"(*v)
                             : "r"(oldVal), "r"(val)
                             : "cc");
    } while (__builtin_expect(status != 0, 0));

    return prevVal != oldVal;
}

STATIC INLINE INT64 ArchAtomic64Read(const Atomic64 *v)
{
    INT64 val;
    UINT32 intSave;

    intSave = LOS_IntLock();
    val = *v;
    LOS_IntRestore(intSave);

    return val;
}

STATIC INLINE VOID ArchAtomic64Set(Atomic64 *v, INT64 setVal)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    *v = setVal;
    LOS_IntRestore(intSave);
}

STATIC INLINE INT64 ArchAtomic64Add(Atomic64 *v, INT64 addVal)
{
    INT64 val;
    UINT32 intSave;

    intSave = LOS_IntLock();
    *v += addVal;
    val = *v;
    LOS_IntRestore(intSave);

    return val;
}

STATIC INLINE INT64 ArchAtomic64Sub(Atomic64 *v, INT64 subVal)
{
    INT64 val;
    UINT32 intSave;

    intSave = LOS_IntLock();
    *v -= subVal;
    val = *v;
    LOS_IntRestore(intSave);

    return val;
}

STATIC INLINE VOID ArchAtomic64Inc(Atomic64 *v)
{
    (VOID)ArchAtomic64Add(v, 1);
}

STATIC INLINE INT64 ArchAtomic64IncRet(Atomic64 *v)
{
    return ArchAtomic64Add(v, 1);
}

STATIC INLINE VOID ArchAtomic64Dec(Atomic64 *v)
{
    (VOID)ArchAtomic64Sub(v, 1);
}

STATIC INLINE INT64 ArchAtomic64DecRet(Atomic64 *v)
{
    return ArchAtomic64Sub(v, 1);
}

STATIC INLINE INT64 ArchAtomicXchg64bits(Atomic64 *v, INT64 val)
{
    INT64 prevVal;
    UINT32 intSave;

    intSave = LOS_IntLock();
    prevVal = *v;
    *v = val;
    LOS_IntRestore(intSave);

    return prevVal;
}

STATIC INLINE BOOL ArchAtomicCmpXchg64bits(Atomic64 *v, INT64 val, INT64 oldVal)
{
    INT64 prevVal;
    UINT32 intSave;

    intSave = LOS_IntLock();
    prevVal = *v;
    if (prevVal == oldVal) {
        *v = val;
    }
    LOS_IntRestore(intSave);

    return prevVal != oldVal;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_ARCH_ATOMIC_H */
