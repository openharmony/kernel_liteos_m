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
#include "los_arch_regs.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_interrupt.h"
#include "los_arch_timer.h"
#include "los_timer.h"
#include "los_debug.h"

STATIC UINT32 g_sysNeedSched = FALSE;

UINT32 g_stackDefault[] = {
    0x00000000,     /* REG_OFF_PC */
    0x00000000,     /* REG_OFF_PS */
    0x00000A00,     /* REG_OFF_AR00 */
    0x00000A01,     /* REG_OFF_AR01 */
    0x00000A02,     /* REG_OFF_AR02 */
    0x00000A03,     /* REG_OFF_AR03 */
    0x00000A04,     /* REG_OFF_AR04 */
    0x00000A05,     /* REG_OFF_AR05 */
    0x00000A06,     /* REG_OFF_AR06 */
    0x00000A07,     /* REG_OFF_AR07 */
    0x00000A08,     /* REG_OFF_AR08 */
    0x00000A09,     /* REG_OFF_AR09 */
    0x00000A10,     /* REG_OFF_AR10 */
    0x00000A11,     /* REG_OFF_AR11 */
    0x00000A12,     /* REG_OFF_AR12 */
    0x00000A13,     /* REG_OFF_AR13 */
    0x00000A14,     /* REG_OFF_AR14 */
    0x00000A15,     /* REG_OFF_AR15 */
    0x00000000,     /* REG_OFF_RESERVED */
    0x00000000,     /* REG_OFF_EXCCAUSE */
    0x00000000,     /* REG_OFF_EXCVASSR */
    0x00000000,     /* REG_OFF_LCOUNT */
    0x00000000,     /* REG_OFF_LEND */
    0x00000000,     /* REG_OFF_LBEG */
#if (defined (__FPU_PRESENT) && (__FPU_PRESENT == 1U))
    0x00000000,     /* REG_OFF_TMP0 */
    0x00000001,     /* REG_OFF_CPENABLE | CONTEXT_OFF_CPSTORED */
    0x00000000,     /* REG_OFF_FCR */
    0x00000000,     /* REG_OFF_FSR */
    0x00000000,     /* REG_OFF_F0 */
    0x00000000,     /* REG_OFF_F1 */
    0x00000000,     /* REG_OFF_F2 */
    0x00000000,     /* REG_OFF_F3 */
    0x00000000,     /* REG_OFF_F4 */
    0x00000000,     /* REG_OFF_F5 */
    0x00000000,     /* REG_OFF_F6 */
    0x00000000,     /* REG_OFF_F7 */
    0x00000000,     /* REG_OFF_F8 */
    0x00000000,     /* REG_OFF_F9 */
    0x00000000,     /* REG_OFF_F10 */
    0x00000000,     /* REG_OFF_F11 */
    0x00000000,     /* REG_OFF_F12 */
    0x00000000,     /* REG_OFF_F13 */
    0x00000000,     /* REG_OFF_F14 */
    0x00000000,     /* REG_OFF_F15 */
#endif
    0x00000000,     /* REG_OFF_SPILL_RESERVED */
    0x00000000,     /* REG_OFF_SPILL_RESERVED */
    0x00000000,     /* REG_OFF_SPILL_RESERVED */
    0x00000000,     /* REG_OFF_SPILL_RESERVED */
};

LITE_OS_SEC_TEXT_INIT VOID HalArchInit(VOID)
{
    HalHwiInit();
}

LITE_OS_SEC_TEXT_MINOR VOID HalSysExit(VOID)
{
    LOS_IntLock();
    while (1) {
    }
}

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

    context = (TaskContext *)((((UINTPTR)topStack + stackSize) - sizeof(TaskContext)));

    /* initialize the task context */
    result = memcpy_s(context, sizeof(TaskContext), g_stackDefault, sizeof(TaskContext));
    if (result != EOK) {
        PRINT_ERR("[%s] memcpy_s failed!\n", __func__);
    }

    context->pc       = (UINT32)(UINTPTR)OsTaskEntry;
    context->regA[INDEX_OF_SP]  = (UINTPTR)topStack + stackSize; /* endStack */
    context->regA[INDEX_OF_ARGS0]  = taskID; /* argument1 */
    context->ps       = SPREG_PS_STACK_CROSS | WOE_ENABLE | 1 << BIT_CALLINC; /* set to kernel stack */

    return (VOID *)context;
}

VOID HalStartToRun(VOID)
{
    __asm__ volatile ("call0 OsStartToRun");
}

LITE_OS_SEC_TEXT_INIT UINT32 HalStartSchedule(VOID)
{
    UINT32 ret;

    (VOID)LOS_IntLock();

    ret = HalTickStart(OsTickHandler);
    if (ret != LOS_OK) {
        PRINT_ERR("Tick start failed!\n");
    }

    OsSchedStart();
    HalStartToRun();
    return LOS_OK;
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

VOID HalIrqEndCheckNeedSched(VOID)
{
    if (g_sysNeedSched && g_taskScheduled && LOS_CHECK_SCHEDULE) {
        HalTaskSchedule();
    }
}
