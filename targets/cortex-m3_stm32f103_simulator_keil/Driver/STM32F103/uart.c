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

#include <stdio.h>
#include "los_compiler.h"

#define OS_USART1  0x40013800
#define OS_USART2  0x40004400
#define OS_USART3  0x40004800
#define OS_USART4  0x40004C00
#define OS_USART5  0x40005000

#define OS_USART_USR   OS_USART1


#define GPIO_Pin_0                 ((UINT16)0x0001)  /* Pin 0 selected */
#define GPIO_Pin_1                 ((UINT16)0x0002)  /* Pin 1 selected */
#define GPIO_Pin_2                 ((UINT16)0x0004)  /* Pin 2 selected */
#define GPIO_Pin_3                 ((UINT16)0x0008)  /* Pin 3 selected */
#define GPIO_Pin_4                 ((UINT16)0x0010)  /* Pin 4 selected */
#define GPIO_Pin_5                 ((UINT16)0x0020)  /* Pin 5 selected */
#define GPIO_Pin_6                 ((UINT16)0x0040)  /* Pin 6 selected */
#define GPIO_Pin_7                 ((UINT16)0x0080)  /* Pin 7 selected */
#define GPIO_Pin_8                 ((UINT16)0x0100)  /* Pin 8 selected */
#define GPIO_Pin_9                 ((UINT16)0x0200)  /* Pin 9 selected */
#define GPIO_Pin_10                ((UINT16)0x0400)  /* Pin 10 selected */
#define GPIO_Pin_11                ((UINT16)0x0800)  /* Pin 11 selected */
#define GPIO_Pin_12                ((UINT16)0x1000)  /* Pin 12 selected */
#define GPIO_Pin_13                ((UINT16)0x2000)  /* Pin 13 selected */
#define GPIO_Pin_14                ((UINT16)0x4000)  /* Pin 14 selected */
#define GPIO_Pin_15                ((UINT16)0x8000)  /* Pin 15 selected */
#define GPIO_Pin_All               ((UINT16)0xFFFF)  /* All pins selected */

#define   GPIOA                 0x40010800
#define   GPIOB                 0x40010C00
#define   GPIOC                 0x40011000
#define   GPIOD                 0x40011400
#define   GPIOE                 0x40011800
#define   GPIOF                 0x40011C00
#define   GPIOG                 0x40012000

#define  RCC_REG_BASE           0x40021000
#define  RCC_APB2ENR_OFFSET     0x18
#define  RCC_APB1ENR_OFFSET     0x1C

#if (OS_USART_USR == OS_USART1)
#define RCC_APBENR_USART_EN  0x4004
#define  GPIO_RxPin                 GPIO_Pin_10
#define  GPIO_TxPin                 GPIO_Pin_9
#define  GPIOx                      GPIOA

#elif (OS_USART_USR == OS_USART2)
#define RCC_APBENR_USART_EN  0x20000
#define  GPIO_TxPin                 GPIO_Pin_5
#define  GPIO_RxPin                 GPIO_Pin_6
#define  GPIOx                      GPIOD

#elif (OS_USART_USR == OS_USART3)
#define RCC_APBENR_USART_EN  0x40000
#define  GPIO_RxPin                 GPIO_Pin_11
#define  GPIO_TxPin                 GPIO_Pin_10
#define  GPIOx                      GPIOB

#elif (OS_USART_USR == OS_USART4)
#define RCC_APBENR_USART_EN  0x80000
#define  GPIO_RxPin                 GPIO_Pin_11
#define  GPIO_TxPin                 GPIO_Pin_10
#define  GPIOx                      GPIOC

#elif (OS_USART_USR == OS_USART5)
#define RCC_APBENR_USART_EN  0x100000
#define  GPIO_RxPin                 GPIO_Pin_2
#define  GPIO_TxPin                 GPIO_Pin_12
#define  GPIOx                      GPIOC

#endif

typedef struct tagUsartMap
{
    volatile UINT32 SR;
    volatile UINT32 DR;
    volatile UINT32 BRR;
    volatile UINT32 CR1;
    volatile UINT32 CR2;
    volatile UINT32 CR3;
    volatile UINT32 GTPR;
}USART_MAP_S;

