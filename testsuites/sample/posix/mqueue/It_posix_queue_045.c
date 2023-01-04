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
    CHAR mqname[MQUEUE_STANDARD_NAME_LENGTH] = "";
    mqd_t mqdes;
    struct mq_attr mqstat, nmqstat;
    INT32 ret = 0;

    ret = snprintf_s(mqname, MQUEUE_STANDARD_NAME_LENGTH, MQUEUE_STANDARD_NAME_LENGTH - 1, \
                     "/mq045-1_%d", LOS_CurTaskIDGet());
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT1);

    ret = memset_s(&mqstat, sizeof(mqstat), 0, sizeof(mqstat));
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    mqstat.mq_msgsize = MQUEUE_STANDARD_NAME_LENGTH;
    mqstat.mq_maxmsg = 40; // 40, queue max message size.
    mqdes = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &mqstat);
    ICUNIT_GOTO_NOT_EQUAL(mqdes, (mqd_t)-1, mqdes, EXIT1);

    ret = memset_s(&nmqstat, sizeof(nmqstat), 0, sizeof(nmqstat));
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = mq_getattr(mqdes, &nmqstat);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ICUNIT_GOTO_EQUAL(mqstat.mq_maxmsg, nmqstat.mq_maxmsg, mqstat.mq_maxmsg, EXIT);
    ICUNIT_GOTO_EQUAL(mqstat.mq_msgsize, nmqstat.mq_msgsize, mqstat.mq_msgsize, EXIT);

    ret = mq_close(mqdes);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    ret = mq_unlink(mqname);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    return LOS_OK;
EXIT:
    mq_close(mqdes);
EXIT2:
    mq_unlink(mqname);
EXIT1:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixQueue045
 * @tc.desc: Test interface mq_getattr
 * @tc.type: FUNC
 * @tc.require: issueI61YO0
 */

VOID ItPosixQueue045(VOID)
{
    TEST_ADD_CASE("ItPosixQueue045", Testcase, TEST_POSIX, TEST_QUE, TEST_LEVEL2, TEST_FUNCTION);
}
