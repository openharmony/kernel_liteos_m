/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
 * Copyright (c) 2021 Nuclei Limited. All rights reserved.
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

#include <stdio.h>
#include <stdarg.h>
#include "los_arch.h"
#include "los_arch_interrupt.h"
#include "los_arch_context.h"
#include "los_hwi_pri.h"
#include "los_task.h"
#include "los_debug.h"
#include "nuclei_sdk_hal.h"

STATIC UINT32 HwiUnmask(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    ECLIC_EnableIRQ(hwiNum);

    return LOS_OK;
}

STATIC UINT32 HwiMask(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    ECLIC_DisableIRQ(hwiNum);

    return LOS_OK;
}

STATIC UINT32 HwiSetPriority(HWI_HANDLE_T hwiNum, HWI_PRIOR_T priority)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (priority > OS_HWI_PRIO_HIGHEST || priority < OS_HWI_PRIO_LOWEST) {
        return OS_ERRNO_HWI_PRIO_INVALID;
    }

    ECLIC_SetPriorityIRQ(hwiNum, (hwiPrio & 0xffff));

    return LOS_OK;
}

LITE_OS_SEC_TEXT_INIT VOID HalHwiInit(VOID)
{
    // already setup interrupt vectors
}

/* ****************************************************************************
 Function    : HalDisplayTaskInfo
 Description : display the task list
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
VOID HalDisplayTaskInfo(VOID)
{
    TSK_INFO_S taskInfo;
    UINT32 index;
    UINT32 ret;

    PRINTK("ID  Pri    Status     name \r\n");
    PRINTK("--  ---    ---------  ----\r\n");

    for (index = 0; index < LOSCFG_BASE_CORE_TSK_LIMIT; index++) {
        ret = LOS_TaskInfoGet(index, &taskInfo);
        if (ret != LOS_OK) {
            continue;
        }
        PRINTK("%d    %d     %s      %s \r\n",
               taskInfo.uwTaskID, taskInfo.usTaskPrio, OsConvertTskStatus(taskInfo.usTaskStatus), taskInfo.acName);
    }
    return;
}

/* ****************************************************************************
 Function    : HalUnalignedAccessFix
 Description : Unaligned access fixes are not supported by default
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
WEAK UINT32 HalUnalignedAccessFix(UINTPTR mcause, UINTPTR mepc, UINTPTR mtval, VOID *sp)
{
    /* Unaligned access fixes are not supported by default */
    PRINTK("Unaligned access fixes are not support by default!\r\n");
    return LOS_NOK;
}

__attribute__((always_inline)) inline VOID HalIntEnter(VOID)
{
    g_intCount += 1;
}

__attribute__((always_inline)) inline VOID HalIntExit(VOID)
{
    g_intCount -= 1;
}

/* Un-migrated arch variant: this file still owns g_hwiControllerOps + HwiControllerOpsGet
 * (pre-driver-layer model). Migrated pattern: driver-owned static ops table
 * under drivers/interrupt/ (see arm_nvic.c), selected via Kconfig. Migrate
 * this variant when it is next touched. */
STATIC HwiControllerOps g_hwiControllerOps = {
    .enableIrq      = HwiUnmask,
    .disableIrq     = HwiMask,
    .setIrqPriority = HwiSetPriority,
};

HwiControllerOps *HwiControllerOpsGet(VOID)
{
    return &g_hwiControllerOps;
}