typedef struct
{
  UINT32 USART_BaudRate;
  UINT16 USART_WordLength;
  UINT16 USART_StopBits;
  UINT16 USART_Parity;
  UINT16 USART_Mode;
  UINT16 USART_HardwareFlowControl;
  UINT16 USART_Clock;
  UINT16 USART_CPOL;
  UINT16 USART_CPHA;
  UINT16 USART_LastBit;
}USART_INIT_S;

/* USART Word Length ---------------------------------------------------------*/
#define USART_WordLength_8b                  ((UINT16)0x0000)
#define USART_WordLength_9b                  ((UINT16)0x1000)

/* USART Stop Bits -----------------------------------------------------------*/
#define USART_StopBits_1                     ((UINT16)0x0000)
#define USART_StopBits_0_5                   ((UINT16)0x1000)
#define USART_StopBits_2                     ((UINT16)0x2000)
#define USART_StopBits_1_5                   ((UINT16)0x3000)

/* USART Parity --------------------------------------------------------------*/
#define USART_Parity_No                      ((UINT16)0x0000)
#define USART_Parity_Even                    ((UINT16)0x0400)
#define USART_Parity_Odd                     ((UINT16)0x0600)

/* USART Mode ----------------------------------------------------------------*/
#define USART_Mode_Rx                        ((UINT16)0x0004)
#define USART_Mode_Tx                        ((UINT16)0x0008)

/* USART Hardware Flow Control -----------------------------------------------*/
#define USART_HardwareFlowControl_None       ((UINT16)0x0000)
#define USART_HardwareFlowControl_RTS        ((UINT16)0x0100)
#define USART_HardwareFlowControl_CTS        ((UINT16)0x0200)
#define USART_HardwareFlowControl_RTS_CTS    ((UINT16)0x0300)

/* USART Clock ---------------------------------------------------------------*/
#define USART_Clock_Disable                  ((UINT16)0x0000)
#define USART_Clock_Enable                   ((UINT16)0x0800)

/* USART Clock Polarity ------------------------------------------------------*/
#define USART_CPOL_Low                       ((UINT16)0x0000)
#define USART_CPOL_High                      ((UINT16)0x0400)

/* USART Clock Phase ---------------------------------------------------------*/
#define USART_CPHA_1Edge                     ((UINT16)0x0000)
#define USART_CPHA_2Edge                     ((UINT16)0x0200)


/* USART Last Bit ------------------------------------------------------------*/
#define USART_LastBit_Disable                ((UINT16)0x0000)
#define USART_LastBit_Enable                 ((UINT16)0x0100)

enum GPIOSpeed_TypeDef
{
  GPIO_Speed_10MHz = 1,
  GPIO_Speed_2MHz,
  GPIO_Speed_50MHz
};

enum GPIOMode_TypeDef
{ GPIO_Mode_AIN = 0x0,
  GPIO_Mode_IN_FLOATING = 0x04,
  GPIO_Mode_IPD = 0x28,
  GPIO_Mode_IPU = 0x48,
  GPIO_Mode_Out_OD = 0x14,
  GPIO_Mode_Out_PP = 0x10,
  GPIO_Mode_AF_OD = 0x1C,
  GPIO_Mode_AF_PP = 0x18
};

typedef struct
{
  UINT16 GPIO_Pin;
  enum GPIOSpeed_TypeDef GPIO_Speed;
  enum GPIOMode_TypeDef GPIO_Mode;
}GPIO_InitTypeDef;

typedef struct
{
  volatile UINT32 CRL;
  volatile UINT32 CRH;
  volatile UINT32 IDR;
  volatile UINT32 ODR;
  volatile UINT32 BSRR;
  volatile UINT32 BRR;
  volatile UINT32 LCKR;
}GPIO_TypeDef;

