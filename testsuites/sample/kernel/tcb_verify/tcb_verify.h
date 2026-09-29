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

#ifndef _TCB_VERIFY_H
#define _TCB_VERIFY_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/*
 * NOTE: this module lives in the TEST layer (testsuites/) on purpose. It is
 * NOT a kernel LOS_* interface.
 *
 * Upper-layer business that wants to sanity-check the underlying LiteOS task
 * subsystem (TCB fields, stack magic word, SP ownership) is expected to carry
 * its OWN copy of this logic and read kernel memory directly through link-time
 * symbols (g_taskCBArray / g_taskMaxNum / LosTaskCB field offsets /
 * OS_TASK_MAGIC_WORD), so that a kernel which has been patched or replaced
 * cannot simply stub one exported LOS_* symbol to hide the tampering. Shipping
 * this inside the kernel as a LOS_* API would re-introduce the very bypass
 * this module exists to avoid.
 *
 * Bitmask error codes. A single verify call may OR-combine multiple bits.
 * The SP-ownership (R9) check is skipped in interrupt context (SP then points
 * at the IRQ stack, not the current task's stack).
 */
typedef enum {
    TCB_VERIFY_OK                     = 0,
    TCB_VERIFY_ERR_SP_NULL            = 0x01,
    TCB_VERIFY_ERR_SP_ALIGN           = 0x02,
    TCB_VERIFY_ERR_STATUS             = 0x04,
    TCB_VERIFY_ERR_PRIO               = 0x08,
    TCB_VERIFY_ERR_BOTTOM_MAGIC       = 0x10,
    TCB_VERIFY_ERR_STACK_RANGE        = 0x20,
    TCB_VERIFY_ERR_CUR_SP_RANGE       = 0x40,
    TCB_VERIFY_ERR_ARRAY_NULL         = 0x80,
    TCB_VERIFY_ERR_CUR_SP_NO_OWNER    = 0x100,
    TCB_VERIFY_ERR_CUR_SP_MULTI_OWNER = 0x200,
} tcb_verify_err_t;

/* Layer 1: verify taskStatus mask, stackPointer null/alignment and priority of
 * every active TCB. Returns TCB_VERIFY_OK or bitwise-OR of failure bits. */
UINT32 TcbVerifyBasic(VOID);

/* Layer 2: verify bottom magic word and stackPointer range for every active
 * TCB. Returns TCB_VERIFY_OK or bitwise-OR of failure bits. */
UINT32 TcbVerifyStack(VOID);

/* Layer 3: read hardware SP, verify it is within the current task's stack range
 * AND that exactly one task's stack contains the SP (skipped in IRQ context). */
UINT32 TcbVerifyCurSp(VOID);

/* Combined Layer 1 + 2 + 3. Holds SCHEDULER_LOCK for a consistent snapshot. */
UINT32 TcbVerifyAll(VOID);

#ifdef __cplusplus
}
#endif
#endif /* _TCB_VERIFY_H */
