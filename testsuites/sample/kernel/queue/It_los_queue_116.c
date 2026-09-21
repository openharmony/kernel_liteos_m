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

/* 用例简要描述: LOS_Queue 负参数校验：读缓冲 NULL / 读长度指针 NULL →
 * READ_PTR_NULL；写长度 0 → WRITESIZE_ISZERO；InfoGet 输出指针 NULL →
 * PTR_NULL。只调对外接口。 */

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 queueId;
    UINT32 msg = 0x1234;
    UINT32 buf = 0;

    ret = LOS_QueueCreate("queue116", 1, &queueId, 0, sizeof(UINT32));
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Inv: 读缓冲为 NULL → READ_PTR_NULL。 */
    ret = LOS_QueueRead(queueId, NULL, sizeof(UINT32), LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_READ_PTR_NULL, ret, EXIT);

    /* F-Inv: 写长度为 0（Copy 变体做参数校验）→ WRITESIZE_ISZERO。 */
    ret = LOS_QueueWriteCopy(queueId, &msg, 0, LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_WRITESIZE_ISZERO, ret, EXIT);

    /* F-Inv: InfoGet 输出指针为 NULL → PTR_NULL。 */
    ret = LOS_QueueInfoGet(queueId, NULL);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_QUEUE_PTR_NULL, ret, EXIT);

    /* F-Pos: 队列仍可正常读写（负参数不破坏状态）。 */
    ret = LOS_QueueWrite(queueId, &msg, sizeof(UINT32), LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_QueueRead(queueId, &buf, sizeof(UINT32), LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    (VOID)LOS_QueueDelete(queueId);
    return LOS_OK;
}

VOID ItLosQueue116(VOID)
{
    TEST_ADD_CASE("ItLosQueue116", Testcase, TEST_LOS, TEST_QUE, TEST_LEVEL1, TEST_FUNCTION);
}
