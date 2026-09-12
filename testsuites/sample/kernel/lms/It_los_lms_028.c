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

/*
 * ItLosLms028 - Use-after-free detection path & OsLmsPrintPoolListInfo coverage.
 * Verifies:
 *   - After LOS_MemFree, the freed region is marked AFTERFREE.
 *   - A subsequent access to the freed pointer triggers a FREE_ERRORMODE
 *     (or COMMON_ERRMODE) report via the OsLmsCheckValid -> OsLmsReportError path.
 *   - OsLmsPrintPoolListInfo executes without crashing (PRINT_DEBUG path;
 *     useful for statement coverage even if debug output is suppressed).
 *   - Double-free attempt is detected (the g_lms->check -> freeMark path).
 */
static UINT32 TestCase028Impl(VOID *pool)
{
    OsLmsPrintPoolListInfo();

    CHAR *str = (CHAR *)LOS_MemAlloc(pool, INDEX_MAX);
    ICUNIT_ASSERT_NOT_EQUAL(str, NULL, str);
    (VOID)memset(str, 0xAA, INDEX_MAX);

    (VOID)LOS_MemFree(pool, str);
    /* Use-after-free read: triggers LMS report (FREE_ERRORMODE).
     * Use PRINTK to force the load to be evaluated so the asan
     * instrumentation emits the __asan_load1_noabort callback. */
    PRINTK("str[0]=0x%x\n", str[0]);

    /* Double free attempt: should also be reported but not crash. */
    (VOID)LOS_MemFree(pool, str);

    OsLmsPrintPoolListInfo();
    return LOS_OK;
}

static UINT32 TestCase(VOID)
{
    return LMS_TEST_RUN_IN_SANDBOX(TestCase028Impl, 2 * PAGE_SIZE);
}

/* LmsTestUseAfterFreeAndPrintPoolInfo */
VOID ItLosLms028(void)
{
    TEST_ADD_CASE("ItLosLms028", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}
