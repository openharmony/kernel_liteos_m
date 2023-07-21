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

LITE_TEST_SUIT(MEM, MemApi, MemApiTestSuite);

static BOOL MemApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL MemApiTestSuiteTearDown(void)
{
    return TRUE;
}

/**
 * @tc.number SUB_KERNEL_MEM_MEMSET_0100
 * @tc.name   memset function set buffer value test
 * @tc.desc   [C-L*-311]
 */
LITE_TEST_CASE(MemApiTestSuite, testMemset, Function | MediumTest | Level1)
{
    char chr = 'A';
    int i, len, failure;
    len = GetRandom(1024); /* 1024, common data for test, no special meaning */
    errno_t err = EOK;

    char buf[1024]; /* 1024, common data for test, no special meaning */
    err = memset_s(buf, sizeof(buf), chr, len);
    ICUNIT_ASSERT_EQUAL(err, 0, err);
    failure = 0;
    for (i = 0; i < len; i++) {
        if (buf[i] != chr) {
            failure = 1; /* 1, common data for test, no special meaning */
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_MEM_MEMCPY_0100
 * @tc.name   memcpy function copy buffer test
 * @tc.desc   [C-L*-311]
 */
LITE_TEST_CASE(MemApiTestSuite, testMemcpy, Function | MediumTest | Level2)
{
    char chr = 'A';
    int i, len, failure;
    char src[1024]; /* 1024, common data for test, no special meaning */
    char dst[1024]; /* 1024, common data for test, no special meaning */

    len = GetRandom(1024); /* 1024, common data for test, no special meaning */

    for (i = 0; i < len; i++) {
        src[i] = chr + i % 26; /* 26, common data for test, no special meaning */
    }

    errno_t err = memcpy_s(dst, sizeof(dst) / sizeof(dst[0]), src, len);
    ICUNIT_ASSERT_EQUAL(err, 0, err);
    failure = 0;
    for (i = 0; i < len; i++) {
        if (dst[i] != src[i]) {
            failure = 1; /* 1, common data for test, no special meaning */
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_MEM_MEMMOVE_0100
 * @tc.name   memmove function move buffer test
 * @tc.desc   [C-L*-311]
 */
LITE_TEST_CASE(MemApiTestSuite, testMemmove, Function | MediumTest | Level2)
{
    char chr = 'A';
    char buf[1024]; /* 1024, common data for test, no special meaning */
    int i, len, failure;

    len = sizeof(buf);
    for (i = 0; i < len; i++) {
        buf[i] = chr + GetRandom(26); /* 26, common data for test, no special meaning */
    }
    errno_t err = memmove_s(&buf[0], sizeof(buf) / sizeof(buf[0]), &buf[len / 2], len / 2); /* 2, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(err, 0, err);

    failure = 0;
    for (i = 0; i < len / 2; i++) { /* 2, common data for test, no special meaning */
        if (buf[i] != buf[len / 2 + i]) { /* 2, common data for test, no special meaning */
            failure = 1; /* 1, common data for test, no special meaning */
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    return 0;
}

/**
 * @tc.number SUB_KERNEL_MEM_MEMMOVE_0200
 * @tc.name   memmove function overlay move buffer test
 * @tc.desc   [C-L*-311]
 */
LITE_TEST_CASE(MemApiTestSuite, testMemmoveOverlay, Function | MediumTest | Level3)
{
    char chr = 'A';
    char buf[1024]; /* 1024, common data for test, no special meaning */
    char backup[1024]; /* 1024, common data for test, no special meaning */
    int i, len, failure;

    len = sizeof(buf);
    for (i = 0; i < len; i++) {
        buf[i] = chr + GetRandom(26); /* 26, common data for test, no special meaning */
        backup[i] = buf[i];
    }
    errno_t err = memmove_s(&buf[16], sizeof(buf) / sizeof(buf[0]) - 16, &buf[0], len / 2); /* 16, 2, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(err, 0, err);

    failure = 0;
    for (i = 0; i < len / 2; i++) { /* 2, common data for test, no special meaning */
        if (buf[i + 16] != backup[i]) { /* 16, common data for test, no special meaning */
            failure = 1; /* 1, common data for test, no special meaning */
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    return 0;
}


/**
 * @tc.number SUB_KERNEL_MEM_MEMCMP_0100
 * @tc.name   memmove function move buffer test
 * @tc.desc   [C-L*-311]
 */
LITE_TEST_CASE(MemApiTestSuite, testMemcmp, Function | MediumTest | Level2)
{
    char orign[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}; /* 8, common data for test, no special meaning */
    char lt[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x77}; /* 8, common data for test, no special meaning */
    char eq[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}; /* 8, common data for test, no special meaning */
    char gt[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x99}; /* 8, common data for test, no special meaning */

    int ret;
    int len = sizeof(orign);

    ret = memcmp(lt, orign, len);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, INT_MIN, 0, ret);

    ret = memcmp(eq, orign, len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = memcmp(gt, orign, len);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 0, INT_MAX, ret);

    ret = memcmp(gt, orign, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

RUN_TEST_SUITE(MemApiTestSuite);

void MemApiTest(void)
{
    RUN_ONE_TESTCASE(testMemset);
    RUN_ONE_TESTCASE(testMemcpy);
    RUN_ONE_TESTCASE(testMemmove);
    RUN_ONE_TESTCASE(testMemmoveOverlay);
    RUN_ONE_TESTCASE(testMemcmp);
}
