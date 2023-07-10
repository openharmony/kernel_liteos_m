/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "xts_io.h"

LITE_TEST_SUIT(IO, IoOther, IoOtherTestSuite);

static BOOL IoOtherTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IoOtherTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_IO_OTHER_0900
 * @tc.name   strptime basic function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IoOtherTestSuite, testStrptime, Function | MediumTest | Level1)
{
    struct tm tm;
    memset_s(&tm, sizeof(struct tm), 0, sizeof(struct tm));
    char *ret = strptime("2020-10-29 21:24:00abc", "%Y-%m-%d %H:%M:%S", &tm);
    ICUNIT_ASSERT_STRING_EQUAL(ret, "abc", ret);
    ICUNIT_ASSERT_EQUAL(tm.tm_year, 120, tm.tm_year); /* 120 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_mon, 9, tm.tm_mon); /* 9 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_mday, 29, tm.tm_mday); /* 29 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_hour, 21, tm.tm_hour); /* 21 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_min, 24, tm.tm_min); /* 24 common data for test, no special meaning */

    ret = strptime("14 Oct October 20 09:24:00 Sat Saturday 363", "%d %b %B %y %I:%M:%S %a %A %j", &tm);
    ICUNIT_ASSERT_STRING_EQUAL(ret, "", ret);
    ICUNIT_ASSERT_EQUAL(tm.tm_year, 120, tm.tm_year); /* 120 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_mon, 9, tm.tm_mon); /* 9 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_mday, 14, tm.tm_mday); /* 14 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_hour, 9, tm.tm_hour); /* 9 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_wday, 6, tm.tm_wday); /* 6 common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(tm.tm_yday, 362, tm.tm_yday); /* 362 common data for test, no special meaning */
    return 0;
}

RUN_TEST_SUITE(IoOtherTestSuite);

void XtsIoOtherTest(void)
{
    RUN_ONE_TESTCASE(testStrptime);
}
