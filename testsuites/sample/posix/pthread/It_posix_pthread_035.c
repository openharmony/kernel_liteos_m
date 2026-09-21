/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
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
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
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

#include "It_posix_pthread.h"
#include <errno.h>

#define SCHED035_POLL_LIMIT 100 /* 子任务等待上限(tick) */
#define SCHED035_PRIO_LOWEST 31 /* OS_TASK_PRIORITY_LOWEST(los_task.h), setschedparam 拒绝 >=31 */
#define SCHED035_PRIO_INVALID 32 /* 超出优先级上限的非法值 */

static volatile INT32 g_sched035Go = 0; /* 主任务完成断言的放行标志 */

/* 子任务: 仅作为被设置对象存活, 等主任务完成全部参数校验断言后退出 */
static VOID *Pthread035Worker(VOID *arg)
{
    UINT32 i;

    (VOID)arg;
    for (i = 0; i < SCHED035_POLL_LIMIT; i++) {
        if (g_sched035Go == 1) {
            break;
        }
        LOS_TaskDelay(1);
    }
    return NULL;
}

static UINT32 Testcase(VOID)
{
    pthread_t workerThread;
    struct sched_param sp;
    int curPolicy;
    int maxPrio;
    int ret;

    g_sched035Go = 0;

    ret = pthread_create(&workerThread, NULL, Pthread035Worker, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    /* 1. 非法 policy: 当前实现仅支持 SCHED_RR, 非法值返回 ENOTSUP
     * (pthread.c:289-291, POSIX 对不支持策略亦规定 ENOTSUP) */
    sp.sched_priority = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    ret = pthread_setschedparam(workerThread, -1, &sp);
    ICUNIT_GOTO_EQUAL(ret, ENOTSUP, ret, EXIT_JOIN);
    ret = pthread_setschedparam(workerThread, SCHED_FIFO, &sp);
    ICUNIT_GOTO_EQUAL(ret, ENOTSUP, ret, EXIT_JOIN);

    /* 2. 非法优先级: 越界([0,30] 之外)返回 EINVAL(pthread.c:283-286) */
    sp.sched_priority = -1;
    ret = pthread_setschedparam(workerThread, SCHED_RR, &sp);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_JOIN);
    sp.sched_priority = SCHED035_PRIO_LOWEST; /* 31: >=OS_TASK_PRIORITY_LOWEST 被拒 */
    ret = pthread_setschedparam(workerThread, SCHED_RR, &sp);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_JOIN);
    sp.sched_priority = SCHED035_PRIO_INVALID; /* 32: 超上限 */
    ret = pthread_setschedparam(workerThread, SCHED_RR, &sp);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_JOIN);

    /* 3. param 为 NULL: EINVAL */
    ret = pthread_setschedparam(workerThread, SCHED_RR, NULL);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_JOIN);

    /* 4. 正路径锚点: 合法边界最高优先级(sched_get_priority_max(SCHED_RR)==0)设置成功,
     * getschedparam 读回一致(pthread.c:293-297 成功路径) */
    maxPrio = sched_get_priority_max(SCHED_RR);
    ICUNIT_GOTO_EQUAL(maxPrio, 0, maxPrio, EXIT_JOIN);
    sp.sched_priority = maxPrio;
    ret = pthread_setschedparam(workerThread, SCHED_RR, &sp);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);
    ret = pthread_getschedparam(workerThread, &curPolicy, &sp);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);
    ICUNIT_GOTO_EQUAL(curPolicy, SCHED_RR, curPolicy, EXIT_JOIN);
    ICUNIT_GOTO_EQUAL(sp.sched_priority, maxPrio, sp.sched_priority, EXIT_JOIN);

    /* 立即恢复默认优先级, 缩小子任务高优先级运行窗口 */
    sp.sched_priority = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    ret = pthread_setschedparam(workerThread, SCHED_RR, &sp);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);

    /* 5. setschedprio 越界: LOS_TaskPriSet 返回 PRIOR_ERROR → EINVAL(pthread.c:307-309) */
    ret = pthread_setschedprio(workerThread, -1);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_JOIN);
    ret = pthread_setschedprio(workerThread, SCHED035_PRIO_INVALID);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_JOIN);

    /* 6. 边界对照: prio=31 时 LOS_TaskPriSet 校验为 >31 才拒绝 → setschedprio 成功;
     * 与 setschedparam(>=31 拒绝)边界不一致, 按当前实现固化 */
    ret = pthread_setschedprio(workerThread, SCHED035_PRIO_LOWEST);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);

    /* 恢复默认优先级后收尾 */
    ret = pthread_setschedprio(workerThread, LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);

EXIT_JOIN:
    g_sched035Go = 1;
    (VOID)pthread_join(workerThread, NULL);
    return LOS_OK;
}

/**
 * @tc.name: ItPosixPthread035
 * @tc.desc: pthread_setschedparam/setschedprio 参数校验(非法 policy/越界优先级)与边界成功路径
 * @tc.type: FUNC
 * 覆盖目标: pthread.c:283-297, :307-309（setschedparam/setschedprio 参数校验与成功路径, gcov 第 1 轮）
 */
VOID ItPosixPthread035(VOID)
{
    TEST_ADD_CASE("ItPosixPthread035", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL2, TEST_FUNCTION);
}
