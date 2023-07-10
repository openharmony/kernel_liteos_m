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

#ifndef XTS_IPC_H
#define XTS_IPC_H

#include "xts_test.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <securec.h>
#include <semaphore.h>
#include <sys/resource.h>
#include <errno.h>
#include <stdlib.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define NANO_S                          1000000000
#define NANO_MS                         1000000
#define TEN_CONT                        10
#define FIFTY_CONT                      50
#define HUNDRED_CONT                    100
#define TEST_INTP_SIZE                  10
#define TEST_SEEK_SIZE                  12
#define POSIX_IPC_NO_ERROR              0
#define KERNEL_NS_PER_SECOND            1000000000
#define SEM_VALUE_MAX                   0xFFFE
#define KERNEL_NS_PER_SECOND            1000000000
#define KERNEL_100MS_BY_NS              100000000
#define MQ_NAME_LEN                     64
#define MQ_TX_LEN                       64
#define MQ_RX_LEN                       64
#define MQ_MSG_SIZE                     64
#define MQ_MSG_PRIO                     0
#define MQ_MAX_MSG                      16
#define MQ_MAX_QN_LEN                   24
#define MAX_SIGNAL_NUMBER               64
#define MAX_PIPE_BUFFER                 1023
#define MAX_PIPE_NUMBER                 32
#define MAX_MQ_NUMBER                   LOSCFG_BASE_IPC_QUEUE_LIMIT
#define MAX_MQ_NAME_LEN                 2560
#define MAX_MQ_MSG_SIZE                 65530
#define MQ_MSG                          "MessageToSend"
#define MQ_MSG_LEN                      sizeof(MQ_MSG)

#endif