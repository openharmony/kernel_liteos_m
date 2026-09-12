/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted under the following conditions:
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
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ICUNIT_RESUME_H
#define _ICUNIT_RESUME_H

/*
 * 崩溃/卡死续跑配置。
 *
 * 使用场景：全量测试跑到某条用例崩溃或卡死（看门狗复位）后，希望从该用例的
 * 下一条开始继续跑完剩余用例，而不必每次都从头跑。
 *
 * 使用方法：
 *   1. 从上次测试日志中找到崩溃/卡死的那条用例名（即 [Passed]/[Failed] 之后
 *      最后一条打印出来的用例名，或崩溃前最后执行的用例名）。
 *   2. 取消下面 ICUNIT_RESUME_CASE 的注释，把字符串换成该用例名。
 *   3. touch icunit_filter.c && hb build  重新编译。
 *   4. 烧录运行：匹配到该用例名之前的所有用例会被跳过（不执行），
 *      该用例本身被标记为 [Crash-Skip] 并跳过，从下一条用例开始正常执行
 *      直到跑完。
 *   5. 跑完后若又崩溃在新用例，重复步骤 1-4，把名字换成新的崩溃用例。
 *   6. 全部跑通后，务必把 ICUNIT_RESUME_CASE 重新注释掉（或留空），
 *      恢复全量执行，再做一次完整验证。
 *
 * 用例名即各测试文件 TEST_ADD_CASE(...) 的第一个参数，如 "ItLosTask042"。
 *
 * 与 icunit_case_filter.h（白名单）的关系：
 *   - 两者相互独立，可单独使用，也可同时使用（取交集）。
 *   - 白名单非空时：只执行白名单里列出的用例。
 *   - 续跑设定时：跳过崩溃点之前的用例，从崩溃点的下一个开始执行。
 *   - 同时设定时：用例必须同时满足两个条件才执行（既在崩溃点之后、又在白名单里）。
 *
 * ── 重编注意 ──────────────────────────────────────────────────────
 * 本仓库 lite 工具链的 cc 规则未启用头依赖跟踪，仅修改本头文件后
 * hb build 不会自动重编 icunit_filter.c。改完请用下面任一方式确保重编：
 *   touch kernel/liteos_m/testsuites/src/icunit_filter.c && hb build
 * 或   rm -rf out && hb build
 * ──────────────────────────────────────────────────────────────────
 */

/* 取消下一行注释并替换为崩溃用例名，即可启用续跑 */
/* #define ICUNIT_RESUME_CASE "ItLosTask042" */

#ifndef ICUNIT_RESUME_CASE
#define ICUNIT_RESUME_CASE NULL
#endif

#endif /* _ICUNIT_RESUME_H */
