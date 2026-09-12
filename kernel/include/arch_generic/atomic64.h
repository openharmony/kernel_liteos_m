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
 * @defgroup los_atomic Atomic
 * @ingroup kernel
 */

#ifndef _ARCH_GENERIC_ATOMIC64_H
#define _ARCH_GENERIC_ATOMIC64_H

#include "los_typedef.h"
#include "los_hwi.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef LOSCFG_KERNEL_SMP

STATIC INLINE INT64 ArchAtomic64Read(const Atomic64 *v)
{
    UINT32 intSave;
    INT64 val;

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
    UINT32 intSave;
    INT64 val;

    intSave = LOS_IntLock();
    *v += addVal;
    val = *v;
    LOS_IntRestore(intSave);

    return val;
}

STATIC INLINE INT64 ArchAtomic64Sub(Atomic64 *v, INT64 subVal)
{
    UINT32 intSave;
    INT64 val;

    intSave = LOS_IntLock();
    *v -= subVal;
    val = *v;
    LOS_IntRestore(intSave);

    return val;
}

STATIC INLINE VOID ArchAtomic64Inc(Atomic64 *v)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    *v += 1;
    LOS_IntRestore(intSave);
}

STATIC INLINE INT64 ArchAtomic64IncRet(Atomic64 *v)
{
    UINT32 intSave;
    INT64 val;

    intSave = LOS_IntLock();
    *v += 1;
    val = *v;
    LOS_IntRestore(intSave);

    return val;
}

STATIC INLINE VOID ArchAtomic64Dec(Atomic64 *v)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    *v -= 1;
    LOS_IntRestore(intSave);
}

STATIC INLINE INT64 ArchAtomic64DecRet(Atomic64 *v)
{
    UINT32 intSave;
    INT64 val;

    intSave = LOS_IntLock();
    *v -= 1;
    val = *v;
    LOS_IntRestore(intSave);

    return val;
}

STATIC INLINE INT64 ArchAtomicXchg64bits(Atomic64 *v, INT64 val)
{
    UINT32 intSave;
    INT64 prevVal;

    intSave = LOS_IntLock();
    prevVal = *v;
    *v = val;
    LOS_IntRestore(intSave);

    return prevVal;
}

STATIC INLINE BOOL ArchAtomicCmpXchg64bits(Atomic64 *v, INT64 val, INT64 oldVal)
{
    UINT32 intSave;
    INT64 prevVal;

    intSave = LOS_IntLock();
    prevVal = *v;
    if (prevVal == oldVal) {
        *v = val;
    }
    LOS_IntRestore(intSave);

    return (prevVal != oldVal);
}

#else
#error "Generic atomic implementation is not supported on SMP!"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ARCH_GENERIC_ATOMIC64_H */

