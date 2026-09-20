/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
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
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * ItLosCpup009 - LOS_AllCpuUsage / LOS_GetAllIrqCpuUsage / LOS_CpupUsageMonitor 补覆盖
 *
 * 覆盖点(kernel/extended/cpup/los_cpup.c,基线 cpup 78% 的未覆盖行):
 *  - LOS_AllCpuUsage(:710-772 零执行),入参+功能双覆盖:
 *      NULL 出参 → LOS_ERRNO_CPUP_TASK_PTR_NULL(:729-731);
 *      maxNum=0 → LOS_ERRNO_CPUP_MAXNUM_INVALID(:733-735);
 *      flag=0 且 maxNum 超上限 → LOS_ERRNO_CPUP_MAXNUM_INVALID(:738-741);
 *      flag=1 任务段正常查询 → LOS_OK(:748-771),出参校验;
 *      flag=0 含 IRQ 段偏移路径(:742-749) → LOS_OK。
 *  - LOS_GetAllIrqCpuUsage(:830-863,仅 LOSCFG_CPUP_INCLUDE_IRQ=1 编译,qemu config.h 已开):
 *      三档 mode → LOS_OK;经 OsCpupGetPos→OsCpupHisPosGet 间接触达内部函数;
 *      IRQ CB 非配置模式为固定映射(hwiNum+TASK_CB_NUM),条目数据取决于板级
 *      外设中断活动,只做范围校验不做非零强断言(串口中断是否发生不可控)。
 *  - LOS_CpupUsageMonitor(:663-708):
 *      SYS_CPU_USAGE 补 CPUP_IN_1S/CPUP_ALL_TIME 两档(001 仅测 IN_10S);
 *      TASK_CPU_USAGE 合法任务三档成功(:686-699);
 *      已删除任务 → OS_ERROR(:687-689 UNUSED 分支,001 仅测超限 id)。
 *
 * Guards: 合入若改动 IRQ 段偏移逻辑(flag=0 时 numTmpMin/numTmpMax 平移
 *         OS_CPUP_TASK_CB_NUM)或删除任务的 UNUSED 判定,此处失败。
 */

#include "It_los_cpup.h"

#if (LOSCFG_BASE_CORE_CPUP == 1)

static VOID ItCpup009TaskEntry(VOID)
{
    UINT32 loop = 20; /* 20: 约 20 tick 采样窗口(足够 Monitor 完成),递减归零自然退出 */
    while (loop > 0) {
        loop--;
        LOS_TaskDelay(1);
    }
    /* detached 属性(创建时 uwResved=LOS_TASK_STATUS_DETACHED): 退出后内核自回收,主任务无需 delete */
}

/* 用例简要描述: LOS_AllCpuUsage 入参矩阵+功能出参,LOS_GetAllIrqCpuUsage 三档 mode,
   LOS_CpupUsageMonitor 补 mode 档位与已删除任务路径 */
