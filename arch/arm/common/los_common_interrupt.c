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
#include "los_hwi_pri.h"
#include "los_exc.h"
#include "los_interrupt.h"

EXC_PROC_FUNC g_excRegHook = NULL;

UINT32 ArchSetExcHook(EXC_PROC_FUNC excHook)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    g_excRegHook = excHook;
    LOS_IntRestore(intSave);
    return LOS_OK;
}

HwiHandleInfo g_hwiHandleForm[OS_VECTOR_CNT] = {0};

#ifdef __ICCARM__
#pragma location = ".data.vector"
#pragma data_alignment = LOSCFG_ARCH_HWI_VECTOR_ALIGN
/* *
 * @ingroup los_hwi
 * Hardware interrupt form mapping handling function array.
 */
HWI_PROC_FUNC g_hwiForm[OS_VECTOR_CNT] = {0};
#elif defined(__CC_ARM) || defined(__GNUC__)
LITE_OS_SEC_VEC
/* *
 * @ingroup los_hwi
 * Hardware interrupt form mapping handling function array.
 */
HWI_PROC_FUNC __attribute__((aligned(LOSCFG_ARCH_HWI_VECTOR_ALIGN))) g_hwiForm[OS_VECTOR_CNT] = {0};
#endif

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
LITE_OS_SEC_TEXT_MINOR VOID HalHwiDefaultHandler(VOID)
{
    PRINT_ERR("%s irqnum:%u\n", __FUNCTION__, LOS_HwiCurIrqNum());
    while (1) {}
}

#if (LOSCFG_DEBUG_TOOLS == 1)
STATIC UINT32 g_hwiFormCnt[OS_HWI_MAX_NUM] = {0};
STATIC CHAR *g_hwiFormName[OS_HWI_MAX_NUM] = {0};

UINT32 OsGetHwiFormCnt(UINT32 index)
{
    return g_hwiFormCnt[index];
}

CHAR *OsGetHwiFormName(UINT32 index)
{
    return g_hwiFormName[index];
}

BOOL OsHwiIsCreated(UINT32 index)
{
    if (g_hwiForm[index] != (HWI_PROC_FUNC)HalHwiDefaultHandler) {
        return TRUE;
    }

    return FALSE;
}
#endif

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)

/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector, VOID *arg)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalInterrupt;
        g_hwiHandleForm[num + OS_SYS_VECTOR_CNT].hook = vector;
        g_hwiHandleForm[num + OS_SYS_VECTOR_CNT].registerInfo = (HWI_ARG_T)(UINTPTR)arg;
    }
}
#else
/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = HalInterrupt;
        g_hwiHandleForm[num + OS_SYS_VECTOR_CNT].hook = vector;
    }
}
#endif

VOID *HalGetHandleForm(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return NULL;
    }
    return &g_hwiHandleForm[hwiNum + OS_SYS_VECTOR_CNT];
}
