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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
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

/* 用例简要描述: LOS_QueueWriteHeadCopy 头插 LIFO 语义(先读 B 再读 A) */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 queueID;
    CHAR buffA[QUEUE_BASE_MSGSIZE] = "HEAD_A01";
    CHAR buffB[QUEUE_BASE_MSGSIZE] = "HEAD_B01";
    CHAR readBuff[QUEUE_BASE_MSGSIZE] = "";
    UINT32 i;
    UINT32 msgSize = QUEUE_BASE_MSGSIZE;

    ret = LOS_QueueCreate("Q_H301", QUEUE_BASE_NUM, &queueID, 0, QUEUE_BASE_MSGSIZE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* 尾部写 A */
    ret = LOS_QueueWriteCopy(queueID, buffA, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Pos: 头部 Copy 写 B */
    ret = LOS_QueueWriteHeadCopy(queueID, buffB, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* 头插先出(LIFO): 先读 B 再读 A(ReadCopy 数据语义与写入匹配) */
    ret = LOS_QueueReadCopy(queueID, readBuff, &msgSize, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    for (i = 0; i < QUEUE_BASE_MSGSIZE; i++) {
        ICUNIT_GOTO_EQUAL(readBuff[i], buffB[i], readBuff[i], EXIT);
    }

    msgSize = QUEUE_BASE_MSGSIZE; /* ReadCopy 的 msgSize 为 IN/OUT, 二次读前重置容量 */
    ret = LOS_QueueReadCopy(queueID, readBuff, &msgSize, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    for (i = 0; i < QUEUE_BASE_MSGSIZE; i++) {
        ICUNIT_GOTO_EQUAL(readBuff[i], buffA[i], readBuff[i], EXIT);
    }

EXIT:
    ret = LOS_QueueDelete(queueID);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    return LOS_OK;
}

VOID ItLosQueueHead301(VOID)
{
    TEST_ADD_CASE("ItLosQueueHead301", Testcase, TEST_LOS, TEST_QUE, TEST_LEVEL0, TEST_FUNCTION);
}
