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

#include "los_exc_pri.h"
#include "securec.h"
#include "los_interrupt.h"
#include "los_memory_pri.h"
#include "los_config.h"

#ifdef LOSCFG_SHELL_EXCINFO_DUMP
typedef struct {
    CHAR *buf;                  /* pointer to the buffer for storing the exception information */
    UINT32 offset;              /* the offset of the buffer for storing the exception information */
    UINT32 len;                 /* the size of storing the exception information */
    UINTPTR dumpAddr;           /* the address of storing the exception information */
} ExcInfoDumpFormat;

STATIC ExcInfoDumpFormat g_excInfoPool = {0};
/* the hook of read-writing exception information */
STATIC LogReadWriteFunc g_dumpHook = NULL;

VOID LOS_ExcInfoRegHook(UINTPTR startAddr, UINT32 space, CHAR *buf, LogReadWriteFunc hook)
{
    if ((hook == NULL) || (buf == NULL)) {
        PRINT_ERR("Buf or hook is null.\n");
        return;
    }

    g_excInfoPool.dumpAddr = startAddr;
    g_excInfoPool.len = space;
    /*
     * offset was initialized to MAX, and then was assigned to 0 in exc handler when it happens,
     * which make sure that PrintExcInfo only be called in exc handler.
     */
    g_excInfoPool.offset = 0xFFFFFFFF;
    g_excInfoPool.buf = buf;
    g_dumpHook = hook;
}

VOID OsSetExcInfoRW(LogReadWriteFunc func)
{
    g_dumpHook = func;
}

LogReadWriteFunc OsGetExcInfoRW(VOID)
{
    return g_dumpHook;
}

VOID OsSetExcInfoBuf(CHAR *buf)
{
    g_excInfoPool.buf = buf;
}

CHAR *OsGetExcInfoBuf(VOID)
{
    return g_excInfoPool.buf;
}

VOID OsSetExcInfoOffset(UINT32 offset)
{
    g_excInfoPool.offset = offset;
}

UINT32 OsGetExcInfoOffset(VOID)
{
    return g_excInfoPool.offset;
}

VOID OsSetExcInfoDumpAddr(UINTPTR addr)
{
    g_excInfoPool.dumpAddr = addr;
}

UINTPTR OsGetExcInfoDumpAddr(VOID)
{
    return g_excInfoPool.dumpAddr;
}

VOID OsSetExcInfoLen(UINT32 len)
{
    g_excInfoPool.len = len;
}

UINT32 OsGetExcInfoLen(VOID)
{
    return g_excInfoPool.len;
}

WEAK VOID WriteExcBufVa(const CHAR *format, va_list arglist)
{
    errno_t ret;
    CHAR *startAddr;

    if ((g_excInfoPool.buf != NULL) && (g_excInfoPool.len > g_excInfoPool.offset)) {
        startAddr = g_excInfoPool.buf + g_excInfoPool.offset;
        ret = vsnprintf_s(startAddr, (g_excInfoPool.len - g_excInfoPool.offset),
                          (g_excInfoPool.len - g_excInfoPool.offset - 1), format, arglist);
        if (ret == -1) {
            PRINT_ERR("exc info buffer is not enough or vsnprintf_s is error.\n");
            return;
        }

        g_excInfoPool.offset += (UINT32)ret;
    }
}

VOID WriteExcInfoToBuf(const CHAR *format, ...)
{
    va_list arglist;

    va_start(arglist, format);
    WriteExcBufVa(format, arglist);
    va_end(arglist);
}
#endif

#if (LOSCFG_PLATFORM_EXC == 1)
STATIC VOID OsMemExcInfoGetSub(VOID *pool, MemInfoCB *memExcInfo)
{
    LOS_MEM_POOL_STATUS status = {0};

    (VOID)memset_s(memExcInfo, sizeof(MemInfoCB), 0, sizeof(MemInfoCB));
    if (LOS_MemInfoGet(pool, &status) != LOS_OK) {
        return;
    }
    memExcInfo->type = MEM_MANG_MEMORY;
    memExcInfo->startAddr = (UINT32)(UINTPTR)pool;
    memExcInfo->size = LOS_MemPoolSizeGet(pool);
    memExcInfo->free = status.totalFreeSize;
    memExcInfo->blockSize = status.usedNodeNum + status.freeNodeNum;
#ifdef LOSCFG_KERNEL_MEM_TLSF
    OsMemExcErrInfoGet(pool, memExcInfo);
#endif
}

UINT32 OsMemExcInfoGet(UINT32 memNumMax, MemInfoCB *memExcInfo)
{
    UINT8 *buffer = (UINT8 *)memExcInfo;
    UINT32 count = 0;

#if (LOSCFG_MEM_MUL_POOL == 1)
    VOID *memPool = OsMemMulPoolHeadGet();
    while (memPool != NULL) {
        OsMemExcInfoGetSub(memPool, (MemInfoCB *)buffer);
        count++;
        if (count >= memNumMax) {
            break;
        }
        buffer += sizeof(MemInfoCB);
        memPool = ((LosMemPoolInfo *)memPool)->nextPool;
    }
#else
    OsMemExcInfoGetSub((VOID *)m_aucSysMem0, (MemInfoCB *)buffer);
    count++;
#endif
    return count;
}
#endif
