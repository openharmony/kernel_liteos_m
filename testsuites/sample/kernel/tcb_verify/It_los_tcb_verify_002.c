/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary, with or without modification,
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

/* ItLosTcbVerify002 - R1: stackPointer NULL detected. */

#include "osTest.h"
#include "It_los_tcb_verify.h"
#include "tcb_verify.h"
#include "los_task_pri.h"

static UINT32 TestCase(VOID)
{
    UINT32 intSave;
    LosTaskCB *tcb = NULL;

    SCHEDULER_LOCK(intSave);

    for (UINT32 i = 0; i < g_taskMaxNum; i++) {
        if (g_taskCBArray[i].taskStatus & OS_TASK_STATUS_UNUSED) {
            tcb = &g_taskCBArray[i];
            break;
        }
    }
    if (tcb == NULL) {
        SCHEDULER_UNLOCK(intSave);
        return 1;
    }

    UINT16 savedStatus = tcb->taskStatus;
    VOID *savedSp = tcb->stackPointer;

    tcb->taskStatus = OS_TASK_STATUS_SUSPEND;
    tcb->stackPointer = NULL;

    UINT32 ret = TcbVerifyBasic();

    tcb->taskStatus = savedStatus;
    tcb->stackPointer = savedSp;

    SCHEDULER_UNLOCK(intSave);

    ICUNIT_ASSERT_EQUAL(ret & TCB_VERIFY_ERR_SP_NULL, TCB_VERIFY_ERR_SP_NULL, ret);
    return LOS_OK;
}

VOID ItLosTcbVerify002(VOID)
{
    TEST_ADD_CASE("ItLosTcbVerify002", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}
