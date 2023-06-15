/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_arch_interrupt.h"
#include "los_debug.h"

UINT32 g_intCount = 0;

/* *
 * @ingroup los_hwi
 * Hardware interrupt form mapping handling function array.
 */
HWI_PROC_FUNC __attribute__((aligned(LOSCFG_ARCH_HWI_VECTOR_ALIGN))) g_hwiForm[OS_VECTOR_CNT] = {0};

VOID *ArchGetHwiFrom(VOID)
{
    return g_hwiForm;
}

/* ****************************************************************************
 Function    : HalHwiDefaultHandler
 Description : default handler of the hardware interrupt
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
VOID HalHwiDefaultHandler(VOID)
{
    PRINT_ERR("%s irqnum:%u\n", __FUNCTION__, ArchIntCurIrqNum());
    while (1) {}
}

WEAK VOID HalPreInterruptHandler(UINT32 arg)
{
    (VOID)arg;
    return;
}

WEAK VOID HalAftInterruptHandler(UINT32 arg)
{
    (VOID)arg;
    return;
}

STATIC UINT32 HwiNumValid(UINT32 num)
{
    return (num >= OS_SYS_VECTOR_CNT) && (num <= OS_VECTOR_CNT);
}

UINT32 ArchIntTrigger(HWI_HANDLE_T hwiNum)
{
    if (!HwiNumValid(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->triggerIrq == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->triggerIrq(hwiNum);
}

UINT32 ArchIntEnable(HWI_HANDLE_T hwiNum)
{
    if (!HwiNumValid(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->enableIrq == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->enableIrq(hwiNum);
}

UINT32 ArchIntDisable(HWI_HANDLE_T hwiNum)
{
    if (!HwiNumValid(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->disableIrq == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->disableIrq(hwiNum);
}

UINT32 ArchIntClear(HWI_HANDLE_T hwiNum)
{
    if (!HwiNumValid(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->clearIrq == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->clearIrq(hwiNum);
}

UINT32 ArchIntCurIrqNum(VOID)
{
    HwiControllerOps *hwiOps = ArchIntOpsGet();
    return hwiOps->getCurIrqNum();
}

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
typedef struct {
    HWI_PROC_FUNC pfnHandler;
    VOID *pParm;
} HWI_HANDLER_FUNC;

/* *
 * @ingroup los_hwi
 * Hardware interrupt handler form mapping handling function array.
 */
HWI_HANDLER_FUNC g_hwiHandlerForm[OS_VECTOR_CNT] = {{ (HWI_PROC_FUNC)0, (HWI_ARG_T)0 }};

/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector, VOID *arg)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalInterrupt;
        g_hwiHandlerForm[num + OS_SYS_VECTOR_CNT].pfnHandler = vector;
        g_hwiHandlerForm[num + OS_SYS_VECTOR_CNT].pParm = arg;
    }
}

#else
/* *
 * @ingroup los_hwi
 * Hardware interrupt handler form mapping handling function array.
 */
HWI_PROC_FUNC g_hwiHandlerForm[OS_VECTOR_CNT] = {0};

/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = HalInterrupt;
        g_hwiHandlerForm[num + OS_SYS_VECTOR_CNT] = vector;
    }
}
#endif

/* ****************************************************************************
 Function    : ArchHwiCreate
 Description : create hardware interrupt
 Input       : hwiNum   --- hwi num to create
               hwiPrio  --- priority of the hwi
               hwiMode  --- unused
               hwiHandler --- hwi handler
               irqParam --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 **************************************************************************** */
UINT32 ArchHwiCreate(HWI_HANDLE_T hwiNum,
                     HWI_PRIOR_T hwiPrio,
                     HWI_MODE_T hwiMode,
                     HWI_PROC_FUNC hwiHandler,
                     HwiIrqParam *irqParam)
{
    (VOID)hwiMode;
    UINT32 intSave;

    if (hwiHandler == NULL) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (g_hwiForm[hwiNum + OS_SYS_VECTOR_CNT] != (HWI_PROC_FUNC)HalHwiDefaultHandler) {
        return OS_ERRNO_HWI_ALREADY_CREATED;
    }

    if (hwiPrio > OS_HWI_PRIO_LOWEST) {
        return OS_ERRNO_HWI_PRIO_INVALID;
    }

    intSave = LOS_IntLock();
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
    if (irqParam != NULL) {
        OsSetVector(hwiNum, hwiHandler, irqParam->pDevId);
    } else {
        OsSetVector(hwiNum, hwiHandler, NULL);
    }
#else
    (VOID)irqParam;
    OsSetVector(hwiNum, hwiHandler);
#endif

    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->createIrq == NULL) {
        LOS_IntRestore(intSave);
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }
    hwiOps->createIrq(hwiNum, hwiPrio);

    LOS_IntRestore(intSave);

    return LOS_OK;
}

/* ****************************************************************************
 Function    : ArchHwiDelete
 Description : Delete hardware interrupt
 Input       : hwiNum   --- hwi num to delete
               irqParam --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT UINT32 ArchHwiDelete(HWI_HANDLE_T hwiNum, HwiIrqParam *irqParam)
{
    (VOID)irqParam;
    UINT32 intSave;

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    ArchIntDisable(hwiNum);

    intSave = LOS_IntLock();

    g_hwiForm[hwiNum + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalHwiDefaultHandler;

    LOS_IntRestore(intSave);

    return LOS_OK;
}

UINT32 ArchIsIntActive(VOID)
{
    return (g_intCount > 0);
}
