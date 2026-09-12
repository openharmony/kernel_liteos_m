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
 * @tc.name      ItLosTrace004
 * @tc.desc      exercise the 5 newly added events via LOS_TRACE: TASK_DELAY/WAKE/INSCHED,
 *               EVENT_COND_READ/COND_WRITE; verify the hook path executes without fault.
 * @tc.coverage interface: new event types + PARAMS macros; branch: OsTraceHook TASK_FLAG / EVENT_FLAG
 */
static UINT32 Testcase(VOID)
{
    UINT32 tid = LOS_CurTaskIDGet();
    UINTPTR ev = (UINTPTR)0x1234;

    /* enable all module masks so the new events are recorded */
    LOS_TraceEventMaskSet(TRACE_USER_DEFAULT_FLAG);

    LOS_TRACE(TASK_DELAY, tid, 1);
    LOS_TRACE(TASK_WAKE, tid);
    LOS_TRACE(TASK_INSCHED, tid, tid);
    LOS_TRACE(EVENT_COND_READ, ev, 0x1);
    LOS_TRACE(EVENT_COND_WRITE, ev, 0x2);

    /* hook still installed, buffer still valid after emitting new events */
    ICUNIT_ASSERT_NOT_EQUAL(g_traceEventHook, NULL, 0);
    ICUNIT_ASSERT_NOT_EQUAL(LOS_TraceRecordGet(), NULL, 0);
    return LOS_OK;
}

VOID ItLosTrace004(VOID)
{
    TEST_ADD_CASE("ItLosTrace004", Testcase, TEST_LOS, TEST_TRACE, TEST_LEVEL1, TEST_FUNCTION);
}