VOID GPIO_Init(GPIO_TypeDef * pGPIO, GPIO_InitTypeDef * GPIO_InitStruct)
{
  UINT32 currentmode = 0x00, currentpin = 0x00, pinpos = 0x00, pos = 0x00;
  UINT32 tmpreg = 0x00, pinmask = 0x00;

/*---------------------------- GPIO Mode Configuration -----------------------*/
  currentmode = ((UINT32)GPIO_InitStruct->GPIO_Mode) & ((UINT32)0x0F);

  if ((((UINT32)GPIO_InitStruct->GPIO_Mode) & ((UINT32)0x10)) != 0x00)
  {
    /* Output mode */
    currentmode |= (UINT32)GPIO_InitStruct->GPIO_Speed;
  }

/*---------------------------- GPIO CRL Configuration ------------------------*/
  /* Configure the eight low port pins */
  if (((UINT32)GPIO_InitStruct->GPIO_Pin & ((UINT32)0x00FF)) != 0x00)
  {
    tmpreg = pGPIO->CRL;

    for (pinpos = 0x00; pinpos < 0x08; pinpos++)
    {
      pos = ((UINT32)0x01) << pinpos;
      /* Get the port pins position */
      currentpin = (GPIO_InitStruct->GPIO_Pin) & pos;

      if (currentpin == pos)
      {
        pos = pinpos << 2;
        /* Clear the corresponding low control register bits */
        pinmask = ((UINT32)0x0F) << pos;
        tmpreg &= ~pinmask;

        /* Write the mode configuration in the corresponding bits */
        tmpreg |= (currentmode << pos);

        /* Reset the corresponding ODR bit */
        if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPD)
        {
          pGPIO->BRR = (((UINT32)0x01) << pinpos);
        }
        /* Set the corresponding ODR bit */
        if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPU)
        {
          pGPIO->BSRR = (((UINT32)0x01) << pinpos);
        }
      }
    }
    pGPIO->CRL = tmpreg;
  }
/*---------------------------- GPIO CRH Configuration ------------------------*/
  /* Configure the eight high port pins */
  if (GPIO_InitStruct->GPIO_Pin > 0x00FF)
  {
    tmpreg = pGPIO->CRH;
    for (pinpos = 0x00; pinpos < 0x08; pinpos++)
    {
      pos = (((UINT32)0x01) << (pinpos + 0x08));
      /* Get the port pins position */
      currentpin = ((GPIO_InitStruct->GPIO_Pin) & pos);
      if (currentpin == pos)
      {
        pos = pinpos << 2;
        /* Clear the corresponding high control register bits */
        pinmask = ((UINT32)0x0F) << pos;
        tmpreg &= ~pinmask;

        /* Write the mode configuration in the corresponding bits */
        tmpreg |= (currentmode << pos);

        /* Reset the corresponding ODR bit */
        if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPD)
        {
          pGPIO->BRR = (((UINT32)0x01) << (pinpos + 0x08));
        }
        /* Set the corresponding ODR bit */
        if (GPIO_InitStruct->GPIO_Mode == GPIO_Mode_IPU)
        {
          pGPIO->BSRR = (((UINT32)0x01) << (pinpos + 0x08));
        }
      }
    }
    pGPIO->CRH = tmpreg;
  }
}

