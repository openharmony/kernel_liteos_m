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
#include "It_los_sem.h"

/* 用例简要描述: LOS_Sem 越界句柄负路径：Delete/Pend 传入超出
 * LOSCFG_BASE_IPC_SEM_LIMIT 的句柄 → LOS_ERRNO_SEM_INVALID。只调对外接口。 */

static UINT32 TestCase(VOID)
{
    UINT32 ret;

    /* F-Inv: 越界句柄删除 → INVALID。 */
    ret = LOS_SemDelete(0xFFFF);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SEM_INVALID, ret);

    /* F-Inv: 越界句柄 pend → INVALID。 */
    ret = LOS_SemPend(0xFFFF, LOS_NO_WAIT);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SEM_INVALID, ret);

    return LOS_OK;
}

VOID ItLosSem045(void)
{
    TEST_ADD_CASE("ItLosSem045", TestCase, TEST_LOS, TEST_SEM, TEST_LEVEL1, TEST_FUNCTION);
}
