/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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
#include "it_los_hwi.h"

static UINT32 Testcase(VOID)
{
    UINT32 intSave;
    UINT32 deltaTicks;
    UINT64 timeRecordNS;
    UINT64 timeUpdateNS;
    UINT64 delayMs = 10; // delay 10 MS
    UINT32 deltaMs;
    UINT64 delayTicks;
    UINT32 loop = 10; // loop 10 time
    delayTicks = delayMs * LOSCFG_BASE_CORE_TICK_PER_SECOND / OS_SYS_MS_PER_SECOND;

    for (int i = 0; i <= loop; i++, delayTicks++) {
        timeRecordNS = LOS_CurrNanosec();
        LOS_TaskDelay(delayTicks);
        timeUpdateNS = LOS_CurrNanosec();
        deltaTicks = ((timeUpdateNS - timeRecordNS) * LOSCFG_BASE_CORE_TICK_PER_SECOND / OS_SYS_NS_PER_SECOND);

        if (deltaTicks >= (delayTicks - 1) && deltaTicks <= (delayTicks + 1)) {
            continue;
        } else {
            ICUNIT_ASSERT_EQUAL(deltaTicks, delayTicks, deltaTicks);
        }
    }

    intSave = LOS_IntLock();
    for (int i = 1; i <= loop; i++) {
        timeRecordNS = LOS_CurrNanosec();
        LOS_MDelay(i);
        timeUpdateNS = LOS_CurrNanosec();
        deltaMs = (timeUpdateNS - timeRecordNS) / OS_SYS_NS_PER_MS;
        ICUNIT_ASSERT_EQUAL(deltaMs, i, deltaMs);
    }
    LOS_IntRestore(intSave);

    return LOS_OK;
}

VOID ItLosHwi036(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosHwi036", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL3, TEST_PRESSURE);
}
