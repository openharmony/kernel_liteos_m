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
 * "AS IS AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
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

#ifndef _LOS_SPINLOCK_H
#define _LOS_SPINLOCK_H

#include "los_compiler.h"
#include "los_interrupt.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SPIN_LOCK_INITIALIZER(lockName) { 0U }
#define SPIN_LOCK_INIT(lock)  SPIN_LOCK_S lock = SPIN_LOCK_INITIALIZER(lock)

struct Spinlock {
    UINT32 rawLock;
};
typedef struct Spinlock SPIN_LOCK_S;

STATIC INLINE VOID LOS_SpinInit(SPIN_LOCK_S *lock)
{
    (VOID)lock;
}

STATIC INLINE VOID LOS_SpinLock(SPIN_LOCK_S *lock)
{
    (VOID)lock;
}

STATIC INLINE VOID LOS_SpinUnlock(SPIN_LOCK_S *lock)
{
    (VOID)lock;
}

STATIC INLINE VOID LOS_SpinLockSave(SPIN_LOCK_S *lock, UINT32 *intSave)
{
    (VOID)lock;
    *intSave = LOS_IntLock();
}

STATIC INLINE VOID LOS_SpinUnlockRestore(SPIN_LOCK_S *lock, UINT32 intSave)
{
    (VOID)lock;
    LOS_IntRestore(intSave);
}

STATIC INLINE BOOL LOS_SpinHeld(const SPIN_LOCK_S *lock)
{
    (VOID)lock;
    return TRUE;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_SPINLOCK_H */
