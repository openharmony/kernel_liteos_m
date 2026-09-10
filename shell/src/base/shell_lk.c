/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
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

#include "shell_lk.h"
#include "los_printf_pri.h"
#include "limits.h"
#include "shcmd.h"
#include "fcntl.h"
#include "errno.h"
#include "securec.h"

#ifdef LOSCFG_SHELL_DMESG
#include "dmesg_pri.h"
#endif

#ifdef LOSCFG_SHELL_LK

#define   TRACE_EMG        0
#define   TRACE_COMMON     1
#define   TRACE_ERROR      2
#define   TRACE_WARN       3
#define   TRACE_INFO       4
#define   TRACE_DEBUG      5
#define   TRACE_DEFAULT      TRACE_ERROR

#define   MODULE0          0
#define   MODULE1          1
#define   MODULE2          2
#define   MODULE3          3
#define   MODULE4          4

typedef struct {
    INT32 moduleLevel;
    INT32 traceLevel;
    INT32 fd;
} Logger;

STATIC INT32 g_traceLevel = TRACE_DEFAULT;
STATIC INT32 g_moduleLevel;

STATIC Logger g_logger = { 0 };

STATIC const CHAR *g_logString[] = {
    "EMG",
    "COMMON",
    "ERR",
    "WARN",
    "INFO",
    "DEBUG"
};

VOID OsLkDefaultFunc(INT32 level, const CHAR *func, INT32 line, const CHAR *fmt, va_list ap);

LK_FUNC g_osLkHook = (LK_FUNC)OsLkDefaultFunc;

STATIC INLINE INT32 OsLkTraceLvGet(VOID)
{
    return g_traceLevel;
}

const CHAR *OsLkCurLogLvGet(VOID)
{
    return g_logString[g_traceLevel];
}


VOID OsLkTraceLvSet(INT32 level)
{
    g_traceLevel = level;
    g_logger.traceLevel = level;
    return;
}

VOID OsLkModuleLvSet(INT32 level)
{
    g_moduleLevel = level;
    g_logger.moduleLevel = level;
    return;
}


VOID OsLkLogFileSet(const CHAR *str)
{
    int fd = -1;

    if (str == NULL) {
        return;
    }

    fd = open(str, O_CREAT | O_RDWR | O_TRUNC, 0666); /* 0666: read | write permission */
    if (fd < 0) {
        PRINTK("Error can't open the %s file,errno %d\n", str, errno);
        return;
    }

    g_logger.fd = fd;
}


#if (TRACE_EMG == 0)
#define IS_PRINT_LEVEL_INVALID(level)  (level > TRACE_DEBUG)
#else
#define IS_PRINT_LEVEL_INVALID(level)  ((level > TRACE_DEBUG) || (level < TRACE_EMG))
#endif

#if (MODULE0 == 0)
#define IS_MODULE_INVALID(module)      (module > MODULE4)
#else
#define IS_MODULE_INVALID(module)      ((module > MODULE4) || (module < MODULE0))
#endif

INT32 CmdLog(INT32 argc, const CHAR **argv)
{
    size_t level;
    size_t module;
    CHAR *p = NULL;

    if (argc != 2) { /* 2:count of parameter */
        PRINTK("Usage: log level <num>\n");
        PRINTK("Usage: log module <num>\n");
        PRINTK("Usage: log path <PATH>\n");
        return -1;
    }

    if (!strncmp(argv[0], "level", strlen(argv[0]) + 1)) {
        level = strtoul(argv[1], &p, 0);
        if ((*p != 0) || IS_PRINT_LEVEL_INVALID(level)) {
            PRINTK("current log level %s\n", OsLkCurLogLvGet());
            PRINTK("log %s [num] can access as 0:EMG 1:COMMON 2:ERROR 3:WARN 4:INFO 5:DEBUG\n", argv[0]);
        } else {
            OsLkTraceLvSet((INT32)level);
            PRINTK("Set current log level %s\n", OsLkCurLogLvGet());
        }
    } else if (!strncmp(argv[0], "module", strlen(argv[0]) + 1)) {
        module = strtoul(argv[1], &p, 0);
        if ((*p != 0) || IS_MODULE_INVALID(module)) {
            PRINTK("log %s can't access %s\n", argv[0], argv[1]);
            PRINTK("not support yet\n");
            return -1;
        } else {
            OsLkModuleLvSet((INT32)module);
            PRINTK("not support yet\n");
        }
    } else if (!strncmp(argv[0], "path", strlen(argv[0]) + 1)) {
        OsLkLogFileSet(argv[1]);
        PRINTK("not support yet\n");
    } else {
        PRINTK("Usage: log level <num>\n");
        PRINTK("Usage: log module <num>\n");
        PRINTK("Usage: log path <PATH>\n");
        return -1;
    }

    return 0;
}

#ifdef LOSCFG_SHELL_DMESG
STATIC INLINE VOID OsLogCycleRecord(INT32 level)
{
    UINT32 tmpLen;
    if ((level != TRACE_COMMON) && ((level > TRACE_EMG) && (level < TRACE_DEBUG))) {
        tmpLen = strlen(g_logString[level]);
        const CHAR *tmpPtr = g_logString[level];
        (VOID)OsLogRecordStr(tmpPtr, tmpLen);
    }
}
#endif

VOID OsLkDefaultFunc(INT32 level, const CHAR *func, INT32 line, const CHAR *fmt, va_list ap)
{
    (VOID)func;
    (VOID)line;
    if (level > OsLkTraceLvGet()) {
#ifdef LOSCFG_SHELL_DMESG
        if ((UINT32)level <= OsDmesgLvGet()) {
            OsLogCycleRecord(level);
            DmesgPrintf(fmt, ap);
        }
#endif
        return;
    }
    if ((level != TRACE_COMMON) && ((level > TRACE_EMG) && (level < TRACE_DEBUG))) {
        PRINTK("[%s]", g_logString[level]);
    }
    LkDprintf(fmt, ap);
}

VOID LOS_LkPrint(INT32 level, const CHAR *func, INT32 line, const CHAR *fmt, ...)
{
    va_list ap;
    LK_FUNC osLkHook = g_osLkHook;
    if (osLkHook != NULL) {
        va_start(ap, fmt);
        osLkHook(level, func, line, fmt, ap);
        va_end(ap);
    }
}

VOID LOS_LkRegHook(LK_FUNC hook)
{
    g_osLkHook = hook;
}

VOID OsLkLoggerInit(const CHAR *str)
{
    (VOID)str;
    (VOID)memset_s(&g_logger, sizeof(Logger), 0, sizeof(Logger));
    OsLkTraceLvSet(TRACE_DEFAULT);
    LOS_LkRegHook(OsLkDefaultFunc);
#ifdef LOSCFG_SHELL_DMESG
    (VOID)LOS_DmesgLvSet(TRACE_DEFAULT);
#endif
}

#endif
