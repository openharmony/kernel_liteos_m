/*
 * Copyright (c) 2023-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
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

#include "xts_ipc.h"

LITE_TEST_SUIT(IPC, IpcMqExceptionApi, IpcMqExceptionApiTestSuite);

static BOOL IpcMqExceptionApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IpcMqExceptionApiTestSuiteTearDown(void)
{
    return TRUE;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_RECEIVE_0200
 * @tc.name   mq_receive function errno for EBADF and EMSGSIZE test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqReceiveEBADFEMSGSIZE001, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct mq_attr attr = { 0 };
    struct mq_attr getAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN], rMsgErr[1];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqReceiveEAGAIN_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */
    ret = mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_receive(NULL, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);
    ret = mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_receive(queue, rMsgErr, sizeof(rMsgErr), &prio);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EMSGSIZE, errno);
    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_RECEIVE_0200
 * @tc.name   mq_receive function errno for EBADF and EMSGSIZE test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqReceiveEBADFEMSGSIZE002, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct mq_attr attr = { 0 };
    struct mq_attr getAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqReceiveEAGAIN_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_WRONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */
    ret = mq_getattr(queue, &getAttr);
    ret = mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);
    attr.mq_flags |= O_NONBLOCK;
    ret = mq_setattr(queue, &attr, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);
    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_RECEIVE_0300
 * @tc.name   mq_receive function errno for EINVAL test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqReceiveEINVAL, Function | MediumTest | Level3)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct mq_attr attr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqReceiveEINVAL_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = mq_receive(queue, rMsg, 0, &prio);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_TIMEDSEND_0100
 * @tc.name   mq_timedsend function errno for EAGAIN and EBADF test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqTimedSendEAGAINEBADF, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    struct timespec ts = { 0, 0 };
    struct mq_attr attr = { 0 };
    char qName[MQ_NAME_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqTimedSendEAGAIN_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = 1; /* 1, common data for test, no special meaning */
    queue = mq_open(qName, O_CREAT | O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    ts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ts.tv_sec = 0;
    ts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EAGAIN, errno);
    ret = mq_timedsend(NULL, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);

    attr.mq_flags |= O_NONBLOCK;
    ret = mq_setattr(queue, &attr, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_TIMEDSEND_0200
 * @tc.name   mq_timedsend function errno for EINVAL test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqTimedSendEINVAL, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    struct timespec ts = { 0, 0 };
    struct mq_attr attr = { 0 };
    char qName[MQ_NAME_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqTimedSendEINVAL_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    for (int i = 0; i < 3; i++) { /* 3, common data for test, no special meaning */
        switch (i) {
            case 0:
                ts.tv_sec = -1; /* -1, common data for test, no special meaning */
                ts.tv_nsec = 0;
                break;
            case 1:
                ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
                ts.tv_nsec = -1; /* -1, common data for test, no special meaning */
                break;
            case 2:
                ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
                ts.tv_nsec = 1000000000UL + 1UL;
                break;
        }

        ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
        ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
        ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    }
    ret = mq_timedsend(queue, MQ_MSG, 0, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_TIMEDSEND_0300
 * @tc.name   mq_timedsend function errno for EMSGSIZE test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqTimedSendEMSGSIZE, Function | MediumTest | Level3)
{
    int ret;
    mqd_t queue;
    struct timespec ts = { 0, 0 };
    struct mq_attr attr = { 0 };
    char qName[MQ_NAME_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqTimedSendEMSGSIZE_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = 1; /* 1, common data for test, no special meaning */
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    ts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EMSGSIZE, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_TIMEDSEND_0400
 * @tc.name   mq_timedsend function errno for ETIMEDOUT test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqTimedSendETIMEDOUT, Function | MediumTest | Level3)
{
    int ret;
    mqd_t queue;
    struct timespec ts = { 0, 0 };
    struct mq_attr attr = { 0 };
    char qName[MQ_NAME_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqTimedSendETIMEDOUT_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = 1; /* 1, common data for test, no special meaning */
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    ts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ts.tv_sec = 0;
    ts.tv_nsec = 100; /* 100, common data for test, no special meaning */
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, ETIMEDOUT, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_TIMEDRECEIVE_0100
 * @tc.name   mq_timedreceive function errno for EAGAIN and EBADF test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqTimedReceiveEAGAINEBADF, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct timespec ts = { 0, 0 };
    struct mq_attr attr = { 0 };
    struct mq_attr getAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqTimedReceiveEAGAIN_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    ts.tv_nsec = 0;
    ret = mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EAGAIN, errno);
    ret = mq_timedreceive(NULL, rMsg, getAttr.mq_msgsize, &prio, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_WRONLY | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = mq_timedreceive(NULL, rMsg, getAttr.mq_msgsize, &prio, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);

    attr.mq_flags |= O_NONBLOCK;
    ret = mq_setattr(queue, &attr, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_timedreceive(NULL, rMsg, getAttr.mq_msgsize, &prio, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_TIMEDRECEIVE_0200
 * @tc.name   mq_timedreceive function errno for EINVAL test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqTimedReceiveEINVAL, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct timespec ts = { 0, 0 };
    struct mq_attr attr = { 0 };
    struct mq_attr getAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqTimedReceiveEINVAL_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    for (int i = 0; i < 3; i++) { /* 3, common data for test, no special meaning */
        switch (i) {
            case 0:
                ts.tv_sec = -1; /* -1, common data for test, no special meaning */
                ts.tv_nsec = 0;
                break;
            case 1:
                ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
                ts.tv_nsec = -1; /* -1, common data for test, no special meaning */
                break;
            case 2:
                ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
                ts.tv_nsec = 1000000000UL + 1UL;
                break;
        }

        ret = mq_getattr(queue, &getAttr);
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
        ret = mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &ts);
        ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
        ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    }
    ret = mq_timedreceive(queue, rMsg, 0, &prio, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_TIMEDRECEIVE_0300
 * @tc.name   mq_timedreceive function errno for ETIMEDOUT test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqTimedReceiveETIMEDOUT, Function | MediumTest | Level3)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct timespec ts = { 0, 0 };
    struct mq_attr attr = { 0 };
    struct mq_attr getAttr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqTimedReceiveETIMEDOUT_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ts.tv_sec = 0;
    ts.tv_nsec = 100; /* 100, common data for test, no special meaning */
    ret = mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &ts);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, ETIMEDOUT, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_TIMEDRECEIVE_0400
 * @tc.name   mq_timedreceive function errno for EMSGSIZE test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqTimedReceiveEMSGSIZE, Function | MediumTest | Level3)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct timespec ts = { 0, 0 };
    struct mq_attr attr = { 0 };
    char qName[MQ_NAME_LEN], rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqTimedReceiveEMSGSIZE_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    attr.mq_msgsize = MQ_MSG_SIZE;
    attr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &attr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    ts.tv_nsec = 0;
    ret = mq_timedreceive(queue, rMsg, 1, &prio, &ts); /* 1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EMSGSIZE, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_UNLINK_0100
 * @tc.name   mq_unlink function errno for ENAMETOOLONG test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqUnlinkENAMETOOLONG, Function | MediumTest | Level2)
{
    char qName[MAX_MQ_NAME_LEN + 10]; /* 10, common data for test, no special meaning */
    int i;
    for (i = 0; i < MAX_MQ_NAME_LEN + 5; i++) { /* 5, common data for test, no special meaning */
        qName[i] = '8';
    }
    qName[i] = '\0';
    int ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, ENAMETOOLONG, errno);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_UNLINK_0200
 * @tc.name   mq_unlink function errno for ENOENT test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqUnlinkENOENT, Function | MediumTest | Level2)
{
    char qName[64] = "/mq_file-does-not-exit"; /* 64, common data for test, no special meaning */
    int ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, ENOENT, errno);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_UNLINK_0300
 * @tc.name   mq_unlink function errno for EINVAL test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqUnlinkEINVAL, Function | MediumTest | Level3)
{
    int ret = mq_unlink("");
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_GETATTR_0100
 * @tc.name   mq_getattr function errno for EBAD and EINVALF test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqGetAttrEBADFEINVAL, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    struct mq_attr mqstat = { 0 };
    char qName[MQ_NAME_LEN];

    memset_s(&mqstat, sizeof(mqstat), 0, sizeof(mqstat));
    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqSendEINVAL_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    queue = mq_open(qName, O_CREAT | O_RDWR | O_NONBLOCK, S_IRUSR | S_IWUSR, NULL);

    ret = mq_getattr(NULL, &mqstat);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);

    ret = mq_getattr(queue, NULL);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_SETATTR_0100
 * @tc.name   mq_receive function errno for EBADF and EINVAL test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqExceptionApiTestSuite, testMqSetAttrEBADFEINVAL, Function | MediumTest | Level2)
{
    int ret;
    char qName[MQ_NAME_LEN];
    mqd_t queue;
    struct mq_attr gMqstat = { 0 }, sMqstat = { 0 };

    ret = sprintf_s(qName, MQ_NAME_LEN, "testMqSetAttrEBADFEINVAL_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, NULL);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    memset_s(&gMqstat, sizeof(gMqstat), 0, sizeof(gMqstat));
    memset_s(&sMqstat, sizeof(sMqstat), 0, sizeof(sMqstat));
    ret = mq_getattr(queue, &gMqstat);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    sMqstat.mq_flags |= O_NONBLOCK;
    ret = mq_setattr(queue, &sMqstat, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_getattr(queue, &gMqstat);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_NOT_EQUAL(gMqstat.mq_flags, sMqstat.mq_flags, gMqstat.mq_flags);

    ret = mq_setattr(NULL, &sMqstat, NULL);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EBADF, errno);
    ret = mq_setattr(queue, NULL, NULL);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

RUN_TEST_SUITE(IpcMqExceptionApiTestSuite);

void IpcMqExceptionFuncTest(void)
{
    RUN_ONE_TESTCASE(testMqReceiveEBADFEMSGSIZE001);
    RUN_ONE_TESTCASE(testMqReceiveEBADFEMSGSIZE002);
    RUN_ONE_TESTCASE(testMqReceiveEINVAL);
    RUN_ONE_TESTCASE(testMqTimedSendEAGAINEBADF);
    RUN_ONE_TESTCASE(testMqTimedSendEINVAL);
    RUN_ONE_TESTCASE(testMqTimedSendEMSGSIZE);
    RUN_ONE_TESTCASE(testMqTimedSendETIMEDOUT);
    RUN_ONE_TESTCASE(testMqTimedReceiveEAGAINEBADF);
    RUN_ONE_TESTCASE(testMqTimedReceiveEINVAL);
    RUN_ONE_TESTCASE(testMqTimedReceiveETIMEDOUT);
    RUN_ONE_TESTCASE(testMqTimedReceiveEMSGSIZE);
    RUN_ONE_TESTCASE(testMqUnlinkENAMETOOLONG);
    RUN_ONE_TESTCASE(testMqUnlinkENOENT);
    RUN_ONE_TESTCASE(testMqUnlinkEINVAL);
    RUN_ONE_TESTCASE(testMqGetAttrEBADFEINVAL);
    RUN_ONE_TESTCASE(testMqSetAttrEBADFEINVAL);
}
