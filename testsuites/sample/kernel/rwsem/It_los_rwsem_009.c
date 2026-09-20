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

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 rwsemHandle;

    /* PendWrite with invalid handle */
    ret = LOS_RwsemPendWrite(LOSCFG_BASE_IPC_RWSEM_LIMIT, LOS_WAIT_FOREVER);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret);

    ret = LOS_RwsemPendWrite(LOSCFG_BASE_IPC_RWSEM_LIMIT + 1, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret);

    /* PostWrite with invalid handle */
    ret = LOS_RwsemPostWrite(LOSCFG_BASE_IPC_RWSEM_LIMIT);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret);

    ret = LOS_RwsemCreate(&rwsemHandle);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* PostWrite on a rwsem with no write lock held → INVALID_STATUS */
    ret = LOS_RwsemPostWrite(rwsemHandle);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID_STATUS, ret, EXIT);

EXIT:
    ret = LOS_RwsemDelete(rwsemHandle);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

/**
 * @ingroup TEST_RWSEM
 * @par TestCase_Number
 * ItLosRwsem009
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_RwsemPendWrite / LOS_RwsemPostWrite error paths
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: call LOS_RwsemPendWrite with invalid handle
 * step2: call LOS_RwsemPostWrite with invalid handle
 * step3: create a rwsem, call LOS_RwsemPostWrite without holding a write lock
 * @par TestCase_Expected_Result
 * 1. step1 returns LOS_ERRNO_RWSEM_INVALID
 * 2. step2 returns LOS_ERRNO_RWSEM_INVALID
 * 3. step3 returns LOS_ERRNO_RWSEM_INVALID_STATUS
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */
VOID ItLosRwsem009(VOID)
{
    TEST_ADD_CASE("ItLosRwsem009", Testcase, TEST_LOS, TEST_RWSEM, TEST_LEVEL0, TEST_FUNCTION);
}
#endif