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

#define PIPE006_BUF_LEN  64   /* 单实例读写校验长度 */
#define PIPE006_FILL_A   0x11 /* 管道 a 数据填充模式(与 b/c 区分) */
#define PIPE006_FILL_B   0x22 /* 管道 b 数据填充模式 */
#define PIPE006_FILL_C   0x33 /* 管道 c 数据填充模式 */

static UINT32 Testcase(VOID)
{
    INT32 a[2] = {-1, -1}; /* 第一个 pipe 实例读写端 */
    INT32 b[2] = {-1, -1}; /* 第二个 pipe 实例读写端(与 a 并存) */
    INT32 c[2] = {-1, -1}; /* close a 后重建实例(验证设备号与 fd 复用) */
    CHAR wbuf[PIPE006_BUF_LEN];
    CHAR rbuf[PIPE006_BUF_LEN];
    ssize_t ret;
    INT32 ret32;

    /* 1. 两实例并存: pipe(a) 与 pipe(b) 各自注册设备并打开读写端
     * (PipeDevRegister→PipeDevNumAlloc 依次取 bitmap 最低空闲位, pipe.c:117-133;
     *  每实例 PipeOpen 两次, pipe.c:346-400) */
    ret32 = pipe(a);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);
    ret32 = pipe(b);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);

    /* 2. 并存期间互不串扰: 两管道各写 64B 模式数据, 数据同时在各自缓冲中停留,
     *    再分别读出校验内容一致(共享缓冲才会串扰出混合数据) */
    (VOID)memset_s(wbuf, PIPE006_BUF_LEN, PIPE006_FILL_A, PIPE006_BUF_LEN);
    ret = write(a[1], wbuf, PIPE006_BUF_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE006_BUF_LEN, ret, EXIT_CLOSE_ALL);

    (VOID)memset_s(wbuf, PIPE006_BUF_LEN, PIPE006_FILL_B, PIPE006_BUF_LEN);
    ret = write(b[1], wbuf, PIPE006_BUF_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE006_BUF_LEN, ret, EXIT_CLOSE_ALL);

    (VOID)memset_s(rbuf, PIPE006_BUF_LEN, 0, PIPE006_BUF_LEN);
    ret = read(a[0], rbuf, PIPE006_BUF_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE006_BUF_LEN, ret, EXIT_CLOSE_ALL);
    /* a 读出应为写入时的 FILL_A 模式(wbuf 现为 FILL_B, 重新填充期望值再比对) */
    (VOID)memset_s(wbuf, PIPE006_BUF_LEN, PIPE006_FILL_A, PIPE006_BUF_LEN);
    ICUNIT_GOTO_EQUAL((UINT32)memcmp(rbuf, wbuf, PIPE006_BUF_LEN), 0, rbuf[0], EXIT_CLOSE_ALL);

    (VOID)memset_s(wbuf, PIPE006_BUF_LEN, PIPE006_FILL_B, PIPE006_BUF_LEN);
    (VOID)memset_s(rbuf, PIPE006_BUF_LEN, 0, PIPE006_BUF_LEN);
    ret = read(b[0], rbuf, PIPE006_BUF_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE006_BUF_LEN, ret, EXIT_CLOSE_ALL);
    ICUNIT_GOTO_EQUAL((UINT32)memcmp(rbuf, wbuf, PIPE006_BUF_LEN), 0, rbuf[0], EXIT_CLOSE_ALL);

    /* 3. 关闭 a 读端: readerCnt 归零 → POST writeSem + POLLOUT(pipe.c:447-450);
     *    关闭 a 写端: writerCnt 归零 → POST readSem + POLLIN(pipe.c:452-455);
     *    两次 close 后 ref 归零 → PipeDevUnregister 释放设备号(pipe.c:299)并
     *    PipeDevFdFree 释放 fd 位图(pipe.c:437) */
    ret32 = close(a[0]);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);
    ret32 = close(a[1]);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);

    /* 4. 设备号与 fd 复用: close a 释放最低设备号与最低两位 fd 后,
     *    pipe(c) 的 PipeDevNumAlloc 走低位复用分支(pipe.c:122-130 空位扫描),
     *    PipeDevFdAlloc 同样复用最低空闲 fd 槽位(pipe.c:310-328),
     *    应用层可观察: c 的读写端 fd 与已关闭的 a 完全相同 */
    ret32 = pipe(c);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);
    ICUNIT_GOTO_EQUAL(c[0], a[0], c[0], EXIT_CLOSE_ALL);
    ICUNIT_GOTO_EQUAL(c[1], a[1], c[1], EXIT_CLOSE_ALL);

    /* 5. 复用实例功能正常: c 写读校验(模式 0x33, 区别于 a 的历史数据) */
    (VOID)memset_s(wbuf, PIPE006_BUF_LEN, PIPE006_FILL_C, PIPE006_BUF_LEN);
    ret = write(c[1], wbuf, PIPE006_BUF_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE006_BUF_LEN, ret, EXIT_CLOSE_ALL);
    (VOID)memset_s(rbuf, PIPE006_BUF_LEN, 0, PIPE006_BUF_LEN);
    ret = read(c[0], rbuf, PIPE006_BUF_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE006_BUF_LEN, ret, EXIT_CLOSE_ALL);
    ICUNIT_GOTO_EQUAL((UINT32)memcmp(rbuf, wbuf, PIPE006_BUF_LEN), 0, rbuf[0], EXIT_CLOSE_ALL);

    ret32 = close(c[0]);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);
    ret32 = close(c[1]);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);

    /* 6. EOF 语义: 关闭 b 写端后 writerCnt 归零 POST readSem(pipe.c:452-455),
     *    读端再 read: 缓冲空 → 初始等待被 close 唤醒立即通过(pipe.c:515-525),
     *    循环内读空且写者已全部退出 → 返回已读 0 字节(pipe.c:529-536),
     *    即写端全关后 read 返回 0 的 EOF 语义 */
    ret32 = close(b[1]);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);
    ret = read(b[0], rbuf, PIPE006_BUF_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)0, ret, EXIT_CLOSE_ALL);
    ret32 = close(b[0]);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT_CLOSE_ALL);

    return LOS_OK;

