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
#include "It_los_swtmr.h"

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)

static VOID SwtmrApi004Func(UINT32 arg)
{
    g_testCount++;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 swTmrID;

    g_testCount = 0;

    /* invalid rouses: value 2 is neither IGNORE(0) nor ALLOW(1) */
    ret = LOS_SwtmrCreate(4, LOS_SWTMR_MODE_ONCE, SwtmrApi004Func, &swTmrID, 0xffff,
        2, OS_SWTMR_ALIGN_INSENSITIVE);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SWTMR_ROUSES_INVALID, ret);

    /* invalid rouses: value 255 */
    ret = LOS_SwtmrCreate(4, LOS_SWTMR_MODE_ONCE, SwtmrApi004Func, &swTmrID, 0xffff,
        0xFF, OS_SWTMR_ALIGN_INSENSITIVE);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SWTMR_ROUSES_INVALID, ret);

    /* invalid sensitive: value 2 is neither SENSITIVE(0) nor INSENSITIVE(1) */
    ret = LOS_SwtmrCreate(4, LOS_SWTMR_MODE_ONCE, SwtmrApi004Func, &swTmrID, 0xffff,
        OS_SWTMR_ROUSES_ALLOW, 2);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SWTMR_ALIGN_INVALID, ret);

    /* invalid sensitive: value 255 */
    ret = LOS_SwtmrCreate(4, LOS_SWTMR_MODE_ONCE, SwtmrApi004Func, &swTmrID, 0xffff,
        OS_SWTMR_ROUSES_ALLOW, 0xFF);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERRNO_SWTMR_ALIGN_INVALID, ret);

    /* valid creation after errors */
    ret = LOS_SwtmrCreate(4, LOS_SWTMR_MODE_ONCE, SwtmrApi004Func, &swTmrID, 0xffff,
        OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_SwtmrDelete(swTmrID);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

VOID ItLosSwtmrApi004(VOID)
{
    TEST_ADD_CASE("ItLosSwtmrApi004", Testcase, TEST_LOS, TEST_SWTMR, TEST_LEVEL1, TEST_FUNCTION);
}

#endif
