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
               topStack    --- Top of task's stack
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

#if ((defined(__FPU_PRESENT) && (__FPU_PRESENT == 1U)) && \
     (defined(__FPU_USED) && (__FPU_USED == 1U)))
    context->S16 = 0xAA000010;
    context->S17 = 0xAA000011;
    context->S18 = 0xAA000012;
    context->S19 = 0xAA000013;
    context->S20 = 0xAA000014;
    context->S21 = 0xAA000015;
    context->S22 = 0xAA000016;
    context->S23 = 0xAA000017;
    context->S24 = 0xAA000018;
    context->S25 = 0xAA000019;
    context->S26 = 0xAA00001A;
    context->S27 = 0xAA00001B;
    context->S28 = 0xAA00001C;
    context->S29 = 0xAA00001D;
    context->S30 = 0xAA00001E;
    context->S31 = 0xAA00001F;
    context->S0 = 0xAA000000;
    context->S1 = 0xAA000001;
    context->S2 = 0xAA000002;
    context->S3 = 0xAA000003;
    context->S4 = 0xAA000004;
    context->S5 = 0xAA000005;
    context->S6 = 0xAA000006;
    context->S7 = 0xAA000007;
    context->S8 = 0xAA000008;
    context->S9 = 0xAA000009;
    context->S10 = 0xAA00000A;
    context->S11 = 0xAA00000B;
    context->S12 = 0xAA00000C;
    context->S13 = 0xAA00000D;
    context->S14 = 0xAA00000E;
    context->S15 = 0xAA00000F;
    context->FPSCR = 0x00000000;
    context->NO_NAME = 0xAA000011;
#endif

    context->secureContext = 0UL;
    context->stackLmit = (UINT32)topStack;
    context->excReturn = 0xFFFFFFBC;

    context->uwR4 = 0x04040404L;
    context->uwR5 = 0x05050505L;
    context->uwR6 = 0x06060606L;
    context->uwR7 = 0x07070707L;
    context->uwR8 = 0x08080808L;
    context->uwR9 = 0x09090909L;
    context->uwR10 = 0x10101010L;
    context->uwR11 = 0x11111111L;
    context->uwPriMask = 0;
    context->uwR0 = taskID;
    context->uwR1 = 0x01010101L;
    context->uwR2 = 0x02020202L;
    context->uwR3 = 0x03030303L;
    context->uwR12 = 0x12121212L;
    context->uwLR = (UINT32)(UINTPTR)HalSysExit;
    context->uwPC = (UINT32)(UINTPTR)OsTaskEntry;
    context->uwxPSR = 0x01000000L;

    return (VOID *)context;
}

LITE_OS_SEC_TEXT_INIT UINT32 HalStartSchedule(VOID)
{
    (VOID)LOS_IntLock();
    OsSchedStart();
    HalStartToRun();
    return LOS_OK; /* never return */
}
