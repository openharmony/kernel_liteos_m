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

#ifndef XTS_CMSID_H
#define XTS_CMSID_H

#include "xts_test.h"
#include <securec.h>
#include <limits.h>
#include "cmsis_os2.h"

#define LOS_WAIT_FOREVER 0xFFFFFFFF

#define TESTCOUNT_NUM_1 1
#define TESTCOUNT_NUM_2 2
#define TESTCOUNT_NUM_3 3
#define TESTCOUNT_NUM_4 4
#define TESTCOUNT_NUM_5 5

#define MSGQUEUE_COUNT    16
#define MSGQUEUE_SPACE_COUNT    13
#define MSGQUEUE_PUT_COUNT    3
#define MSG_SIZE    4
#define MSGINFO_LEN    4
#define TIMEOUT_COUNT    1000
#define BUF_LEN    32
#define MSGQUEUE_COUNT_INDEX_0    0
#define MSGQUEUE_COUNT_INDEX_1    1
#define MSGQUEUE_COUNT_INDEX_2    2

#define SEMAPHHORE_COUNT_HEX_MAX    0xFE
#define SEMAPHHORE_COUNT_INT0    0
#define SEMAPHHORE_COUNT_INT1    1
#define SEMAPHHORE_COUNT_INT10    10

#define EVENT_MASK_HEX_1 0x01
#define EVENT_MASK_HEX_2 0x02
#define EVENT_MASK_HEX_4 0x04
#define EVENT_MASK_HEX_10 0x10
#define EVENT_MASK_HEX_11 0x11
#define TIMEOUT_NUM_3 3
#define TIMEOUT_NUM_10 10
#define INVALID_FLAG_OPTION 0x00000004U

#define MILLISEC_NUM_INT10 10U
#define MILLISEC_NUM_INT4 4U
#define INVALID_TIMER_TYPE 10
#define TIMER_PERIODIC_COUNT 2

#define DELAY_TICKS_1 1
#define DELAY_TICKS_5 5
#define DELAY_TICKS_10 10

#endif