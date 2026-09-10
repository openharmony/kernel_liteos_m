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


#include "bug_isolate.h" /* BUG_03 等宏统一在此定义 */

#ifndef PRODUCT_BUG_ISOLATE
#include "It_posix_signal.h"

/* 用例简要描述: sigtimedwait 无 pending 短超时返回-1 */
static UINT32 Testcase(VOID)
{
    sigset_t set;
    siginfo_t info;
    struct timespec ts = { 0, 10000000 }; /* 10ms */
    int ret;

    (VOID)memset_s(&info, sizeof(info), 0, sizeof(info));
    (VOID)sigemptyset(&set);
    (VOID)sigaddset(&set, SIGUSR2);

    /* F-Inv: 无 pending 信号时短超时返回 -1 */
    ret = sigtimedwait(&set, &info, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    return LOS_OK;
}

VOID ItPosixSignal091(VOID)
{
    TEST_ADD_CASE("ItPosixSignal091", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
#else
typedef int bug03_placeholder;
#endif /* PRODUCT_BUG_ISOLATE: signal_091 */
