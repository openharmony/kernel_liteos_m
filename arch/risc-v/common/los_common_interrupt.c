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

LITE_OS_SEC_BSS UINT32  g_intCount = 0;

UINT32 ArchIsIntActive(VOID)
{
    return (g_intCount > 0);
}

LITE_OS_SEC_TEXT_INIT VOID HalHwiDefaultHandler(VOID *arg)
{
    (VOID)arg;
    PRINT_ERR("default handler\n");
    while (1) {
    }
}

UINT32 ArchIntTrigger(HWI_HANDLE_T hwiNum)
{
    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->triggerIrq == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->triggerIrq(hwiNum);
}

UINT32 ArchIntEnable(HWI_HANDLE_T hwiNum)
{
    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->enableIrq == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->enableIrq(hwiNum);
}

UINT32 ArchIntDisable(HWI_HANDLE_T hwiNum)
{
    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->disableIrq == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->disableIrq(hwiNum);
}

UINT32 ArchIntClear(HWI_HANDLE_T hwiNum)
{
    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->clearIrq == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->clearIrq(hwiNum);
}

UINT32 ArchIntSetPriority(HWI_HANDLE_T hwiNum, HWI_PRIOR_T priority)
{
    HwiControllerOps *hwiOps = ArchIntOpsGet();
    if (hwiOps->setIrqPriority == NULL) {
        return OS_ERRNO_HWI_OPS_FUNC_NULL;
    }

    return hwiOps->setIrqPriority(hwiNum, priority);
}

UINT32 ArchIntCurIrqNum(VOID)
{
    HwiControllerOps *hwiOps = ArchIntOpsGet();
    return hwiOps->getCurIrqNum();
}
