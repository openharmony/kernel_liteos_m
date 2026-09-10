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

/*
 * @tc.name      ItLosTrace003
 * @tc.desc      LOS_TraceInit idempotency: already initialized returns LOS_ERRNO_TRACE_ERROR_STATUS,
 *               regardless of buf/size (the early guard short-circuits).
 * @tc.coverage exception branch: g_traceState != TRACE_UNINIT -> error; interface: LOS_TraceInit
 */
static UINT32 Testcase(VOID)
{
    UINT32 ret = LOS_TraceInit(NULL, LOSCFG_TRACE_BUFFER_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_TRACE_ERROR_STATUS, ret);

    /* a user-provided buffer must also be rejected because trace is already inited */
    UINT8 userBuf[64];  /* size is irrelevant: the early guard rejects before buf/size are used */
    ret = LOS_TraceInit(userBuf, sizeof(userBuf));
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_TRACE_ERROR_STATUS, ret);
    return LOS_OK;
}

VOID ItLosTrace003(VOID)
{
    TEST_ADD_CASE("ItLosTrace003", Testcase, TEST_LOS, TEST_TRACE, TEST_LEVEL1, TEST_FUNCTION);
}
