/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 Nuclei Limited. All rights reserved.
  * All rights reserved.
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "nuclei_sdk_hal.h"
#include "task_sample.h"
#include "target_config.h"

void user_key_exti_config()
{
    /* enable the AF clock */
    rcu_periph_clock_enable(RCU_AF);

    /* connect EXTI line to key GPIO pin */
    gpio_exti_source_select(WAKEUP_KEY_EXTI_PORT_SOURCE, WAKEUP_KEY_EXTI_PIN_SOURCE);

    /* configure key EXTI line */
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_0);
}
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN Init */
    gd_rvstar_key_init(WAKEUP_KEY_GPIO_PORT, KEY_MODE_EXTI);
    user_key_exti_config();
    RunTaskSample();
    while (1) {
    }
}
/************************ (C) COPYRIGHT Nuclei *****END OF FILE****/
