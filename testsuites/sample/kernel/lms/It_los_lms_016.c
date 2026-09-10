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

#include "osTest.h"
#include "It_los_lms.h"

static volatile UINT32 g_lms016HookHit = 0;
static VOID Lms016ErrHook(UINTPTR p, UINT32 size, UINT32 errMod)
{
    (VOID)p;
    (VOID)size;
    (VOID)errMod;
    g_lms016HookHit++;
}

static UINT32 TestCase016Impl(VOID *pool)
{
    CHAR *string = "LMS_TestCase";
    CHAR *src;
    CHAR *buf;
    CHAR *str;

    src = LOS_MemAlloc(pool, strlen(string));
    ICUNIT_ASSERT_NOT_EQUAL(src, NULL, src);

    buf = LOS_MemAlloc(pool, (strlen(string) + 10)); /* mem size 10 */
    ICUNIT_ASSERT_NOT_EQUAL(buf, NULL, buf);

    g_lms016HookHit = 0;
    /* Register LMS error hook to verify detection of the deliberate overflow below.
     * LMS detects but does not prevent corruption; the subsequent free() may fail,
     * which is expected behavior - the test's purpose is to verify LMS detection. */
    OsLmsErrorHookSet(Lms016ErrHook);

    str = strcpy(src, string); /* write overflow: src is strlen(string) bytes, copies strlen+1 */
    ICUNIT_ASSERT_NOT_EQUAL(str, NULL, str);

    (VOID)strcpy(buf, src); /* Check LMS detection information when the strcpy src overflows. */

    ICUNIT_ASSERT_NOT_EQUAL(g_lms016HookHit, 0, g_lms016HookHit);

    return LOS_OK;
}

static UINT32 TestCase(VOID)
{
    return LMS_TEST_RUN_IN_SANDBOX(TestCase016Impl, 2 * PAGE_SIZE);
}

/* LmsTestStrcpyOverflow */
VOID ItLosLms016(void)
{
    TEST_ADD_CASE("ItLosLms016", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}
