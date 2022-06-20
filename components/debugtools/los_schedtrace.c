/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_debugtools.h"
#include "securec.h"
#include "los_debug.h"
#include "los_memory.h"
#include "los_arch.h"

#if (LOSCFG_DEBUG_TOOLS == 1)
STATIC BOOL g_startTrace = FALSE;
STATIC SchedTraceInfo *g_traceRingBuf = NULL;
STATIC UINT32 g_schedCount = 0;
STATIC SchedTraceRecordCB g_recordCB = NULL;
STATIC SchedTraceShowCB g_showCB = NULL;

STATIC VOID DefaultShowFormat(SchedTraceInfo *buf, UINT32 count)
{
    INT32 i;
    UINT32 cycle = count / TRACE_NUM;
    UINT32 point = count % TRACE_NUM;

    PRINTK ("sched %u time, last %u is:\r\n", count, TRACE_NUM);
    PRINTK("RunTaskID     RunTaskName      NewTaskID    NewTaskName  \r\n");
    if (cycle > 0) {
        for (i = point; i < TRACE_NUM; i++) {
            PRINTK("%4u %20s,  %4u %20s\n",
                   g_traceRingBuf[i].runTaskID, g_traceRingBuf[i].runTaskName,
                   g_traceRingBuf[i].newTaskID, g_traceRingBuf[i].newTaskName);
        }
    }

    for (i = 0; i < point; i++) {
        PRINTK("%4u %20s,  %4u %20s\n",
               g_traceRingBuf[i].runTaskID, g_traceRingBuf[i].runTaskName,
               g_traceRingBuf[i].newTaskID, g_traceRingBuf[i].newTaskName);
    }

    PRINTK("\r\n");
}

STATIC VOID DefaultRecordHandle(LosTaskCB *newTask, LosTaskCB *runTask)
{
    UINT32 point = g_schedCount % TRACE_NUM;

    g_traceRingBuf[point].newTaskID = newTask->taskID;
    (VOID)memcpy_s(g_traceRingBuf[point].newTaskName, LOS_TASK_NAMELEN, newTask->taskName, LOS_TASK_NAMELEN);
    g_traceRingBuf[point].runTaskID = runTask->taskID;
    (VOID)memcpy_s(g_traceRingBuf[point].runTaskName, LOS_TASK_NAMELEN, runTask->taskName, LOS_TASK_NAMELEN);

    g_schedCount++;
}

STATIC VOID ShowFormat(SchedTraceInfo *buf, UINT32 count)
{
    if (count == 0) {
        PRINT_ERR("none shed happened\n");
        return;
    }

    if (g_showCB != NULL) {
        g_showCB(buf, count);
    }
}

VOID OsSchedTraceRecord(LosTaskCB *newTask, LosTaskCB *runTask)
{
    if (g_startTrace == FALSE) {
        return;
    }

    if (g_recordCB != NULL) {
        g_recordCB(newTask, runTask);
    }
}

VOID LOS_SchedTraceStart(VOID)
{
    if (g_recordCB == NULL) {
        g_recordCB = DefaultRecordHandle;
    }

    g_traceRingBuf = (SchedTraceInfo *)LOS_MemAlloc(OS_SYS_MEM_ADDR, TRACE_NUM * sizeof(SchedTraceInfo));
    if (g_traceRingBuf == NULL) {
        PRINT_ERR("alloc failed for dump\n");
        return;
    }
    (VOID)memset_s(g_traceRingBuf, TRACE_NUM * sizeof(SchedTraceInfo), 0, TRACE_NUM * sizeof(SchedTraceInfo));

    g_startTrace = TRUE;
}

VOID LOS_SchedTraceStop(VOID)
{
    if (g_showCB == NULL) {
        g_showCB = DefaultShowFormat;
    }
    g_startTrace = FALSE;
    ShowFormat(g_traceRingBuf, g_schedCount);
    g_schedCount = 0;

    if (g_traceRingBuf != NULL) {
        (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, g_traceRingBuf);
        g_traceRingBuf = NULL;
    }

    return;
}

VOID LOS_SchedTraceHandleRegister(SchedTraceRecordCB recordCB, SchedTraceShowCB showCB)
{
    g_recordCB = recordCB;
    g_showCB = showCB;
}

UINT32 OsShellCmdSchedTrace(INT32 argc, const CHAR **argv)
{
    if (argc != 1) {
        PRINT_ERR("\nUsage: st -h\n");
        return LOS_NOK;
    }

    if (strcmp(argv[0], "-s") == 0) {
        LOS_SchedTraceStart();
    } else if (strcmp(argv[0], "-e") == 0) {
        LOS_SchedTraceStop();
    } else if (strcmp(argv[0], "-h") == 0) {
        PRINTK("\nUsage: \nst -s , SchedTrace start\n");
        PRINTK("st -e , SchedTrace end and show\n");
    } else {
        PRINTK("\nUsage: st -h\n");
        return OS_ERROR;
    }

    return LOS_OK;
}
#endif /* LOSCFG_STACK_DUMP == 1 */
