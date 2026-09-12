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
 *    of the conditions and the following disclaimer in the documentation and/or other materials
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
#include "It_los_trace.h"

STATIC BOOL TraceHwiFilterHook(UINT32 hwiNum)
{
    /* filter out the dummy hw num 0xfe so it is not recorded */
    return (hwiNum == 0xfe) ? TRUE : FALSE;
}

/*
 * @tc.name      ItLosTrace005
 * @tc.desc      register a hwi filter hook, emit HWI events (filtered and non-filtered),
 *               verify the hook is consulted and the trace path does not fault.
 * @tc.coverage interface: LOS_TraceHwiFilterHookReg; branch: OsTraceHwiFilter (hook != NULL, return TRUE/FALSE)
 */
static UINT32 Testcase(VOID)
{
    LOS_TraceEventMaskSet(TRACE_USER_DEFAULT_FLAG);

    LOS_TraceHwiFilterHookReg(TraceHwiFilterHook);
    /* non-filtered hw num: should be recorded */
    LOS_TRACE(HWI_TRIGGER, 0x100);
    /* filtered hw num: OsTraceHwiFilter returns TRUE, hook returns early */
    LOS_TRACE(HWI_TRIGGER, 0xfe);
    /* clear the hook */
    LOS_TraceHwiFilterHookReg(NULL);

    ICUNIT_ASSERT_NOT_EQUAL(LOS_TraceRecordGet(), NULL, 0);
    return LOS_OK;
}

VOID ItLosTrace005(VOID)
{
    TEST_ADD_CASE("ItLosTrace005", Testcase, TEST_LOS, TEST_TRACE, TEST_LEVEL1, TEST_FUNCTION);
}