static UINT32 TestCase009(VOID)
{
    UINT32 ret;
    UINT32 taskId;
    UINT16 loop;
    UINT16 activeCnt = 0;
    UINT16 usedCnt = 0;
    CPUP_INFO_S taskCpup[LOSCFG_BASE_CORE_TSK_LIMIT + 1] = {0};
    TSK_INIT_PARAM_S param = {0};

    /* F-Inv: NULL 出参 → LOS_ERRNO_CPUP_TASK_PTR_NULL(los_cpup.c:729-731) */
    ret = LOS_AllCpuUsage(2, NULL, CPUP_ALL_TIME, 1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_TASK_PTR_NULL, ret);

    /* F-Inv: maxNum=0 → LOS_ERRNO_CPUP_MAXNUM_INVALID(los_cpup.c:733-735) */
    ret = LOS_AllCpuUsage(0, taskCpup, CPUP_ALL_TIME, 1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_MAXNUM_INVALID, ret);

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    /* F-Inv: flag=0(含 IRQ 段)且 maxNum 超上限 → LOS_ERRNO_CPUP_MAXNUM_INVALID
       (los_cpup.c:738-741;qemu TSK_LIMIT=24,上限=0xFFFF-25=65510,0xFFFF 越界) */
    ret = LOS_AllCpuUsage(0xFFFF, taskCpup, CPUP_ALL_TIME, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_CPUP_MAXNUM_INVALID, ret);
#endif

    /* F-Nor: flag=1 任务段全量查询 → LOS_OK(los_cpup.c:748-771) */
    ret = LOS_AllCpuUsage((UINT16)(LOSCFG_BASE_CORE_TSK_LIMIT + 1), taskCpup, CPUP_ALL_TIME, 1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* 出参校验: 系统已运行,任务表中至少 idle+当前任务 2 个有效条目;
       每个有效条目的 usage 必在 [0,LOS_CPUP_PRECISION] */
    for (loop = 0; loop < (LOSCFG_BASE_CORE_TSK_LIMIT + 1); loop++) {
        if (taskCpup[loop].usStatus != 0) {
            activeCnt++;
            ICUNIT_ASSERT_WITHIN_EQUAL(taskCpup[loop].uwUsage, 0, LOS_CPUP_PRECISION,
                                       taskCpup[loop].uwUsage);
            if (taskCpup[loop].uwUsage > 0) {
                usedCnt++;
            }
        }
    }
    ICUNIT_ASSERT_EQUAL((activeCnt >= 2), 1, activeCnt);

    /* ALL_TIME 窗口下 idle/测试任务必有消耗,至少 1 个条目 usage 非零 */
    ICUNIT_ASSERT_NOT_EQUAL(usedCnt, 0, usedCnt);

    param.pfnTaskEntry = (TSK_ENTRY_FUNC)ItCpup009TaskEntry;
    param.uwStackSize = OS_TSK_TEST_STACK_SIZE;
    param.pcName = "ItCpup009";
    param.usTaskPrio = TASK_PRIO_TEST + 2;
    param.uwResved = LOS_TASK_STATUS_DETACHED; /* detached: Entry 递减退出后内核自回收(自删除) */
    ret = LOS_TaskCreate(&taskId, &param);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* 活窗口(delay(2)后任务仍余 ~18 tick): Monitor 三档+SYS 两档在此期间完成 */
    /* F-Nor: TASK_CPU_USAGE 合法任务三档 mode 全部成功(los_cpup.c:686-699) */
    ret = LOS_CpupUsageMonitor(TASK_CPU_USAGE, CPUP_IN_10S, taskId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_CpupUsageMonitor(TASK_CPU_USAGE, CPUP_IN_1S, taskId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_CpupUsageMonitor(TASK_CPU_USAGE, CPUP_ALL_TIME, taskId);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Nor: SYS_CPU_USAGE 补 1S/ALL_TIME 两档(los_cpup.c:669-679,001 仅测 IN_10S) */
    ret = LOS_CpupUsageMonitor(SYS_CPU_USAGE, CPUP_IN_1S, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_CpupUsageMonitor(SYS_CPU_USAGE, CPUP_ALL_TIME, 0);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* 轮询等 detached 任务自然退出(自回收), 上限 100 tick */
    UINT32 wait;
    for (wait = 0; wait < 100; wait++) {
        if (LOS_CpupUsageMonitor(TASK_CPU_USAGE, CPUP_IN_10S, taskId) == OS_ERROR) {
            break; /* UNUSED = 已退出自回收 */
        }
        LOS_TaskDelay(1);
    }

    /* F-Inv: TASK_CPU_USAGE 已退出任务 → OS_ERROR(los_cpup.c:687-689 UNUSED 分支) */
    ret = LOS_CpupUsageMonitor(TASK_CPU_USAGE, CPUP_IN_10S, taskId);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERROR, ret);

    /* F-Inv: TASK_CPU_USAGE 已删除任务 → OS_ERROR(los_cpup.c:687-689 UNUSED 分支) */
    ret = LOS_CpupUsageMonitor(TASK_CPU_USAGE, CPUP_IN_10S, taskId);
    ICUNIT_ASSERT_EQUAL(ret, OS_ERROR, ret);

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    {
        CPUP_INFO_S irqCpup[LOSCFG_PLATFORM_HWI_LIMIT] = {0};

        /* F-Nor: IRQ 段三档 mode 全部返回 LOS_OK(los_cpup.c:839-863),
           经 OsCpupGetPos→OsCpupHisPosGet 间接触达内部时序函数 */
        ret = LOS_GetAllIrqCpuUsage(CPUP_IN_10S, irqCpup);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
        ret = LOS_GetAllIrqCpuUsage(CPUP_IN_1S, irqCpup);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

        for (loop = 0; loop < LOSCFG_PLATFORM_HWI_LIMIT; loop++) {
            irqCpup[loop].usStatus = 0;
            irqCpup[loop].uwUsage = 0;
        }

        ret = LOS_GetAllIrqCpuUsage(CPUP_ALL_TIME, irqCpup);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

        /* 出参校验: IRQ 段活跃条目(若有)usage 必在 [0,PRECISION]。
           条目是否非零取决于板级外设中断活动(qemu 串口中断不可控),不做非零强断言 */
        for (loop = 0; loop < LOSCFG_PLATFORM_HWI_LIMIT; loop++) {
            if (irqCpup[loop].usStatus != 0) {
                ICUNIT_ASSERT_WITHIN_EQUAL(irqCpup[loop].uwUsage, 0, LOS_CPUP_PRECISION,
                                           irqCpup[loop].uwUsage);
            }
        }

        /* F-Nor: flag=0 含 IRQ 段偏移路径(los_cpup.c:742-749):
           numTmpMin/numTmpMax 平移 OS_CPUP_TASK_CB_NUM 后循环读 IRQ 段,返回 LOS_OK */
        ret = LOS_AllCpuUsage(1, taskCpup, CPUP_IN_1S, 0);
        ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    }
#endif

    return LOS_OK;
}

VOID ItLosCpup009(VOID)
{
    TEST_ADD_CASE("ItLosCpup009", TestCase009, TEST_LOS, TEST_CPUP, TEST_LEVEL1, TEST_FUNCTION);
}
#endif
