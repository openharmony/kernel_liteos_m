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
#include "It_los_queue.h"

/* 用例简要描述: LOS_QueueInfoGet 记录阻塞任务位图。同一队列上"写阻塞(满)"与
 * "读阻塞(空)"互斥，故用双队列：QA 阻塞写者、QB 阻塞读者，分别 InfoGet 并断言
 * waitWriteTask/waitReadTask 位图中对应 taskId 位被置位（覆盖 los_queue.c
 * L806-812）。只调对外接口。 */

static UINT32 g_qFullId;
static UINT32 g_qEmptyId;
static UINT32 g_gateW;
static UINT32 g_gateR;
static UINT32 g_writerTsk;
static UINT32 g_readerTsk;
static volatile UINT32 g_wDone;
static volatile UINT32 g_rDone;

static VOID WriterTask115(VOID)
{
    UINT32 msg = 0x1234;
    (VOID)LOS_SemPend(g_gateW, LOS_WAIT_FOREVER);
    (VOID)LOS_QueueWrite(g_qFullId, &msg, sizeof(UINT32), LOS_WAIT_FOREVER);
    g_wDone = 1;
}

static VOID ReaderTask115(VOID)
{
    UINT32 buf = 0;
    UINT32 size = sizeof(UINT32);
    (VOID)LOS_SemPend(g_gateR, LOS_WAIT_FOREVER);
    (VOID)LOS_QueueRead(g_qEmptyId, &buf, size, LOS_WAIT_FOREVER);
    g_rDone = 1;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 msg = 0x5678;
    UINT32 buf = 0;
    UINT32 size = sizeof(UINT32);
    UINT32 loop;
    QUEUE_INFO_S queueInfo = {0};
    TSK_INIT_PARAM_S task = {0};

    g_wDone = 0;
    g_rDone = 0;

    ret = LOS_QueueCreate("q115full", 1, &g_qFullId, 0, sizeof(UINT32));
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_QueueCreate("q115empty", 1, &g_qEmptyId, 0, sizeof(UINT32));
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SemCreate(0, &g_gateW);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_SemCreate(0, &g_gateR);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    task.uwStackSize = TASK_STACK_SIZE_TEST;
    task.uwResved = LOS_TASK_STATUS_DETACHED;

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)WriterTask115;
    task.usTaskPrio = 10; // 10, writer priority, higher than the test task.
    task.pcName = "Q115W";
    ret = LOS_TaskCreate(&g_writerTsk, &task);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    task.pfnTaskEntry = (TSK_ENTRY_FUNC)ReaderTask115;
    task.usTaskPrio = 10; // 10, reader priority, higher than the test task.
    task.pcName = "Q115R";
    ret = LOS_TaskCreate(&g_readerTsk, &task);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* QA：填满(len=1)→放行写任务→阻塞于满队列。 */
    ret = LOS_QueueWrite(g_qFullId, &msg, sizeof(UINT32), LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    (VOID)LOS_SemPost(g_gateW);
    (VOID)LOS_TaskDelay(1); // 1, let the writer run and block.

    /* QB：空队列→放行读任务→阻塞于空队列。 */
    (VOID)LOS_SemPost(g_gateR);
    (VOID)LOS_TaskDelay(1); // 1, let the reader run and block.

    /* InfoGet(QA)：遍历写等待链表 → waitWriteTask 位图置位。 */
    ret = LOS_QueueInfoGet(g_qFullId, &queueInfo);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(queueInfo.waitWriteTask[g_writerTsk / 32] & (1U << (g_writerTsk % 32)),
                          0, queueInfo.waitWriteTask[g_writerTsk / 32], EXIT);

    /* InfoGet(QB)：遍历读等待链表 → waitReadTask 位图置位。 */
    (void)memset_s(&queueInfo, sizeof(QUEUE_INFO_S), 0, sizeof(QUEUE_INFO_S));
    ret = LOS_QueueInfoGet(g_qEmptyId, &queueInfo);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(queueInfo.waitReadTask[g_readerTsk / 32] & (1U << (g_readerTsk % 32)),
                          0, queueInfo.waitReadTask[g_readerTsk / 32], EXIT);

    /* 收尾：QA 读取腾出空间→写任务完成写入；QB 写入→读任务完成读取。 */
    ret = LOS_QueueRead(g_qFullId, &buf, size, LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_QueueWrite(g_qEmptyId, &msg, sizeof(UINT32), LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    for (loop = 0; loop < 200; loop++) { // 200, bounded poll ticks.
        if ((g_wDone == 1) && (g_rDone == 1)) {
            break;
        }
        (VOID)LOS_TaskDelay(1);
    }
    ICUNIT_GOTO_EQUAL(g_rDone, 1, g_rDone, EXIT);
    ICUNIT_GOTO_EQUAL(g_wDone, 1, g_wDone, EXIT);

EXIT:
    (VOID)LOS_SemDelete(g_gateW);
    (VOID)LOS_SemDelete(g_gateR);
    (VOID)LOS_QueueDelete(g_qFullId);
    (VOID)LOS_QueueDelete(g_qEmptyId);
    return LOS_OK;
}

VOID ItLosQueue115(VOID)
{
    TEST_ADD_CASE("ItLosQueue115", Testcase, TEST_LOS, TEST_QUE, TEST_LEVEL1, TEST_FUNCTION);
}
