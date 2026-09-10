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

#include "los_interrupt.h"
#include "los_trace.h"
#include "los_debug.h"
#include "los_hwi_pri.h"
#include "los_hook.h"
#ifdef LOSCFG_BASE_CORE_CPUP
#include "los_cpup_pri.h"
#endif
#include "los_init.h"
#if (LOSCFG_HWI_BOTTOM_HALF == 1)
#include "los_task.h"
#include "los_event.h"
STATIC UINT32 g_bhTaskId;
STATIC LOS_DL_LIST g_bhworkList;
STATIC LOS_DL_LIST g_bhworkFreeList;
STATIC HwiBhworkItem g_bhWorkBuf[LOSCFG_HWI_BOTTOM_HALF_WORK_LIMIT];
STATIC EVENT_CB_S g_bhEvent;
#endif

UINT32 g_intCount = 0;

UINT32 IntActive(VOID)
{
    UINT32 intSave = LOS_IntLock();
    UINT32 intCount = g_intCount;
    LOS_IntRestore(intSave);
    return intCount;
}

/* ===========================================================================
 * Kernel-level LOS_Hwi* APIs.
 *
 * Arch-independent concerns (NULL check, LOS_TRACE, orchestration) live here;
 * arch-specific work (dispatch table / NVIC / vector setup / createIrq +
 * rollback) is delegated to the existing ArchHwi* interfaces implemented per
 * arch in arch/<arch>/common/los_common_interrupt.c (and per-toolchain for
 * risc-v). LOS_TRACE is a no-op when LOSCFG_KERNEL_TRACE is off (default).
 *
 * Two-level trace: HWI_CREATE/HWI_DELETE marks the API entry
 * (hwiNum + prio + mode + handler); HWI_CREATE_SHARE/HWI_DELETE_SHARE marks
 * the per-node outcome (hwiNum + pDevId + ret).
 * ===========================================================================
 */

