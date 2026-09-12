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


#include "it_los_exc.h"
#include "los_debug.h" /* EXC_TYPE/ExcHookFn */

/* exchook 头目录不在 testsuites include 路径(qemu 侧),按工程先例 extern 声明
   (实现: kernel/extended/exchook/los_exchook.c) */
extern UINT32 LOS_RegExcHook(EXC_TYPE excType, ExcHookFn excHookFn);
extern UINT32 LOS_UnRegExcHook(EXC_TYPE excType, ExcHookFn excHookFn);

static UINT32 g_excDispatchCnt = 0;
static UINT32 g_excDispatchType = 0xFF; /* 255, init as not-run */

static VOID ExcDispatchHook(EXC_TYPE excType)
{
    g_excDispatchCnt++;
    g_excDispatchType = (UINT32)excType;
}

/* 用例简要描述: 板级双路径异常钩子派发(ws63 组件级 Reg/UnReg;qemu 内核级单例) */
static UINT32 Testcase(VOID)
{
    UINT32 ret;

    #if defined(LOSCFG_SHELL_EXCINFO_DUMP)
    /* ===== ws63 路径: exchook 组件编入(分级钩子链) ===== */
    /* F-Inv: NULL 钩子 -> LOS_ERRNO_SYS_PTR_NULL(los_exchook.c:95) */
    ret = LOS_RegExcHook(EXC_ASSERT, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SYS_PTR_NULL, ret);

    /* F-Pos: 组件级注册 -> 内核级派发精确透传类型 */
    g_excDispatchCnt = 0;
    g_excDispatchType = 0xFF;
    ret = LOS_RegExcHook(EXC_ASSERT, ExcDispatchHook);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    OsDoExcHook(EXC_ASSERT);
    ICUNIT_ASSERT_EQUAL(g_excDispatchCnt, 1, g_excDispatchCnt);
    ICUNIT_ASSERT_EQUAL(g_excDispatchType, (UINT32)EXC_ASSERT, g_excDispatchType);

    /* F-Pos: 注销后不再派发 */
    ret = LOS_UnRegExcHook(EXC_ASSERT, ExcDispatchHook);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    OsDoExcHook(EXC_ASSERT);
    ICUNIT_ASSERT_EQUAL(g_excDispatchCnt, 1, g_excDispatchCnt);
#else
    /* ===== qemu 路径: exchook 未编入,内核级单例机制(los_debug.c) ===== */
    (VOID)ret; /* 本路径无返回值接口 */
    /* F-Pos: 注册 -> 派发精确透传类型 */
    g_excDispatchCnt = 0;
    g_excDispatchType = 0xFF;
    OsExcHookRegister(ExcDispatchHook);

    OsDoExcHook(EXC_ASSERT);
    ICUNIT_ASSERT_EQUAL(g_excDispatchCnt, 1, g_excDispatchCnt);
    ICUNIT_ASSERT_EQUAL(g_excDispatchType, (UINT32)EXC_ASSERT, g_excDispatchType);

    /* 契约: 首注册不可覆盖(los_debug.c:68 if(!g_excHook)),二次注册(NULL)为 no-op */
    OsExcHookRegister(NULL);
    OsDoExcHook(EXC_ASSERT);
    ICUNIT_ASSERT_EQUAL(g_excDispatchCnt, 2, g_excDispatchCnt);
#endif

    return LOS_OK;
}

VOID ItLosExc011(VOID)
{
    TEST_ADD_CASE("ItLosExc011", Testcase, TEST_LOS, TEST_EXC, TEST_LEVEL1, TEST_FUNCTION);
}
