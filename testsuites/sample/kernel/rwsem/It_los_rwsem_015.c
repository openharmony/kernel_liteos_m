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

static UINT32 g_rwsemHandle015;

static VOID HwiF01(VOID)
{
    UINT32 ret;
    TestHwiClear(HWI_NUM_TEST);

    /* PendRead/PendWrite with non-zero timeout during an interrupt is forbidden */
    ret = LOS_RwsemPendRead(g_rwsemHandle015, LOS_WAIT_FOREVER);
    ICUNIT_TRACK_EQUAL(ret, LOS_ERRNO_RWSEM_PEND_INTERR, ret);

    ret = LOS_RwsemPendRead(g_rwsemHandle015, 1);
    ICUNIT_TRACK_EQUAL(ret, LOS_ERRNO_RWSEM_PEND_INTERR, ret);

    ret = LOS_RwsemPendWrite(g_rwsemHandle015, LOS_WAIT_FOREVER);
    ICUNIT_TRACK_EQUAL(ret, LOS_ERRNO_RWSEM_PEND_INTERR, ret);

    ret = LOS_RwsemPendWrite(g_rwsemHandle015, 1);
    ICUNIT_TRACK_EQUAL(ret, LOS_ERRNO_RWSEM_PEND_INTERR, ret);

    g_testCount++;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;

    g_testCount = 0;

    ret = LOS_RwsemCreate(&g_rwsemHandle015);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_HwiCreate(HWI_NUM_TEST, 1, 0, (HWI_PROC_FUNC)HwiF01, 0);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    TestHwiTrigger(HWI_NUM_TEST);
    TestHwiDelete(HWI_NUM_TEST);

    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);

EXIT:
    TestHwiDelete(HWI_NUM_TEST);
    ret = LOS_RwsemDelete(g_rwsemHandle015);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

/**
 * @ingroup TEST_RWSEM
 * @par TestCase_Number
 * ItLosRwsem015
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_RwsemPendRead / LOS_RwsemPendWrite during an interrupt
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: create a rwsem
 * step2: create a hardware interrupt handler that calls PendRead/PendWrite with non-zero timeout
 * step3: trigger the interrupt
 * step4: delete the rwsem
 * @par TestCase_Expected_Result
 * 1. PendRead and PendWrite in the interrupt handler return LOS_ERRNO_RWSEM_PEND_INTERR
 * @par TestCase_Level
 * Level 2
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * Requires HWI test support. Gated by LOS_KERNEL_TEST_FULL and LOS_KERNEL_HWI_TEST.
 */
VOID ItLosRwsem015(VOID)
{
    TEST_ADD_CASE("ItLosRwsem015", Testcase, TEST_LOS, TEST_RWSEM, TEST_LEVEL2, TEST_FUNCTION);
}
