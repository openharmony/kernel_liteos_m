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

#include "It_posix_io.h"
#include <pthread.h>

#define PIPE004_BUF_LEN       64  /* 单次读写数据长度 */
#define PIPE004_WRITER_DELAY  10  /* 子任务写前延时(tick), 制造主任务读阻塞窗口 */
#define PIPE004_POLL_LIMIT    100 /* 跨任务握手轮询上限(tick) */

static INT32 g_pipe004Fds[2] = {-1, -1}; /* pipe 读写端 fd, 供子任务写入 */
static volatile INT32 g_pipe004Ready = 0; /* 主任务已发起读的握手标志 */
static CHAR g_pipe004Wbuf[PIPE004_BUF_LEN]; /* 预期写入内容 */

/* 子任务: 轮询等主任务发起读后, 再延时写入, 保证主任务先阻塞在空管读上 */
static VOID *Pipe004Writer(VOID *arg)
{
    ssize_t ret;
    UINT32 i;

    (VOID)arg;
    /* 轮询等待主任务就绪(上限 100 tick) */
    for (i = 0; i < PIPE004_POLL_LIMIT; i++) {
        if (g_pipe004Ready == 1) {
            break;
        }
        LOS_TaskDelay(1);
    }

    /* 再延时确保主任务已真正挂入 readSem 等待(pipe.c:519) */
    LOS_TaskDelay(PIPE004_WRITER_DELAY);

    /* 写入后 PipeWrite 末尾 PIPE_RW_POST(readSem)(pipe.c:594) 唤醒阻塞的读者 */
    ret = write(g_pipe004Fds[1], g_pipe004Wbuf, PIPE004_BUF_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE004_BUF_LEN, ret, EXIT);

EXIT:
    return NULL;
}

static UINT32 Testcase(VOID)
{
    CHAR rbuf[PIPE004_BUF_LEN] = {0};
    pthread_t writerThread;
    UINT64 tickStart;
    UINT64 tickEnd;
    UINT32 i;
    ssize_t ret;
    INT32 ret32;

    g_pipe004Ready = 0;
    g_pipe004Fds[0] = -1;
    g_pipe004Fds[1] = -1;
    for (i = 0; i < PIPE004_BUF_LEN; i++) {
        g_pipe004Wbuf[i] = (CHAR)(i & 0xFF); /* 构造可校验内容 */
    }

    ret32 = pipe(g_pipe004Fds);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT);

    ret32 = pthread_create(&writerThread, NULL, Pipe004Writer, NULL);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE);

    /* 置位就绪标志后立即读空管: PipeRead 发现 readIndex==writeIndex 且未回绕,
     * 释放设备锁并挂入 readSem 阻塞等待(pipe.c:515-523), 等待子任务写入唤醒 */
    g_pipe004Ready = 1;
    tickStart = LOS_TickCountGet();
    ret = read(g_pipe004Fds[0], rbuf, PIPE004_BUF_LEN);
    tickEnd = LOS_TickCountGet();

    /* 唤醒后一次读满 64B 且内容一致 */
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE004_BUF_LEN, ret, EXIT_JOIN);
    ICUNIT_GOTO_EQUAL(memcmp(rbuf, g_pipe004Wbuf, PIPE004_BUF_LEN), 0, rbuf[0], EXIT_JOIN);

    /* 弱断言: 阻塞时长覆盖子任务写前延时(留 1 tick 调度边界) */
    ICUNIT_GOTO_EQUAL((tickEnd - tickStart) >= (UINT64)(PIPE004_WRITER_DELAY - 1), TRUE,
                      (UINT32)(tickEnd - tickStart), EXIT_JOIN);

    (VOID)pthread_join(writerThread, NULL);
    (VOID)close(g_pipe004Fds[0]);
    (VOID)close(g_pipe004Fds[1]);
    return LOS_OK;

EXIT_JOIN:
    (VOID)pthread_join(writerThread, NULL);
EXIT_CLOSE:
    (VOID)close(g_pipe004Fds[0]);
    (VOID)close(g_pipe004Fds[1]);
EXIT:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixPipe004
 * @tc.desc: pipe 读空阻塞唤醒(主任务读空管阻塞, 子任务延时写入唤醒)
 * @tc.type: FUNC
 * 覆盖目标: pipe.c:515-523（PipeRead 读空阻塞等待+唤醒分支, gcov 第 1 轮）
 */
VOID ItPosixPipe004(VOID)
{
    TEST_ADD_CASE("ItPosixPipe004", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL0, TEST_FUNCTION);
}
