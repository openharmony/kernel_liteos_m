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

/* 用例简要描述: 信号量 ABA 行为暴露:Delete 头插+Create 取链头 index 立即复用 */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 semId1;
    UINT32 semId2;
    INT32 currVal = 0xFF; /* 255, init as invalid */

    /* D3(ABA 暴露): 社区版 ID 为纯 index, Delete 头插 unused 链表(los_sem.c:189 LOS_ListAdd),
       下一次 Create 取链表头(LOS_DL_LIST_FIRST) -> 立即复用同 index, 无 ID 分裂防 ABA 保护 */
    ret = LOS_SemCreate(1, &semId1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SemDelete(semId1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SemCreate(1, &semId2);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* 复用断言: 新句柄 == 旧句柄 */
    ICUNIT_GOTO_EQUAL(semId2, semId1, semId2, EXIT);

    /* 旧句柄(==新句柄) 操作完全有效 */
    ret = LOS_SemPend(semId1, LOS_WAIT_FOREVER);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SemGetValue(semId1, &currVal);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ICUNIT_GOTO_EQUAL(currVal, 0, currVal, EXIT);

    ret = LOS_SemPost(semId1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SemDelete(semId1);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    return LOS_OK;

EXIT:
    (VOID)LOS_SemDelete(semId2); /* semId1==semId2, 删一次 */
    return LOS_OK;
}

VOID ItLosSem305(VOID)
{
    TEST_ADD_CASE("ItLosSem305", Testcase, TEST_LOS, TEST_SEM, TEST_LEVEL1, TEST_FUNCTION);
}
