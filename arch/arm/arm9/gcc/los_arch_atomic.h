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
    UINT32 intSave;

    intSave = LOS_IntLock();
    LOS_IntRestore(intSave);
    return *v;
}

STATIC INLINE VOID ArchAtomicSet(Atomic *v, INT32 setVal)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    *v = setVal;
    LOS_IntRestore(intSave);
}

STATIC INLINE INT32 ArchAtomicAdd(Atomic *v, INT32 addVal)
{
    INT32 val;
    UINT32 intSave;

    intSave = LOS_IntLock();
    *v += addVal;
    val = *v;
    LOS_IntRestore(intSave);

    return val;
}

STATIC INLINE INT32 ArchAtomicSub(Atomic *v, INT32 subVal)
{
    INT32 val;
    UINT32 intSave;

    intSave = LOS_IntLock();
    *v -= subVal;
    val = *v;
    LOS_IntRestore(intSave);

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

STATIC INLINE INT32 ArchAtomicXchg32bits(Atomic *v, INT32 val)
{
    INT32 prevVal;
    UINT32 intSave;

    intSave = LOS_IntLock();
    prevVal = *v;
    *v = val;
    LOS_IntRestore(intSave);

    return prevVal;
}

STATIC INLINE BOOL ArchAtomicCmpXchg32bits(Atomic *v, INT32 val, INT32 oldVal)
{
    INT32 prevVal;
    UINT32 intSave;

    intSave = LOS_IntLock();
    prevVal = *v;
    if (prevVal == oldVal) {
        *v = val;
    }
    LOS_IntRestore(intSave);

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

