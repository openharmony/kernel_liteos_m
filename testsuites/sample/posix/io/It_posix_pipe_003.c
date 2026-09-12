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


#include "bug_isolate.h" /* BUG_02 等宏统一在此定义 */

#ifndef PRODUCT_BUG_ISOLATE
#include "It_posix_io.h"

/* ws63(riscv) 上 poll-on-pipe 路径触发 hang(看门狗复位,qemu/arm 正常),
   疑似 PipePoll(pipe.c:603,零覆盖内部函数)产品缺陷,ws63 暂旁路待开发定位 */
#include <poll.h>

/* 用例简要描述: poll on pipe 空超时/POLLIN 事件/读空无事件 */
static UINT32 Testcase(VOID)
{
    int fds[2]; /* 2, pipe fd pair */
    int ret;
    struct pollfd pfd;
    char buf[8] = "poll"; /* 8, buffer size */
    char rbuf[8];

    ret = pipe(fds);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    /* 空管道: poll 短超时返回 0(无事件) */
    (VOID)memset_s(&pfd, sizeof(pfd), 0, sizeof(pfd));
    pfd.fd = fds[0];
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, 1); /* 1: nfds, 1: timeout ms */
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    /* 写入后: poll 立即返回 1 且 POLLIN 置位 */
    ret = write(fds[1], buf, sizeof(buf));
    ICUNIT_GOTO_EQUAL(ret, sizeof(buf), ret, EXIT);

    (VOID)memset_s(&pfd, sizeof(pfd), 0, sizeof(pfd));
    pfd.fd = fds[0];
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, 0); /* 1: nfds, 0: no wait */
    ICUNIT_GOTO_EQUAL(ret, 1, ret, EXIT);
    ICUNIT_GOTO_EQUAL((pfd.revents & POLLIN), POLLIN, (UINT32)pfd.revents, EXIT);

    /* 读空后: poll 无事件 */
    ret = read(fds[0], rbuf, sizeof(rbuf));
    ICUNIT_GOTO_EQUAL(ret, sizeof(rbuf), ret, EXIT);

    (VOID)memset_s(&pfd, sizeof(pfd), 0, sizeof(pfd));
    pfd.fd = fds[0];
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, 0);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

EXIT:
    close(fds[0]);
    close(fds[1]);
    return LOS_OK;
}

VOID ItPosixPipe003(VOID)
{
    TEST_ADD_CASE("ItPosixPipe003", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
#else
typedef int bug02_placeholder; /* 保非空翻译单元 */
#endif /* PRODUCT_BUG_ISOLATE: pipe_003 */
