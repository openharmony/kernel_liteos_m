/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#include "los_base.h"
#include "los_hwi_pri.h"
#include "riscv_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

LITE_OS_SEC_BSS UINT32  g_intCount = 0;
LITE_OS_SEC_BSS UINT32 g_hwiFormCnt[OS_HWI_MAX_NUM];

LITE_OS_SEC_DATA_INIT HWI_HANDLE_FORM_S g_hwiForm[OS_HWI_MAX_NUM] = {
    { .pfnHook = NULL, .uwParam = 0 }, // 0 User software interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 1 Supervisor software interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 2 Reserved
    { .pfnHook = OsHwiDefaultHandler, .uwParam = 0 }, // 3 Machine software interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 4 User timer interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 5 Supervisor timer interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 6  Reserved
    { .pfnHook = OsHwiDefaultHandler, .uwParam = 0 }, // 7 Machine timer interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 8  User external interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 9 Supervisor external interrupt handler
    { .pfnHook = NULL, .uwParam = 0 }, // 10 Reserved
    { .pfnHook = OsHwiDefaultHandler, .uwParam = 0 }, // 11 Machine external interrupt handler
    { .pfnHook = OsHwiDefaultHandler, .uwParam = 0 }, // 12 NMI handler
    { .pfnHook = NULL, .uwParam = 0 }, // 13 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 14 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 15 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 16 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 17 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 18 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 19 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 20 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 21 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 22 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 23 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 24 Reserved
    { .pfnHook = NULL, .uwParam = 0 }, // 25 Reserved
};

LITE_OS_SEC_TEXT_INIT VOID OsHwiDefaultHandler(UINTPTR arg)
{
    (VOID)arg;
    PRINT_ERR("default handler\n");
    while (1) {
    }
}

LITE_OS_SEC_TEXT_INIT VOID OsHwiInit(VOID)
{
    UINT32 index;
    for (index = OS_RISCV_SYS_VECTOR_CNT; index < OS_HWI_MAX_NUM; index++) {
        g_hwiForm[index].pfnHook = OsHwiDefaultHandler;
        g_hwiForm[index].uwParam = 0;
    }
}

typedef VOID (*HwiProcFunc)(UINTPTR);
LITE_OS_SEC_TEXT_INIT VOID OsHwiInterruptDone(HWI_HANDLE_T hwiNum)
{
    g_intCount++;

    HWI_HANDLE_FORM_S *hwiForm = &g_hwiForm[hwiNum];
    HwiProcFunc func = (HwiProcFunc)(hwiForm->pfnHook);
    func(hwiForm->uwParam);

    ++g_hwiFormCnt[hwiNum];

    g_intCount--;
}

LITE_OS_SEC_TEXT UINT32 OsGetHwiFormCnt(HWI_HANDLE_T hwiNum)
{
    if (hwiNum < OS_HWI_MAX_NUM) {
        return g_hwiFormCnt[hwiNum];
    }

    return LOS_NOK;
}

LITE_OS_SEC_TEXT HWI_HANDLE_FORM_S *OsGetHwiForm(VOID)
{
    return g_hwiForm;
}

/*****************************************************************************
 Function    : LOS_HwiCreate
 Description : create hardware interrupt
 Input       : hwiNum     --- hwi num to create
               hwiPrio    --- priority of the hwi
               hwiMode    --- hwi interrupt mode
               hwiHandler --- hwi handler
               irqParam   --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_HwiCreate(HWI_HANDLE_T hwiNum,
                                      HWI_PRIOR_T hwiPrio,
                                      HWI_MODE_T hwiMode,
                                      HWI_PROC_FUNC hwiHandler,
                                      HWI_IRQ_PARAM_S irqParam)
{
    UINT32 intSave;

    if (hwiHandler == NULL) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    if (g_hwiForm[hwiNum].pfnHook == NULL) {
        return OS_ERRNO_HWI_NUM_INVALID;
    } else if (g_hwiForm[hwiNum].pfnHook != OsHwiDefaultHandler) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }
    if ((hwiPrio < OS_HWI_PRIO_LOWEST) || (hwiPrio > OS_HWI_PRIO_HIGHEST)) {
        return OS_ERRNO_HWI_PRIO_INVALID;
    }

    intSave = LOS_IntLock();
    g_hwiForm[hwiNum].pfnHook = hwiHandler;
    g_hwiForm[hwiNum].uwParam = irqParam;

    if (hwiNum >= OS_RISCV_SYS_VECTOR_CNT) {
        OsSetLocalInterPri(hwiNum, hwiPrio);
    }

    LOS_IntRestore(intSave);

    return LOS_OK;
}

/*****************************************************************************
 Function    : LOS_HwiDelete
 Description : Delete hardware interrupt
 Input       : hwiNum   --- hwi num to delete
 Return      : LOS_OK on success or error code on failure
 *****************************************************************************/
LITE_OS_SEC_TEXT UINT32 LOS_HwiDelete(HWI_HANDLE_T hwiNum, HWI_IRQ_PARAM_S irqParam)
{
    UINT32 intSave;

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    intSave = LOS_IntLock();
    g_hwiForm[hwiNum].pfnHook = OsHwiDefaultHandler;
    g_hwiForm[hwiNum].uwParam = 0;
    LOS_IntRestore(intSave);
    return LOS_OK;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
