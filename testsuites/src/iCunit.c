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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
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

#include "iCunit.h"
#include "iCunit.inc"
#include "string.h"
#include "osTest.h"
#include "iCunit_config.h"
#include "icunit_filter.h"
#include "los_resleak_test.h"

#include <stdio.h>
#include "securec.h"

extern UINT32 g_failResult;
extern UINT32 g_passResult;

const char *ICunitLayerToStr(iUINT16 l)
{
    switch (l) {
#define ICUNIT_LAYER(name) case name: return #name;
#include "icunit_layer_def.h"
#undef ICUNIT_LAYER
        default: return "UNKNOWN";
    }
}

const char *ICunitModuleToStr(iUINT16 m)
{
    switch (m) {
#define ICUNIT_MOD(name) case name: return #name;
#include "icunit_module_def.h"
#undef ICUNIT_MOD
        default: return "UNKNOWN";
    }
}

const char *ICunitLevelToStr(iUINT16 l)
{
    switch (l) {
#define ICUNIT_LEVEL(name) case name: return #name;
#include "icunit_level_def.h"
#undef ICUNIT_LEVEL
        default: return "UNKNOWN";
    }
}

const char *ICunitTypeToStr(iUINT16 t)
{
    switch (t) {
#define ICUNIT_TYPE(name) case name: return #name;
#include "icunit_type_def.h"
#undef ICUNIT_TYPE
        default: return "UNKNOWN";
    }
}

#define MODULE_COUNT (TEST_TRACE + 1)
ICUNIT_MODULE_STAT_S g_moduleStat[MODULE_COUNT];

void ICunitSaveErr(iiUINT32 line, iiUINT32 retCode)
{
    iCunit_errLineNo = (iCunit_errLineNo == 0) ? line : iCunit_errLineNo;
    iCunit_errCode = (iCunit_errCode == 0) ? (iiUINT32)retCode : iCunit_errCode;
}

iUINT32 ICunitAddCase(iCHAR *caseName, CASE_FUNCTION caseFunc, iUINT16 testcaseLayer, iUINT16 testcaseModule,
    iUINT16 testcaseLevel, iUINT16 testcaseType)
{
    iUINT16 idx;

    if (iCunit_Init_Success) {
        return (iUINT32)ICUNIT_UNINIT;
    }

    /* 调试过滤：未命中过滤表的用例直接返回，不存表也不执行（遍历但不进入） */
    if (!IcFilterHit(caseName)) {
        return (iUINT32)ICUNIT_SUCCESS;
    }

    /* 崩溃续跑：崩溃点之前的用例直接返回，崩溃点本身跳过并计数，之后正常执行 */
    if (!IcResumeHit(caseName)) {
        return (iUINT32)ICUNIT_SUCCESS;
    }

    idx = 0;
    if (idx == ICUNIT_CASE_SIZE) {
        iCunit_ErrLog_AddCase++;
        return (iUINT32)ICUNIT_CASE_FULL;
    }

    iCunit_CaseArray[idx].pcCaseID = caseName;
    iCunit_CaseArray[idx].pstCaseFunc = caseFunc;
    iCunit_CaseArray[idx].testcase_layer = testcaseLayer;
    iCunit_CaseArray[idx].testcase_module = testcaseModule;
    iCunit_CaseArray[idx].testcase_level = testcaseLevel;
    iCunit_CaseArray[idx].testcase_type = testcaseType;

    ICunitRun();
    return (iUINT32)ICUNIT_SUCCESS;
}

iUINT32 ICunitInit()
{
    iCunit_Init_Success = 0x0000;
    iCunit_Case_Cnt = 0x0000;

    iCunit_Case_FailedCnt = 0;

    iCunit_ErrLog_AddCase = 0;

    (void)memset_s(iCunit_CaseArray, sizeof(iCunit_CaseArray), 0, sizeof(iCunit_CaseArray));
    (void)memset_s(g_failLogArray, sizeof(g_failLogArray), 0, sizeof(g_failLogArray));
    (void)memset_s(g_moduleStat, sizeof(g_moduleStat), 0, sizeof(g_moduleStat));
    g_failLogCount = 0;
    return (iUINT32)ICUNIT_SUCCESS;
}

void ICunitRecordFailLog(iCHAR *caseID, iUINT16 layer, iUINT16 module, iUINT16 level, iUINT16 type,
    iUINT16 errLine, iiUINT32 retCode)
{
    ICUNIT_FAIL_LOG_S *pLog;
    if (g_failLogCount >= ICUNIT_FAIL_LOG_MAX) {
        return;
    }
    pLog = &g_failLogArray[g_failLogCount];
    pLog->pcCaseID = caseID;
    pLog->testcase_layer = layer;
    pLog->testcase_module = module;
    pLog->testcase_level = level;
    pLog->testcase_type = type;
    pLog->errLine = errLine;
    pLog->retCode = retCode;
    g_failLogCount++;
}

