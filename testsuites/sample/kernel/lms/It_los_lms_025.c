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
#include "los_lms_pri.h"

/*
 * ItLosLms025 - LMS compatibility regression.
 * Verifies:
 *   1. OsLmsInit idempotency: a second call returns LOS_OK and does not reset
 *      the existing g_lms hook table.
 *   2. LMS_SHADOW_ACCESSABLE spelling alias compiles and is equal to
 *      LMS_SHADOW_ACCESSIBLE (A-lib).
 *   3. OsLmsErrorHookSet registers a user callback that fires on out-of-bounds.
 */
static volatile UINT32 g_lmsErrHookHit = 0;
static UINT32 g_lmsErrHookLastErrMod = 0xFFFF;

static VOID LmsTestErrHook(UINTPTR p, UINT32 size, UINT32 errMod)
{
    (VOID)p;
    (VOID)size;
    g_lmsErrHookHit++;
    g_lmsErrHookLastErrMod = errMod;
}

static UINT32 TestCase(VOID)
{
    /* 1. Idempotency: a redundant OsLmsInit must not break g_lms. */
    UINT32 ret = OsLmsInit();
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ICUNIT_ASSERT_NOT_EQUAL(g_lms, NULL, g_lms);

    /* 2. Spelling compatibility. */
    ICUNIT_ASSERT_EQUAL(LMS_SHADOW_ACCESSABLE,      LMS_SHADOW_ACCESSIBLE,      LOS_NOK);
    ICUNIT_ASSERT_EQUAL(LMS_SHADOW_ACCESSABLE_U8,  LMS_SHADOW_ACCESSIBLE_U8,  LOS_NOK);

    /* 3. Register error hook and trigger an out-of-bounds read. */
    g_lmsErrHookHit = 0;
    g_lmsErrHookLastErrMod = 0xFFFF;
    OsLmsErrorHookSet(LmsTestErrHook);

    CHAR *str = (CHAR *)LOS_MemAlloc(g_testLmsPool, INDEX_MAX);
    ICUNIT_ASSERT_NOT_EQUAL(str, NULL, str);
    /* Index INDEX_MAX (out-of-bounds) triggers LMS report -> hook fires.
     * Use PRINTK to force the load to be evaluated (matches ItLosLms001 pattern),
     * so the asan instrumentation emits the __asan_load1_noabort callback. */
    PRINTK("str[%d]=0x%x\n", INDEX_MAX, str[INDEX_MAX]);
    (VOID)LOS_MemFree(g_testLmsPool, str);

    /* 4. Hook must have fired at least once with a LOAD_ERRMODE (0). */
    ICUNIT_ASSERT_NOT_EQUAL(g_lmsErrHookHit, 0, g_lmsErrHookHit);
    ICUNIT_ASSERT_EQUAL(g_lmsErrHookLastErrMod, LOAD_ERRMODE, g_lmsErrHookLastErrMod);

    /* 5. Unregister hook and ensure subsequent overflow does not call it. */
    OsLmsErrorHookSet(NULL);
    g_lmsErrHookHit = 0;
    CHAR *str2 = (CHAR *)LOS_MemAlloc(g_testLmsPool, INDEX_MAX);
    ICUNIT_ASSERT_NOT_EQUAL(str2, NULL, str2);
    PRINTK("str2[%d]=0x%x\n", INDEX_MAX, str2[INDEX_MAX]);
    (VOID)LOS_MemFree(g_testLmsPool, str2);
    ICUNIT_ASSERT_EQUAL(g_lmsErrHookHit, 0, g_lmsErrHookHit);

    return LOS_OK;
}

/* LmsTestFbbCompatApiAndErrHook */
VOID ItLosLms025(void)
{
    TEST_ADD_CASE("ItLosLms025", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}
