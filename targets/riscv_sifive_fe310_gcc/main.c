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

#include "los_tick.h"
#include "los_task.h"
#include "los_config.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "uart.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

UINT8 g_memStart[OS_SYS_MEM_SIZE];

VOID TaskSampleEntry2(VOID)
{
    while(1) {
        LOS_TaskDelay(1000);
        printf("TaskSampleEntry2 running...\n\r");
    }
}


VOID TaskSampleEntry1(VOID)
{
    while(1) {
        LOS_TaskDelay(1000);
        printf("TaskSampleEntry1 running...\n\r");
    }

}

UINT32 TaskSample(VOID)
{
    UINT32  ret;
    UINT32  taskID1, taskID2;
    TSK_INIT_PARAM_S task1 = { 0 };
    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskSampleEntry1;
    task1.uwStackSize  = 0x1000;
    task1.pcName       = "TaskSampleEntry1";
    task1.usTaskPrio   = 6;
    ret = LOS_TaskCreate(&taskID1, &task1);

    task1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskSampleEntry2;
    task1.uwStackSize  = 0x1000;
    task1.pcName       = "TaskSampleEntry2";
    task1.usTaskPrio   = 7;
    ret = LOS_TaskCreate(&taskID2, &task1);

    return ret;
}

/*****************************************************************************
 Function    : main
 Description : Main function entry
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT INT32 main(VOID)
{
    UINT32 ret;

    UartInit();

    PRINTK("\n OHOS start \n\r");

    ret = LOS_KernelInit();
    if (ret != LOS_OK) {
        goto START_FAILED;
    }

    TaskSample();

    PRINTK("\n OHOS scheduler!!! \n\r");

    LOS_Start();

START_FAILED:
    while (1) {
        __asm volatile("wfi");
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
