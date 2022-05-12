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
typedef struct {
    UINT32 waterLine;
    UINT32 taskSPTop;
    UINT32 taskSPLimit;
    UINTPTR taskSP;
} DumpInfo;

STATIC VOID ShowFormat(UINTPTR *buf, DumpInfo *info)
{
    INT32 i;

    UINT32 len = info->waterLine / sizeof(UINTPTR);
    UINTPTR addr = (info->taskSPLimit - info->waterLine);

    for (i = 0; i < len; i++) {
        if ((i % PRINT_PER_ROW) == 0) {
            PRINTK("\r\n 0x%08x: ", addr);
        }
        if (addr == info->taskSP) {
            PRINTK(" *%08x", buf[i]);
        } else {
            PRINTK("  %08x", buf[i]);
        }
        addr += sizeof(UINTPTR);
    }

    PRINTK("\r\n");
}

STATIC INT32 DumpTaskInfo(UINT32 taskID, UINTPTR *buf, DumpInfo *info)
{
    errno_t ret;
    LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);

    if (taskID == LOS_CurTaskIDGet()) {
        info->taskSP = ArchSpGet();
    } else {
        info->taskSP = (UINTPTR)taskCB->stackPointer;
    }

    info->taskSPTop = taskCB->topOfStack;
    info->taskSPLimit = taskCB->topOfStack + taskCB->stackSize;
    if ((info->taskSP > info->taskSPLimit) || (info->taskSP < info->taskSPTop)) {
        return LOS_NOK;
    }

    ret = memcpy_s(buf, info->waterLine, (const VOID *)(info->taskSPLimit - info->waterLine), info->waterLine);
    if (ret != EOK) {
        return LOS_NOK;
    }

    return LOS_OK;
}

VOID LOS_TaskStackDump(UINT32 taskID)
{
    UINTPTR *buf = NULL;
    DumpInfo info;
    UINT32 intSave;
    INT32 ret;

    if (taskID > g_taskMaxNum) {
        PRINT_ERR("error taskID %u\r\n", taskID);
        return;
    }

    if (OS_INT_ACTIVE) {
        PRINT_ERR("called during an interrupt.\r\n");
        return;
    }

    intSave = LOS_IntLock();
    info.waterLine = OsGetTaskWaterLine(taskID);
    if (info.waterLine == OS_NULL_INT) {
        LOS_IntRestore(intSave);
        return;
    }

    buf = (UINTPTR *)LOS_MemAlloc(OS_SYS_MEM_ADDR, info.waterLine);
    if (buf == NULL) {
        LOS_IntRestore(intSave);
        PRINT_ERR("alloc failed for dump\n");
        return;
    }
    (VOID)memset_s(buf, info.waterLine, 0, info.waterLine);

    ret = DumpTaskInfo(taskID, buf, &info);
    if (ret != LOS_OK) {
        LOS_IntRestore(intSave);
        (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, buf);
        PRINT_ERR("SP 0x%x may error or memcpy_s failed, stack space from 0x%x to 0x%x\r\n", \
                  info.taskSP, info.taskSPTop, info.taskSPLimit);
        return;
    }

    LOS_IntRestore(intSave);
    PRINTK("Task %u, SP 0x%x, WaterLine 0x%x", taskID, info.taskSP, info.waterLine);
    ShowFormat(buf, &info);
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, buf);

    return;
}

UINT32 OsShellCmdStackDump(INT32 argc, const CHAR **argv)
{
    UINT32 taskID;

    if (argc != 1) {
        PRINT_ERR("\nUsage: stack taskID\n");
        return LOS_NOK;
    }

    taskID = (UINT32)atoi(argv[0]);

    LOS_TaskStackDump(taskID);
    return LOS_OK;
}
#endif /* LOSCFG_STACK_DUMP == 1 */
