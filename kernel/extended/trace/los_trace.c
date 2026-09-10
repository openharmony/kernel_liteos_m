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

#include "los_trace_pri.h"
#include "trace_pipeline.h"
#include "los_memory.h"
#include "los_config.h"
#include "securec.h"
#include "trace_cnv.h"
#include "los_init.h"

#if (LOSCFG_KERNEL_SMP == 1)
#include "los_mp_pri.h"
#endif

#if (LOSCFG_SHELL == 1)
#include "shcmd.h"
#include "shell.h"
#endif

#if (LOSCFG_KERNEL_TRACE == 1)
LITE_OS_SEC_BSS STATIC UINT32 g_traceEventCount;
LITE_OS_SEC_BSS STATIC volatile enum TraceState g_traceState = TRACE_UNINIT;
LITE_OS_SEC_DATA_INIT STATIC volatile BOOL g_enableTrace = FALSE;
LITE_OS_SEC_DATA_INIT STATIC UINT32 g_traceMask = TRACE_DEFAULT_MASK;

LITE_OS_SEC_BSS TRACE_EVENT_HOOK g_traceEventHook = NULL;
LITE_OS_SEC_BSS TRACE_DUMP_HOOK g_traceDumpHook = NULL;
LITE_OS_SEC_BSS VOID *g_osTraceBuf = NULL;

#if (LOSCFG_TRACE_CONTROL_AGENT == 1)
LITE_OS_SEC_BSS STATIC UINT32 g_traceTaskId;
#endif

#define EVENT_MASK            0xFFFFFFF0
#define MIN(x, y)             ((x) < (y) ? (x) : (y))

LITE_OS_SEC_BSS STATIC TRACE_HWI_FILTER_HOOK g_traceHwiFliterHook = NULL;

#define TRACE_LITE_RING_SIZE    256
#define TRACE_PACKER_TASK_PRIO  28
#define MEM_INFO_PARAM_COUNT    2
#define TRACE_TASK_LOCK_CNT_MAX 0xF
typedef struct {
    UINT32  eventType;
    UINT32  curTask;
    UINT64  curTime;
    UINTPTR identity;
    UINT8   cpuId;
    UINT8   hwiActive;
    UINT8   taskLockCnt;
    UINT8   paramCount;
    UINT32  eventCount;
    UINTPTR params[LOSCFG_TRACE_FRAME_MAX_PARAMS];
} TraceLiteEvent;
LITE_OS_SEC_BSS STATIC TraceLiteEvent g_traceLiteRing[TRACE_LITE_RING_SIZE];
LITE_OS_SEC_BSS STATIC volatile UINT16 g_liteWrite = 0;
LITE_OS_SEC_BSS STATIC volatile UINT16 g_liteRead = 0;
LITE_OS_SEC_DATA_INIT STATIC UINT32 g_deferPackerTaskId = (UINT32)-1;

#if (LOSCFG_KERNEL_SMP == 1)
LITE_OS_SEC_BSS SPIN_LOCK_INIT(g_traceSpin);
#endif

STATIC_INLINE BOOL OsTraceHwiFilter(UINT32 hwiNum)
{
    BOOL ret = ((hwiNum == NUM_HAL_INTERRUPT_UART) || (hwiNum == OS_TICK_INT_NUM));
#if (LOSCFG_KERNEL_SMP == 1)
    ret |= (hwiNum == LOS_MP_IPI_SCHEDULE);
#endif
    if (g_traceHwiFliterHook != NULL) {
        ret |= g_traceHwiFliterHook(hwiNum);
    }
    return ret;
}

VOID OsTraceSetObj(ObjData *obj, const LosTaskCB *tcb)
{
    errno_t ret;
    (VOID)memset_s(obj, sizeof(ObjData), 0, sizeof(ObjData));

    obj->id   = OsTraceGetMaskTid(tcb->taskId);
    obj->prio = tcb->priority;

    ret = strncpy_s(obj->name, LOSCFG_TRACE_OBJ_MAX_NAME_SIZE, tcb->taskName, LOSCFG_TRACE_OBJ_MAX_NAME_SIZE - 1);
    if (ret != EOK) {
        TRACE_ERROR("Task name copy failed!\n");
    }
}

