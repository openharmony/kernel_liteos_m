/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "posix_test.h"
extern VOID ItSuitePosixRwlock(VOID);
extern VOID ItSuitePosixTime(VOID);
extern VOID ItSuitePosixIo(VOID);
#if defined(LOSCFG_ARCH_ARM) && defined(LOSCFG_POSIX_SIGNAL_API)
extern VOID ItSuitePosixSignal(VOID);
#endif

void ItSuitePosix(void)
{
    PthreadFuncTestSuite();
    ItSuitePosixPthread();
    ItSuitePosixMutex();
    ItSuitePosixRwlock();
    ItSuitePosixMqueue();
    ItSuitePosixTime();
#ifdef LOSCFG_POSIX_PIPE_API
    ItSuitePosixIo();
#endif
#if defined(LOSCFG_ARCH_ARM) && defined(LOSCFG_POSIX_SIGNAL_API)
    ItSuitePosixSignal();
#endif
    PosixCtypeFuncTest();
    PosixIsdigitFuncTest();
    PosixIslowerFuncTest();
    PosixIsxdigitFuncTest();
    PosixTolowerFuncTest();
    PosixToupperFuncTest();
    PosixStrerrorTest();
#if !defined(LOSCFG_ARCH_FPU_DISABLE)
    PosixMathFuncTest();
#endif
    PosixMqueueFuncTest();
    PosixStdargFuncTest();
    PosixStdlibAtoiFuncTest();
    PosixStdlibAtolFuncTest();
    PosixStdlibAtollFuncTest();
    PosixStdlibStrtolFuncTest();
    PosixStdlibStrtoulFuncTest();
    PosixStdlibStrtoullFuncTest();
    PosixStringMemTest03();
    PosixStringStrchrTest();
    PosixStringFuncTest02();
    PosixStringStrcasecmpFuncTest();
    PosixStringFuncTest03();
#if (LOS_KERNEL_TEST_FULL == 1)
    PosixSemaphoreFuncTest();
    PosixTimeFuncTest();
#endif
#if (LOSCFG_SUPPORT_LITTLEFS == 1 || LOSCFG_SUPPORT_FATFS == 1)
    PosixFsFuncTest();
#endif
}
