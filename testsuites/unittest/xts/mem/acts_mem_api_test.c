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

#include "xts_mem.h"

LITE_TEST_SUIT(MEM, ActsMemApi, ActsMemApiTestSuite);

static BOOL ActsMemApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL ActsMemApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
* @tc.number     SUB_KERNEL_NDKAPI_MEM_MEMCHR_0100
* @tc.name       test memchr api
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(ActsMemApiTestSuite, testMemchr_0100, Function | MediumTest | Level1) {
    char srcStr[] = "this is str a;";
    char *pos = (char *)memchr(srcStr, 'a', 14); /* 14, common data for test, no special meaning */
    ICUNIT_ASSERT_STRING_EQUAL(pos, (char *)"a;", pos);
    return 0;
}

/**
* @tc.number     SUB_KERNEL_NDKAPI_MEM_MEMCHR_1000
* @tc.name       test memchr api para len not enough
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(ActsMemApiTestSuite, testMemchr_1000, Function | MediumTest | Level1) {
    char srcStr[] = "this is str a;";
    char *pos = (char *)memchr(srcStr, 'a', 4); /* 4, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(pos, 0, pos);
    return 0;
}

/**
* @tc.number     SUB_KERNEL_NDKAPI_MEM_MEMCHR_1100
* @tc.name       test memchr api para c not found
* @tc.desc       [C- SOFTWARE -0200]
*/
LITE_TEST_CASE(ActsMemApiTestSuite, testMemchr_1100, Function | MediumTest | Level1) {
    char srcStr[] = "this is str a;";
    char *pos = (char *)memchr(srcStr, 'b', 14); /* 14, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(pos, 0, pos);
    return 0;
}

RUN_TEST_SUITE(ActsMemApiTestSuite);

void ActsMemApiTest(void)
{
    RUN_ONE_TESTCASE(testMemchr_0100);
    RUN_ONE_TESTCASE(testMemchr_1000);
    RUN_ONE_TESTCASE(testMemchr_1100);
}

