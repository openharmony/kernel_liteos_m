/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_arch_timer.h"
#include "los_task.h"
#include "los_task_pri.h"
#include "los_sched.h"
#include "los_sched_pri.h"
#include "los_memory.h"
#include "soc_common.h"

LITE_OS_SEC_BSS LosTaskCB *g_runTask = NULL;

LITE_OS_SEC_TEXT_INIT VOID ArchInit(VOID)
{
    HalHwiInit();
}

LITE_OS_SEC_TEXT_MINOR VOID ArchTaskExit(VOID)
{
    ArchIntLock();
    while (1) {
    }
}

LITE_OS_SEC_TEXT_INIT VOID *ArchTaskStackInit(UINT32 taskId, UINT32 stackSize, VOID *topStack)
{
    TaskContext *context = (TaskContext *)((UINTPTR)topStack + stackSize - sizeof(TaskContext));

    context->mstatus = RISCV_MSTATUS_MPP | RISCV_MSTATUS_MPIE;
#ifndef LOSCFG_ARCH_FPU_DISABLE
    /* Enable FPU (mstatus.FS = Dirty) so the task can use FP instructions.
     * The task switch / interrupt entry will save/restore FP registers
     * because FS != Off. */
    context->mstatus |= RISCV_MSTATUS_FS;
    /* Zero FPU register area (ft0-fs11 + fcsr + reserved) by looping from
     * the first FPU field to the end of TaskContext, 
     * which uses a similar loop from S11_REG_INDEX+1 to sizeof(TaskContext). */
    UINT32 fpuStart = (UINT32)(UINTPTR)&(((TaskContext *)0)->ft11) / sizeof(UINT32);
    UINT32 fpuEnd = (UINT32)sizeof(TaskContext) / sizeof(UINT32);
    UINT32 index;
    for (index = fpuStart; index < fpuEnd; index++) {
        ((UINT32 *)context)[index] = 0;
    }
#endif
    context->mepc = (UINT32)(UINTPTR)OsTaskEntry;
    context->tp = TP_INIT_VALUE;
    context->sp = SP_INIT_VALUE;
    context->s11 = S11_INIT_VALUE;
    context->s10 = S10_INIT_VALUE;
    context->s9 = S9_INIT_VALUE;
    context->s8 = S8_INIT_VALUE;
    context->s7 = S7_INIT_VALUE;
    context->s6 = S6_INIT_VALUE;
    context->s5 = S5_INIT_VALUE;
    context->s4 = S4_INIT_VALUE;
    context->s3 = S3_INIT_VALUE;
    context->s2 = S2_INIT_VALUE;
    context->s1 = S1_INIT_VALUE;
    context->s0 = FP_INIT_VALUE;
    context->t6 = T6_INIT_VALUE;
    context->t5 = T5_INIT_VALUE;
    context->t4 = T4_INIT_VALUE;
    context->t3 = T3_INIT_VALUE;
    context->a7 = A7_INIT_VALUE;
    context->a6 = A6_INIT_VALUE;
    context->a5 = A5_INIT_VALUE;
    context->a4 = A4_INIT_VALUE;
    context->a3 = A3_INIT_VALUE;
    context->a2 = A2_INIT_VALUE;
    context->a1 = A1_INIT_VALUE;
    context->a0 = taskId;
    context->t2 = T2_INIT_VALUE;
    context->t1 = T1_INIT_VALUE;
    context->t0 = T0_INIT_VALUE;
    context->ra = (UINT32)(UINTPTR)ArchTaskExit;
    return (VOID *)context;
}

LITE_OS_SEC_TEXT VOID wfi(VOID)
{
    __asm__ __volatile__("wfi");
}

LITE_OS_SEC_TEXT VOID mb(VOID)
{
    __asm__ __volatile__("fence":::"memory");
}

LITE_OS_SEC_TEXT VOID dsb(VOID)
{
    __asm__ __volatile__("fence":::"memory");
}
