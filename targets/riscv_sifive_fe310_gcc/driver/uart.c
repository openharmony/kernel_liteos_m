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

#include "uart.h"
#include "soc.h"
#include "los_reg.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/******* uart register offset *****/
#define UART_TX_DATA         0x00
#define UART_RX_DATA         0x04
#define UART_TX_CTRL         0x08
#define UART_RX_CTRL         0x0C
#define UART_IN_ENAB         0x10
#define UART_IN_PEND         0x14
#define UART_BR_DIV          0x18

INT32 UartPutc(INT32 c, VOID *file)
{
    (VOID)file;

    while (GET_UINT32(UART0_BASE + UART_TX_DATA) & 0x80000000) {

    }

    WRITE_UINT32((INT32)(c & 0xFF), UART0_BASE + UART_TX_DATA);

    return c;
}

INT32 UartOut(INT32 c, VOID *file)
{
    if (c == '\n') {
        return UartPutc('\r', file);
    }

    return UartPutc(c, file);
}

VOID UartInit()
{
    UINT32 uartDiv;

    /* Enable TX and RX channels */
    WRITE_UINT32(1 << 0, UART0_BASE + UART_TX_CTRL);
    WRITE_UINT32(1 << 0, UART0_BASE + UART_RX_CTRL);

    /* Set baud rate */
    uartDiv = UART0_CLK_FREQ / UART0_BAUDRAT - 1;
    WRITE_UINT32(uartDiv, UART0_BASE + UART_BR_DIV);

    /* Ensure that uart IRQ is disabled initially */
    WRITE_UINT32(0, UART0_BASE + UART_IN_ENAB);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
