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

/* 用例简要描述: LOS_BinarySemCreate count=2 越界 OVERFLOW/NULL 句柄 PTR_NULL */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 semHandle;

    /* F-Inv: count=2 超过二值上限 OS_SEM_BINARY_MAX_COUNT(1), OsSemCreate count>maxCount 早返 */
    ret = LOS_BinarySemCreate(2, &semHandle);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SEM_OVERFLOW, ret);

    /* F-Inv: 输出句柄 NULL */
    ret = LOS_BinarySemCreate(1, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_SEM_PTR_NULL, ret);

    return LOS_OK;
}

VOID ItLosSem302(VOID)
{
    TEST_ADD_CASE("ItLosSem302", Testcase, TEST_LOS, TEST_SEM, TEST_LEVEL0, TEST_FUNCTION);
}
