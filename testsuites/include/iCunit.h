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


#ifndef _LOS_ICUNIT_H
#define _LOS_ICUNIT_H
#ifdef __clang__
#pragma clang diagnostic ignored "-Wreturn-type"
#endif
#include "osTest.h"

#ifdef TST_DRVPRINT
#include "VOS_typdef.h"
#include "uartdriver.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

typedef unsigned short iUINT16;
typedef unsigned int iUINT32;
typedef signed short iINT16;
typedef signed long iINT32;
typedef char iCHAR;
typedef void iVOID;

typedef unsigned long iiUINT32;


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define FUNCTION_TEST (1 << 0)

#define PRESSURE_TEST (1 << 1)

#define PERFORMANCE_TEST (1 << 2)

#define TEST_MODE (FUNCTION_TEST)

typedef iUINT32 (*CASE_FUNCTION)(void);

typedef struct {
    iCHAR *pcCaseID;
    CASE_FUNCTION pstCaseFunc;
    iUINT16 testcase_layer;
    iUINT16 testcase_module;
    iUINT16 testcase_level;
    iUINT16 testcase_type;
    iiUINT32 retCode;
    iUINT16 errLine;
} ICUNIT_CASE_S;

typedef struct {
    iUINT16 uwCaseCnt;
    iCHAR *pcSuitID;
    iCHAR *pucFilename;
    ICUNIT_CASE_S *pstCaseList;
    iUINT16 passCnt;
    iUINT16 failCnt;
} ICUNIT_SUIT_S;

typedef enum {
#define ICUNIT_MOD(name) name,
#include "icunit_module_def.h"
#undef ICUNIT_MOD
} LiteOS_test_module;

typedef enum {
#define ICUNIT_LAYER(name) name,
#include "icunit_layer_def.h"
#undef ICUNIT_LAYER
} LiteOS_test_layer;

typedef enum {
#define ICUNIT_LEVEL(name) name,
#include "icunit_level_def.h"
#undef ICUNIT_LEVEL
} LiteOS_test_level;

typedef enum {
#define ICUNIT_TYPE(name) name,
#include "icunit_type_def.h"
#undef ICUNIT_TYPE
} LiteOS_test_type;


#define ICUNIT_UNINIT 0x0EF00000
#define ICUNIT_OPENFILE_FAILED 0x0EF00001
#define ICUNIT_ALLOC_FAIL 0x0EF00002
#define ICUNIT_SUIT_FULL 0x0EF00002
#define ICUNIT_CASE_FULL 0x0EF00003
#define ICUNIT_SUIT_ALL 0x0EF0FFFF

#define ICUNIT_SUCCESS 0x00000000

#define ICUNIT_FAIL_LOG_MAX 50

typedef struct {
    iCHAR *pcCaseID;
    iUINT16 testcase_layer;
    iUINT16 testcase_module;
    iUINT16 testcase_level;
    iUINT16 testcase_type;
    iUINT16 errLine;
    iiUINT32 retCode;
} ICUNIT_FAIL_LOG_S;

typedef struct {
    iUINT32 execCount;
    iUINT32 failCount;
    iUINT32 passCount;
} ICUNIT_MODULE_STAT_S;


extern iUINT16 iCunit_errLineNo;
extern iiUINT32 iCunit_errCode;
extern void ICunitSaveErr(iiUINT32 line, iiUINT32 retCode);

extern ICUNIT_FAIL_LOG_S g_failLogArray[ICUNIT_FAIL_LOG_MAX];
extern iUINT32 g_failLogCount;
extern ICUNIT_MODULE_STAT_S g_moduleStat[];
extern void ICunitRecordFailLog(iCHAR *caseID, iUINT16 layer, iUINT16 module, iUINT16 level, iUINT16 type,
    iUINT16 errLine, iiUINT32 retCode);
extern void ICunitPrintFailLogs(void);
extern const char *ICunitLayerToStr(iUINT16 layer);
extern const char *ICunitModuleToStr(iUINT16 module);
extern const char *ICunitLevelToStr(iUINT16 level);
extern const char *ICunitTypeToStr(iUINT16 type);

#define ICUNIT_TRACK_EQUAL(param, value, retcode)       \
    do {                                                \
        if ((param) != (value)) {                       \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode); \
        }                                               \
    } while (0)

#define ICUNIT_TRACK_NOT_EQUAL(param, value, retcode)   \
    do {                                                \
        if ((param) == (value)) {                       \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode); \
        }                                               \
    } while (0)

#define ICUNIT_ASSERT_EQUAL_VOID(param, value, retcode) \
    do {                                                \
        if ((param) != (value)) {                       \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode); \
            return;                                     \
        }                                               \
    } while (0)

#define ICUNIT_ASSERT_NOT_EQUAL_VOID(param, value, retcode) \
    do {                                                    \
        if ((param) == (value)) {                           \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);     \
            return;                                         \
        }                                                   \
    } while (0)
#define ICUNIT_ASSERT_EQUAL(param, value, retcode)      \
    do {                                                \
        if ((param) != (value)) {                       \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode); \
            return 1;                                   \
        }                                               \
    } while (0)

