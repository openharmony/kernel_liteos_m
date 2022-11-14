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

#include "stdio.h"
#include "It_posix_queue.h"

CHAR *g_mqueueMsessage[MQUEUE_SHORT_ARRAY_LENGTH] = {"0123456789", "1234567890", "2345678901",
                                                     "3456789012", "4567890123", "5678901234",
                                                     "6789012345", "7890123456", "lalalalala",
                                                     "hahahahaha"};
CHAR g_gqname[MQUEUE_STANDARD_NAME_LENGTH];
CHAR g_mqueueName[LOSCFG_BASE_IPC_QUEUE_CONFIG + 1][MQUEUE_STANDARD_NAME_LENGTH];
mqd_t g_mqueueId[LOSCFG_BASE_IPC_QUEUE_CONFIG + 1];
SEM_HANDLE_T g_mqueueSem;
mqd_t g_messageQId;
mqd_t g_gqueue;

VOID TestAssertWaitDelay(UINT32 *testCount, UINT32 flag)
{
    while (*testCount != flag) {
        usleep(1);
    }
}

VOID ItSuitePosixMqueue(void)
{
    printf("*********** BEGIN SAMPLE POSIX MQUEUE TEST ************\n");
    ItPosixQueue001();
    ItPosixQueue002();
    ItPosixQueue003();
    ItPosixQueue004();
    ItPosixQueue005();
    ItPosixQueue006();
    ItPosixQueue007();
    ItPosixQueue008();
    ItPosixQueue009();
    ItPosixQueue010();
    ItPosixQueue011();
    ItPosixQueue012();
    ItPosixQueue013();
    ItPosixQueue014();
    ItPosixQueue015();
    ItPosixQueue016();
    ItPosixQueue017();
    ItPosixQueue018();
}
