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

#include "osTest.h"
#include "It_los_swtmr.h"

static UINT32 g_testCount1 = 0;
static UINT64 g_timeRecordNS = 0;
static UINT64 g_timeUpdateNS = 0;

#define SWTMR_PERIODIC 4

static VOID Case1(UINT32 arg)
{
    g_testCount1++;
    g_timeUpdateNS = LOS_CurrNanosec();
    return;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    g_testCount1 = 0;
    UINT64 tickRecord;
    UINT64 tickUpdate;
    UINT64 deltaTicks;

    // 4, Timeout interval of a periodic software timer.
    ret = LOS_SwtmrCreate(SWTMR_PERIODIC, LOS_SWTMR_MODE_ONCE, Case1, &g_swtmrId1, 0xffff
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    , OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE
#endif
    );
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    g_timeRecordNS = LOS_CurrNanosec();

    ret = LOS_SwtmrStart(g_swtmrId1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    // 10, SSet task delay time.
    LOS_TaskDelay(10);

    deltaTicks = (g_timeUpdateNS - g_timeRecordNS) * LOSCFG_BASE_CORE_TICK_PER_SECOND / OS_SYS_NS_PER_SECOND;
    ICUNIT_ASSERT_EQUAL(deltaTicks, SWTMR_PERIODIC, deltaTicks);

EXIT:
    LOS_SwtmrDelete(g_swtmrId1);
    return LOS_OK;
}

VOID ItLosSwtmr081() // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosSwtmr081", Testcase, TEST_LOS, TEST_SWTMR, TEST_LEVEL1, TEST_FUNCTION);
}

