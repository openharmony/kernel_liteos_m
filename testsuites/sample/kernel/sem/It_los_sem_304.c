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
#include "It_los_sem.h"

/* 用例简要描述: LOS_SemGetValue 越界句柄 INVALID(不测 NULL currVal) */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    INT32 currVal = 0;

    /* F-Inv: 句柄 == LIMIT 即越界, 实现先算指针后校验, 早返不解引用(los_sem.c:336) */
    ret = LOS_SemGetValue((UINT32)LOSCFG_BASE_IPC_SEM_LIMIT, &currVal);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SEM_INVALID, ret);

    /* F-Inv: 更大句柄同样拒绝 */
    ret = LOS_SemGetValue(0xFFFF, &currVal);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SEM_INVALID, ret);

    return LOS_OK;
}

VOID ItLosSem304(VOID)
{
    TEST_ADD_CASE("ItLosSem304", Testcase, TEST_LOS, TEST_SEM, TEST_LEVEL0, TEST_FUNCTION);
}
