/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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
 *
 * Description: Provide a task example.
 */

#include "task_sample.h"
#include "nuclei_sdk_hal.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_interrupt.h"
#include "los_arch_interrupt.h"
#include "los_task.h"
#include "los_tick.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

UINT8 __attribute__ ((aligned (8))) g_memStart[OS_SYS_MEM_SIZE];

VOID TaskSampleEntry2(VOID)
{
    while (1) {
        printf("TaskSampleEntry2 running...\n");
        LOS_TaskDelay(10000); /* 10 Seconds */
    }
}

VOID TaskSampleEntry1(VOID)
{
    while (1) {
        printf("TaskSampleEntry1 running...\n");
        LOS_TaskDelay(2000); /* 2 Seconds */
    }
}

VOID EXTI0_IRQHandler(VOID)
{
    if (RESET != exti_interrupt_flag_get(WAKEUP_KEY_PIN)) {
        if (RESET == gd_rvstar_key_state_get(KEY_WAKEUP)) {
            HalDisplayTaskInfo();
        }
    }
    exti_interrupt_flag_clear(WAKEUP_KEY_PIN);
}

VOID TaskSample(VOID)
{
    UINT32 uwRet;
    UINT32 taskID1;
    UINT32 taskID2;
    TSK_INIT_PARAM_S stTask = {0};

    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskSampleEntry1;
    stTask.uwStackSize = 0x0800;
    stTask.pcName = "TaskSampleEntry1";
    stTask.usTaskPrio = 6; /* Os task priority is 6 */
    uwRet = LOS_TaskCreate(&taskID1, &stTask);
    if (uwRet != LOS_OK) {
        printf("Task1 create failed\n");
    }

    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskSampleEntry2;
    stTask.uwStackSize = 0x0800;
    stTask.pcName = "TaskSampleEntry2";
    stTask.usTaskPrio = 7; /* Os task priority is 7 */
    uwRet = LOS_TaskCreate(&taskID2, &stTask);
    if (uwRet != LOS_OK) {
        printf("Task2 create failed\n");
    }

    HalHwiInit();
    HalHwiCreate(EXTI0_IRQn, 9, ECLIC_NON_VECTOR_INTERRUPT, EXTI0_IRQHandler, ECLIC_LEVEL_TRIGGER);
}

VOID RunTaskSample(VOID)
{
    UINT32 ret;
    ret = LOS_KernelInit();
    if (ret == LOS_OK) {
        TaskSample();
        LOS_Start();
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