STATIC VOID OsTraceEnqueue(UINT32 eventType, UINTPTR identity, const UINTPTR *params, UINT16 paramCount)
{
    UINT32 intSave;
    UINT16 w;
    UINT16 next;
    UINT16 i;
    TraceLiteEvent *e;

    if (paramCount > LOSCFG_TRACE_FRAME_MAX_PARAMS) {
        paramCount = LOSCFG_TRACE_FRAME_MAX_PARAMS;
    }

    TRACE_LOCK(intSave);
    w = g_liteWrite;
    next = (UINT16)((w + 1) % TRACE_LITE_RING_SIZE);
    if (next == g_liteRead) {
        TRACE_UNLOCK(intSave);
        return;
    }
    e = &g_traceLiteRing[w];
    e->eventType  = eventType;
    e->identity   = identity;
    e->curTask    = OsTraceGetMaskTid(LOS_CurTaskIDGet());
    e->curTime    = LOS_SysCycleGet();
    e->paramCount = (UINT8)paramCount;
#if (LOSCFG_TRACE_FRAME_CORE_MSG == 1)
    e->cpuId       = ArchCurrCpuid();
    e->hwiActive   = OS_INT_ACTIVE ? TRUE : FALSE;
    e->taskLockCnt = (UINT8)MIN(OsPercpuGet()->taskLockCnt, TRACE_TASK_LOCK_CNT_MAX);
#endif
#if (LOSCFG_TRACE_FRAME_EVENT_COUNT == 1)
    e->eventCount = g_traceEventCount;
    g_traceEventCount++;
#endif
    for (i = 0; i < paramCount; i++) {
        e->params[i] = params[i];
    }
    g_liteWrite = next;
    TRACE_UNLOCK(intSave);
}

STATIC VOID OsTracePackFrame(const TraceLiteEvent *lite)
{
    TraceEventFrame frame;
    UINT16 i;
    UINT16 n = lite->paramCount;

    (VOID)memset_s(&frame, sizeof(TraceEventFrame), 0, sizeof(TraceEventFrame));
    frame.eventType  = lite->eventType;
    frame.curTask    = lite->curTask;
    frame.curTime    = lite->curTime;
    frame.identity   = lite->identity;
#if (LOSCFG_TRACE_FRAME_CORE_MSG == 1)
    frame.core.cpuId      = lite->cpuId;
    frame.core.hwiActive  = lite->hwiActive;
    frame.core.taskLockCnt = lite->taskLockCnt;
    frame.core.paramCount = lite->paramCount;
#endif
#if (LOSCFG_TRACE_FRAME_EVENT_COUNT == 1)
    frame.eventCount = lite->eventCount;
#endif
    for (i = 0; i < n; i++) {
        frame.params[i] = lite->params[i];
    }
    OsTraceWriteOrSendEvent(&frame);
}

STATIC VOID OsTraceFlush(VOID)
{
    while (g_liteRead != g_liteWrite) {
        UINT32 intSave;
        TraceLiteEvent lite;
        TRACE_LOCK(intSave);
        if (g_liteRead == g_liteWrite) {
            TRACE_UNLOCK(intSave);
            break;
        }
        lite = g_traceLiteRing[g_liteRead];
        g_liteRead = (UINT16)((g_liteRead + 1) % TRACE_LITE_RING_SIZE);
        TRACE_UNLOCK(intSave);
        OsTracePackFrame(&lite);
    }
}

STATIC VOID OsTraceDeferPacker(VOID)
{
    while (1) {
        OsTraceFlush();
        LOS_TaskDelay(1);
    }
}

STATIC UINT32 OsCreateDeferPackerTask(VOID)
{
    TSK_INIT_PARAM_S param = {0};
    param.pfnTaskEntry  = (TSK_ENTRY_FUNC)OsTraceDeferPacker;
    param.usTaskPrio    = TRACE_PACKER_TASK_PRIO;
    param.pcName        = "TracePacker";
    param.uwStackSize   = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    return LOS_TaskCreate(&g_deferPackerTaskId, &param);
}

VOID OsTraceHook(UINT32 eventType, UINTPTR identity, const UINTPTR *params, UINT16 paramCount)
{
    if ((eventType == TASK_CREATE) || (eventType == TASK_PRIOSET)) {
        OsTraceObjAdd(eventType, identity);
    }

    if ((g_enableTrace == TRUE) && (eventType & g_traceMask)) {
        UINTPTR id = identity;
        if (TRACE_GET_MODE_FLAG(eventType) == TRACE_HWI_FLAG) {
            if (OsTraceHwiFilter(identity)) {
                return;
            }
        } else if (TRACE_GET_MODE_FLAG(eventType) == TRACE_TASK_FLAG) {
            id = OsTraceGetMaskTid(identity);
        } else if (eventType == MEM_INFO_REQ) {
            LOS_MEM_POOL_STATUS status;
            LOS_MemInfoGet((VOID *)identity, &status);
            UINTPTR mp[MEM_INFO_PARAM_COUNT] = { (UINTPTR)status.totalUsedSize, (UINTPTR)status.totalFreeSize };
            OsTraceEnqueue(MEM_INFO, identity, mp, MEM_INFO_PARAM_COUNT);
            return;
        }
        OsTraceEnqueue(eventType, id, params, paramCount);
    }
}

