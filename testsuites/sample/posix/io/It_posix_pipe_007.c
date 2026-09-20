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

#define PIPE007_W1_LEN    600 /* 首轮写入长度: 写指针 0→600 */
#define PIPE007_R1_LEN    400 /* 首轮读取长度: 部分读, 读指针 0→400 */
#define PIPE007_W2_LEN    500 /* 次轮写入长度: 424B 填至尾部 1024 后回绕, 76B 绕回头部 */
#define PIPE007_R2_LEN    700 /* 次轮读取长度: 624B 读至尾部 1024 后回绕, 76B 绕回头部 */
#define PIPE007_TOTAL_LEN (PIPE007_W1_LEN + PIPE007_W2_LEN) /* 总流量 1100B, 环形缓冲容量为 PIPE_DEV_BUF_SIZE=1024B(pipe.c:47) */

static UINT32 Testcase(VOID)
{
    INT32 fds[2] = {-1, -1};
    CHAR wstream[PIPE007_TOTAL_LEN]; /* 完整写入字节流 */
    CHAR rstream[PIPE007_TOTAL_LEN]; /* 完整读出字节流 */
    UINT32 i;
    ssize_t ret;
    INT32 ret32;

    /* 构造伪随机可校验字节流(区分度强, 便于发现错位与串扰) */
    for (i = 0; i < PIPE007_TOTAL_LEN; i++) {
        wstream[i] = (CHAR)((i * 7 + 3) & 0xFF);
    }
    (VOID)memset_s(rstream, PIPE007_TOTAL_LEN, 0, PIPE007_TOTAL_LEN);

    ret32 = pipe(fds);
    ICUNIT_GOTO_EQUAL(ret32, 0, ret32, EXIT);

    /* 步骤 1: 首写 600B — 初始 readIndex==writeIndex==0 且未回绕,
     * PipeRingbufferWrite 走 else 未回绕分支自尾部起取整段空间
     * (pipe.c:196-199), 单次写完, 写指针 0→600 */
    ret = write(fds[1], wstream, PIPE007_W1_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE007_W1_LEN, ret, EXIT_CLOSE);

    /* 步骤 2: 部分读 400B — readIndex(0)<writeIndex(600), 可读 600B 只取 400B
     * (pipe.c:164-165), 读指针 0→400, 缓冲余 200B 未读 */
    ret = read(fds[0], rstream, PIPE007_R1_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE007_R1_LEN, ret, EXIT_CLOSE);

    /* 步骤 3: 再写 500B 触发写跨回绕 — readIndex(400)<writeIndex(600) 先填
     * 尾部空间 424B(pipe.c:190-191)至 1024, 触发回绕 roll=TRUE 且写指针归 0
     * (pipe.c:206-209); 一次写不完, PipeWrite 循环二次调 PipeRingbufferWrite,
     * readIndex(400)>writeIndex(0) 填头部剩余 76B(pipe.c:192-193) */
    ret = write(fds[1], &wstream[PIPE007_W1_LEN], PIPE007_W2_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE007_W2_LEN, ret, EXIT_CLOSE);

    /* 步骤 4: 再读 700B 触发读跨回绕 — readIndex(400)>writeIndex(76) 先读
     * 尾段 624B(pipe.c:166-167)至 1024, 触发读指针归 0 且 roll 复位 FALSE
     * (pipe.c:178-181); PipeRead 循环二次调 PipeRingbufferRead,
     * readIndex(0)<writeIndex(76) 读头段 76B(pipe.c:164-165) */
    ret = read(fds[0], &rstream[PIPE007_R1_LEN], PIPE007_R2_LEN);
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)PIPE007_R2_LEN, ret, EXIT_CLOSE);

    /* 步骤 5: 流式语义 — 两轮共写入 1100B, 两轮共读出 1100B,
     * 读出流与写入流字节序完全一致(部分读+回绕不丢字节不错序) */
    ICUNIT_GOTO_EQUAL((UINT32)memcmp(rstream, wstream, PIPE007_TOTAL_LEN), 0,
                      rstream[0], EXIT_CLOSE);

    (VOID)close(fds[0]);
    (VOID)close(fds[1]);
    return LOS_OK;

EXIT_CLOSE:
    (VOID)close(fds[0]);
    (VOID)close(fds[1]);
EXIT:
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixPipe007
 * @tc.desc: pipe 环形缓冲跨回绕读写矩阵(写 600 部分读 400 再写 500 跨回绕再读 700, 流式一致性校验)
 * @tc.type: FUNC
 * 覆盖目标: pipe.c:190-191（PipeRingbufferWrite read<write 尾部空间分支, gcov 第 3 轮）
 *   pipe.c:192-193（PipeRingbufferWrite read>write 头部空间分支）
 *   pipe.c:206-209（写指针到尾回绕: roll=TRUE, writeIndex 归 0）
 *   pipe.c:166-167（PipeRingbufferRead read>write 尾段分支）
 *   pipe.c:178-181（读指针到尾回绕: readIndex 归 0, roll 复位 FALSE）
 *   pipe.c:527-548（PipeRead while 循环跨段多次读 + :549 PIPE_RW_POST）
 *   pipe.c:577-593（PipeWrite while 循环跨段多次写 + :594 PIPE_RW_POST）
 */
VOID ItPosixPipe007(VOID)
{
    TEST_ADD_CASE("ItPosixPipe007", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL0, TEST_FUNCTION);
}
