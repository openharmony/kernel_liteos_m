/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of the conditions and the following disclaimer in the documentation and/or other materials
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

#ifndef _LOS_HWI_PRI_H
#define _LOS_HWI_PRI_H

#include "los_interrupt.h"
#include "los_list.h"
#if (LOSCFG_HWI_BOTTOM_HALF == 1)
#include "los_task.h"
#include "los_event.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @ingroup los_hwi_private
 * Per-IRQ interrupt handle form.
 *
 * Each IRQ has one HwiHandleInfo as the dispatch-table node. The table is
 * kernel-owned (kernel/base/los_hwi.c); per-arch vector entries (HalInterrupt
 * / IrqEntry / HalHwiInterruptDone) call OsIntHandle(hwiNum) which operates on
 * this table, so the dispatch body (g_intCount bookkeeping, hooks, pre/post,
 * handler call, respCount increment) lives once in the kernel instead of
 * being copy-pasted across ~17 per-variant HalInterrupt files.
 *
 * @par Attention:
 * <ul>
 * <li>hook: the interrupt handler (NULL when the IRQ is not created).</li>
 * <li>registerInfo: handler arg (pDevId / HWI_IRQ_PARAM_S*); 0 if none.
 *     Absorbs the old WITH_ARG g_hwiHandlerForm[].pParm.</li>
 * <li>respCount: number of times this IRQ has been dispatched
 *     (read via LOS_HwiRespCntGet, cleared on create).</li>
 * <li>shareMode: reserved (currently always 0).</li>
 * <li>next: reserved (currently always NULL).</li>
 * </ul>
 */
typedef struct HwiHandleInfo {
    HWI_PROC_FUNC hook;
    union {
        HWI_ARG_T shareMode;
        HWI_ARG_T registerInfo;
    };
    struct HwiHandleInfo *next;
    UINT32 respCount;
} HwiHandleInfo;

/**
 * @ingroup los_hwi_private
 * The kernel-owned interrupt handle form table.
 *
 * Defined in kernel/base/los_hwi.c, indexed by hwiNum. Per-arch dispatch
 * entries call OsIntHandle to dispatch against this table (the arch keeps
 * the real HW vector table g_hwiForm separately; this is the software
 * dispatch table).
 */
extern HwiHandleInfo g_hwiHandleForm[];

extern UINT32 g_intCount;

#if (LOSCFG_HWI_BOTTOM_HALF == 1)
#define HWI_BH_EVENT_MASK  0x01

typedef struct {
    LOS_DL_LIST entry;
    HWI_BOTTOM_HALF_FUNC workFunc;
    VOID *data;
} HwiBhworkItem;

/**
 * @ingroup los_hwi_private
 * @brief Initialize the interrupt bottom half subsystem.
 *
 * Creates the bottom half task + work list + event. Registered via
 * LOS_SYS_INIT(LEVEL_KERNEL, SYNC_3) in los_hwi.c.
 */
extern UINT32 OsHwiBottomHalfInit(VOID);
#endif

/**
 * @ingroup los_hwi_private
 * @brief Kernel-level interrupt dispatch entry.
 *
 * @par Description:
 * Called by the per-arch vector entry (e.g. arm/csky/xtensa HalInterrupt,
 * riscv32 HalHwiInterruptDone) with the active IRQ number and the already
 * resolved per-IRQ dispatch node. The arch entry resolves the node out of
 * its own g_hwiHandleForm (indexed consistently with its vector-table
 * convention) and passes it in, so the kernel never bare-indexes the
 * dispatch table itself (kernel-owned). Performs the arch-independent
 * dispatch body: ISR enter/exit hooks, pre/post interrupt hooks,
 * handler call, and respCount increment (read via LOS_HwiRespCntGet).
 * Implementation in kernel/base/los_hwi.c.
 *
 * @par Attention:
 * Per-arch quirks (e.g. xtensa HwiClear before the handler, csky
 * HalIrqEndCheckNeedSched, arm RUNSTOP SCR scrub) stay in the per-arch
 * vector entry around the OsIntHandle call.
 *
 * @param hwiNum      [IN] The hardware interrupt number.
 * @param handleForm  [IN] Pointer to the per-IRQ HwiHandleInfo dispatch node.
 */
extern VOID OsIntHandle(HWI_HANDLE_T hwiNum, HwiHandleInfo *handleForm);

/**
 * Delete a hardware interrupt.
 */
extern UINT32 ArchHwiDelete(HWI_HANDLE_T hwiNum, HwiIrqParam *irqParam);

/**
 * Create a hardware interrupt.
 *
 * Implemented per arch in arch/<arch>/common/los_common_interrupt.c.
 */
extern UINT32 ArchHwiCreate(HWI_HANDLE_T hwiNum, HWI_PRIOR_T hwiPrio, HWI_MODE_T mode,
                            HWI_PROC_FUNC handler, HwiIrqParam *irqParam);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_HWI_PRI_H */
