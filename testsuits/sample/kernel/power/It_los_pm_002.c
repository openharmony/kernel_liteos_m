/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd. All rights reserved.
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
#include "It_los_pm.h"
#include "los_timer.h"

static UINT32 g_deviceCount = 0;
static UINT32 g_sysCount = 0;
static UINT32 g_sysTickTimerCount = 0;
static UINT32 DeviceSuspend(UINT32 mode)
{
    g_deviceCount++;
    g_testCount++;
    return LOS_OK;
}

static VOID DeviceResume(UINT32 mode)
{
    g_deviceCount--;
    return;
}

static LosPmDevice g_device = {
    .suspend = DeviceSuspend,
    .resume = DeviceResume,
};

static VOID TickLock(VOID)
{
    g_testCount++;
    g_sysTickTimerCount++;
}

static VOID TickUnlock(VOID)
{
    g_sysTickTimerCount--;
}

static LosPmTickTimer g_tickTimer = {
    .tickLock = TickLock,
    .tickUnlock = TickUnlock,
    .timerStart = NULL,
    .timerStop = NULL,
    .timerCycleGet = NULL,
    .freq = 0,
};

static VOID SysResume(VOID)
{
    if (g_sysCount != (UINT32)-1) {
        g_sysCount--;
    }
}

static UINT32 SysSuspend(VOID)
{
    g_testCount++;
    g_sysCount++;

    if ((g_sysTickTimerCount != 1) || (g_deviceCount != 1) || (g_sysCount != 1)) {
        g_sysCount = (UINT32)-1;
    }

    return HalEnterSleep();
}

static LosPmSysctrl g_sysctrl = {
    .normalSuspend = HalEnterSleep,
    .normalResume = NULL,
    .lightSuspend = SysSuspend,
    .lightResume = SysResume,
};

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    g_sysCount = 0;
    g_deviceCount = 0;
    g_sysTickTimerCount = 0;
    g_testCount = 0;

    ret = LOS_PmRegister(LOS_PM_TYPE_TICK_TIMER, &g_tickTimer);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_PmRegister(LOS_PM_TYPE_DEVICE, &g_device);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_PmRegister(LOS_PM_TYPE_SYSCTRL, &g_sysctrl);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_PmModeSet(LOS_SYS_LIGHT_SLEEP);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_PmLockRequest("testlock");
    LOS_TaskDelay(100); /* delay 100 ticks */
    LOS_PmLockRelease("testlock");

    ICUNIT_GOTO_EQUAL(g_testCount, 0, g_testCount, EXIT);

    LOS_TaskDelay(100); /* delay 100 ticks */

    ICUNIT_GOTO_NOT_EQUAL(g_testCount, 0, g_sysCount, EXIT);
    ICUNIT_GOTO_EQUAL(g_sysCount, 0, g_sysCount, EXIT);

    ret = LOS_PmUnregister(LOS_PM_TYPE_DEVICE, &g_device);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_PmUnregister(LOS_PM_TYPE_TICK_TIMER, &g_tickTimer);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;

EXIT:
    LOS_PmUnregister(LOS_PM_TYPE_DEVICE, &g_device);

    LOS_PmUnregister(LOS_PM_TYPE_TICK_TIMER, &g_tickTimer);
    return LOS_NOK;
}

VOID ItLosPm002(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosPm002", TestCase, TEST_LOS, TEST_TASK, TEST_LEVEL0, TEST_FUNCTION);
}

