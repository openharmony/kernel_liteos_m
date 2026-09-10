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
 * ItLosLms026 - LOS_LmsAddrProtect / LOS_LmsAddrDisableProtect coverage.
 * Exercises:
 *   - LOS_LmsAddrProtect marks a sub-region as REDZONE -> access traps LMS.
 *   - LOS_LmsAddrDisableProtect restores region to ACCESSIBLE -> access OK.
 *   - NULL/invalid addrStart >= addrEnd branches return early without crash.
 */
static UINT32 TestCase(VOID)
{
    CHAR *str = (CHAR *)LOS_MemAlloc(g_testLmsPool, INDEX_MAX);
    ICUNIT_ASSERT_NOT_EQUAL(str, NULL, str);

    /* Protect the trailing 4 bytes of the allocation. */
    LOS_LmsAddrProtect((UINTPTR)(str + INDEX_MAX - 4), (UINTPTR)(str + INDEX_MAX));
    /* Accessing within the protected region triggers an LMS write report. */
    str[INDEX_MAX - 2] = 'x';

    /* Disable protection; subsequent access is fine. */
    LOS_LmsAddrDisableProtect((UINTPTR)(str + INDEX_MAX - 4), (UINTPTR)(str + INDEX_MAX));
    str[INDEX_MAX - 2] = 'y';

    /* Boundary-condition branches: addrEnd <= addrStart should be no-op. */
    LOS_LmsAddrProtect((UINTPTR)str, (UINTPTR)str);
    LOS_LmsAddrDisableProtect((UINTPTR)str, (UINTPTR)str);

    (VOID)LOS_MemFree(g_testLmsPool, str);
    return LOS_OK;
}

/* LmsTestAddrProtectToggle */
VOID ItLosLms026(void)
{
    TEST_ADD_CASE("ItLosLms026", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}
