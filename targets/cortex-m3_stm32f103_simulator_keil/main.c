/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
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
#include "los_compiler.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#pragma data_alignment=8
UINT8 g_memStart[LOSCFG_SYS_HEAP_SIZE];

VOID taskSampleEntry2(VOID)
{
    while(1) {
      LOS_TaskDelay(10000);
      printf("taskSampleEntry2 running...\n");
    }
}


VOID taskSampleEntry1(VOID)
{
    while(1) {
      LOS_TaskDelay(2000);
      printf("taskSampleEntry1 running...\n");
    }

}

UINT32 taskSample(VOID)
{
    UINT32  uwRet;
    UINT32 taskID1,taskID2;
    TSK_INIT_PARAM_S stTask1={0};
    stTask1.pfnTaskEntry = (TSK_ENTRY_FUNC)taskSampleEntry1;
    stTask1.uwStackSize  = 0X1000;
    stTask1.pcName       = "taskSampleEntry1";
    stTask1.usTaskPrio   = 6;
    uwRet = LOS_TaskCreate(&taskID1, &stTask1);
    if (uwRet != LOS_OK) {
        printf("create task1 failed\n");
    }

    stTask1.pfnTaskEntry = (TSK_ENTRY_FUNC)taskSampleEntry2;
    stTask1.uwStackSize  = 0X1000;
    stTask1.pcName       = "taskSampleEntry2";
    stTask1.usTaskPrio   = 7;
    uwRet = LOS_TaskCreate(&taskID2, &stTask1);
    if (uwRet != LOS_OK) {
        printf("create task2 failed\n");
    }
    return LOS_OK;
}


void LOS_HardBootInit()
{
    UINT32 uwRet = LOS_OK;

    uwRet = LOS_UartBaseInit();
    if (uwRet != LOS_OK)
    {
        return ;
    }

    return ;
}


/*****************************************************************************
 Function    : main
 Description : Main function entry
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT int main(void)
{
    unsigned int ret;

    //USART_Config();

    printf("\n\rhello world!!\n\r");

    ret = LOS_KernelInit();
    taskSample();
    if (ret == LOS_OK) {
        LOS_Start();
    }

    while (1) {
        __asm volatile("wfi");
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
