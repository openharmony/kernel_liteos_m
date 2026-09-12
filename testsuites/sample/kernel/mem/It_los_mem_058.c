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

#include "osTest.h"
#include "It_los_mem.h"


#if (LOSCFG_KERNEL_MEM_TLSF == 1) && (LOSCFG_MEM_FREE_BY_TASKID == 1)

/*
 * testcasename: TLSF 按任务 ID 批量释放内存（LOS_MemFreeByTaskID）功能与边界验证
 * testcasecode: ItLosMem058
 * level:        LEVEL1
 * type:         FUNCTION
 *
 * precond（预置条件）
 *   1. g_testCount = 0
 *   2. LOSCFG_KERNEL_MEM_TLSF == 1 && LOSCFG_MEM_FREE_BY_TASKID == 1（仅 TLSF+按任务释放下编译/注册）
 *   3. MemStart() 已从系统堆申请 g_memPool，MemInit() 完成初始化
 *   4. g_memPool 为全新池，无当前任务的既有分配
 *
 * step（测试步骤）
 *   1. MemStart() 与 MemInit() 初始化 g_memPool
 *   2. LOS_CurTaskIDGet() 取当前任务 ID
 *   3. LOS_MemAlloc(g_memPool, 0x100) 与 LOS_MemAlloc(g_memPool, 0x100)（记录当前 taskId）
 *   4. LOS_MemTotalUsedGet(g_memPool) 取基线
 *   5. LOS_MemFreeByTaskID(g_memPool, taskId)
 *   6. LOS_MemTotalUsedGet(g_memPool) 取释放后用量
 *   7. LOS_MemFreeByTaskID(g_memPool, LOSCFG_BASE_CORE_TSK_LIMIT)（边界：非法 taskId）
 *   8. LOS_MemFreeByTaskID(NULL, taskId)（边界：NULL pool）
 *
 * exp（预期结果）
 *   1. g_memPool 初始化成功
 *   2. taskId 为有效值（> 0）
 *   3. 两次 alloc 返回 != NULL
 *   4. usedBefore > 0
 *   5. 返回 LOS_OK
 *   6. usedAfter < usedBefore
 *   7. 返回 LOS_NOK
 *   8. 返回 LOS_NOK
 *
 * rstenv（环境清理）
 *   1. EXIT 标签执行 MemFree() 反初始化 g_memPool
 *   2. MemEnd() 将 g_memPool 释放回系统堆
 */

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    UINT32 taskId;
    void *p1 = NULL;
    void *p2 = NULL;
    UINT32 usedBefore;
    UINT32 usedAfter;

    MemStart();
    MemInit();

    /* Get the current task ID — allocations record this ID in node->taskId */
    taskId = LOS_CurTaskIDGet();

    /* Alloc two blocks — they record this task's ID */
    p1 = LOS_MemAlloc(g_memPool, 0x100);
    ICUNIT_GOTO_NOT_EQUAL(p1, NULL, 0, EXIT);
    p2 = LOS_MemAlloc(g_memPool, 0x100);
    ICUNIT_GOTO_NOT_EQUAL(p2, NULL, 0, EXIT);

    usedBefore = LOS_MemTotalUsedGet(g_memPool);

    /* Free all memory allocated by this task */
    ret = LOS_MemFreeByTaskID(g_memPool, taskId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* Used size should have decreased after freeing the task's allocations */
    usedAfter = LOS_MemTotalUsedGet(g_memPool);
    ICUNIT_GOTO_EQUAL((usedAfter < usedBefore), 1, 0, EXIT);

    /* Boundary: invalid taskId (>= TSK_LIMIT) should be rejected */
    ret = LOS_MemFreeByTaskID(g_memPool, LOSCFG_BASE_CORE_TSK_LIMIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_NOK, ret, EXIT);

    /* Boundary: NULL pool should be rejected */
    ret = LOS_MemFreeByTaskID(NULL, taskId);
    ICUNIT_GOTO_EQUAL(ret, LOS_NOK, ret, EXIT);

EXIT:
    MemFree();
    MemEnd();
    return LOS_OK;
}

#endif /* LOSCFG_KERNEL_MEM_TLSF && LOSCFG_MEM_FREE_BY_TASKID */

VOID ItLosMem058(void)
{
#if (LOSCFG_KERNEL_MEM_TLSF == 1) && (LOSCFG_MEM_FREE_BY_TASKID == 1)
    TEST_ADD_CASE("ItLosMem058", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
