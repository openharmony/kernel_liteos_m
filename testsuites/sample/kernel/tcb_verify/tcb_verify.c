/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary, with or without modification,
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

/* TCB sanity verify -- reference implementation for the test layer.
 *
 * Lives in testsuites/ (NOT in the kernel) on purpose: see tcb_verify.h for
 * why the judge must be kept out of the very image it verifies.
 *
 * It reads the kernel's task array directly through link-time symbols
 * (g_taskCBArray / g_taskMaxNum, extern'd by los_task_pri.h) and uses the same
 * kernel internals (SCHEDULER_LOCK, OsCurrTaskGet, OS_TASK_*, OS_TASK_MAGIC_WORD,
 * LOSCFG_STACK_POINT_ALIGN_SIZE) that the kernel test suite already relies on.
 */

#include "tcb_verify.h"
#include "los_task_pri.h"

/*
 * Valid bits in LosTaskCB::taskStatus. In liteos_new this single UINT16 field
 * stores BOTH the runtime status (OS_TASK_STATUS_*, 0x0001..0x0100) AND the
 * attribute flags (OS_TASK_FLAG_*): STACK_FREE is set on every LOS_TaskCreate(),
 * SYSTEM_TASK on idle/swtmr, SIGNAL/FREEZE/JOINABLE/USER_TASK as applicable.
 * All of them are legitimate, so the verify mask must cover them -- otherwise
 * every normally created task would be falsely flagged with ERR_STATUS.
 *
 * Bit 0x0400 is intentionally NOT covered: no status/flag uses it in
 * liteos_new, so a set 0x0400 bit is genuinely invalid.
 *
 * Two reserved/optional feature switches compose on top of the base mask:
 *   - LOSCFG_SECURE       : adds OS_TASK_FLAG_USER_TASK (only #defined when
 *                           secure is on, so it must be guarded to keep
 *                           non-secure builds compiling).
 *   - LOSCFG_SCHED_MQ     : reserved forward-compat for the message-queue
 *                           scheduler extension. When it lands it will bring
 *                           OS_TASK_STATUS_SCHED/ENQUE/DEQUE/WAKEUP, which are
 *                           legitimate runtime bits and must be covered too --
 *                           the guard below ensures they are, so P0-1 does not
 *                           silently regress for MQ tasks when the feature is
 *                           enabled. The branch is a no-op today (macros are
 *                           not yet present in liteos_new headers).
 */
#define TCB_VERIFY_VALID_MASK_BASE  \
    (OS_TASK_STATUS_UNUSED   | OS_TASK_STATUS_SUSPEND   | \
     OS_TASK_STATUS_READY    | OS_TASK_STATUS_PEND      | \
     OS_TASK_STATUS_RUNNING  | OS_TASK_STATUS_DELAY     | \
     OS_TASK_STATUS_TIMEOUT  | OS_TASK_STATUS_PEND_TIME | \
     OS_TASK_STATUS_ZOMBIE   | OS_TASK_FLAG_SIGNAL      | \
     OS_TASK_FLAG_FREEZE)

#if (LOSCFG_SECURE == 1)
#define TCB_VERIFY_VALID_MASK_SEC  \
    (TCB_VERIFY_VALID_MASK_BASE | OS_TASK_FLAG_USER_TASK)
#else
#define TCB_VERIFY_VALID_MASK_SEC  TCB_VERIFY_VALID_MASK_BASE
#endif

#ifdef LOSCFG_SCHED_MQ
#define TCB_VERIFY_VALID_MASK  \
    (TCB_VERIFY_VALID_MASK_SEC | \
     OS_TASK_STATUS_SCHED  | OS_TASK_STATUS_ENQUE  | \
     OS_TASK_STATUS_DEQUE  | OS_TASK_STATUS_WAKEUP)
#else
#define TCB_VERIFY_VALID_MASK  TCB_VERIFY_VALID_MASK_SEC
#endif

#define TCB_STACK_MAGIC_CHECK(top) \
    (*(volatile UINT32 *)(UINTPTR)(top) == OS_TASK_MAGIC_WORD)

/* Read the current hardware SP register. */
static inline UINTPTR TcbVerifyGetCurSp(VOID)
{
    UINTPTR val;
#if defined(LOSCFG_ARCH_RISCV)
    __asm__ volatile("mv %0, sp" : "=r"(val));
#elif defined(LOSCFG_ARCH_ARM)
    __asm__ volatile("mov %0, sp" : "=r"(val));
#elif defined(LOSCFG_ARCH_CSKY)
    __asm__ volatile("mov %0, sp" : "=r"(val));
#elif defined(LOSCFG_ARCH_XTENSA)
    __asm__ volatile("mov %0, sp" : "=r"(val));
#else
    /* Unknown arch: SP approximation via frame pointer. */
    val = (UINTPTR)__builtin_frame_address(0);
#endif
    return val;
}

