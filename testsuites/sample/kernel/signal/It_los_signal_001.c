/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

#include "It_los_signal.h"
#include "los_task.h"

STATIC volatile INT32 g_sigValue = -1;

STATIC VOID SigHandler(INT32 sig)
{
    g_sigValue = sig;
}

static UINT32 Testcase(VOID)
{
    SIG_HANDLER h = NULL;
    UINT32 ret;

    h = LOS_SignalSet(0, SigHandler);
    ICUNIT_ASSERT_EQUAL(h, SIG_ERR, h);

    h = LOS_SignalSet(LOS_SIGNAL_SUPPORT_MAX + 1, SigHandler);
    ICUNIT_ASSERT_EQUAL(h, SIG_ERR, h);

    h = LOS_SignalSet(SIGUSR1, SIG_DFL);
    ICUNIT_ASSERT_NOT_EQUAL(h, SIG_ERR, h);

    ret = LOS_SignalSend(LOS_CurTaskIDGet(), SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ICUNIT_ASSERT_EQUAL(g_sigValue, -1, g_sigValue);

    h = LOS_SignalSet(SIGUSR1, SigHandler);
    ICUNIT_ASSERT_NOT_EQUAL(h, SIG_ERR, h);

    ret = LOS_SignalSend(LOS_CurTaskIDGet(), SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ICUNIT_ASSERT_EQUAL(g_sigValue, SIGUSR1, g_sigValue);
    g_sigValue = -1;

    h = LOS_SignalSet(SIGUSR1, SIG_IGN);
    ICUNIT_ASSERT_NOT_EQUAL(h, SIG_ERR, h);

    ret = LOS_SignalSend(LOS_CurTaskIDGet(), SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SIGNAL_NO_SET, ret);
    ICUNIT_ASSERT_EQUAL(g_sigValue, -1, g_sigValue);

    return LOS_OK;
}

VOID ItLosSignal001(VOID)
{
    TEST_ADD_CASE("ItLosSignal001", Testcase, TEST_LOS, TEST_SIGNAL, TEST_LEVEL0, TEST_FUNCTION);
}
