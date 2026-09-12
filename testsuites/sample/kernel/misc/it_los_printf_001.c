/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "it_los_misc.h"
#include "los_printf.h"

/* 用例简要描述: UartPuts 带锁/无锁双模式输出+len=0 不崩溃 */
static UINT32 TestCase(VOID)
{
    const CHAR msg[] = "[ItLosPrintf001] UartPuts direct output test\n";

    /* F-Pos: 带锁模式输出(板级强符号,经串口原语) */
    UartPuts(msg, (UINT32)(sizeof(msg) - 1), UART_WITH_LOCK);

    /* F-Pos: 无锁模式输出 */
    UartPuts(msg, (UINT32)(sizeof(msg) - 1), UART_WITHOUT_LOCK);

    /* F-Inv: len=0 空输出不崩溃 */
    UartPuts(msg, 0, UART_WITH_LOCK);

    return LOS_OK;
}

VOID ItLosPrintf001(VOID)
{
    TEST_ADD_CASE("ItLosPrintf001", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL1, TEST_FUNCTION);
}
