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

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 rwsemHandle;

    ret = LOS_RwsemCreate(&rwsemHandle);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* Acquire a read lock so that PendWrite will block and time out */
    ret = LOS_RwsemPendRead(rwsemHandle, LOS_WAIT_FOREVER);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* PendWrite with non-zero timeout will block and eventually time out */
    ret = LOS_RwsemPendWrite(rwsemHandle, 2); /* 2, timeout in ticks */
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_TIMEOUT, ret, EXIT);

    ret = LOS_RwsemPostRead(rwsemHandle);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

EXIT:
    ret = LOS_RwsemDelete(rwsemHandle);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

/**
 * @ingroup TEST_RWSEM
 * @par TestCase_Number
 * ItLosRwsem008
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_RwsemPendWrite timeout behavior
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: create a rwsem and acquire a read lock
 * step2: call LOS_RwsemPendWrite with timeout=2
 * step3: release the read lock and delete the rwsem
 * @par TestCase_Expected_Result
 * 1. step2 returns LOS_ERRNO_RWSEM_TIMEOUT after 2 ticks
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */
VOID ItLosRwsem008(VOID)
{
    TEST_ADD_CASE("ItLosRwsem008", Testcase, TEST_LOS, TEST_RWSEM, TEST_LEVEL0, TEST_FUNCTION);
}
