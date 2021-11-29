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

#include "osTest.h"
#include "It_los_lms.h"

static UINT32 TestCase(VOID)
{
    CHAR *string = "LMS_TestCase";
    CHAR *src;
    CHAR *buf;
    CHAR *str;
    UINT32 ret;
    src = LOS_MemAlloc(g_testLmsPool, strlen(string));
    ICUNIT_ASSERT_NOT_EQUAL(src, NULL, src);
    (VOID)__memset(src, 0, strlen(string));

    PRINTK("%d\n", __LINE__);
    str = __strcpy(src, string);
    ICUNIT_ASSERT_NOT_EQUAL(str, NULL, str);
    PRINTK("%d\n", __LINE__);

    buf = LOS_MemAlloc(g_testLmsPool, 8); /* mem size 8 */
    ICUNIT_ASSERT_NOT_EQUAL(buf, NULL, buf);
    buf[7] = '\0'; /* end index 7 */
    PRINTK("%d\n", __LINE__);
    ret = strcpy_s(buf, 100, src); /* Check LMS detection information when the strcpy_s dest max set 100 overflows. */
    PRINTK("%d\n", __LINE__);

    ret = LOS_MemFree(g_testLmsPool, buf);
    ICUNIT_ASSERT_NOT_EQUAL(ret, LOS_NOK, ret);
    ret = LOS_MemFree(g_testLmsPool, src);
    ICUNIT_ASSERT_NOT_EQUAL(ret, LOS_NOK, ret);

    return LOS_OK;
}

/* LmsTestStrcpy_sOverflow */
VOID ItLosLms024(void)
{
    TEST_ADD_CASE("ItLosLms024", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}

