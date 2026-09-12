/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of the conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
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

#include "It_los_shell.h"

#ifdef LOSCFG_SHELL_DMESG

static UINT32 Testcase(VOID)
{
    static CHAR dmesgBuf[1024];
    CHAR readSink[8];
    UINT32 ret;
    UINT32 savedLevel;
    INT32 readRet;

    savedLevel = OsDmesgLvGet();

    ret = LOS_DmesgMemSet(dmesgBuf, sizeof(dmesgBuf));
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    LOS_DmesgClear();

    readRet = LOS_DmesgRead(readSink, sizeof(readSink));
    ICUNIT_GOTO_EQUAL(readRet, 0, readRet, EXIT);

    readRet = LOS_DmesgRead(NULL, sizeof(readSink));
    ICUNIT_GOTO_EQUAL(readRet, -1, readRet, EXIT);

    readRet = LOS_DmesgRead(readSink, 0);
    ICUNIT_GOTO_EQUAL(readRet, 0, readRet, EXIT);

    ret = LOS_DmesgLvSet(TRACE_DEBUG);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    LOS_DmesgClear();
    readRet = LOS_DmesgRead(readSink, sizeof(readSink));
    ICUNIT_GOTO_EQUAL(readRet, 0, readRet, EXIT);

EXIT:
    (VOID)LOS_DmesgLvSet(savedLevel);
    ret = LOS_DmesgMemSet(NULL, KERNEL_LOG_BUF_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

/**
 * @ingroup TEST_SHELL
 * @par TestCase_Number
 * ItLosShell001
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test interface LOS_DmesgMemSet / LOS_DmesgRead / LOS_DmesgClear / LOS_DmesgLvSet
 * @par TestCase_Pretreatment_Condition
 * NA.
 * @par TestCase_Test_Steps
 * step1: save the current dmesg log level, install a fresh ring buffer via
 *        LOS_DmesgMemSet(buf, 1024), then clear
 * step2: read empty buffer, expect 0
 * step3: read with NULL buf, expect -1
 * step4: read with 0 length, expect 0
 * step5: set log level to TRACE_DEBUG, expect LOS_OK
 * step6: clear and read, expect 0
 * step7: cleanup (runs on pass and fail paths): restore the saved log level,
 *        restore dmesg to default 8KB malloc pool, expect LOS_OK
 * @par TestCase_Expected_Result
 * 1. each call returns the documented value
 * 2. no crash, dmesg state (buffer and log level) restored on every path
 *    after the test
 * @par TestCase_Level
 * Level 1
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * null
 */
VOID ItLosShell001(VOID)
{
    TEST_ADD_CASE("ItLosShell001", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL1, TEST_FUNCTION);
}

#endif /* LOSCFG_SHELL_DMESG */
