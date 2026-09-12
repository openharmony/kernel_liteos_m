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
 * ItLosLms027 - Direct API + del/re-add pool coverage.
 * Drives LOS_LmsAddCheckPool / LOS_LmsDelCheckPool as primary entry points,
 * verifying:
 *   - AddCheckPool on a fresh pool returns the resized real size (>0).
 *   - DelCheckPool removes the pool; re-adding is allowed.
 *   - NULL-pool argument branches return without crash.
 *   - Repeated add on the same pool re-initializes (delete-then-add path).
 */
static UINT32 TestCase(VOID)
{
    static CHAR poolBuf[2 * PAGE_SIZE];

    /* NULL pool: AddCheckPool returns 0 (early-exit branch). */
    UINT32 ret0 = LOS_LmsAddCheckPool(NULL, 2 * PAGE_SIZE);
    ICUNIT_ASSERT_EQUAL(ret0, 0, ret0);

    /* NULL pool: DelCheckPool returns silently. */
    LOS_LmsDelCheckPool(NULL);

    /* Real pool: init via LOS_MemInit (which calls g_lms->init = AddCheckPool). */
    UINT32 ret = LOS_MemInit(poolBuf, 2 * PAGE_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* Direct DelCheckPool on the pool. */
    LOS_LmsDelCheckPool(poolBuf);

    /* Re-add (exercises the delete-old-then-add-new path inside AddCheckPool). */
    UINT32 realSize = LOS_LmsAddCheckPool(poolBuf, 2 * PAGE_SIZE);
    ICUNIT_ASSERT_NOT_EQUAL(realSize, 0, realSize);

    /* Cleanup: remove from LMS tracking then a final alloc/free sanity. */
    LOS_LmsDelCheckPool(poolBuf);
    (VOID)LOS_MemInit(poolBuf, 2 * PAGE_SIZE);
    CHAR *p = (CHAR *)LOS_MemAlloc(poolBuf, 32);
    ICUNIT_ASSERT_NOT_EQUAL(p, NULL, p);
    (VOID)LOS_MemFree(poolBuf, p);

    return LOS_OK;
}

/* LmsTestBLibAddDelCheckPoolDirect */
VOID ItLosLms027(void)
{
    TEST_ADD_CASE("ItLosLms027", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}
