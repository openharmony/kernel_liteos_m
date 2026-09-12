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


#include "It_los_rwsem.h"

#if (LOSCFG_BASE_IPC_RWSEM == 1)

/* 用例简要描述: PendRead/PostRead 多读共享与超释放 INVALID_STATUS */
static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 rwsemHandle;

    ret = LOS_RwsemCreate(&rwsemHandle);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Pos: 同任务多读锁(count 0->2, 读共享) */
    ret = LOS_RwsemPendRead(rwsemHandle, LOS_WAIT_FOREVER);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_RwsemPendRead(rwsemHandle, LOS_WAIT_FOREVER);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Pos: 释放两把读锁 */
    ret = LOS_RwsemPostRead(rwsemHandle);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_RwsemPostRead(rwsemHandle);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Inv: 超释放(count 已归 0) -> INVALID_STATUS(los_rwsem.c:300) */
    ret = LOS_RwsemPostRead(rwsemHandle);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID_STATUS, ret, EXIT);

EXIT:
    ret = LOS_RwsemDelete(rwsemHandle);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    return LOS_OK;
}

#endif /* LOSCFG_BASE_IPC_RWSEM */

VOID ItLosRwsem003(VOID)
{
#if (LOSCFG_BASE_IPC_RWSEM == 1)
    TEST_ADD_CASE("ItLosRwsem003", Testcase, TEST_LOS, TEST_RWSEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
