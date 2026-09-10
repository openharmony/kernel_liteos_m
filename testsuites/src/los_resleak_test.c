/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
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

#include "osTest.h"
#include "los_resleak_test.h"
#include "los_mux.h"
#include "los_sem.h"
#include "los_queue.h"
#include "los_swtmr.h"
#include "los_task_pri.h"
#include "los_memory.h"
#include "los_config.h"
#if (LOSCFG_POSIX_MQUEUE_API == 1)
#include <mqueue.h>
#include <fcntl.h>
#endif

extern UINT32 QueueUsedCountGet(VOID);
extern UINT32 TaskUsedCountGet(VOID);
extern UINT32 SwtmrCountGetTest(VOID);

STATIC ResLeakStat g_resLeakBeforeStat;
STATIC volatile UINT32 g_resLeakTotalCnt = 0;

STATIC UINT32 OsResLeakMemUsedGet(VOID)
{
#if (LOSCFG_MEM_WATERLINE == 1)
    return ((struct OsMemPoolHead *)m_aucSysMem0)->info.curUsedSize;
#else
    return LOS_MemTotalUsedGet(m_aucSysMem0);
#endif
}

STATIC VOID OsResLeakForceReclaim(VOID)
{
    (VOID)LOS_TaskResRecycle();
}

#if (LOSCFG_POSIX_MQUEUE_API == 1)
STATIC VOID OsResLeakMqueuePreInit(VOID)
{
    STATIC UINT32 s_preInitialized = 0;
    mqd_t mqd;
    const CHAR *name = "/resleak_mq_prewarm";

    if (s_preInitialized != 0) {
        return;
    }
    s_preInitialized = 1;

    mqd = mq_open(name, O_CREAT | O_RDWR, 0, NULL);
    if (mqd != (mqd_t)-1) {
        (VOID)mq_close(mqd);
        (VOID)mq_unlink(name);
    }
}
#endif

VOID OsResLeakInit(VOID)
{
#if (LOSCFG_POSIX_MQUEUE_API == 1)
    OsResLeakMqueuePreInit();
#endif
    OsResLeakForceReclaim();
}

UINT32 OsResLeakMuxCountGet(VOID)
{
    UINT32 intSave;
    UINT32 count = 0;
    UINT32 index;

    intSave = LOS_IntLock();
    for (index = 0; index < LOSCFG_BASE_IPC_MUX_LIMIT; index++) {
        LosMuxCB *muxNode = ((LosMuxCB *)g_allMux) + index;
        if (muxNode->muxStat == OS_MUX_USED) {
            count++;
        }
    }
    (VOID)LOS_IntRestore(intSave);
    return count;
}

UINT32 OsResLeakSemCountGet(VOID)
{
    UINT32 intSave;
    UINT32 count = 0;
    UINT32 index;

    intSave = LOS_IntLock();
    for (index = 0; index < LOSCFG_BASE_IPC_SEM_LIMIT; index++) {
        LosSemCB *semNode = ((LosSemCB *)g_allSem) + index;
        if (semNode->semStat == OS_SEM_USED) {
            count++;
        }
    }
    (VOID)LOS_IntRestore(intSave);
    return count;
}

VOID OsResLeakCheckBegin(VOID)
{
    OsResLeakForceReclaim();
    g_resLeakBeforeStat.muxCnt = OsResLeakMuxCountGet();
    g_resLeakBeforeStat.queueCnt = QueueUsedCountGet();
    g_resLeakBeforeStat.semCnt = OsResLeakSemCountGet();
    g_resLeakBeforeStat.swtmrCnt = SwtmrCountGetTest();
    g_resLeakBeforeStat.taskCnt = TaskUsedCountGet();
    g_resLeakBeforeStat.memUsed = OsResLeakMemUsedGet();
}

VOID OsResLeakCheckEnd(const CHAR *caseName)
{
    ResLeakStat after;
    UINT32 leakCnt = 0;

    OsResLeakForceReclaim();
    after.muxCnt = OsResLeakMuxCountGet();
    after.queueCnt = QueueUsedCountGet();
    after.semCnt = OsResLeakSemCountGet();
    after.swtmrCnt = SwtmrCountGetTest();
    after.taskCnt = TaskUsedCountGet();
    after.memUsed = OsResLeakMemUsedGet();

    if (after.muxCnt != g_resLeakBeforeStat.muxCnt) {
        dprintf("[ResLeak] %s: mux leak! before=%u after=%u\n",
                caseName, g_resLeakBeforeStat.muxCnt, after.muxCnt);
        leakCnt++;
    }
    if (after.queueCnt != g_resLeakBeforeStat.queueCnt) {
        dprintf("[ResLeak] %s: queue leak! before=%u after=%u\n",
                caseName, g_resLeakBeforeStat.queueCnt, after.queueCnt);
        leakCnt++;
    }
    if (after.semCnt != g_resLeakBeforeStat.semCnt) {
        dprintf("[ResLeak] %s: sem leak! before=%u after=%u\n",
                caseName, g_resLeakBeforeStat.semCnt, after.semCnt);
        leakCnt++;
    }
    if (after.swtmrCnt != g_resLeakBeforeStat.swtmrCnt) {
        dprintf("[ResLeak] %s: swtmr leak! before=%u after=%u\n",
                caseName, g_resLeakBeforeStat.swtmrCnt, after.swtmrCnt);
        leakCnt++;
    }
    if (after.taskCnt != g_resLeakBeforeStat.taskCnt) {
        dprintf("[ResLeak] %s: task leak! before=%u after=%u\n",
                caseName, g_resLeakBeforeStat.taskCnt, after.taskCnt);
        leakCnt++;
    }
    if (after.memUsed != g_resLeakBeforeStat.memUsed) {
        dprintf("[ResLeak] %s: memory leak! before=%u after=%u\n",
                caseName, g_resLeakBeforeStat.memUsed, after.memUsed);
        leakCnt++;
    }

    if (leakCnt > 0) {
        g_resLeakTotalCnt += leakCnt;
        dprintf("[ResLeak] %s: %u resource(s) leaked in this case\n", caseName, leakCnt);
    }
}

UINT32 OsResLeakGetTotalLeakCnt(VOID)
{
    return g_resLeakTotalCnt;
}

