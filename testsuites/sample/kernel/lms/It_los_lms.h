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

#ifndef IT_LOS_LMS_H
#define IT_LOS_LMS_H

#include "osTest.h"
#include "los_memory.h"
#include "los_config.h"
#include "iCunit.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define INDEX_MAX                        20
#define PAGE_SIZE                        (0x1000U)

extern char g_testLmsPool[2 * PAGE_SIZE];

VOID ItLosLms001(void);
VOID ItLosLms002(void);
VOID ItLosLms003(void);
VOID ItLosLms004(void);
VOID ItLosLms005(void);
VOID ItLosLms006(void);
VOID ItLosLms007(void);
VOID ItLosLms008(void);
VOID ItLosLms009(void);
VOID ItLosLms010(void);
VOID ItLosLms011(void);
VOID ItLosLms012(void);
VOID ItLosLms013(void);
VOID ItLosLms014(void);
VOID ItLosLms015(void);
VOID ItLosLms016(void);
VOID ItLosLms017(void);
VOID ItLosLms018(void);
VOID ItLosLms019(void);
VOID ItLosLms020(void);
VOID ItLosLms021(void);
VOID ItLosLms022(void);
VOID ItLosLms023(void);
VOID ItLosLms024(void);
/* LMS compatibility & new feature coverage tests */
VOID ItLosLms025(void);
VOID ItLosLms026(void);
VOID ItLosLms027(void);
VOID ItLosLms028(void);
VOID ItLosLms029(void);
VOID ItLosLms030(void);
VOID ItLosLms031(void);

#if defined(LOSCFG_PLATFORM_WS63_M) && defined(LOSCFG_KERNEL_LMS)
/* LMS 错误报告在关中断状态下打印 backtrace+mem dump(gcov 插桩下单份约 4s),
 * 期间 tick 大量丢失, 喂狗任务 app_task 无法按 100 tick 周期唤醒, 连续报告
 * 会耗尽 60s 看门狗窗口导致单板复位。在用例边界/报告组之间直接调用板级
 * 喂狗接口(app_task 喂的是同一个看门狗), 不依赖调度恢复。仅 WS63 且开启
 * LMS 时生效, 其它平台/配置编译为空。 */
extern void ws63_watchdog_feed(void);
#define LMS_FEED_WDT() ws63_watchdog_feed()
#else
#define LMS_FEED_WDT()
#endif

/* ==================== LMS Test Sandbox ==================== */
#include "los_lms_pri.h"

#define LMS_TEST_SANDBOX_SIZE  (2 * PAGE_SIZE)
extern CHAR g_lmsSandboxBuf[LMS_TEST_SANDBOX_SIZE];
extern BOOL g_lmsSandboxFirstCall;

typedef UINT32 (*LmsTestImplFn)(VOID *pool);

static inline UINT32 LmsTestRunInSandbox(LmsTestImplFn impl, UINT32 poolSize)
{
    UINT32 ret;

    if (poolSize > LMS_TEST_SANDBOX_SIZE) {
        poolSize = LMS_TEST_SANDBOX_SIZE;
    }
    if (!g_lmsSandboxFirstCall) {
        (VOID)memset_s(g_lmsSandboxBuf, LMS_TEST_SANDBOX_SIZE, 0, LMS_TEST_SANDBOX_SIZE);
        (VOID)LOS_MemDeInit(g_lmsSandboxBuf);
    }
    g_lmsSandboxFirstCall = FALSE;
#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
    if (LOS_MemPoolInit(g_lmsSandboxBuf, poolSize, 0) != LOS_OK) {
        return LOS_NOK;
    }
#else
    if (LOS_MemInit(g_lmsSandboxBuf, poolSize) != LOS_OK) {
        return LOS_NOK;
    }
#endif
    ret = impl(g_lmsSandboxBuf);
    OsLmsErrorHookSet(NULL);
    return ret;
}

#define LMS_TEST_RUN_IN_SANDBOX(impl_fn, pool_size) \
    LmsTestRunInSandbox((impl_fn), (pool_size))

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* IT_LOS_LMS_H */
