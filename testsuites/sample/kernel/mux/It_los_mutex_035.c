/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
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
#include "It_los_mux.h"

/* 用例简要描述: LOS_MuxPend 在软件定时器回调中被调用。swtmr 回调运行于系统级
 * 定时器任务（OS_TASK_FLAG_SYSTEM，los_swtmr.c:183），OsMuxValidCheck 应拒绝
 * 并返回 LOS_ERRNO_MUX_PEND_IN_SYSTEM_TASK（覆盖 los_mux.c L191-192）。只调对外接口。 */

static UINT32 g_testMux035;
static UINT32 g_swtmrPendRet;
static volatile UINT32 g_swtmrRan;

static VOID SwtmrFunc035(UINT32 arg)
{
    (VOID)arg;
    g_swtmrPendRet = LOS_MuxPend(g_testMux035, 10); // 10, timeout ticks; rejected before pend.
    g_swtmrRan = 1;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 swtmrId = 0;
    UINT32 loop;

    g_swtmrRan = 0;
    g_swtmrPendRet = LOS_OK;

    ret = LOS_MuxCreate(&g_testMux035);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_SwtmrCreate(4, LOS_SWTMR_MODE_ONCE, SwtmrFunc035, &swtmrId, 0xffff
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
        , OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE
#endif
    );
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SwtmrStart(swtmrId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    for (loop = 0; loop < 100; loop++) { // 100, bounded poll ticks for the once-timer.
        if (g_swtmrRan == 1) {
            break;
        }
        (VOID)LOS_TaskDelay(1);
    }
    ICUNIT_GOTO_EQUAL(g_swtmrRan, 1, g_swtmrRan, EXIT);
#if defined(LOSCFG_BASE_CORE_SWTMR_IN_ISR) && (LOSCFG_BASE_CORE_SWTMR_IN_ISR == 1)
    /* 板级 target_config.h 定义 SWTMR_IN_ISR（如 qemu mps2-an386）：swtmr 回调
     * 直接运行于 tick 中断上下文 → 命中 OsMuxValidCheck 的中断检查。 */
    ICUNIT_GOTO_EQUAL(g_swtmrPendRet, LOS_ERRNO_MUX_IN_INTERR, g_swtmrPendRet, EXIT);
#else
    /* 默认派发模式：swtmr 回调运行于系统级定时器任务（OS_TASK_FLAG_SYSTEM）
     * → 命中系统任务检查（los_mux.c L191-192）。 */
    ICUNIT_GOTO_EQUAL(g_swtmrPendRet, LOS_ERRNO_MUX_PEND_IN_SYSTEM_TASK, g_swtmrPendRet, EXIT);
#endif

EXIT:
    (VOID)LOS_SwtmrDelete(swtmrId);
    (VOID)LOS_MuxDelete(g_testMux035);
    return LOS_OK;
}

VOID ItLosMux035(void)
{
    TEST_ADD_CASE("ItLosMux035", Testcase, TEST_LOS, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
