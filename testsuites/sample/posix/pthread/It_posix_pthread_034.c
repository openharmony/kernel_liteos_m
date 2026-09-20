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

#define COND034_POLL_LIMIT 100 /* 跨任务握手轮询上限(tick) */

/* 静态初始化 cond: PTHREAD_COND_INITIALIZER={0}(musl porting pthread.h:68),
 * event 链表指针为 NULL → CondInitCheck 为真 → cond_wait 内惰性 init(pthread_cond.c:326-331) */
static pthread_cond_t g_cond034 = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_mtx034;
static volatile INT32 g_cond034Ready = 0; /* 主任务已进入等待流程的握手标志 */

/* 子任务: 等主任务发起 wait 后, 持外部 mutex 期间 signal, 保证此时 cond 已完成惰性 init */
static VOID *Pthread034Signaler(VOID *arg)
{
    int ret;
    UINT32 i;

    (VOID)arg;
    /* 轮询等主任务就绪(上限 100 tick) */
    for (i = 0; i < COND034_POLL_LIMIT; i++) {
        if (g_cond034Ready == 1) {
            break;
        }
        LOS_TaskDelay(1);
    }

    /* 拿到外部 mutex 即表明主任务已进入 cond_wait 并释放该锁(内部已完成惰性 init
     * 与 count++), 此时 signal 安全(否则对未 init 的 cond->mutex 操作会异常) */
    ret = pthread_mutex_lock(&g_mtx034);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    /* count>0 → LOS_EventWrite 写事件, 唤醒阻塞的主任务 */
    ret = pthread_cond_signal(&g_cond034);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_UNLOCK);

EXIT_UNLOCK:
    (VOID)pthread_mutex_unlock(&g_mtx034);
EXIT:
    return NULL;
}

static UINT32 Testcase(VOID)
{
    pthread_t signalerThread;
    int ret;

    g_cond034Ready = 0;

    ret = pthread_mutex_init(&g_mtx034, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&signalerThread, NULL, Pthread034Signaler, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MTX);

    /* 持锁后置位握手标志再 wait(规范用法): cond_wait 内 CondInitCheck 发现
     * 静态 cond 未初始化 → 惰性 pthread_cond_init(pthread_cond.c:326-331),
     * 随后 count++/释放外部锁/挂入事件等待 */
    ret = pthread_mutex_lock(&g_mtx034);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);

    g_cond034Ready = 1;
    ret = pthread_cond_wait(&g_cond034, &g_mtx034);

    /* 子任务 signal 唤醒后返回 0, 且外部锁被重新持有 */
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_UNLOCK);

    ret = pthread_mutex_unlock(&g_mtx034);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);

    /* 成功路径收尾 */
    (VOID)pthread_join(signalerThread, NULL);
    /* 惰性 init 会 malloc 内部 mutex, destroy 释放避免泄漏 */
    (VOID)pthread_cond_destroy(&g_cond034);
    (VOID)pthread_mutex_destroy(&g_mtx034);
    return LOS_OK;

EXIT_UNLOCK:
    /* wait 异常返回时外部锁已被 cond_wait 重新持有, 补一次解锁 */
    (VOID)pthread_mutex_unlock(&g_mtx034);
EXIT_JOIN:
    (VOID)pthread_join(signalerThread, NULL);
    (VOID)pthread_cond_destroy(&g_cond034);
    (VOID)pthread_mutex_destroy(&g_mtx034);
    return LOS_OK;

EXIT_MTX:
    (VOID)pthread_mutex_destroy(&g_mtx034);
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixPthread034
 * @tc.desc: PTHREAD_COND_INITIALIZER 静态初始化 cond 的 wait 惰性 init + signal 唤醒
 * @tc.type: FUNC
 * 覆盖目标: pthread_cond.c:326-331（cond_wait 静态初始化惰性 init 分支, gcov 第 1 轮）
 */
VOID ItPosixPthread034(VOID)
{
    TEST_ADD_CASE("ItPosixPthread034", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL2, TEST_FUNCTION);
}
