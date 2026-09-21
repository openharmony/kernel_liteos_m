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
#include "los_mp_pri.h"
#include "los_trace.h"
#include "los_debug.h"
#include "los_hwi_pri.h"
#include "los_percpu_pri.h"
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
#ifdef LOSCFG_KERNEL_SMP
LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_hwiBottomHalfSpin);
#endif
#endif

UINT32 g_intCount[LOSCFG_KERNEL_CORE_NUM] = {0};
#define OS_INT_COUNT()  g_intCount[ArchCurrCpuid()]

LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_hwiSpin);

UINT32 IntActive(VOID)
{
    UINT32 intSave = LOS_IntLock();
    UINT32 intCount = OS_INT_COUNT();
    LOS_IntRestore(intSave);
    return intCount;
}

/* ===========================================================================
 * Kernel-level LOS_Hwi* APIs.
 *
 * Arch-independent concerns (NULL check, LOS_TRACE, orchestration) live here;
 * arch-specific work (dispatch table fill / vector setup) is delegated to the
 * existing ArchHwi* interfaces implemented per
 * arch in arch/<arch>/common/los_common_interrupt.c (and per-toolchain for
 * risc-v). LOS_TRACE is a no-op when LOSCFG_KERNEL_TRACE is off (default).
 *
 * Two-level trace: HWI_CREATE/HWI_DELETE marks the API entry
 * (hwiNum + prio + mode + handler); HWI_CREATE_SHARE/HWI_DELETE_SHARE marks
 * the per-node outcome (hwiNum + pDevId + ret).
 * ===========================================================================
 */

STATIC UINT32 OsHwiCreate(HwiHandleInfo *hwiForm, HWI_MODE_T hwiMode, HWI_PROC_FUNC hwiHandler,
                          const HWI_IRQ_PARAM_S *irqParam)
{
    UINT32 intSave;

    (VOID)hwiMode;
    intSave = LOS_IntLock();
    if (hwiForm->hook == NULL) {
        hwiForm->hook = hwiHandler;
        hwiForm->registerInfo = (irqParam != NULL) ? (HWI_ARG_T)(UINTPTR)irqParam->pDevId : 0;
        hwiForm->respCount = 0;
        hwiForm->next = NULL;
    } else {
        LOS_IntRestore(intSave);
        return OS_ERRNO_HWI_ALREADY_CREATED;
    }
    LOS_IntRestore(intSave);
    return LOS_OK;
}