void ICunitPrintFailLogs(void)
{
    iUINT32 i, j;
    ICUNIT_FAIL_LOG_S *pLog;
    ICUNIT_MODULE_STAT_S *pStat;

    dprintf("\n========== Failed Case Logs (%d / %d) ==========\n", g_failLogCount, ICUNIT_FAIL_LOG_MAX);
    for (i = 0; i < MODULE_COUNT; i++) {
        pStat = &g_moduleStat[i];
        if (pStat->execCount == 0) {
            continue;
        }
        dprintf("Executed Model: %s, Executed Model_Num: %d, failed_count: %d, success_count: %d\n",
            ICunitModuleToStr((iUINT16)i), (iUINT32)i, pStat->failCount, pStat->passCount);
        for (j = 0; j < g_failLogCount; j++) {
            pLog = &g_failLogArray[j];
            if (pLog->testcase_module != (iUINT16)i) {
                continue;
            }
            dprintf("  [Failed]-%s-%s-%s-%s-%s-[Errline: %d RetCode:0x%lx]\n", pLog->pcCaseID,
                ICunitLayerToStr(pLog->testcase_layer), ICunitModuleToStr(pLog->testcase_module),
                ICunitLevelToStr(pLog->testcase_level), ICunitTypeToStr(pLog->testcase_type),
                pLog->errLine, pLog->retCode);
        }
    }
    dprintf("\n Not Executed Model: ");
    for (i = 0; i < MODULE_COUNT; i++) {
        pStat = &g_moduleStat[i];
        if (pStat->execCount != 0) {
            continue;
        }
        dprintf("%s    ", ICunitModuleToStr((iUINT16)i));
    }
    dprintf("\n========== End of Failed Case Logs ==========\n");
}

iUINT32 ICunitRunF(void)
{
    iUINT32 idx, idx1;
    ICUNIT_CASE_S *psubCaseArray;
    iUINT32 caseRet;

    psubCaseArray = iCunit_CaseArray;
    idx1 = 1;

    for (idx = 0; idx < idx1; idx++, psubCaseArray++) {
        iCunit_errLineNo = 0;
        iCunit_errCode = 0;

        OsResLeakCheckBegin();
        caseRet = psubCaseArray->pstCaseFunc();
        psubCaseArray->errLine = iCunit_errLineNo;
        psubCaseArray->retCode = (0 == iCunit_errLineNo) ? (caseRet) : (iCunit_errCode);

        if (0 == iCunit_errLineNo && 0 == caseRet) {
            g_passResult++;
            if (psubCaseArray->testcase_module < MODULE_COUNT) {
                g_moduleStat[psubCaseArray->testcase_module].execCount++;
                g_moduleStat[psubCaseArray->testcase_module].passCount++;
            }
            dprintf("  [Passed]-%s-%s-%s-%s-%s\n", psubCaseArray->pcCaseID,
                ICunitLayerToStr(psubCaseArray->testcase_layer),
                ICunitModuleToStr(psubCaseArray->testcase_module),
                ICunitLevelToStr(psubCaseArray->testcase_level),
                ICunitTypeToStr(psubCaseArray->testcase_type));
        } else {
            g_failResult++;
            iCunit_Case_FailedCnt++;
            if (psubCaseArray->testcase_module < MODULE_COUNT) {
                g_moduleStat[psubCaseArray->testcase_module].execCount++;
                g_moduleStat[psubCaseArray->testcase_module].failCount++;
            }
            dprintf("  [Failed]-%s-%s-%s-%s-%s-[Errline: %d RetCode:0x%lx]\n", psubCaseArray->pcCaseID,
                ICunitLayerToStr(psubCaseArray->testcase_layer),
                ICunitModuleToStr(psubCaseArray->testcase_module),
                ICunitLevelToStr(psubCaseArray->testcase_level),
                ICunitTypeToStr(psubCaseArray->testcase_type),
                psubCaseArray->errLine, psubCaseArray->retCode);
            ICunitRecordFailLog(psubCaseArray->pcCaseID, psubCaseArray->testcase_layer,
                psubCaseArray->testcase_module, psubCaseArray->testcase_level, psubCaseArray->testcase_type,
                psubCaseArray->errLine, psubCaseArray->retCode);
        }
        OsResLeakCheckEnd(psubCaseArray->pcCaseID);
    }

    return (iUINT32)ICUNIT_SUCCESS;
}

iUINT32 ICunitRun()
{
    if (iCunit_Init_Success) {
        return (iUINT32)ICUNIT_UNINIT;
    }

    ICunitRunF();

    return (iUINT32)ICUNIT_SUCCESS;
}

