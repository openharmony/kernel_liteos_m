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


#include "it_los_misc.h"
#include "los_error.h"

static UINT32 g_errHookCalled = 0;
static UINT32 g_errHookErrNo = 0;
static UINT32 g_errHookLine = 0;

/* 测试回调:记录调用与参数 */
static VOID ErrHookFunc(CHAR *fileName, UINT32 lineNo, UINT32 errorNo, UINT32 paraLen, VOID *para)
{
    (VOID)fileName;
    (VOID)paraLen;
    (VOID)para;
    g_errHookCalled++;
    g_errHookLine = lineNo;
    g_errHookErrNo = errorNo;
}

/* 用例简要描述: LOS_RegErrHandle 注册→ErrHandle 转发(计数/lineNo/errorNo 精确)→注销 */
static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 para = 0xA5A5;

    /* F-Pos: 注册 -> 触发 -> 回调收到参数 */
    LOS_RegErrHandle(ErrHookFunc);
    g_errHookCalled = 0;

    ret = LOS_ErrHandle("it_los_err_001.c", 42, 0x1234, sizeof(para), &para);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(g_errHookCalled, 1, g_errHookCalled, EXIT);
    ICUNIT_GOTO_EQUAL(g_errHookLine, 42, g_errHookLine, EXIT);
    ICUNIT_GOTO_EQUAL(g_errHookErrNo, 0x1234, g_errHookErrNo, EXIT);

    /* F-Pos: 注销 -> 再触发 -> 回调不再调用(恒 LOS_OK) */
    LOS_RegErrHandle(NULL);
    ret = LOS_ErrHandle("it_los_err_001.c", 84, 0x5678, 0, NULL);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(g_errHookCalled, 1, g_errHookCalled, EXIT);

    return LOS_OK;

EXIT:
    LOS_RegErrHandle(NULL);
    return LOS_OK;
}

VOID ItLosErr001(VOID)
{
    TEST_ADD_CASE("ItLosErr001", TestCase, TEST_LOS, TEST_MISC, TEST_LEVEL0, TEST_FUNCTION);
}
