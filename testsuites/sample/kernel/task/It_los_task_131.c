/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

#include "osTest.h"
#include "It_los_task.h"

/* 用例简要描述: 任务模块低频公共接口一揽子：空闲钩子注册/注销、单核 CPU 亲和性
 * Set/Get 桩、非 TrustZone 安全上下文分配桩、新建任务 ID 双查询口径一致性、
 * 任务切换信息读取（正常索引/索引回绕/空指针负路径）。只调对外接口。 */

static volatile UINT32 g_idleCount131;

static VOID IdleHook131(VOID)
{
    g_idleCount131++;
}

static UINT32 Testcase(VOID)
{
    UINT32 ret;
#ifndef LOSCFG_KERNEL_SMP
    UINT16 affi;
#endif
    UINT32 newId;
    UINT32 nextId;
    UINT32 curTaskId;
    UINT32 switchInfo[16] = {0}; // 16 words: 1 pid + LOS_TASK_NAMELEN name.

    curTaskId = LOS_CurTaskIDGet();

    /* F-Pos: 空闲钩子注册 → idle 任务执行 → 计数增长 → 注销恢复。 */
    g_idleCount131 = 0;
    LOS_IdleHandlerHookReg(IdleHook131);
    (VOID)LOS_TaskDelay(2); // 2, let the idle task run.
    ret = (g_idleCount131 > 0) ? LOS_OK : LOS_NOK;
    LOS_IdleHandlerHookReg(NULL); // restore before any assert below.
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

#ifndef LOSCFG_KERNEL_SMP
    /* F-Pos: 单核（UP）构建下亲和性接口为桩实现（Set 恒 OK，Get 恒 1）。
     * SMP 构建下为真实实现，语义不同，本段用 SMP 宏隔离仅测 UP 桩。 */
    ret = LOS_TaskCpuAffiSet(curTaskId, 1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    affi = LOS_TaskCpuAffiGet(curTaskId);
    ICUNIT_ASSERT_EQUAL(affi, 1, affi); // 1: single-cpu affinity mask.
#endif

    /* F-Pos: 非 TrustZone 构建下安全上下文分配为桩实现（恒 OK）。 */
    ret = LOS_TaskAllocSecureContext(curTaskId, 128); // 128, dummy size for the stub.
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    /* F-Pos: 新建任务 ID 两种查询口径一致。 */
    newId = LOS_NewTaskIDGet();
    nextId = LOS_NextTaskIDGet();
    ICUNIT_ASSERT_EQUAL(newId, nextId, newId);

#if (LOSCFG_BASE_CORE_EXC_TSK_SWITCH == 1)
    /* F-Pos: 任务切换信息读取（正常索引）。 */
    ret = LOS_TaskSwitchInfoGet(0, switchInfo);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    /* F-Inv: 索引越界 → 内部取模回绕到合法槽位，仍正常返回。 */
    ret = LOS_TaskSwitchInfoGet(OS_TASK_SWITCH_INFO_COUNT, switchInfo);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    /* F-Inv: 输出指针为空 → LOS_ERRNO_TSK_PTR_NULL。 */
    ret = LOS_TaskSwitchInfoGet(0, NULL);
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_TSK_PTR_NULL, ret);
#endif

    return LOS_OK;
}

VOID ItLosTask131(VOID) // IT_Layer_ModuleORFeature_No
{
    TEST_ADD_CASE("ItLosTask131", Testcase, TEST_LOS, TEST_TASK, TEST_LEVEL1, TEST_FUNCTION);
}
