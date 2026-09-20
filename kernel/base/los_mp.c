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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */


#ifdef LOSCFG_KERNEL_SMP

#include "los_mp_pri.h"
#include "los_task_pri.h"
#include "los_percpu_pri.h"
#include "los_sched_pri.h"
#include "los_swtmr.h"
#include "los_init.h"

VOID LOS_MpSchedule(UINT32 target)
{
    UINT32 cpuid = ArchCurrCpuid();

    (VOID)LOS_HwiSendIpi(LOS_MP_IPI_SCHEDULE, target & (~(CPUID_TO_AFFI_MASK(cpuid))));
}

VOID OsMpWakeHandler(VOID)
{
    /* generic wakeup ipi, do nothing */
}

VOID OsMpScheduleHandler(VOID)
{
    /*
     * set schedule flag to differ from wake function,
     * so that the scheduler can be triggered at the end of irq.
     */
    OsSetSchedFlag(INT_PEND_RESCH);
}

VOID OsMpHaltHandler(VOID)
{
    (VOID)LOS_IntLock();
    OsPercpuGet()->excFlag = CPU_HALT;

    while (1) {
    }
}

VOID OsMpCollectTasks(VOID)
{
    LosTaskCB *taskCB = NULL;
    UINT32 taskId = 0;
    UINT32 ret;

    /* recursive checking all the available task */
    for (; taskId < g_taskMaxNum; taskId++) {
        taskCB = &g_taskCBArray[taskId];

        if ((taskCB->taskStatus & OS_TASK_STATUS_ZOMBIE) ||
            (taskCB->taskStatus & OS_TASK_STATUS_RUNNING)) {
            continue;
        }

        /*
         * though task status is not atomic, this check may success but not accomplish
         * the deletion; this deletion will be handled until the next run.
         */
        if (taskCB->mpSignal & SIGNAL_TERMINATE) {
            ret = LOS_TaskDelete(taskId);
            if ((ret != LOS_OK) && (ret != LOS_ERRNO_TSK_NOT_CREATED)) {
                PRINT_WARN("GC collect task failed err:0x%x\n", ret);
            }
        }
    }
}

LITE_OS_SEC_TEXT_INIT_LEVEL_ONE UINT32 OsMpInit(VOID)
{
#ifdef LOSCFG_KERNEL_SMP_GC_SWTMR
    UINT32 swtmrId;
    UINT32 ret;

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    ret = LOS_SwtmrCreate(OS_MP_GC_PERIOD, LOS_SWTMR_MODE_PERIOD,
                          (SWTMR_PROC_FUNC)OsMpCollectTasks, &swtmrId, 0,
                          OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE);
#else
    ret = LOS_SwtmrCreate(OS_MP_GC_PERIOD, LOS_SWTMR_MODE_PERIOD,
                          (SWTMR_PROC_FUNC)OsMpCollectTasks, &swtmrId, 0);
#endif
    if (ret != LOS_OK) {
        PRINT_ERR("Swtmr Create failed err:0x%x\n", ret);
        return ret;
    }
    ret = LOS_SwtmrStart(swtmrId);
    if (ret != LOS_OK) {
        PRINT_ERR("Swtmr Start failed err:0x%x\n", ret);
        return ret;
    }
#endif
    return LOS_OK;
}
SYS_INITCALL(OsMpInit, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_4);

#endif /* LOSCFG_KERNEL_SMP */
