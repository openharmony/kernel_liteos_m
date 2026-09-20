/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#include "osTest.h"
#include "It_los_lms.h"

/* 用例简要描述: 在足量静态缓冲区上有界调用 strcat/strncat, 走 LMS libc 包装的
 * 正常路径（影子检查通过、无任何越界）, 覆盖 strcat/strncat 函数入口与
 * strncat 的截断/提前遇 NUL 两个循环出口。 */

static CHAR g_lms031Buf[128];

static UINT32 TestCase(VOID)
{
    UINT32 i;
    UINT32 len;
    CHAR *ret;

    for (i = 0; i < sizeof(g_lms031Buf); i++) {
        g_lms031Buf[i] = '\0';
    }

    ret = strcat(g_lms031Buf, "Hello");
    ICUNIT_ASSERT_EQUAL((UINTPTR)ret, (UINTPTR)g_lms031Buf, (UINTPTR)ret);

    /* n smaller than src length: appends "Wor" (truncation exit). */
    ret = strncat(g_lms031Buf, "World", 3); /* 3: append count */
    ICUNIT_ASSERT_EQUAL((UINTPTR)ret, (UINTPTR)g_lms031Buf, (UINTPTR)ret);

    /* n larger than src length: appends "!" and stops at NUL (early exit). */
    ret = strncat(g_lms031Buf, "!", 8); /* 8: append count larger than src */
    ICUNIT_ASSERT_EQUAL((UINTPTR)ret, (UINTPTR)g_lms031Buf, (UINTPTR)ret);

    len = (UINT32)strlen(g_lms031Buf);
    ICUNIT_ASSERT_EQUAL(len, 9, len); /* 9: "HelloWor!" */
    ICUNIT_ASSERT_EQUAL(g_lms031Buf[0], 'H', g_lms031Buf[0]);

    PRINTK("g_lms031Buf = %s\n", g_lms031Buf);
    return LOS_OK;
}

/* LmsTestBoundedStrcatStrncat */
VOID ItLosLms031(void)
{
    TEST_ADD_CASE("ItLosLms031", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}