UINT32 USART_Init(USART_MAP_S * USARTx, USART_INIT_S * pstInitParam)
{
    UINT32  tmpreg;
    UINT32  apbclock;
    UINT32  integerdivider;
    UINT32  fractionaldivider;

    /*---------------------------- USART CR2 Configuration -----------------------*/
    tmpreg = USARTx->CR2;
    /* Clear STOP[13:12], CLKEN, CPOL, CPHA and LBCL bits */
    tmpreg &= 0xC0FF;

    /* Configure the USART Stop Bits, Clock, CPOL, CPHA and LastBit ------------*/
    /* Set STOP[13:12] bits according to USART_Mode value */
    /* Set CPOL bit according to USART_CPOL value */
    /* Set CPHA bit according to USART_CPHA value */
    /* Set LBCL bit according to USART_LastBit value */
    tmpreg |= (UINT32)pstInitParam->USART_StopBits | pstInitParam->USART_Clock |
    pstInitParam->USART_CPOL | pstInitParam->USART_CPHA |
    pstInitParam->USART_LastBit;

    /* Write to USART CR2 */
    USARTx->CR2 = (UINT16)tmpreg;

    /*---------------------------- USART CR1 Configuration -----------------------*/
    tmpreg = 0x00;
    tmpreg = USARTx->CR1;
    /* Clear M, PCE, PS, TE and RE bits */
    tmpreg &= 0xE9F3;

    /* Configure the USART Word Length, Parity and mode ----------------------- */
    /* Set the M bits according to USART_WordLength value */
    /* Set PCE and PS bits according to USART_Parity value */
    /* Set TE and RE bits according to USART_Mode value */
    tmpreg |= (UINT32)pstInitParam->USART_WordLength | pstInitParam->USART_Parity |
    pstInitParam->USART_Mode;

    /* Write to USART CR1 */
    USARTx->CR1 = (UINT16)tmpreg;

    /*---------------------------- USART CR3 Configuration -----------------------*/
    tmpreg = 0x00;
    tmpreg = USARTx->CR3;
    /* Clear CTSE and RTSE bits */
    tmpreg &= 0xFCFF;

    /* Configure the USART HFC -------------------------------------------------*/
    /* Set CTSE and RTSE bits according to USART_HardwareFlowControl value */
    tmpreg |= pstInitParam->USART_HardwareFlowControl;

    /* Write to USART CR3 */
    USARTx->CR3 = (UINT16)tmpreg;

    /*---------------------------- USART BRR Configuration -----------------------*/
    tmpreg = 0x00;

    /* Configure the USART Baud Rate -------------------------------------------*/
    if ((*(UINT32*)&USARTx) == OS_USART1)
    {
        apbclock = 8000000;
    }
    else
    {
        apbclock = 36000000;
    }

    /* Determine the integer part */
    integerdivider = ((0x19 * apbclock) / (0x04 * (pstInitParam->USART_BaudRate)));
    tmpreg = (integerdivider / 0x64) << 0x04;

    /* Determine the fractional part */
    fractionaldivider = integerdivider - (0x64 * (tmpreg >> 0x04));
    tmpreg |= ((((fractionaldivider * 0x10) + 0x32) / 0x64)) & ((UINT8)0x0F);

    /* Write to USART BRR */
    USARTx->BRR = (UINT16)tmpreg;

    /* UART enable */
    USARTx->CR1 |= 0x2000;

    return LOS_OK;
}

int fputc(int ch, FILE *f)
{
  USART_MAP_S * USARTx =  (USART_MAP_S *)OS_USART_USR;

  USARTx->DR = (UINT8)ch;

  while((USARTx->SR & 0x80) == 0);

  return ch;
}

void uart_puts(char * s)
{
  USART_MAP_S * USARTx =  (USART_MAP_S *)OS_USART_USR;

  while (*s)
  {
    USARTx->DR = *s++;
    while((USARTx->SR & 0x80) == 0);
  }
}

UINT32 LOS_UartBaseInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_INIT_S   USART_InitStructure;
  USART_MAP_S * USARTx =  (USART_MAP_S *)OS_USART_USR;

    /* uart rcc enable */
#if (OS_USART_USR == OS_USART1)
    *(volatile UINT32 *)(RCC_REG_BASE + RCC_APB2ENR_OFFSET) |= RCC_APBENR_USART_EN;
#else
    *(volatile UINT32 *)(RCC_REG_BASE + RCC_APB1ENR_OFFSET) |= RCC_APBENR_USART_EN;
#endif

  /* Configure USARTx_Tx as alternate function push-pull */
  GPIO_InitStructure.GPIO_Pin = GPIO_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init((GPIO_TypeDef*)GPIOx, &GPIO_InitStructure);

  /* Configure USARTx_Rx as input floating */
  GPIO_InitStructure.GPIO_Pin = GPIO_RxPin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init((GPIO_TypeDef*)GPIOx, &GPIO_InitStructure);

  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

  USART_InitStructure.USART_Clock = USART_Clock_Disable;
  USART_InitStructure.USART_CPOL = USART_CPOL_Low;
  USART_InitStructure.USART_CPHA = USART_CPHA_2Edge;
  USART_InitStructure.USART_LastBit = USART_LastBit_Disable;


  USART_Init(USARTx, &USART_InitStructure);

  return 0;
}
