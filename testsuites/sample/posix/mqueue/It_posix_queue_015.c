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

#define QUEUE_NAME_MAX_LEN (MQUEUE_NAME_MAX_TEST * 2)

static UINT32 Testcase(VOID)
{
    CHAR mqname[QUEUE_NAME_MAX_LEN];
    mqd_t mqueue;
    UINT32 ret;
    INT32 i;

    ret = snprintf_s(mqname, QUEUE_NAME_MAX_LEN, QUEUE_NAME_MAX_LEN - 1, \
                     "/mq015_%d", LOS_CurTaskIDGet());
    ICUNIT_GOTO_NOT_EQUAL(ret, -1, ret, EXIT1);

    for (i = 0; i < MQUEUE_NAME_MAX_TEST; i++) {
        ret = strcat_s(mqname, QUEUE_NAME_MAX_LEN, "0");
        ICUNIT_ASSERT_EQUAL(ret, EOK, ret);
    }

    mqueue = mq_open(mqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
    ICUNIT_GOTO_EQUAL(mqueue, (mqd_t)-1, mqueue, EXIT);
    ICUNIT_ASSERT_EQUAL(errno, ENAMETOOLONG, errno);

    return LOS_OK;
EXIT:
    mq_close(mqueue);
    mq_unlink(mqname);
EXIT1:
    return LOS_OK;
}

/**
* @tc.name: ItPosixQueue015
* @tc.desc: Test interface mq_open
* @tc.type: FUNC
* @tc.require: issueI603SR6
 */

VOID ItPosixQueue015(VOID)
{
    TEST_ADD_CASE("ItPosixQueue015", Testcase, TEST_POSIX, TEST_QUE, TEST_LEVEL2, TEST_FUNCTION);
}
