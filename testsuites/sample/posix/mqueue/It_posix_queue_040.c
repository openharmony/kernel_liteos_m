/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
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
#include "It_posix_queue.h"

static UINT32 Testcase(VOID)
{
    INT32 ret = 0;
    CHAR mqname[MQUEUE_STANDARD_NAME_LENGTH] = "";
    mqd_t mqueue;
    CHAR msgrv[MQUEUE_STANDARD_NAME_LENGTH] = {0};
    struct mq_attr attr = {0};

    ret = snprintf_s(mqname, MQUEUE_STANDARD_NAME_LENGTH, MQUEUE_STANDARD_NAME_LENGTH - 1, \
                     "/mq040_%d", LOS_CurTaskIDGet());
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT2);

    attr.mq_msgsize = MQUEUE_STANDARD_NAME_LENGTH;
    attr.mq_maxmsg = MQUEUE_STANDARD_NAME_LENGTH;
    mqueue = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_GOTO_NOT_EQUAL(mqueue, (mqd_t)-1, mqueue, EXIT2);

    mqueue = (mqd_t)((UINTPTR)mqueue + PER_ADDED_VALUE);
    ret = mq_receive(mqueue, msgrv, MQUEUE_STANDARD_NAME_LENGTH, NULL);
    ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT1);
    ICUNIT_GOTO_EQUAL(errno, EBADF, errno, EXIT1);

    ret = mq_close((mqd_t)((UINTPTR)mqueue - PER_ADDED_VALUE));
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = mq_unlink(mqname);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    return LOS_OK;
EXIT1:
    mq_close(mqueue);
EXIT:
    mq_unlink(mqname);
EXIT2:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixQueue040
 * @tc.desc: Test interface mq_receive
 * @tc.type: FUNC
 * @tc.require: issueI61YO0
 */

VOID ItPosixQueue040(VOID)
{
    TEST_ADD_CASE("ItPosixQueue040", Testcase, TEST_POSIX, TEST_QUE, TEST_LEVEL2, TEST_FUNCTION);
}
