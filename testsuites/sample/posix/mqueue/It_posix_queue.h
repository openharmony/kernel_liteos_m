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

#ifndef IT_POSIX_QUEUE_H
#define IT_POSIX_QUEUE_H

#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sched.h>
#include <signal.h>
#include <osTest.h>
#include "pthread.h"

#define MAXMSG5 5
#define MSGLEN 10
#define MAXMSG 10

#define MQUEUE_SEND_STRING_TEST "0123456789"
#define MQUEUE_SHORT_ARRAY_LENGTH 10 // 10: strlen(MQUEUE_SEND_STRING_TEST)
#define MQUEUE_STANDARD_NAME_LENGTH 50

#define MQUEUE_PTHREAD_PRIORITY_TEST1 3
#define MQUEUE_PTHREAD_PRIORITY_TEST2 4
#define MQUEUE_PATH_MAX_TEST PATH_MAX
#define MQUEUE_NAME_MAX_TEST NAME_MAX
#define MQUEUE_PTHREAD_NUM_TEST 5
#define MQUEUE_PRIORITY_TEST 0
#define MQUEUE_TIMEOUT_TEST 7
#define MQUEUE_PRIORITY_NUM_TEST 3
#define MQUEUE_MAX_NUM_TEST (LOSCFG_BASE_IPC_QUEUE_CONFIG - QUEUE_EXISTED_NUM)
#define MQ_MAX_MSG_NUM 16
#define MQ_MAX_MSG_LEN 64
#define HWI_NUM_TEST 1
#define HWI_NUM_TEST1 2

#if (LOSCFG_LIBC_NEWLIB == 1)
#define MQ_PRIO_MAX 1
#endif

#define LOSCFG_BASE_IPC_QUEUE_CONFIG 1024

#define LOS_AtomicInc(a) (++*(a))
#define MqueueTaskDelay(tick) (usleep((tick) * 10000))

#ifdef __LP64__
#define PER_ADDED_VALUE 8
#else
#define PER_ADDED_VALUE 4
#endif

#define ICUNIT_GOTO_WITHIN_EQUAL(param, value1, value2, retcode, label) \
    do {                                                                \
        if ((param) < (value1) || (param) > (value2)) {                 \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);                 \
            goto label;                                                 \
        }                                                               \
    } while (0)

typedef UINT32 TSK_HANDLE_T;
typedef UINT32 SEM_HANDLE_T ;

extern SEM_HANDLE_T g_mqueueSem;

static TSK_HANDLE_T g_mqueueTaskPID;
extern CHAR g_gqname[MQUEUE_STANDARD_NAME_LENGTH];
extern CHAR g_mqueueName[LOSCFG_BASE_IPC_QUEUE_CONFIG + 1][MQUEUE_STANDARD_NAME_LENGTH];
extern mqd_t g_mqueueId[LOSCFG_BASE_IPC_QUEUE_CONFIG + 1];

extern CHAR *g_mqueueMsessage[MQUEUE_SHORT_ARRAY_LENGTH];
extern mqd_t g_messageQId;
extern mqd_t g_gqueue;

extern VOID ItSuitePosixMqueue(VOID);
extern UINT32 PosixPthreadInit(pthread_attr_t *attr, int pri);
extern UINT32 PosixPthreadDestroy(pthread_attr_t *attr, pthread_t thread);
extern VOID TestAssertWaitDelay(UINT32 *testCount, UINT32 flag);

VOID ItPosixQueue001(VOID);
VOID ItPosixQueue002(VOID);
VOID ItPosixQueue003(VOID);
VOID ItPosixQueue004(VOID);
VOID ItPosixQueue005(VOID);
VOID ItPosixQueue006(VOID);
VOID ItPosixQueue007(VOID);
VOID ItPosixQueue008(VOID);
VOID ItPosixQueue009(VOID);
VOID ItPosixQueue010(VOID);
VOID ItPosixQueue011(VOID);
VOID ItPosixQueue012(VOID);
VOID ItPosixQueue013(VOID);
VOID ItPosixQueue014(VOID);
VOID ItPosixQueue015(VOID);
VOID ItPosixQueue016(VOID);
VOID ItPosixQueue017(VOID);
VOID ItPosixQueue018(VOID);
VOID ItPosixQueue019(VOID);
VOID ItPosixQueue020(VOID);
VOID ItPosixQueue021(VOID);
VOID ItPosixQueue022(VOID);
VOID ItPosixQueue023(VOID);
VOID ItPosixQueue024(VOID);
VOID ItPosixQueue025(VOID);
VOID ItPosixQueue026(VOID);
VOID ItPosixQueue027(VOID);
VOID ItPosixQueue028(VOID);
VOID ItPosixQueue029(VOID);
VOID ItPosixQueue030(VOID);
VOID ItPosixQueue031(VOID);
VOID ItPosixQueue032(VOID);
VOID ItPosixQueue033(VOID);
VOID ItPosixQueue034(VOID);
VOID ItPosixQueue035(VOID);
#endif
