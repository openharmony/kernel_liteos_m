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

LITE_TEST_SUIT(IPC, IpcMqApi, IpcMqApiTestSuite);

static BOOL IpcMqApiTestSuiteSetUp(void)
{
    return TRUE;
}

static BOOL IpcMqApiTestSuiteTearDown(void)
{
    return TRUE;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0100
 * @tc.name   mq_send and mq_receive function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqOneLevelCom, Function | MediumTest | Level0)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];
    char rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, sizeof(qName), "testMqOneLevelCom_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */
    ret = mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_msgsize, MQ_MSG_SIZE, getAttr.mq_msgsize);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_maxmsg, MQ_MAX_MSG, getAttr.mq_maxmsg);

    ret = mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(prio, 0, prio);

    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0200
 * @tc.name   mq_timedsend and mq_timedreceive function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqTimedOneLevelCom, Function | MediumTest | Level1)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct timespec tts = { 0 };
    struct timespec rts = { 0 };
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];
    char rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, sizeof(qName), "testMqTimedOneLevelCom_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    tts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    tts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_msgsize, MQ_MSG_SIZE, getAttr.mq_msgsize);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_maxmsg, MQ_MAX_MSG, getAttr.mq_maxmsg);

    rts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    rts.tv_nsec = 0;
    ret = mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(prio, 0, prio);
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0300
 * @tc.name   all send and all receive function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqAllOneLevelCom001, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];
    char rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, sizeof(qName), "testMqAllOneLevelCom_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    memset_s(&getAttr, sizeof(getAttr), 0, sizeof(getAttr));
    memset_s(&setAttr, sizeof(setAttr), 0, sizeof(setAttr));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */
    ret = mq_getattr(queue, &setAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    ret = mq_setattr(queue, &setAttr, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_msgsize, setAttr.mq_msgsize, getAttr.mq_msgsize);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_maxmsg, setAttr.mq_maxmsg, getAttr.mq_maxmsg);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_curmsgs, 1, getAttr.mq_curmsgs); /* 1, common data for test, no special meaning */
    ret = mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0300
 * @tc.name   all send and all receive function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqAllOneLevelCom002, Function | MediumTest | Level2)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct timespec tts = { 0 };
    struct timespec rts = { 0 };
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];
    char rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, sizeof(qName), "testMqAllOneLevelCom_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    memset_s(&getAttr, sizeof(getAttr), 0, sizeof(getAttr));
    memset_s(&setAttr, sizeof(setAttr), 0, sizeof(setAttr));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);

    tts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    tts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_msgsize, setAttr.mq_msgsize, getAttr.mq_msgsize);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_maxmsg, setAttr.mq_maxmsg, getAttr.mq_maxmsg);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_curmsgs, 1, getAttr.mq_curmsgs); /* 1, common data for test, no special meaning */

    rts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    rts.tv_nsec = 0;
    ret = mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

static void *PthreadCom(void *arg)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    char rMsg[MQ_RX_LEN];

    queue = (mqd_t)arg;
    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(prio, MQ_MSG_PRIO, prio);
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return NULL;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0700
 * @tc.name   mq_send and mq_receive function test in thread and process
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqTwoThreadCom, Function | MediumTest | Level1)
{
    int ret;
    mqd_t queue;
    pthread_t tid;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];
    char rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, sizeof(qName), "testMqTwoLevelCom_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = pthread_create(&tid, NULL, PthreadCom, (void *)queue);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_curmsgs, 1, getAttr.mq_curmsgs); /* 1, common data for test, no special meaning */
    ret = mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(prio, MQ_MSG_PRIO, prio);
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

