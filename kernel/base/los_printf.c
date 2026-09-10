/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Device Co., Ltd. 2020-2026. All rights reserved.
 * Description: LiteOS Printf Implementation
 * Author: Huawei LiteOS Team
 * Create: 2013-01-01
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --------------------------------------------------------------------------- */
/* ---------------------------------------------------------------------------
 * Notice of los_printf.c Modification
 *===============================================
 * When this file is getting modified, it should consider the compatibility of SE LiteOS.
 * It should not add OS inner invocation, which is not allowed to be accessed in user space.
 * But it's allowed to invoke syscall interface.
 *---------------------------------------------------------------------------- */

#ifdef LOSCFG_LIB_LIBC
#include "unistd.h"
#endif
#include "los_debug.h"
#include "los_memory.h"

#ifdef LOSCFG_SHELL_DMESG
#include "dmesg_pri.h"
#endif
#ifdef LOSCFG_SHELL_EXCINFO_DUMP
#include "los_exc_pri.h"
#endif
#include "los_printf_pri.h"

#include <string.h>
#include "securec.h"

#define SIZEBUF  256

typedef enum {
    NO_OUTPUT = 0,
    UART_OUTPUT = 1,
    CONSOLE_OUTPUT = 2,
    EXC_OUTPUT = 3
} OutputType;

/* Weak default UartPuts: no-op. Boards provide a strong UartPuts for actual output. */
WEAK VOID UartPuts(const CHAR *s, UINT32 len, BOOL isLock)
{
    (VOID)s;
    (VOID)len;
    (VOID)isLock;
}

STATIC VOID ErrorMsg(VOID)
{
    const CHAR *p = "Output illegal string! vsnprintf_s failed!\n";
    UartPuts(p, (UINT32)strlen(p), UART_WITH_LOCK);
}

STATIC VOID UartOutput(const CHAR *str, UINT32 len, BOOL isLock)
{
#ifdef LOSCFG_SHELL_DMESG
    if (!OsCheckUartLock()) {
        UartPuts(str, len, isLock);
    }
    if (isLock != UART_WITHOUT_LOCK) {
        (VOID)OsLogMemcpyRecord(str, len);
    }
#else
    UartPuts(str, len, isLock);
#endif
}

STATIC VOID OutputControl(const CHAR *str, UINT32 len, OutputType type)
{
    switch (type) {
        case CONSOLE_OUTPUT:
#ifdef LOSCFG_KERNEL_CONSOLE
            if (OsConsoleIsEnabled() == TRUE) {
                (VOID)write(STDOUT_FILENO, str, (size_t)len);
                break;
            }
#endif
            /* fall-through */
        case UART_OUTPUT:
            UartOutput(str, len, UART_WITH_LOCK);
            break;
        case EXC_OUTPUT:
            UartOutput(str, len, UART_WITHOUT_LOCK);
            break;
        default:
            break;
    }
    return;
}

STATIC VOID OsVprintf(const CHAR *fmt, va_list ap, OutputType type)
{
    INT32 len = 0;
    CHAR aBuf[SIZEBUF] = {0};
    CHAR *bBuf = NULL;
    UINT32 bufLen = SIZEBUF;

    bBuf = aBuf;
    len = vsnprintf_s(bBuf, bufLen, bufLen - 1, fmt, ap);
#ifdef LOSCFG_KERNEL_PRINTF_SIZE_EXTEND
    if ((len == -1) && (*bBuf == '\0')) {
        /* parameter is illegal or some features in fmt dont support */
        ErrorMsg();
        return;
    }

    const CHAR *errMsgMalloc = "OsVprintf, malloc failed!\n";
    while (len == -1) {
        /* bBuf is not enough */
        if (bufLen != SIZEBUF) {
            (VOID)LOS_MemFree(m_aucSysMem0, bBuf);
        }

        bufLen = bufLen << 1;
        bBuf = (CHAR *)LOS_MemAlloc(m_aucSysMem0, bufLen);
        if (bBuf == NULL) {
            UartPuts(errMsgMalloc, (UINT32)strlen(errMsgMalloc), UART_WITH_LOCK);
            return;  /* must return here, otherwise NULL dereference below */
        }

        len = vsnprintf_s(bBuf, bufLen, bufLen - 1, fmt, ap);

        if (*bBuf == '\0') {
            /* parameter is illegal or some features in fmt dont support */
            (VOID)LOS_MemFree(m_aucSysMem0, bBuf);
            ErrorMsg();
            return;
        }
    }
#else
    if (len == -1) {
        /* parameter is illegal or some features in fmt dont support */
        ErrorMsg();
        return;
    }
#endif

    *(bBuf + len) = '\0';
    OutputControl(bBuf, len, type);
#ifdef LOSCFG_KERNEL_PRINTF_SIZE_EXTEND
    if (bufLen != SIZEBUF) {
        (VOID)LOS_MemFree(m_aucSysMem0, bBuf);
    }
#endif
}

VOID UartVprintf(const CHAR *fmt, va_list ap)
{
    OsVprintf(fmt, ap, UART_OUTPUT);
}

VOID ConsoleVprintf(const CHAR *fmt, va_list ap)
{
    OsVprintf(fmt, ap, CONSOLE_OUTPUT);
}

#if (LOSCFG_KERNEL_PRINTF == 1)
VOID LOS_Printf(LogModuleType type, INT32 level, const CHAR *fmt, ...)
{
    (VOID)type;
    if (OsLogLevelCheck(level) != LOS_OK) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    OsVprintf(fmt, ap, CONSOLE_OUTPUT);
    va_end(ap);
}
#endif

VOID LkDprintf(const CHAR *fmt, va_list ap)
{
    OsVprintf(fmt, ap, CONSOLE_OUTPUT);
}

#ifdef LOSCFG_SHELL_DMESG
VOID DmesgPrintf(const CHAR *fmt, va_list ap)
{
    OsVprintf(fmt, ap, CONSOLE_OUTPUT);
}
#endif

VOID PrintExcInfo(const CHAR *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    /* uart output without print-spinlock */
    OsVprintf(fmt, ap, EXC_OUTPUT);
#ifdef LOSCFG_SHELL_EXCINFO_DUMP
    WriteExcBufVa(fmt, ap);
#endif
    va_end(ap);
}
