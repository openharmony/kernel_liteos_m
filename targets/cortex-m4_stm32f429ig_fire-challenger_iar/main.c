/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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
#include "los_sched.h"
#include "iar_stm32f429ig_fire-challenger.h"


STATIC VOID TaskSampleEntry2(VOID)
{
    while (1) {
        printf("TaskSampleEntry2 running...\n");
        LOS_TaskDelay(10000); /* 10 Seconds */
    }
}

STATIC VOID TaskSampleEntry1(VOID)
{
    while (1) {
        printf("TaskSampleEntry1 running...\n");
        LOS_TaskDelay(2000); /* 2 Seconds */
    }
}

WEAK VOID TaskSample(VOID)
{
    UINT32 uwRet;
    UINT32 taskID1;
    UINT32 taskID2;
    TSK_INIT_PARAM_S stTask = {0};

    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskSampleEntry1;
    stTask.uwStackSize = 0x1000;
    stTask.pcName = "TaskSampleEntry1";
    stTask.usTaskPrio = 6; /* Os task priority is 6 */
    uwRet = LOS_TaskCreate(&taskID1, &stTask);
    if (uwRet != LOS_OK) {
        printf("Task1 create failed\n");
    }

    stTask.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskSampleEntry2;
    stTask.uwStackSize = 0x1000;
    stTask.pcName = "TaskSampleEntry2";
    stTask.usTaskPrio = 7; /* Os task priority is 7 */
    uwRet = LOS_TaskCreate(&taskID2, &stTask);
    if (uwRet != LOS_OK) {
        printf("Task2 create failed\n");
    }
}

#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
#define SLEEP_TIME_CYCLE              90000000UL
#define SLEEP_TIME_MAX_RESPONSE_TIME  ((UINT32)-1)
#define TIM2_INI_PRIORITY             3
unsigned int g_sleepTimePeriod;
TIM_TimeBaseInitTypeDef g_timerInit = { 0 };
unsigned long long GetSleepTimeNs(VOID)
{
    unsigned long long currTime = g_sleepTimePeriod + TIM_GetCounter(TIM2);
    currTime = (currTime * OS_SYS_NS_PER_SECOND) / SLEEP_TIME_CYCLE;
    return currTime;
}

void SleepTimerStop(void)
{
    TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
    TIM_DeInit(TIM2);
    TIM_Cmd(TIM2, DISABLE);
}

void SleepTimerStart(unsigned long long sleepTimeNs)
{
    unsigned long long period = (sleepTimeNs * SLEEP_TIME_CYCLE) / OS_SYS_NS_PER_SECOND;
    if (period > SLEEP_TIME_MAX_RESPONSE_TIME) {
        period = SLEEP_TIME_MAX_RESPONSE_TIME;
    }

    g_timerInit.TIM_Period = period;
    g_sleepTimePeriod = period;
    TIM_TimeBaseInit(TIM2, &g_timerInit);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)
{
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);

    SleepTimerStop();
}

unsigned int SleepTimerInit(void)
{
    NVIC_InitTypeDef nvic = { 0 };

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseStructInit(&g_timerInit);
    g_timerInit.TIM_Period = SLEEP_TIME_MAX_RESPONSE_TIME;
    g_timerInit.TIM_Prescaler = 0;
    TIM_TimeBaseInit(TIM2, &g_timerInit);

    nvic.NVIC_IRQChannel = TIM2_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = TIM2_INI_PRIORITY;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
    TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);
#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
    UINT32 ret = HalHwiCreate(TIM2_IRQn, TIM2_INI_PRIORITY, 0, TIM2_IRQHandler, 0);
    if (ret != LOS_OK) {
        printf("SleepTimerInit create time2 irq failed! ERROR: 0x%x\n", ret);
        return ret;
    }
#endif

    return LOS_OK;
}
#endif

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

    USART_Config();

    printf("\n\rhello world!!\n\r");

    ret = LOS_KernelInit();
    if (ret == LOS_OK) {
#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
    ret = LOS_SchedSleepInit(SleepTimerInit, SleepTimerStart, SleepTimerStop, GetSleepTimeNs);
    if (ret != LOS_OK) {
        goto EXIT;
    }
#endif
        TaskSample();
        LOS_Start();
    }

#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
EXIT:
#endif
    while (1) {
        __asm volatile("wfi");
    }
}

