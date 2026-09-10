/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of the conditions and the following disclaimer in the documentation and/or other materials
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

#include "it_los_exc.h"

#if (LOSCFG_BACKTRACE_TYPE != 0)
static UINT32 Testcase(VOID)
{
    UINT32 invalidId;
    UINT32 unusedId = LOSCFG_BASE_CORE_TSK_LIMIT;
    invalidId = LOSCFG_BASE_CORE_TSK_LIMIT + 1;
    LOS_TaskBackTrace(invalidId);
    LOS_TaskBackTrace(0xFFFF);
    LOS_TaskBackTrace(unusedId);
    ICUNIT_ASSERT_EQUAL(1, 1, 0);
    return LOS_OK;
}

/**
 * @ingroup TEST_EXC
 * @par TestCase_Number
 * ItLosExc001
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_TaskBackTrace
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: call LOS_TaskBackTrace(out-of-range id = LOSCFG_BASE_CORE_TSK_LIMIT + 1)
 * step2: call LOS_TaskBackTrace(0xFFFF)
 * step3: call LOS_TaskBackTrace(unused slot = LOSCFG_BASE_CORE_TSK_LIMIT)
 * @par TestCase_Expected_Result
 * 1. each call returns cleanly via the error branch (no fault)
 * 2. reach the end ICUNIT_ASSERT_EQUAL(1, 1, 0)
 * @par TestCase_Level
 * Level 0
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */
VOID ItLosExc001(VOID)
{
    TEST_ADD_CASE("ItLosExc001", Testcase, TEST_LOS, TEST_EXC, TEST_LEVEL0, TEST_FUNCTION);
}
#endif /* LOSCFG_BACKTRACE_TYPE != 0 */
