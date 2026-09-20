/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
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
#include "It_los_mux.h"

#if (LOSCFG_MUTEX_WAITMODE_PRIO == 1)

/* 用例简要描述: WAITMODE_PRIO 模式下 4 任务按优先级受控 pend。主任务持锁，
 * 4 个 DETACHED 任务经独立信号量按序（10/28/20/20）pend 同一互斥锁，覆盖
 * OsMuxPendFindPosSub 全三分支（prio<run→continue、prio>run→前插、prio==run→后插）
 * 与 OsMuxPendFindPos 非首非尾的中间路径；最终验证按优先级唤醒顺序 10→20→20→28。
 * 只调对外接口。 */

#define TEST_TASK_NUM 4
#define TASK_IDX_HIGH 0 // priority 10
#define TASK_IDX_LOW  1 // priority 28
#define TASK_IDX_MID  2 // priority 20
#define TASK_IDX_MID2 3 // priority 20

static UINT32 g_testMux036;
static UINT32 g_gateSem[TEST_TASK_NUM];
static volatile UINT32 g_wakeOrder[TEST_TASK_NUM];
static volatile UINT32 g_wakeIdx;

/* 每个任务：等自己的门闸信号量 → pend 互斥锁（阻塞）→ 获得后记录序号 → 释放。 */
static VOID TaskFunc036(UINT32 taskIdx)
{
    (VOID)LOS_SemPend(g_gateSem[taskIdx], LOS_WAIT_FOREVER);
    (VOID)LOS_MuxPend(g_testMux036, LOS_WAIT_FOREVER);
    g_wakeOrder[g_wakeIdx++] = taskIdx;
    (VOID)LOS_MuxPost(g_testMux036);
}

static VOID TaskFuncHigh(VOID)
{
    TaskFunc036(TASK_IDX_HIGH);
}

static VOID TaskFuncLow(VOID)
{
    TaskFunc036(TASK_IDX_LOW);
}

static VOID TaskFuncMid(VOID)
{
    TaskFunc036(TASK_IDX_MID);
}

static VOID TaskFuncMid2(VOID)
{
    TaskFunc036(TASK_IDX_MID2);
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 i;
    UINT32 loop;
    TSK_INIT_PARAM_S task = {0};
    UINT32 taskId[TEST_TASK_NUM];

    g_wakeIdx = 0;
    for (i = 0; i < TEST_TASK_NUM; i++) {
        g_wakeOrder[i] = 0xffffffff;
    }

    ret = LOS_MuxCreate(&g_testMux036);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    for (i = 0; i < TEST_TASK_NUM; i++) {
        ret = LOS_SemCreate(0, &g_gateSem[i]);
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    }

    /* 主任务先持锁，使所有 worker pend 阻塞。 */
    ret = LOS_MuxPend(g_testMux036, LOS_WAIT_FOREVER);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.uwResved = LOS_TASK_STATUS_DETACHED;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskFuncHigh;
    task.usTaskPrio = 10; // 10, highest pender.
    task.pcName = "Tsk036High";
    ret = LOS_TaskCreate(&taskId[TASK_IDX_HIGH], &task);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskFuncLow;
    task.usTaskPrio = 28; // 28, lowest pender.
    task.pcName = "Tsk036Low";
    ret = LOS_TaskCreate(&taskId[TASK_IDX_LOW], &task);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskFuncMid;
    task.usTaskPrio = 20; // 20, middle pender.
    task.pcName = "Tsk036Mid";
    ret = LOS_TaskCreate(&taskId[TASK_IDX_MID], &task);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskFuncMid2;
    task.usTaskPrio = 20; // 20, equal to Mid pender.
    task.pcName = "Tsk036Mid2";
    ret = LOS_TaskCreate(&taskId[TASK_IDX_MID2], &task);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* 受控放行：每个门闸放行后让出 1 tick，保证 pend 顺序确定。
     * High(10) 首个 pend（空表路径）；Low(28) 次之（尾插路径）；
     * Mid(20) 第三（触发 OsMuxPendFindPosSub：<continue + >前插）；
     * Mid2(20) 最后（触发 Sub 相等分支：==后插）。 */
    (VOID)LOS_SemPost(g_gateSem[TASK_IDX_HIGH]);
    (VOID)LOS_TaskDelay(1);
    (VOID)LOS_SemPost(g_gateSem[TASK_IDX_LOW]);
    (VOID)LOS_TaskDelay(1);
    (VOID)LOS_SemPost(g_gateSem[TASK_IDX_MID]);
    (VOID)LOS_TaskDelay(1);
    (VOID)LOS_SemPost(g_gateSem[TASK_IDX_MID2]);
    (VOID)LOS_TaskDelay(1);

    /* 主任务释放锁：按优先级顺序 High(10)→Mid(20)→Mid2(20)→Low(28) 依次获得。 */
    ret = LOS_MuxPost(g_testMux036);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    for (loop = 0; loop < 200; loop++) { // 200, bounded poll ticks for 4 handoffs.
        if (g_wakeIdx == TEST_TASK_NUM) {
            break;
        }
        (VOID)LOS_TaskDelay(1);
    }
    ICUNIT_GOTO_EQUAL(g_wakeIdx, TEST_TASK_NUM, g_wakeIdx, EXIT);

    ICUNIT_GOTO_EQUAL(g_wakeOrder[0], TASK_IDX_HIGH, g_wakeOrder[0], EXIT);
    ICUNIT_GOTO_EQUAL(g_wakeOrder[1], TASK_IDX_MID, g_wakeOrder[1], EXIT);
    ICUNIT_GOTO_EQUAL(g_wakeOrder[2], TASK_IDX_MID2, g_wakeOrder[2], EXIT);
    ICUNIT_GOTO_EQUAL(g_wakeOrder[3], TASK_IDX_LOW, g_wakeOrder[3], EXIT);

EXIT:
    for (i = 0; i < TEST_TASK_NUM; i++) {
        (VOID)LOS_SemDelete(g_gateSem[i]);
    }
    (VOID)LOS_MuxDelete(g_testMux036);
    return LOS_OK;
}

VOID ItLosMux036(void)
{
    TEST_ADD_CASE("ItLosMux036", Testcase, TEST_LOS, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
