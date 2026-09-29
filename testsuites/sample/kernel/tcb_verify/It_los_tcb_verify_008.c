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

/* ItLosTcbVerify008 - P0-1: legitimate bits in taskStatus must NOT be
 * reported as ERR_STATUS on a clean system.
 *
 * Background: in liteos_new the LosTaskCB taskStatus field stores the runtime
 * status bits (OS_TASK_STATUS_*, 0x0001..0x0100) plus the SIGNAL/FREEZE
 * attribute bits (0x2000/0x4000). Attribute flags DETACHED/SYSTEM/JOINABLE
 * live in the separate taskFlags bitfield and are not checked here.
 *
 * If TCB_VERIFY_VALID_MASK only enumerates the 9 STATUS bits (0x1FF) and
 * omits SIGNAL/FREEZE, then for any task carrying those bits:
 *     taskStatus & ~(valid mask) != 0
 * and TcbVerifyBasic() falsely sets TCB_VERIFY_ERR_STATUS even though
 * nothing has been tampered with. That makes the whole verify feature report a
 * failure on its very first call on a clean system.
 *
 * This case creates a normal task that suspends itself, proves it carries
 * OS_TASK_STATUS_SUSPEND, then asserts that TcbVerifyBasic() does NOT set
 * ERR_STATUS on this clean system. On the buggy (incomplete) mask this
 * assertion FAILS, which exposes P0-1. Once the mask is fixed to cover all
 * legitimate taskStatus bits, this case passes.
 */

#include "osTest.h"
#include "It_los_tcb_verify.h"
#include "tcb_verify.h"
#include "los_task_pri.h"
#include "los_task.h"

static volatile UINT32 g_tcbVrf008Started;

static VOID TcbVerify008Task(VOID)
{
    g_tcbVrf008Started = 1;
    /* Park this task so it stays alive (SUSPEND) while the test inspects its
     * TCB. The SUSPEND bit is legitimate. */
    (VOID)LOS_TaskSuspend(LOS_CurTaskIDGet());
    /* Reached only if someone resumes us. */
}

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 intSave;
    UINT32 taskID = 0;
    LosTaskCB *tcb = NULL;
    UINT16 snapStatus = 0;
    UINT32 verifyRet = 0;
    TSK_INIT_PARAM_S param = { 0 };

    param.pfnTaskEntry = (TSK_ENTRY_FUNC)TcbVerify008Task;
    param.uwStackSize = TASK_STACK_SIZE_TEST;
    param.pcName = "TcbVrf008";
    param.usTaskPrio = TASK_PRIO_TEST - 1; /* higher prio than the runner: preempts and suspends */
    param.uwResved = LOS_TASK_STATUS_DETACHED;

    g_tcbVrf008Started = 0;
    ret = LOS_TaskCreate(&taskID, &param);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* The new task preempts and suspends itself. If it did not preempt (runner
     * is at a higher prio), yield once so it can run + suspend. */
    if (g_tcbVrf008Started != 1) {
        (VOID)LOS_TaskDelay(1);
    }
    ICUNIT_ASSERT_EQUAL(g_tcbVrf008Started, 1, g_tcbVrf008Started);

    tcb = OS_TCB_FROM_TID(taskID);

    /* Snapshot under the scheduler spinlock. Never assert inside the lock:
     * ICUNIT_ASSERT_* does "return 1;" on failure, which would leave the lock
     * held and deadlock the system. */
    SCHEDULER_LOCK(intSave);
    snapStatus = tcb->taskStatus;
    verifyRet = TcbVerifyBasic();
    SCHEDULER_UNLOCK(intSave);

    (VOID)LOS_TaskDelete(taskID);

    /* Precondition: this really is a normally created task carrying a legitimate
     * STATUS bit (SUSPEND, since the task suspended itself) that the mask must
     * cover. If this ever stops being true, the test below would be vacuous. */
    ICUNIT_ASSERT_NOT_EQUAL((UINT32)snapStatus & OS_TASK_STATUS_SUSPEND, 0, snapStatus);

    /* P0-1 assertion: a clean system must NOT set ERR_STATUS. The FLAG bits
     * (STACK_FREE / SYSTEM_TASK / SIGNAL / FREEZE / JOINABLE / USER_TASK) are
     * legitimate and MUST be covered by TCB_VERIFY_VALID_MASK.
     *
     *   - Expected on FIXED code  : (verifyRet & ERR_STATUS) == 0  -> PASS
     *   - Expected on BUGGY code   : (verifyRet & ERR_STATUS) != 0  -> FAIL
     *     (idle / swtmr / this task all carry FLAG bits outside the 0x1FF mask)
     */
    ICUNIT_ASSERT_EQUAL(verifyRet & TCB_VERIFY_ERR_STATUS, 0, verifyRet);
    return LOS_OK;
}

VOID ItLosTcbVerify008(VOID)
{
    TEST_ADD_CASE("ItLosTcbVerify008", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}
