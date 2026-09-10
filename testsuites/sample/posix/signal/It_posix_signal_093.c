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


#include "It_posix_signal.h"
#include <pthread.h>

static VOID Sig093Handler(int sig)
{
    (VOID)sig;
}

/* 用例简要描述: kill 向自身发送已注册信号/非法任务号-1 */
/* 用例简要描述: kill 正负路径 */
static UINT32 Testcase(VOID)
{
    sigset_t set;
    int ret;

    /* 前置: 注册 SIGUSR1(LOS_SignalSend 要求 sigSetFlag 已置位,los_signal.c NO_SET 守卫) */
    (VOID)signal(SIGUSR1, Sig093Handler);

    (VOID)sigemptyset(&set);
    (VOID)sigaddset(&set, SIGUSR1);

    /* F-Pos: kill 向当前任务(pthread_self==任务ID)发送已注册信号 */
    ret = kill((pid_t)pthread_self(), SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    /* F-Inv: 非法任务号 -> LOS_SignalSend 失败 -> -1
   (注: 不测未注册信号号路径——sigSetFlag 为任务级持久状态,前序 signal 用例
    可能已注册同名信号,全量下注册态不可假设) */
    ret = kill((pid_t)0xFFFF, SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    return LOS_OK;
}

VOID ItPosixSignal093(VOID)
{
    TEST_ADD_CASE("ItPosixSignal093", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