BOOL OsTraceIsEnable(VOID)
{
    return g_enableTrace == TRUE;
}

STATIC VOID OsTraceHookInstall(VOID)
{
    g_traceEventHook = OsTraceHook;
#if (LOSCFG_RECORDER_MODE_OFFLINE == 1)
    g_traceDumpHook = OsTraceRecordDump;
#endif
}

#if (LOSCFG_TRACE_CONTROL_AGENT == 1)
STATIC BOOL OsTraceCmdIsValid(const TraceClientCmd *msg)
{
    return ((msg->end == TRACE_CMD_END_CHAR) && (msg->cmd < TRACE_CMD_MAX_CODE));
}

STATIC VOID OsTraceCmdHandle(const TraceClientCmd *msg)
{
    if (!OsTraceCmdIsValid(msg)) {
        return;
    }

    switch (msg->cmd) {
        case TRACE_CMD_START:
            LOS_TraceStart();
            break;
        case TRACE_CMD_STOP:
            LOS_TraceStop();
            break;
        case TRACE_CMD_SET_EVENT_MASK:
            /* 4 params(UINT8) composition the mask(UINT32) */
            LOS_TraceEventMaskSet(TRACE_MASK_COMBINE(msg->param1, msg->param2, msg->param3, msg->param4));
            break;
        case TRACE_CMD_RECODE_DUMP:
            LOS_TraceRecordDump(TRUE);
            break;
        default:
            break;
    }
}

VOID TraceAgent(VOID)
{
    UINT32 ret;
    TraceClientCmd msg;

    while (1) {
        (VOID)memset_s(&msg, sizeof(TraceClientCmd), 0, sizeof(TraceClientCmd));
        ret = OsTraceDataWait();
        if (ret == LOS_OK) {
            OsTraceDataRecv((UINT8 *)&msg, sizeof(TraceClientCmd), 0);
            OsTraceCmdHandle(&msg);
        }
    }
}

STATIC UINT32 OsCreateTraceAgentTask(VOID)
{
    UINT32 ret;
    TSK_INIT_PARAM_S taskInitParam;

    (VOID)memset_s((VOID *)(&taskInitParam), sizeof(TSK_INIT_PARAM_S), 0, sizeof(TSK_INIT_PARAM_S));
    taskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)TraceAgent;
    taskInitParam.usTaskPrio = LOSCFG_TRACE_TASK_PRIORITY;
    taskInitParam.pcName = "TraceAgent";
    taskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
#if (LOSCFG_KERNEL_SMP == 1)
    taskInitParam.usCpuAffiMask = CPUID_TO_AFFI_MASK(ArchCurrCpuid());
#endif
    ret = LOS_TaskCreate(&g_traceTaskId, &taskInitParam);
    return ret;
}
#endif

UINT32 LOS_TraceInit(VOID *buf, UINT32 size)
{
    UINT32 intSave;
    UINT32 ret;

    TRACE_LOCK(intSave);
    if (g_traceState != TRACE_UNINIT) {
        TRACE_ERROR("trace has been initialized already, the current state is :%d\n", g_traceState);
        ret = LOS_ERRNO_TRACE_ERROR_STATUS;
        goto LOS_ERREND;
    }

#if (LOSCFG_TRACE_CLIENT_INTERACT == 1)
    ret = OsTracePipelineInit();
    if (ret != LOS_OK) {
        goto LOS_ERREND;
    }
#endif

#if (LOSCFG_TRACE_CONTROL_AGENT == 1)
    ret = OsCreateTraceAgentTask();
    if (ret != LOS_OK) {
        TRACE_ERROR("trace init create agentTask error :0x%x\n", ret);
        goto LOS_ERREND;
    }
#endif

#if (LOSCFG_RECORDER_MODE_OFFLINE == 1)
    ret = OsTraceBufInit(buf, size);
    if (ret != LOS_OK) {
#if (LOSCFG_TRACE_CONTROL_AGENT == 1)
        (VOID)LOS_TaskDelete(g_traceTaskId);
#endif
        goto LOS_ERREND;
    }
#else
    (VOID)buf;
    (VOID)size;
#endif

    ret = OsCreateDeferPackerTask();
    if (ret != LOS_OK) {
        TRACE_ERROR("trace init create defer packer task error :0x%x\n", ret);
#if (LOSCFG_TRACE_CONTROL_AGENT == 1)
        (VOID)LOS_TaskDelete(g_traceTaskId);
#endif
#if (LOSCFG_RECORDER_MODE_OFFLINE == 1)
        OsTraceBufDeInit();
#endif
        goto LOS_ERREND;
    }

    OsTraceHookInstall();
    OsTraceCnvInit();

    g_traceEventCount = 0;

#if (LOSCFG_RECORDER_MODE_ONLINE == 1)
    g_enableTrace = FALSE;
    g_traceState = TRACE_INITED;
#else
    g_enableTrace = TRUE;
    g_traceState = TRACE_STARTED;
#endif
    TRACE_UNLOCK(intSave);
    return LOS_OK;
LOS_ERREND:
    TRACE_UNLOCK(intSave);
    return ret;
}

