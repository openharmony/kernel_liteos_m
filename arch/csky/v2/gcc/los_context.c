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

#include "los_context.h"
#include "securec.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_interrupt.h"
#include "los_arch_timer.h"
#include "los_debug.h"

STATIC UINT32 g_sysNeedSched = FALSE;

/* ****************************************************************************
 Function    : HalArchInit
 Description : arch init function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID HalArchInit(VOID)
{
    UINT32 ret;
    HalHwiInit();

    ret = HalTickStart(OsTickHandler);
    if (ret != LOS_OK) {
        PRINT_ERR("Tick start failed!\n");
    }
}

/* ****************************************************************************
 Function    : HalSysExit
 Description : Task exit function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_MINOR VOID HalSysExit(VOID)
{
    LOS_IntLock();
    while (1) {
    }
}

/* ****************************************************************************
 Function    : HalTskStackInit
 Description : Task stack initialization function
 Input       : taskID     --- TaskID
               stackSize  --- Total size of the stack
               topStack   --- Top of task's stack
 Output      : None
 Return      : Context pointer
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID *HalTskStackInit(UINT32 taskID, UINT32 stackSize, VOID *topStack)
{
    TaskContext *context = NULL;
    errno_t result;

    /* initialize the task stack, write magic num to stack top */
    result = memset_s(topStack, stackSize, (INT32)(OS_TASK_STACK_INIT & 0xFF), stackSize);
    if (result != EOK) {
        printf("memset_s is failed:%s[%d]\r\n", __FUNCTION__, __LINE__);
    }
    *((UINT32 *)(topStack)) = OS_TASK_MAGIC_WORD;

    context = (TaskContext *)(((UINTPTR)topStack + stackSize) - sizeof(TaskContext));

    context->R0  = taskID;
    context->R1  = 0x01010101L;
    context->R2  = 0x02020202L;
    context->R3  = 0x03030303L;
    context->R4  = 0x04040404L;
    context->R5  = 0x05050505L;
    context->R6  = 0x06060606L;
    context->R7  = 0x07070707L;
    context->R8  = 0x08080808L;
    context->R9  = 0x09090909L;
    context->R10 = 0x10101010L;
    context->R11 = 0x11111111L;
    context->R12 = 0x12121212L;
    context->R13 = 0x13131313L;
    context->R15 = (UINT32)HalSysExit;
    context->EPSR = 0xe0000144L;
    context->EPC = (UINT32)OsTaskEntry;
    return (VOID *)context;
}

LITE_OS_SEC_TEXT_INIT UINT32 HalStartSchedule(VOID)
{
    (VOID)LOS_IntLock();
    OsSchedStart();
    HalStartToRun();
    return LOS_OK; /* never return */
}

VOID HalIrqEndCheckNeedSched(VOID)
{
    if (g_sysNeedSched && g_taskScheduled && LOS_CHECK_SCHEDULE) {
        HalTaskSchedule();
    }
}

VOID HalTaskSchedule(VOID)
{
    UINT32 intSave;

    if (OS_INT_ACTIVE) {
        g_sysNeedSched = TRUE;
        return;
    }

    intSave = LOS_IntLock();
    g_sysNeedSched = FALSE;
    BOOL isSwitch = OsSchedTaskSwitch();
    if (isSwitch) {
        HalTaskContextSwitch();
    }
    LOS_IntRestore(intSave);
    return;
}
