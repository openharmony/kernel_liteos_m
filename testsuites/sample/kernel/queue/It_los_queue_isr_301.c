/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "osTest.h"
#include "It_los_queue.h"

static UINT32 g_queueId301;
static UINT32 g_ret301;
static CHAR g_readBuff301[QUEUE_BASE_MSGSIZE];

static VOID HwiF01(VOID)
{
    UINT32 msgSize = QUEUE_BASE_MSGSIZE;

    TestHwiClear(HWI_NUM_TEST);
    (VOID)memset_s(g_readBuff301, QUEUE_BASE_MSGSIZE, 0, QUEUE_BASE_MSGSIZE);
    /* ReadCopyIsr = ReadCopy(..., LOS_NO_WAIT) 的 STATIC INLINE 包装(los_queue.h:533) */
    g_ret301 = LOS_QueueReadCopyIsr(g_queueId301, g_readBuff301, &msgSize);
}

/* 用例简要描述: LOS_QueueReadCopyIsr ISR 内读成功与数据一致 */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    HWI_PRIOR_T hwiPrio = 3;
    HWI_MODE_T mode = 0;
    CHAR buff[QUEUE_BASE_MSGSIZE] = "ISRREAD1";
    UINT32 i;

    g_ret301 = 0xFF; /* 255, init as not-run */

    ret = LOS_QueueCreate("Q_I301", QUEUE_BASE_NUM, &g_queueId301, 0, QUEUE_BASE_MSGSIZE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_QueueWriteCopy(g_queueId301, buff, QUEUE_BASE_MSGSIZE, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    HwiIrqParam irqParam;
    (VOID)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = 0;
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF01, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    TestHwiTrigger(HWI_NUM_TEST);

    /* F-Pos: ISR 内 ReadCopyIsr 成功且数据正确 */
    ICUNIT_GOTO_EQUAL(g_ret301, LOS_OK, g_ret301, EXIT1);
    for (i = 0; i < QUEUE_BASE_MSGSIZE; i++) {
        ICUNIT_GOTO_EQUAL(g_readBuff301[i], buff[i], g_readBuff301[i], EXIT1);
    }

EXIT1:
    TestHwiDelete(HWI_NUM_TEST);
EXIT:
    ret = LOS_QueueDelete(g_queueId301);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    return LOS_OK;
}

VOID ItLosQueueIsr301(VOID)
{
    TEST_ADD_CASE("ItLosQueueIsr301", Testcase, TEST_LOS, TEST_QUE, TEST_LEVEL1, TEST_FUNCTION);
}