#define ICUNIT_ASSERT_NOT_EQUAL(param, value, retcode)  \
    do {                                                \
        if ((param) == (value)) {                       \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode); \
            return 1;                                   \
        }                                               \
    } while (0)

#define ICUNIT_ASSERT_WITHIN_EQUAL(param, value1, value2, retcode) \
    do {                                                           \
        if ((param) < (value1) || (param) > (value2)) {            \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);            \
            return 1;                                              \
        }                                                          \
    } while (0)

#define ICUNIT_ASSERT_WITHIN_EQUAL_VOID(param, value1, value2, retcode) \
    do {                                                                \
        if ((param) < (value1) || (param) > (value2)) {                 \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);                 \
            return;                                                     \
        }                                                               \
    } while (0)

#define ICUNIT_ASSERT_EQUAL_VOID(param, value, retcode) \
    do {                                                \
        if ((param) != (value)) {                       \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode); \
            return;                                     \
        }                                               \
    } while (0)

#define ICUNIT_ASSERT_SIZE_STRING_EQUAL(str1, str2, strsize, retcode) \
    do {                                                              \
        if (strncmp((str1), (str2), (strsize)) != 0) {                \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);               \
            return 1;                                                 \
        }                                                             \
    } while (0)

#define ICUNIT_ASSERT_STRING_EQUAL(str1, str2, retcode) \
    do {                                                \
        if ((const VOID *)(str1) != (const VOID *)(str2)) { \
            if ((str1) == NULL || (str2) == NULL || strcmp(str1, str2) != 0) { \
                ICunitSaveErr(__LINE__, (iiUINT32)retcode); \
                return 1;                               \
            }                                           \
        }                                               \
    } while (0)

#define ICUNIT_ASSERT_STRING_EQUAL_VOID(str1, str2, retcode)       \
    do {                                                           \
        if ((const VOID *)(str1) != (const VOID *)(str2)) {        \
            if ((str1) == NULL || (str2) == NULL || strcmp((const char *)str1, (const char *)str2) != 0) { \
                ICunitSaveErr(__LINE__, (iiUINT32)retcode);        \
                return;                                            \
            }                                                       \
        }                                                          \
    } while (0)

#define ICUNIT_ASSERT_STRING_NOT_EQUAL(str1, str2, retcode) \
    do {                                                    \
        if ((const VOID *)(str1) == (const VOID *)(str2)) { \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);     \
            return 1;                                       \
        }                                                   \
        if ((str1) != NULL && (str2) != NULL && strcmp(str1, str2) == 0) { \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);     \
            return 1;                                       \
        }                                                   \
    } while (0)

#define ICUNIT_GOTO_EQUAL(param, value, retcode, label) \
    do {                                                \
        if ((param) != (value)) {                       \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode); \
            goto label;                                 \
        }                                               \
    } while (0)

#define ICUNIT_GOTO_NOT_EQUAL(param, value, retcode, label) \
    do {                                                    \
        if ((param) == (value)) {                           \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);     \
            goto label;                                     \
        }                                                   \
    } while (0)

#define ICUNIT_GOTO_STRING_EQUAL(str1, str2, retcode, label) \
    do {                                                     \
        if (strcmp(str1, str2) != 0) {                       \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);      \
            goto label;                                      \
        }                                                    \
    } while (0)

#define ICUNIT_GOTO_STRING_NOT_EQUAL(str1, str2, retcode, label) \
    do {                                                         \
        if (strcmp(str1, str2) == 0) {                           \
            ICunitSaveErr(__LINE__, (iiUINT32)retcode);          \
            goto label;                                          \
        }                                                        \
    } while (0)

extern iUINT32 iCunitAddSuit_F(iCHAR *suitName, iCHAR *pfileName);
#define iCunitAddSuit(suitName) iCunitAddSuit_F(suitName, __FILE__)

extern iUINT32 ICunitAddCase(iCHAR *caseName, CASE_FUNCTION caseFunc, iUINT16 testcaseLayer, iUINT16 testcaseModule,
    iUINT16 testcaseLevel, iUINT16 testcaseType);

extern iUINT32 ICunitRun(void);
extern iUINT32 ICunitInit(void);
extern iUINT32 iCunitPrintReport(void);


#define TEST_ADD_CASE(name, casefunc, testcase_layer, testcase_module, testcase_level, testcase_type)         \
    do {                                                                                                      \
        iUINT32 uwRet = 1;                                                                                    \
        uwRet = ICunitAddCase(name, (CASE_FUNCTION)casefunc, testcase_layer, testcase_module, testcase_level, \
            testcase_type);                                                                                   \
        ICUNIT_ASSERT_EQUAL_VOID(uwRet, ICUNIT_SUCCESS, uwRet);                                               \
    } while (0)

#define TEST_RUN_SUITE()                                                   \
    do {                                                                   \
        UINT32 uiRet;                                                      \
        uiRet = ICunitRun();                                               \
        ICUNIT_ASSERT_NOT_EQUAL_VOID(uiRet, ICUNIT_UNINIT, ICUNIT_UNINIT); \
    } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _UNI_ICUNIT_H */
