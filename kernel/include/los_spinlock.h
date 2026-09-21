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

/**
 * @defgroup los_spinlock Spinlock
 * @ingroup kernel
 */

#ifndef _LOS_SPINLOCK_H
#define _LOS_SPINLOCK_H

#include "los_compiler.h"
#include "los_interrupt.h"
#ifdef LOSCFG_KERNEL_SMP
#include "los_task.h"
#include "los_arch_spinlock.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define SPIN_LOCK_INITIALIZER(lockName) { 0U }
#define SPIN_LOCK_INIT(lock)  SPIN_LOCK_S lock = SPIN_LOCK_INITIALIZER(lock)

struct Spinlock {
    UINT32 rawLock;
};
typedef struct Spinlock SPIN_LOCK_S;

#ifdef LOSCFG_KERNEL_SMP


LITE_OS_SEC_TEXT_MINOR STATIC INLINE VOID LOS_SpinInit(SPIN_LOCK_S *lock)
{
    lock->rawLock = 0;
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE VOID LOS_SpinLock(SPIN_LOCK_S *lock)
{
    LOS_TaskLock();
    ArchSpinLock(&lock->rawLock);
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE INT32 LOS_SpinTrylock(SPIN_LOCK_S *lock)
{
    LOS_TaskLock();
    INT32 ret = ArchSpinTrylock(&lock->rawLock);
    if (ret != LOS_OK) {
        /* Try failed: re-enable the scheduler (TaskLock held above). */
        LOS_TaskUnlock();
    }
    return ret;
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE VOID LOS_SpinUnlock(SPIN_LOCK_S *lock)
{
    ArchSpinUnlock(&lock->rawLock);
    /* Restore the scheduler flag. May cause task schedule. */
    LOS_TaskUnlock();
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE VOID LOS_SpinUnlockNoSched(SPIN_LOCK_S *lock)
{
    ArchSpinUnlock(&lock->rawLock);
    /* Restore the scheduler flag only. */
    LOS_TaskUnlockNoSched();
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE VOID LOS_SpinLockSave(SPIN_LOCK_S *lock, UINT32 *intSave)
{
    *intSave = LOS_IntLock();
    LOS_SpinLock(lock);
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE VOID LOS_SpinUnlockRestore(SPIN_LOCK_S *lock, UINT32 intSave)
{
    LOS_SpinUnlock(lock);
    LOS_IntRestore(intSave);
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE BOOL LOS_SpinHeld(const SPIN_LOCK_S *lock)
{
    return (lock->rawLock != 0);
}

#else

/*
 * For Non-SMP system, these apis does not handle with spinlocks,
 * but for unifying the code of drivers, vendors and etc.
 */
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

STATIC INLINE VOID LOS_SpinUnlockNoSched(SPIN_LOCK_S *lock)
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

#endif /* LOSCFG_KERNEL_SMP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_SPINLOCK_H */