UINT32 TcbVerifyBasic(VOID)
{
    UINT32 err = TCB_VERIFY_OK;

    if (g_taskCBArray == NULL) {
        return TCB_VERIFY_ERR_ARRAY_NULL;
    }

    for (UINT32 i = 0; i < g_taskMaxNum; i++) {
        LosTaskCB *tcb = &g_taskCBArray[i];
        UINT16 status = tcb->taskStatus;

        /* Check all slots including UNUSED. */
        if ((UINT32)status & ~((UINT32)TCB_VERIFY_VALID_MASK)) {
            err |= TCB_VERIFY_ERR_STATUS;
        }

        if (status & OS_TASK_STATUS_UNUSED) {
            continue;
        }

        if (tcb->stackPointer == NULL) {
            err |= TCB_VERIFY_ERR_SP_NULL;
            continue;   /* skip alignment check - needs a valid sp */
        }

        if (((UINTPTR)tcb->stackPointer & (LOSCFG_STACK_POINT_ALIGN_SIZE - 1)) != 0) {
            err |= TCB_VERIFY_ERR_SP_ALIGN;
        }

        if (tcb->priority > LOS_TASK_PRIORITY_LOWEST) {
            err |= TCB_VERIFY_ERR_PRIO;
        }
    }

    return err;
}

UINT32 TcbVerifyStack(VOID)
{
    UINT32 err = TCB_VERIFY_OK;

    if (g_taskCBArray == NULL) {
        return TCB_VERIFY_ERR_ARRAY_NULL;
    }

    for (UINT32 i = 0; i < g_taskMaxNum; i++) {
        LosTaskCB *tcb = &g_taskCBArray[i];

        if (tcb->taskStatus & OS_TASK_STATUS_UNUSED) {
            continue;
        }

        /* Stack bottom magic word. */
        if (!TCB_STACK_MAGIC_CHECK(tcb->topOfStack)) {
            err |= TCB_VERIFY_ERR_BOTTOM_MAGIC;
        }

        UINTPTR sp  = (UINTPTR)tcb->stackPointer;
        UINTPTR top = tcb->topOfStack;
        UINT32  sz  = tcb->stackSize;
        if ((sp <= top) || (sp > top + sz)) {
            err |= TCB_VERIFY_ERR_STACK_RANGE;
        }
    }

    return err;
}

UINT32 TcbVerifyCurSp(VOID)
{
    UINT32 err = TCB_VERIFY_OK;
    LosTaskCB *cur = OsCurrTaskGet();
    if (cur == NULL) {
        return TCB_VERIFY_ERR_CUR_SP_RANGE;
    }

    UINTPTR sp = TcbVerifyGetCurSp();

    UINTPTR top = cur->topOfStack;
    UINT32  sz  = cur->stackSize;
    if ((sp <= top) || (sp > top + sz)) {
        err |= TCB_VERIFY_ERR_CUR_SP_RANGE;
    }

    /* SP must belong to exactly one task's stack. Skipped in interrupt context. */
    if (!OS_INT_ACTIVE && (g_taskCBArray != NULL)) {
        UINT32 count = 0;
        for (UINT32 i = 0; i < g_taskMaxNum; i++) {
            LosTaskCB *tcb = &g_taskCBArray[i];
            if (tcb->taskStatus & OS_TASK_STATUS_UNUSED) {
                continue;
            }
            if ((sp > tcb->topOfStack) && (sp <= tcb->topOfStack + tcb->stackSize)) {
                count++;
            }
        }
        if (count == 0) {
            err |= TCB_VERIFY_ERR_CUR_SP_NO_OWNER;
        } else if (count > 1) {
            err |= TCB_VERIFY_ERR_CUR_SP_MULTI_OWNER;
        }
    }

    return err;
}

/* Lock for consistent snapshot. */
UINT32 TcbVerifyAll(VOID)
{
    UINT32 err = TCB_VERIFY_OK;
    UINT32 intSave;

    SCHEDULER_LOCK(intSave);
    err |= TcbVerifyBasic();
    err |= TcbVerifyStack();
    err |= TcbVerifyCurSp();
    SCHEDULER_UNLOCK(intSave);

    return err;
}