STATIC UINT32 OsHwiDel(HwiHandleInfo *hwiForm, HWI_HANDLE_T hwiNum)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    /* Disable IRQ first — prevents a racing interrupt from dispatching
     * via a NULL hook after the fields below are cleared. */
    (VOID)LOS_HwiDisable(hwiNum);
    hwiForm->hook = NULL;
    hwiForm->registerInfo = 0;
    hwiForm->respCount = 0;
    hwiForm->shareMode = 0;
    hwiForm->next = NULL;
    LOS_IntRestore(intSave);
    return LOS_OK;
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiCreate(HWI_HANDLE_T hwiNum, HWI_PRIOR_T hwiPrio,
                                      HWI_MODE_T hwiMode, HWI_PROC_FUNC hwiHandler,
                                      HWI_IRQ_PARAM_S *irqParam)
{
    UINT32 ret;
    HwiHandleInfo *hwiForm = NULL;
    HwiControllerOps *hwiOps = HwiControllerOpsGet();

    if (hwiHandler == NULL) {
        return LOS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    if ((hwiOps == NULL) || (hwiOps->getHandleForm == NULL)) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }

    hwiForm = (HwiHandleInfo *)hwiOps->getHandleForm(hwiNum);
    if (hwiForm == NULL) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    LOS_TRACE(HWI_CREATE, hwiNum, hwiPrio, hwiMode, (UINTPTR)hwiHandler);
    ret = OsHwiCreate(hwiForm, hwiMode, hwiHandler, irqParam);
    LOS_TRACE(HWI_CREATE_SHARE, hwiNum,
              (UINTPTR)(irqParam != NULL ? irqParam->pDevId : NULL), ret);
    if (ret != LOS_OK) {
        return ret;
    }

    if (hwiOps->setIrqPriority != NULL) {
        ret = hwiOps->setIrqPriority(hwiNum, hwiPrio);
        if (ret != LOS_OK) {
            (VOID)OsHwiDel(hwiForm, hwiNum);
            return ret;
        }
    }

    return LOS_OK;
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiDelete(HWI_HANDLE_T hwiNum, HWI_IRQ_PARAM_S *irqParam)
{
    UINT32 ret;
    HwiHandleInfo *hwiForm = NULL;
    HwiControllerOps *hwiOps = HwiControllerOpsGet();

    (VOID)irqParam;
    if ((hwiOps == NULL) || (hwiOps->getHandleForm == NULL)) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }

    hwiForm = (HwiHandleInfo *)hwiOps->getHandleForm(hwiNum);
    if (hwiForm == NULL) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    LOS_TRACE(HWI_DELETE, hwiNum);
    ret = OsHwiDel(hwiForm, hwiNum);
    LOS_TRACE(HWI_DELETE_SHARE, hwiNum,
              (UINTPTR)(irqParam != NULL ? irqParam->pDevId : NULL), ret);
    return ret;
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiTrigger(HWI_HANDLE_T hwiNum)
{
    HwiControllerOps *hwiOps = HwiControllerOpsGet();
    if ((hwiOps == NULL) || (hwiOps->triggerIrq == NULL)) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    LOS_TRACE(HWI_TRIGGER, hwiNum);
    return hwiOps->triggerIrq(hwiNum);
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiEnable(HWI_HANDLE_T hwiNum)
{
    HwiControllerOps *hwiOps = HwiControllerOpsGet();
    if ((hwiOps == NULL) || (hwiOps->enableIrq == NULL)) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    LOS_TRACE(HWI_ENABLE, hwiNum);
    return hwiOps->enableIrq(hwiNum);
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiDisable(HWI_HANDLE_T hwiNum)
{
    HwiControllerOps *hwiOps = HwiControllerOpsGet();
    if ((hwiOps == NULL) || (hwiOps->disableIrq == NULL)) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    LOS_TRACE(HWI_DISABLE, hwiNum);
    return hwiOps->disableIrq(hwiNum);
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiClear(HWI_HANDLE_T hwiNum)
{
    HwiControllerOps *hwiOps = HwiControllerOpsGet();
    if ((hwiOps == NULL) || (hwiOps->clearIrq == NULL)) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    LOS_TRACE(HWI_CLEAR, hwiNum);
    return hwiOps->clearIrq(hwiNum);
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiSetPriority(HWI_HANDLE_T hwiNum, HWI_PRIOR_T priority)
{
    HwiControllerOps *hwiOps = HwiControllerOpsGet();
    if ((hwiOps == NULL) || (hwiOps->setIrqPriority == NULL)) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    LOS_TRACE(HWI_SETPRI, hwiNum, priority);
    return hwiOps->setIrqPriority(hwiNum, priority);
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiCurIrqNum(VOID)
{
    return HwiControllerOpsGet()->getCurIrqNum();
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
    OS_INT_COUNT()++;
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
    OS_INT_COUNT()--;
    LOS_IntRestore(intSave);
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiRespCntGet(HWI_HANDLE_T hwiNum, UINT32 *respCount)
{
    HwiControllerOps *hwiOps = HwiControllerOpsGet();
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
    HwiControllerOps *hwiOps = HwiControllerOpsGet();

    if ((hwiOps == NULL) || (hwiOps->sendIpi == NULL)) {
        return LOS_ERRNO_HWI_ARG_NOT_ENABLED;
    }
    return hwiOps->sendIpi(hwiNum, cpuMask);
}

LITE_OS_SEC_TEXT UINT32 LOS_HwiSetAffinity(HWI_HANDLE_T hwiNum, UINT32 cpuMask)
{
    HwiControllerOps *hwiOps = HwiControllerOpsGet();

    if ((hwiOps == NULL) || (hwiOps->setIrqCpuAffinity == NULL)) {
        return LOS_ERRNO_HWI_AFFI_INVALID;
    }
    return hwiOps->setIrqCpuAffinity(hwiNum, cpuMask);
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
        HWI_BH_LOCK(intSave);
        while (!LOS_ListEmpty(&g_bhworkList)) {
            bhwork = LOS_DL_LIST_ENTRY(g_bhworkList.pstNext, HwiBhworkItem, entry);
            LOS_ListDelInit(g_bhworkList.pstNext);
            func = bhwork->workFunc;
            data = bhwork->data;
            LOS_ListTailInsert(&g_bhworkFreeList, &bhwork->entry);
            HWI_BH_UNLOCK(intSave);

            func(data);

            HWI_BH_LOCK(intSave);
        }
        HWI_BH_UNLOCK(intSave);
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
    HWI_BH_LOCK(intSave);
    if (LOS_ListEmpty(&g_bhworkFreeList)) {
        HWI_BH_UNLOCK(intSave);
        return LOS_ERRNO_HWI_NO_MEMORY;
    }
    bhwork = LOS_DL_LIST_ENTRY(g_bhworkFreeList.pstNext, HwiBhworkItem, entry);
    LOS_ListDelInit(g_bhworkFreeList.pstNext);
    bhwork->data = data;
    bhwork->workFunc = bhHandler;
    LOS_ListTailInsert(&g_bhworkList, &bhwork->entry);
    HWI_BH_UNLOCK(intSave);

    return LOS_EventWrite(&g_bhEvent, HWI_BH_EVENT_MASK);
}

LOS_SYS_INIT(OsHwiBottomHalfInit, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_3);
#endif