STATIC UINT32 OsTraceInitEntry(VOID)
{
    return LOS_TraceInit(g_osTraceBuf, LOSCFG_TRACE_BUFFER_SIZE);
}

LOS_SYS_INIT(OsTraceInitEntry, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_2);

UINT32 LOS_TraceStart(VOID)
{
    UINT32 intSave;
    UINT32 ret = LOS_OK;

    TRACE_LOCK(intSave);
    if (g_traceState == TRACE_STARTED) {
        goto START_END;
    }

    if (g_traceState == TRACE_UNINIT) {
        TRACE_ERROR("trace not inited, be sure LOS_TraceInit excute success\n");
        ret = LOS_ERRNO_TRACE_ERROR_STATUS;
        goto START_END;
    }

    OsTraceNotifyStart();

    g_enableTrace = TRUE;
    g_traceState = TRACE_STARTED;

    TRACE_UNLOCK(intSave);
    LOS_TRACE(MEM_INFO_REQ, m_aucSysMem0);
    return ret;
START_END:
    TRACE_UNLOCK(intSave);
    return ret;
}

VOID LOS_TraceStop(VOID)
{
    UINT32 intSave;

    TRACE_LOCK(intSave);
    if (g_traceState != TRACE_STARTED) {
        goto STOP_END;
    }

    g_enableTrace = FALSE;
    g_traceState = TRACE_STOPED;
    OsTraceNotifyStop();
STOP_END:
    TRACE_UNLOCK(intSave);
    OsTraceFlush();
}

VOID LOS_TraceEventMaskSet(UINT32 mask)
{
    g_traceMask = mask & EVENT_MASK;
}

VOID LOS_TraceRecordDump(BOOL toClient)
{
    if (g_traceState != TRACE_STOPED) {
        TRACE_ERROR("trace dump must after trace stopped , the current state is : %d\n", g_traceState);
        return;
    }
    OsTraceFlush();
    OsTraceRecordDump(toClient);
}

OfflineHead *LOS_TraceRecordGet(VOID)
{
    return OsTraceRecordGet();
}

VOID LOS_TraceReset(VOID)
{
    UINT32 intSave;

    if (g_traceState == TRACE_UNINIT) {
        TRACE_ERROR("trace not inited, be sure LOS_TraceInit excute success\n");
        return;
    }

    OsTraceReset();

    TRACE_LOCK(intSave);
    g_liteWrite = 0;
    g_liteRead = 0;
    TRACE_UNLOCK(intSave);
}

VOID LOS_TraceHwiFilterHookReg(TRACE_HWI_FILTER_HOOK hook)
{
    UINT32 intSave;

    TRACE_LOCK(intSave);
    g_traceHwiFliterHook = hook;
    TRACE_UNLOCK(intSave);
}

#if (LOSCFG_SHELL == 1)
LITE_OS_SEC_TEXT_MINOR UINT32 OsShellCmdTraceSetMask(INT32 argc, const CHAR **argv)
{
    size_t mask;
    CHAR *endPtr = NULL;

    if (argc >= 2) { /* 2:Just as number of parameters */
        PRINTK("\nUsage: trace_mask or trace_mask ID\n");
        return OS_ERROR;
    }

    if (argc == 0) {
        mask = TRACE_DEFAULT_MASK;
    } else {
        mask = strtoul(argv[0], &endPtr, 0);
    }
    LOS_TraceEventMaskSet((UINT32)mask);
    return LOS_OK;
}

LITE_OS_SEC_TEXT_MINOR UINT32 OsShellCmdTraceDump(INT32 argc, const CHAR **argv)
{
    BOOL toClient;
    CHAR *endPtr = NULL;

    if (argc >= 2) { /* 2:Just as number of parameters */
        PRINTK("\nUsage: trace_dump or trace_dump [1/0]\n");
        return OS_ERROR;
    }

    if (argc == 0) {
        toClient = FALSE;
    } else {
        toClient = strtoul(argv[0], &endPtr, 0) != 0 ? TRUE : FALSE;
    }
    LOS_TraceRecordDump(toClient);
    return LOS_OK;
}
#endif

#endif /* LOSCFG_KERNEL_TRACE == 1 */