static void *PthreadTimedCom(void *arg)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct timespec tts = { 0 };
    struct timespec rts = { 0 };
    struct mq_attr getAttr = { 0 };
    char rMsg[MQ_RX_LEN];

    queue = (mqd_t)arg;
    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    rts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    rts.tv_nsec = 0;
    ret = mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(prio, MQ_MSG_PRIO, prio);
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    tts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    tts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return NULL;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0800
 * @tc.name   mq_timedsend and mq_timedreceive function test in thread and process
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqTimedTwoThreadCom, Function | MediumTest | Level1)
{
    int ret;
    mqd_t queue;
    pthread_t tid;
    unsigned int prio;
    struct timespec tts = { 0 };
    struct timespec rts = { 0 };
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];
    char rMsg[MQ_RX_LEN];
    ret = sprintf_s(qName, sizeof(qName), "testMqTimedTwoThreadCom_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = pthread_create(&tid, NULL, PthreadTimedCom, (void *)queue);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    tts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    tts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_curmsgs, 1, getAttr.mq_curmsgs); /* 1, common data for test, no special meaning */
    rts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    rts.tv_nsec = 0;
    ret = mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(prio, MQ_MSG_PRIO, prio);
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

static void *PthreadAllCom(void *arg)
{
    int ret;
    mqd_t queue;
    unsigned int prio;
    struct timespec tts = { 0 };
    struct timespec rts = { 0 };
    struct mq_attr getAttr = { 0 };
    char rMsg[MQ_RX_LEN];

    queue = (mqd_t)arg;
    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    rts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    rts.tv_nsec = 0;
    ret = mq_timedreceive(queue, rMsg, getAttr.mq_msgsize, &prio, &rts);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(prio, MQ_MSG_PRIO, prio);
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    tts.tv_sec = time(NULL) + 1; /* 1, common data for test, no special meaning */
    tts.tv_nsec = 0;
    ret = mq_timedsend(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO, &tts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return NULL;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_0900
 * @tc.name   all send and all receive function test in thread and process
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqAllTwoThreadCom, Function | MediumTest | Level1)
{
    int ret;
    mqd_t queue;
    pthread_t tid;
    unsigned int prio;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];
    char rMsg[MQ_RX_LEN];

    ret = sprintf_s(qName, sizeof(qName), "testMqAllTwoThreadCom_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    ret = pthread_create(&tid, NULL, PthreadAllCom, (void *)queue);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */

    ret = mq_send(queue, MQ_MSG, MQ_MSG_LEN, MQ_MSG_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_join(tid, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(getAttr.mq_curmsgs, 1, getAttr.mq_curmsgs); /* 1, common data for test, no special meaning */
    ret = mq_receive(queue, rMsg, getAttr.mq_msgsize, &prio);
    ICUNIT_ASSERT_NOT_EQUAL(ret, -1, ret); /* -1, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(prio, MQ_MSG_PRIO, prio);
    ret = strncmp(MQ_MSG, rMsg, MQ_MSG_LEN);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

/* *
 * @tc.number SUB_KERNEL_IPC_MQ_1200
 * @tc.name   mq_setattr set and clean mq_flags for O_NONBLOCK function test
 * @tc.desc   [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(IpcMqApiTestSuite, testMqSetGetAttr, Function | MediumTest | Level1)
{
    int ret;
    mqd_t queue;
    struct mq_attr getAttr = { 0 };
    struct mq_attr setAttr = { 0 };
    char qName[MQ_NAME_LEN];

    ret = sprintf_s(qName, sizeof(qName), "testMqFunction_%d", GetRandom(10000)); /* 10000, common data for test, no special meaning */
    ICUNIT_ASSERT_EQUAL(ret, strlen(qName), ret);
    memset_s(&setAttr, sizeof(setAttr), 0, sizeof(setAttr));
    setAttr.mq_msgsize = MQ_MSG_SIZE;
    setAttr.mq_maxmsg = MQ_MAX_MSG;
    setAttr.mq_flags = O_NONBLOCK;
    queue = mq_open(qName, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR, &setAttr);
    ICUNIT_ASSERT_NOT_EQUAL(queue, (mqd_t)-1, queue); /* -1, common data for test, no special meaning */

    memset_s(&getAttr, sizeof(getAttr), 0, sizeof(getAttr));
    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = (getAttr.mq_flags & O_NONBLOCK);
    ICUNIT_ASSERT_EQUAL(ret, O_NONBLOCK, ret);

    setAttr.mq_flags &= ~O_NONBLOCK;
    ret = mq_setattr(queue, &setAttr, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = mq_getattr(queue, &getAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = (getAttr.mq_flags & O_NONBLOCK);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_close(queue);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = mq_unlink(qName);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
}

RUN_TEST_SUITE(IpcMqApiTestSuite);

void IpcMqFuncTest(void)
{
    RUN_ONE_TESTCASE(testMqOneLevelCom);
    RUN_ONE_TESTCASE(testMqTimedOneLevelCom);
    RUN_ONE_TESTCASE(testMqAllOneLevelCom001);
    RUN_ONE_TESTCASE(testMqAllOneLevelCom002);
    RUN_ONE_TESTCASE(testMqTwoThreadCom);
    RUN_ONE_TESTCASE(testMqTimedTwoThreadCom);
    RUN_ONE_TESTCASE(testMqAllTwoThreadCom);
    RUN_ONE_TESTCASE(testMqSetGetAttr);
}
