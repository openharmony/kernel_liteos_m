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

#ifndef _POSIX_TEST_H
#define _POSIX_TEST_H

#include <stdlib.h>
#include "iCunit.h"


#define TEST_STR(func) ItLos##func
#define TEST_TO_STR(x) #x
#define TEST_HEAD_TO_STR(x) TEST_TO_STR(x)
#define ADD_TEST_CASE(func) \
    TEST_ADD_CASE(TEST_HEAD_TO_STR(TEST_STR(func)), func, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION)

#define Function   0
#define MediumTest 0
#define Level1     0
#define LITE_TEST_SUIT(subsystem, module, testsuit)
#define LITE_TEST_CASE(module, function, flag) static int function(void)
#define RUN_TEST_SUITE(testsuit)

#define TEST_ASSERT_EQUAL_FLOAT(expected, actual) \
    ICUNIT_ASSERT_EQUAL(((expected) == (actual)) || (isnan(expected) && isnan(actual)), TRUE, 0)

void ItSuitePosix(void);

extern void ItSuitePosixPthread(void);
extern void ItSuitePosixMutex(void);
extern void PosixCtypeFuncTest(void);
extern void PosixIsdigitFuncTest(void);
extern void PosixIslowerFuncTest(void);
extern void PosixIsxdigitFuncTest(void);
extern void PosixTolowerFuncTest(void);
extern void PosixToupperFuncTest(void);
extern void ItSuitePosixMqueue(void);

extern void PosixStrerrorTest(void);

extern void PosixFsFuncTest(void);

extern void PosixMathFuncTest(void);

extern void PosixMqueueFuncTest(void);

extern int PthreadFuncTestSuite(void);

extern void PosixRegexFuncTest(void);

extern void PosixSemaphoreFuncTest(void);

extern void PosixStdargFuncTest(void);

extern void PosixStdlibAtoiFuncTest(void);
extern void PosixStdlibAtolFuncTest(void);
extern void PosixStdlibAtollFuncTest(void);
extern void PosixStdlibStrtolFuncTest(void);
extern void PosixStdlibStrtoulFuncTest(void);
extern void PosixStdlibStrtoullFuncTest(void);

extern void PosixStringMemTest03(void);
extern void PosixStringStrchrTest(void);
extern void PosixStringFuncTest02(void);
extern void PosixStringStrcasecmpFuncTest(void);
extern void PosixStringFuncTest03(void);
extern void PosixStringStrstrTest(void);

extern void PosixTimeFuncTest(void);

#endif
