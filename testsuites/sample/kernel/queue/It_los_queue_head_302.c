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

/* 用例简要描述: LOS_QueueWriteHeadCopy 满队 ISFULL/越界 ID/NULL buffer */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 queueID;
    CHAR buff[QUEUE_BASE_MSGSIZE] = "HEAD302B";

    ret = LOS_QueueCreate("Q_H302", 1, &queueID, 0, QUEUE_BASE_MSGSIZE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* 写满(len=1) */
    ret = LOS_QueueWriteCopy(queueID, buff, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Inv: 满队头部 Copy 写(NO_WAIT) -> ISFULL(los_queue.c:431) */
    ret = LOS_QueueWriteHeadCopy(queueID, buff, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_ISFULL, ret, EXIT);

    /* F-Inv: 越界 ID -> INVALID(OsQueueWriteParameterCheck: queueID >= OS_ALL_IPC_QUEUE_LIMIT) */
    ret = LOS_QueueWriteHeadCopy(0xFFFF, buff, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_INVALID, ret, EXIT);

    /* F-Inv: NULL buffer -> WRITE_PTR_NULL */
    ret = LOS_QueueWriteHeadCopy(queueID, NULL, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_WRITE_PTR_NULL, ret, EXIT);

EXIT:
    ret = LOS_QueueDelete(queueID);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    return LOS_OK;
}

VOID ItLosQueueHead302(VOID)
{
    TEST_ADD_CASE("ItLosQueueHead302", Testcase, TEST_LOS, TEST_QUE, TEST_LEVEL0, TEST_FUNCTION);
}