LITE_OS_SEC_TEXT UINT32 LOS_HwiCreate(HWI_HANDLE_T hwiNum, HWI_PRIOR_T hwiPrio,
                                      HWI_MODE_T hwiMode, HWI_PROC_FUNC hwiHandler,
                                      HWI_IRQ_PARAM_S *irqParam)
{
    UINT32 ret;

    if (hwiHandler == NULL) {
        return LOS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    LOS_TRACE(HWI_CREATE, hwiNum, hwiPrio, hwiMode, (UINTPTR)hwiHandler);
    ret = ArchHwiCreate(hwiNum, hwiPrio, hwiMode, hwiHandler, irqParam);
    LOS_TRACE(HWI_CREATE_SHARE, hwiNum,
              (UINTPTR)(irqParam != NULL ? irqParam->pDevId : NULL), ret);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiDelete(HWI_HANDLE_T hwiNum, HWI_IRQ_PARAM_S *irqParam)
{
    UINT32 ret;

    LOS_TRACE(HWI_DELETE, hwiNum);
    ret = ArchHwiDelete(hwiNum, irqParam);
    LOS_TRACE(HWI_DELETE_SHARE, hwiNum,
              (UINTPTR)(irqParam != NULL ? irqParam->pDevId : NULL), ret);
    return ret;
}

#if (LOSCFG_HWI_PRE_POST_PROCESS == 1)
STATIC HWI_PROC_HOOK g_hwiHookTable[HWI_PROC_HOOK_BUTT] = {0};

LITE_OS_SEC_TEXT VOID LOS_HwiPreHookReg(HWI_PROC_HOOK intPreHook)
{
    g_hwiHookTable[HWI_PRE_PROC_HOOK] = intPreHook;
}

LITE_OS_SEC_TEXT VOID LOS_HwiPostHookReg(HWI_PROC_HOOK intPostHook)
{
    g_hwiHookTable[HWI_POST_PROC_HOOK] = intPostHook;
}

STATIC INLINE VOID OsIntPre(HWI_HANDLE_T hwiNum)
{
    HWI_PROC_HOOK intHook = g_hwiHookTable[HWI_PRE_PROC_HOOK];
    if (intHook != NULL) {
        intHook(hwiNum);
    }
}

STATIC INLINE VOID OsIntPost(HWI_HANDLE_T hwiNum)
{
    HWI_PROC_HOOK intHook = g_hwiHookTable[HWI_POST_PROC_HOOK];
    if (intHook != NULL) {
        intHook(hwiNum);
    }
}
#endif

/* ===========================================================================
 * Kernel-level interrupt dispatch.
 *
 * Called by the per-arch vector entry (HalInterrupt) with the active IRQ num
 * and the already resolved per-IRQ dispatch node. The arch entry resolves the
 * node out of its own g_hwiHandleForm (indexed consistently with its
 * vector-table convention) and passes it in, so the kernel never bare-indexes
 * the dispatch table itself (-aligned; avoids the OS_SYS_VECTOR_CNT offset
 * mismatch that shadows the array bounds). g_intCount++/-- and CPUP
 * start/end are done here (kernel-owned); per-arch HalInterrupt only
 * does arch-specific init (RUNSTOP/HwiClear) + HwiNumGet + call this
 * function. This function does the arch-independent dispatch: ISR
 * enter/exit hooks, pre/post hooks, handler call, and respCount
 * increment (read via LOS_HwiRespCntGet).
 * LOS_TRACE is a no-op when LOSCFG_KERNEL_TRACE is off (default).
 * ===========================================================================
 */
VOID OsIntHandle(HWI_HANDLE_T hwiNum, HwiHandleInfo *handleForm)
{
    UINT32 intSave;
    if (handleForm == NULL) {
        return;
    }

    intSave = LOS_IntLock();
    g_intCount++;
    LOS_IntRestore(intSave);

    OsHookCall(LOS_HOOK_TYPE_ISR_ENTER, hwiNum);
    LOS_TRACE(HWI_RESPONSE_IN, hwiNum);

#if (LOSCFG_HWI_PRE_POST_PROCESS == 1)
    OsIntPre(hwiNum);
#endif

    handleForm->respCount++;
    /* Dispatch the IRQ handler. */
    if (handleForm->hook != NULL) {
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
        handleForm->hook((VOID *)(UINTPTR)handleForm->registerInfo);
#else
        handleForm->hook();
#endif
    }

#if (LOSCFG_HWI_PRE_POST_PROCESS == 1)
    OsIntPost(hwiNum);
#endif

    LOS_TRACE(HWI_RESPONSE_OUT, hwiNum);
    OsHookCall(LOS_HOOK_TYPE_ISR_EXIT, hwiNum);

    intSave = LOS_IntLock();
    g_intCount--;
    LOS_IntRestore(intSave);
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiRespCntGet(HWI_HANDLE_T hwiNum, UINT32 *respCount)
{
    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (respCount == NULL) {
        return LOS_ERRNO_HWI_PTR_NULL;
    }
    if ((hwiOps == NULL) || (hwiOps->getHandleForm == NULL)) {
        return LOS_ERRNO_HWI_NUM_INVALID;
    }
    HwiHandleInfo *hwiForm = (HwiHandleInfo *)hwiOps->getHandleForm(hwiNum);
    if (hwiForm == NULL) {
        return LOS_ERRNO_HWI_NUM_INVALID;
    }
    *respCount = hwiForm->respCount;
    return LOS_OK;
}

#ifdef LOSCFG_KERNEL_SMP
LITE_OS_SEC_TEXT UINT32 LOS_HwiSendIpi(HWI_HANDLE_T hwiNum, UINT32 cpuMask)
{
    (VOID)hwiNum;
    (VOID)cpuMask;
    return LOS_ERRNO_HWI_ARG_NOT_ENABLED;
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiSetAffinity(HWI_HANDLE_T hwiNum, UINT32 cpuMask)
{
    (VOID)hwiNum;
    (VOID)cpuMask;
    return LOS_ERRNO_HWI_AFFI_INVALID;
}
#endif

#if (LOSCFG_HWI_BOTTOM_HALF == 1)

STATIC VOID OsHwiBhTask(VOID)
{
    HwiBhworkItem *bhwork = NULL;
    UINT32 intSave = 0;
    HWI_BOTTOM_HALF_FUNC func = NULL;
    VOID *data = NULL;

    while (1) {
        if (LOS_ListEmpty(&g_bhworkList)) {
            (VOID)LOS_EventRead(&g_bhEvent, HWI_BH_EVENT_MASK,
                                 LOS_WAITMODE_CLR | LOS_WAITMODE_OR, LOS_WAIT_FOREVER);
        }
        intSave = LOS_IntLock();
        while (!LOS_ListEmpty(&g_bhworkList)) {
            bhwork = LOS_DL_LIST_ENTRY(g_bhworkList.pstNext, HwiBhworkItem, entry);
            LOS_ListDelInit(g_bhworkList.pstNext);
            func = bhwork->workFunc;
            data = bhwork->data;
            LOS_ListTailInsert(&g_bhworkFreeList, &bhwork->entry);
            LOS_IntRestore(intSave);

            func(data);

            intSave = LOS_IntLock();
        }
        LOS_IntRestore(intSave);
    }
}

UINT32 OsHwiBottomHalfInit(VOID)
{
    UINT32 ret;
    UINT32 i;
    TSK_INIT_PARAM_S taskParam = {0};

    LOS_ListInit(&g_bhworkList);
    LOS_ListInit(&g_bhworkFreeList);
    for (i = 0; i < LOSCFG_HWI_BOTTOM_HALF_WORK_LIMIT; i++) {
        LOS_ListTailInsert(&g_bhworkFreeList, &g_bhWorkBuf[i].entry);
    }
    ret = LOS_EventInit(&g_bhEvent);
    if (ret != LOS_OK) {
        return ret;
    }
    taskParam.pfnTaskEntry = (TSK_ENTRY_FUNC)OsHwiBhTask;
    taskParam.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    taskParam.pcName = "Int_Bottom_Half_Task";
    taskParam.usTaskPrio = LOS_TASK_PRIORITY_HIGHEST + 1;
    ret = LOS_TaskCreate(&g_bhTaskId, &taskParam);
    if (ret != LOS_OK) {
        LOS_EventDestroy(&g_bhEvent);
    }
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiBhworkAdd(HWI_BOTTOM_HALF_FUNC bhHandler, VOID *data)
{
    HwiBhworkItem *bhwork = NULL;
    UINT32 intSave;

    if (!OS_INT_ACTIVE) {
        return LOS_ERRNO_HWI_NOT_INTERRUPT_CONTEXT;
    }
    if (bhHandler == NULL) {
        return LOS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    intSave = LOS_IntLock();
    if (LOS_ListEmpty(&g_bhworkFreeList)) {
        LOS_IntRestore(intSave);
        return LOS_ERRNO_HWI_NO_MEMORY;
    }
    bhwork = LOS_DL_LIST_ENTRY(g_bhworkFreeList.pstNext, HwiBhworkItem, entry);
    LOS_ListDelInit(g_bhworkFreeList.pstNext);
    bhwork->data = data;
    bhwork->workFunc = bhHandler;
    LOS_ListTailInsert(&g_bhworkList, &bhwork->entry);
    LOS_IntRestore(intSave);

    return LOS_EventWrite(&g_bhEvent, HWI_BH_EVENT_MASK);
}

LOS_SYS_INIT(OsHwiBottomHalfInit, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_3);
#endif
