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

#include "It_posix_rwlock.h"
#include "time.h"

#define RWLOCK076_HOLD_DELAY  20  /* 子任务持锁时长(tick), 需小于主任务超时窗口 */
#define RWLOCK076_POLL_LIMIT  100 /* 跨任务握手轮询上限(tick) */
#define RWLOCK076_TIMEOUT_SEC 2   /* timedwrlock 绝对超时窗口(+2s) */

static pthread_rwlock_t g_rwl076;
static volatile INT32 g_rwl076Ready = 0; /* 子任务已持写锁的握手标志 */

/* 子任务: 持写锁后置 ready, 延时 20 tick 再释放, 为主任务制造竞争窗口 */
static VOID *Rwlock076Writer(VOID *arg)
{
    int ret;

    (VOID)arg;
    ret = pthread_rwlock_wrlock(&g_rwl076);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_rwl076Ready = 1;
    LOS_TaskDelay(RWLOCK076_HOLD_DELAY);

    /* 释放写锁: pending_writers>0 → signal write_cond(pthread_rwlock.c:452-453),
     * 唤醒等待中的主任务 */
    ret = pthread_rwlock_unlock(&g_rwl076);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
EXIT:
    return NULL;
}

static UINT32 Testcase(VOID)
{
    pthread_t writerThread;
    pthread_attr_t writerAttr;
    struct sched_param sp;
    struct timespec ts;
    UINT32 i;
    int ret;

    g_rwl076Ready = 0;

    ret = pthread_rwlock_init(&g_rwl076, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_init(&writerAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    /* 高优先级确保子任务先调度持锁, 主任务 timedwrlock 必然进入等待分支 */
    sp.sched_priority = RWLOCK_TEST_HIGH_PRIO;
    (VOID)pthread_attr_setschedparam(&writerAttr, &sp);
    (VOID)pthread_attr_setinheritsched(&writerAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&writerThread, &writerAttr, Rwlock076Writer, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    /* 轮询等子任务持锁完成(上限 100 tick) */
    for (i = 0; i < RWLOCK076_POLL_LIMIT; i++) {
        if (g_rwl076Ready == 1) {
            break;
        }
        LOS_TaskDelay(1);
    }
    ICUNIT_GOTO_EQUAL(g_rwl076Ready, 1, g_rwl076Ready, EXIT_JOIN);

    /* 绝对超时 +2s, 锁被子任务持有 → 进入 cond_timedwait 等待分支
     * (pthread_rwlock.c:237-242); 子任务 20 tick 内释放 → 窗口内获取成功 */
    (VOID)clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += RWLOCK076_TIMEOUT_SEC;

    ret = pthread_rwlock_timedwrlock(&g_rwl076, &ts);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);

    /* 获取成功后正常释放 */
    ret = pthread_rwlock_unlock(&g_rwl076);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);

EXIT_JOIN:
    (VOID)pthread_join(writerThread, NULL);
    (VOID)pthread_attr_destroy(&writerAttr);
    (VOID)pthread_rwlock_destroy(&g_rwl076);
    return LOS_OK;

EXIT_ATTR:
    (VOID)pthread_attr_destroy(&writerAttr);
    (VOID)pthread_rwlock_destroy(&g_rwl076);
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixRwlock076
 * @tc.desc: timedwrlock 竞争获取(锁被他人持有进入限时等待, 窗口内释放后成功获取)
 * @tc.type: FUNC
 * 覆盖目标: pthread_rwlock.c:237-242（timedwrlock cond_timedwait 等待分支, gcov 第 1 轮）
 */
VOID ItPosixRwlock076(VOID)
{
    TEST_ADD_CASE("ItPosixRwlock076", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL2, TEST_FUNCTION);
}
