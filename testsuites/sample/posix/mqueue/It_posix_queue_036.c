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

static VOID *PthreadF01(VOID *argument)
{
    INT32 ret;
    const CHAR *msgptr = MQUEUE_SEND_STRING_TEST;

    g_testCount = 1;

    TestAssertWaitDelay(&g_testCount, 3); // 3, Here, assert the g_testCount.

    ret = mq_send(g_gqueue, msgptr, strlen(msgptr), 0);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_testCount = 2; // 2, Init test count value.

    return NULL;
EXIT:
    mq_close(g_gqueue);
    mq_unlink(g_gqname);
    g_testCount = 0;
    return NULL;
}

static VOID *PthreadF02(VOID *argument)
{
    INT32 ret;
    CHAR msgrv[MQUEUE_STANDARD_NAME_LENGTH] = {0};

    g_testCount = 3; // 3, Init test count value.

    ret = mq_receive(g_gqueue, msgrv, MQUEUE_STANDARD_NAME_LENGTH, NULL);
    ICUNIT_GOTO_EQUAL(ret, MQUEUE_SHORT_ARRAY_LENGTH, ret, EXIT);

    ret = MqueueTaskDelay(1);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = mq_close(g_gqueue);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    ret = mq_unlink(g_gqname);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    g_testCount = 4; // 4, Init test count value.
    return NULL;
EXIT:
    mq_close(g_gqueue);
EXIT1:
    mq_unlink(g_gqname);
EXIT2:
    g_testCount = 0;
    return NULL;
}

static UINT32 Testcase(VOID)
{
    INT32 ret;
    struct mq_attr attr = {0};
    pthread_attr_t attr1;
    pthread_t newTh1, newTh2;

    (void)snprintf_s(g_gqname, MQUEUE_STANDARD_NAME_LENGTH, MQUEUE_STANDARD_NAME_LENGTH - 1,
                     "/mq036_%d", LOS_CurTaskIDGet());

    attr.mq_msgsize = MQUEUE_STANDARD_NAME_LENGTH;
    attr.mq_maxmsg = MQUEUE_STANDARD_NAME_LENGTH;
    g_gqueue = mq_open(g_gqname, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_GOTO_NOT_EQUAL(g_gqueue, (mqd_t)-1, g_gqueue, EXIT3);

    g_testCount = 0;

    ret = PosixPthreadInit(&attr1, MQUEUE_PTHREAD_PRIORITY_TEST1);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    ret = pthread_create(&newTh1, &attr1, PthreadF01, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    TestAssertWaitDelay(&g_testCount, 1);

    ret = PosixPthreadInit(&attr1, MQUEUE_PTHREAD_PRIORITY_TEST2);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    ret = pthread_create(&newTh2, &attr1, PthreadF02, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    ret = MqueueTaskDelay(10); // 10, Set delay time.
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT2);

    TestAssertWaitDelay(&g_testCount, 4); // 4, Here, assert the g_testCount.

    ret = PosixPthreadDestroy(&attr1, newTh2);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT1);

    ret = PosixPthreadDestroy(&attr1, newTh1);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    return LOS_OK;
EXIT2:
    PosixPthreadDestroy(&attr1, newTh2);
EXIT1:
    PosixPthreadDestroy(&attr1, newTh1);
EXIT:
    mq_close(g_gqueue);
    mq_unlink(g_gqname);
EXIT3:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixQueue036
 * @tc.desc: Test interface mq_receive
 * @tc.type: FUNC
 * @tc.require: issueI61YO0
 */

VOID ItPosixQueue036(VOID)
{
    TEST_ADD_CASE("ItPosixQueue036", Testcase, TEST_POSIX, TEST_QUE, TEST_LEVEL2, TEST_FUNCTION);
}
