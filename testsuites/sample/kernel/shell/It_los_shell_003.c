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

#if defined(LOSCFG_SHELL_LK) && defined(LOSCFG_SHELL_DMESG)

#define SHELL_TEST_DMESG_BUF_SIZE 64
#define SHELL_TEST_DMESG_MARKER "lk_dmesg_roundtrip_marker_0042"

static UINT32 Testcase(VOID)
{
    static CHAR dmesgBuf[SHELL_TEST_DMESG_BUF_SIZE];
    CHAR readSink[256];
    UINT32 ret;
    UINT32 savedLevel;
    INT32 readRet;

    OsLkLoggerInit("");
    savedLevel = OsDmesgLvGet();

    ret = LOS_DmesgMemSet(dmesgBuf, sizeof(dmesgBuf));
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_DmesgLvSet(TRACE_WARN);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    LOS_DmesgClear();
    LOS_LkPrint(TRACE_INFO, __func__, __LINE__, "%s", SHELL_TEST_DMESG_MARKER);
    readRet = LOS_DmesgRead(readSink, sizeof(readSink) - 1);
    ICUNIT_GOTO_EQUAL(readRet, 0, readRet, EXIT);

    ret = LOS_DmesgLvSet(TRACE_DEBUG);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    LOS_DmesgClear();
    LOS_LkPrint(TRACE_ERROR, __func__, __LINE__, "%s", SHELL_TEST_DMESG_MARKER);
    readRet = LOS_DmesgRead(readSink, sizeof(readSink) - 1);
    if (readRet <= 0) {
        ICunitSaveErr(__LINE__, (iiUINT32)readRet);
        goto EXIT;
    }
    readSink[readRet] = '\0';
    if (strstr(readSink, SHELL_TEST_DMESG_MARKER) == NULL) {
        ICunitSaveErr(__LINE__, (iiUINT32)readRet);
        goto EXIT;
    }

EXIT:
    (VOID)LOS_DmesgLvSet(savedLevel);
    ret = LOS_DmesgMemSet(NULL, KERNEL_LOG_BUF_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    return LOS_OK;
}

/**
 * @ingroup TEST_SHELL
 * @par TestCase_Number
 * ItLosShell003
 * @par TestCase_TestCase_Type
 * Function test
 * @brief Test lk→dmesg end-to-end data flow and level filtering
 * @par TestCase_Pretreatment_Condition
 * LOSCFG_SHELL_LK and LOSCFG_SHELL_DMESG enabled
 * @par TestCase_Test_Steps
 * step1: reset the LK logger via OsLkLoggerInit, save the dmesg log level,
 *        install a small (64-byte) dmesg buffer, set level to TRACE_WARN
 * step2: write marker via LOS_LkPrint(TRACE_INFO), verify dmesg read returns 0 (filtered: INFO > WARN)
 * step3: set level to TRACE_DEBUG, clear, write marker via LOS_LkPrint(TRACE_ERROR)
 * step4: read back via LOS_DmesgRead, verify marker present (passed: ERROR <= DEBUG)
 * step5: cleanup (runs on pass and fail paths): restore the saved dmesg log
 *        level and the default 8KB malloc pool; the default LK hook
 *        registered by OsLkLoggerInit stays attached
 * @par TestCase_Expected_Result
 * 1. TRACE_INFO is filtered when level is TRACE_WARN (read returns 0)
 * 2. TRACE_ERROR passes when level is TRACE_DEBUG (read returns > 0, marker present)
 * 3. dmesg state (buffer and log level) restored on every path after the test
 * @par TestCase_Level
 * Level 1
 * @par TestCase_Automated
 * true
 * @par TestCase_Remark
 * Uses 64-byte buffer to increase likelihood of triggering ring-buffer wrap-around
 */
VOID ItLosShell003(VOID)
{
    TEST_ADD_CASE("ItLosShell003", Testcase, TEST_LOS, TEST_SHELL_MOD, TEST_LEVEL1, TEST_FUNCTION);
}

#endif /* LOSCFG_SHELL_LK && LOSCFG_SHELL_DMESG */
