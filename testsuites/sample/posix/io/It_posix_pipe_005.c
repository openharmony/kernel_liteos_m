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

#define PIPE005_FILL_LEN       1024 /* 管道环形缓冲容量(PIPE_DEV_BUF_SIZE, pipe.c:47) */
#define PIPE005_MORE_LEN       64   /* 写满后追加写入长度 */
#define PIPE005_READER_DELAY   20   /* 子任务读前延时(tick), 制造主任务写阻塞窗口 */
#define PIPE005_POLL_LIMIT     100  /* 跨任务握手轮询上限(tick) */

static INT32 g_pipe005Fds[2] = {-1, -1}; /* pipe 读写端 fd, 供子任务读取 */
static volatile INT32 g_pipe005Ready = 0; /* 主任务已写满管道的握手标志 */
static CHAR g_pipe005Fill[PIPE005_FILL_LEN]; /* 首轮写满内容 */
static CHAR g_pipe005More[PIPE005_MORE_LEN]; /* 写满后追加内容 */

/* 子任务: 轮询等主任务写满后, 再延时读走 64B 腾出空间, 唤醒阻塞的写者 */
static VOID *Pipe005Reader(VOID *arg)
{
    CHAR rbuf[PIPE005_MORE_LEN] = {0};
    ssize_t ret;
    UINT32 i;

    (VOID)arg;
    /* 轮询等待主任务写满就绪(上限 100 tick) */
    for (i = 0; i < PIPE005_POLL_LIMIT; i++) {
        if (g_pipe005Ready == 1) {
            break;
        }
        LOS_TaskDelay(1);
    }

    /* 再延时确保主任务已真正挂入 writeSem 等待(pipe.c:585) */
    LOS_TaskDelay(PIPE005_READER_DELAY);

    /* 读走 64B: 缓冲非空(roll==TRUE)不走读阻塞分支, 读后 PIPE_RW_POST(writeSem)
     * (pipe.c:549) 唤醒阻塞的写者; 读出内容应为首轮写入的前 64B */
    ret = read(g_pipe005Fds[0], rbuf, PIPE005_MORE_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE005_MORE_LEN, ret, EXIT);
    ICUNIT_GOTO_EQUAL(memcmp(rbuf, g_pipe005Fill, PIPE005_MORE_LEN), 0, rbuf[0], EXIT);

EXIT:
    return NULL;
}

static UINT32 Testcase(VOID)
{
    CHAR rbuf[PIPE005_FILL_LEN] = {0};
    pthread_t readerThread;
    UINT64 tickStart;
    UINT64 tickEnd;
    UINT32 i;
    ssize_t ret;
    INT32 ret32;

    g_pipe005Ready = 0;
    g_pipe005Fds[0] = -1;
    g_pipe005Fds[1] = -1;
    for (i = 0; i < PIPE005_FILL_LEN; i++) {
        g_pipe005Fill[i] = (CHAR)(i & 0xFF);
    }
    for (i = 0; i < PIPE005_MORE_LEN; i++) {
        g_pipe005More[i] = (CHAR)((i + 1) & 0xFF);
    }

    ret32 = pipe(g_pipe005Fds);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT);

    /* 先一次写满 1024B: 单次环形写完成, 不触发写阻塞(写后 roll=TRUE, 缓冲满) */
    ret = write(g_pipe005Fds[1], g_pipe005Fill, PIPE005_FILL_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE005_FILL_LEN, ret, EXIT);

    ret32 = pthread_create(&readerThread, NULL, Pipe005Reader, NULL);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE);

    /* 缓冲已满再写 64B: PipeRingbufferWrite 返回 0, 走写满阻塞分支
     * (pipe.c:579-591: POST readSem + PIPE_RW_WAIT/pend writeSem), 等子任务读走数据唤醒 */
    g_pipe005Ready = 1;
    tickStart = LOS_TickCountGet();
    ret = write(g_pipe005Fds[1], g_pipe005More, PIPE005_MORE_LEN);
    tickEnd = LOS_TickCountGet();

    /* 子任务腾出 64B 空间后写入成功 */
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE005_MORE_LEN, ret, EXIT_JOIN);

    /* 弱断言: 阻塞时长覆盖子任务读前延时(留 1 tick 调度边界) */
    ICUNIT_GOTO_EQUAL((tickEnd - tickStart) >= (UINT64)(PIPE005_READER_DELAY - 1), TRUE,
                      (UINT32)(tickEnd - tickStart), EXIT_JOIN);

    /* 验证环形缓冲回绕后内容布局: 前 960B 为首轮写入的 fill[64..1023],
     * 后 64B 为腾出空间后写入的 more(子任务已读走 fill[0..63]) */
    ret = read(g_pipe005Fds[0], rbuf, PIPE005_FILL_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE005_FILL_LEN, ret, EXIT_JOIN);
    ICUNIT_GOTO_EQUAL(memcmp(rbuf, &g_pipe005Fill[PIPE005_MORE_LEN],
                             PIPE005_FILL_LEN - PIPE005_MORE_LEN), 0, rbuf[0], EXIT_JOIN);
    ICUNIT_GOTO_EQUAL(memcmp(&rbuf[PIPE005_FILL_LEN - PIPE005_MORE_LEN], g_pipe005More,
                             PIPE005_MORE_LEN), 0, rbuf[0], EXIT_JOIN);

    (VOID)pthread_join(readerThread, NULL);
    (VOID)close(g_pipe005Fds[0]);
    (VOID)close(g_pipe005Fds[1]);
    return LOS_OK;

EXIT_JOIN:
    (VOID)pthread_join(readerThread, NULL);
EXIT_CLOSE:
    (VOID)close(g_pipe005Fds[0]);
    (VOID)close(g_pipe005Fds[1]);
EXIT:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixPipe005
 * @tc.desc: pipe 写满阻塞唤醒(主任务写满后继续写阻塞, 子任务读走数据腾空间唤醒)
 * @tc.type: FUNC
 * 覆盖目标: pipe.c:577-591（PipeWrite 写满阻塞等待分支, gcov 第 1 轮）
 */
VOID ItPosixPipe005(VOID)
{
    TEST_ADD_CASE("ItPosixPipe005", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL0, TEST_FUNCTION);
}
