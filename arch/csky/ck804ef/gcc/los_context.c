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
#include <stdio.h>
#include <assert.h>
#include "securec.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_interrupt.h"
#include "los_debug.h"
#include "los_context.h"

STATIC volatile UINT32 g_sysNeedSched = FALSE;
#define  ATTRIBUTE_ISR __attribute__((isr))

/* ****************************************************************************
 Function    : ArchInit
 Description : arch init function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID ArchInit(VOID)
{
}

/* ****************************************************************************
 Function    : ArchSysExit
 Description : Task exit function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_MINOR VOID ArchSysExit(VOID)
{
    printf("\nArchSysExit\n");
    while (1) {
    }
}

/* ****************************************************************************
 Function    : ArchTskStackRegInit
 Description : Task exit function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
VOID ArchTskStackRegInit(TaskContext *context)
{
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
    context->R15 = 0xfffffffeL;
    context->R16 = 0x16161616L;
    context->R17 = 0x17171717L;
    context->R18 = 0x18181818L;
    context->R19 = 0x19191919L;
    context->R20 = 0x20202020L;
    context->R21 = 0x21212121L;
    context->R22 = 0x22222222L;
    context->R23 = 0x23232323L;
    context->R24 = 0x24242424L;
    context->R25 = 0x25252525L;
    context->R26 = 0x26262626L;
    context->R27 = 0x27272727L;
    context->R28 = 0x28282828L;
    context->R29 = 0x29292929L;
    context->R30 = 0x30303030L;
    context->R31 = 0x31313131L;
    context->VR0 = 0x12345678L;
    context->VR1 = 0x12345678L;
    context->VR2 = 0x12345678L;
    context->VR3 = 0x12345678L;
    context->VR4 = 0x12345678L;
    context->VR5 = 0x12345678L;
    context->VR6 = 0x12345678L;
    context->VR7 = 0x12345678L;
    context->VR8 = 0x12345678L;
    context->VR9 = 0x12345678L;
    context->VR10 = 0x12345678L;
    context->VR11 = 0x12345678L;
    context->VR12 = 0x12345678L;
    context->VR13 = 0x12345678L;
    context->VR14 = 0x12345678L;
    context->VR15 = 0x12345678L;
    context->EPSR = 0x80000340L;
}

/* ****************************************************************************
 Function    : ArchTskStackInit
 Description : Task stack initialization function
 Input       : taskID     --- TaskID
               stackSize  --- Total size of the stack
               topStack   --- Top of task's stack
 Output      : None
 Return      : Context pointer
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID *ArchTskStackInit(UINT32 taskID, UINT32 stackSize, VOID *topStack)
{
    TaskContext *context = NULL;
    context = (TaskContext *)(((UINTPTR)topStack + stackSize) - sizeof(TaskContext));
    context->R0  = taskID;
    ArchTskStackRegInit(context);
    context->EPC = (UINT32)OsTaskEntry;
    return (VOID *)context;
}

LITE_OS_SEC_TEXT_INIT UINT32 ArchStartSchedule(VOID)
{
    (VOID)LOS_IntLock();
    OsSchedStart();
    HalStartToRun();
    return LOS_OK; /* never return */
}


VOID HalIrqEndCheckNeedSched(VOID)
{
}
volatile int g_sysSchedCount = 0;
int *p_sysSchedCount = &g_sysSchedCount;
VOID ArchTaskSchedule(VOID)
{
    HalTaskContextSwitch();
}