EXIT_CLOSE_ALL:
    /* 统一清理: 未打开端为 -1, 重复 close 已关端均安全返回 -1, 无副作用 */
    (VOID)close(a[0]);
    (VOID)close(a[1]);
    (VOID)close(b[0]);
    (VOID)close(b[1]);
    (VOID)close(c[0]);
    (VOID)close(c[1]);
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixPipe006
 * @tc.desc: pipe 多实例并存与设备号和 fd 复用(双 pipe 并存互不串扰, close 后重建复用, 写端关闭 EOF 语义)
 * @tc.type: FUNC
 * 覆盖目标: pipe.c:117-133（PipeDevNumAlloc 设备号分配与低位复用分支, gcov 第 3 轮）
 *   pipe.c:214-273（PipeDevRegister 第二实例注册路径）
 *   pipe.c:346-400（PipeOpen 第二实例两次打开, readerCnt/writerCnt 计数）
 *   pipe.c:422-468（PipeClose: 读端关闭 POST writeSem:447-450, 写端关闭 POST readSem:452-455, ref 归零 Unregister:457-462）
 *   pipe.c:299（PipeDevNumFree 设备号释放）
 *   pipe.c:310-328（PipeDevFdAlloc 最低空闲 fd 复用）
 *   pipe.c:529-536（PipeRead 读空且无写者 → 返回 0 的 EOF 分支）
 *   pipe.c:686-702（OsPipeInit 互斥锁初始化, pipe 首次调用前置路径）
 */
VOID ItPosixPipe006(VOID)
{
    TEST_ADD_CASE("ItPosixPipe006", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL0, TEST_FUNCTION);
}
