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
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _ICUNIT_CASE_FILTER_H
#define _ICUNIT_CASE_FILTER_H

/*
 * 调试用例过滤表（X-macro 风格，由 icunit_filter.c 在数组初始化处
 * #include 本文件，并把每条 ICUNIT_FILTER_ENTRY(name) 展开成 "name",）。
 *
 * 使用方法：
 *   - 留空（下方全部注释或无条目）→ 执行全量用例。
 *   - 新增一行 / 取消注释一行 → 只执行列出的用例，其余遍历但不进入执行。
 *
 * 写法：每行一条，独立成行，无需反斜杠续行、无需逗号：
 *     ICUNIT_FILTER_ENTRY("ItLosTask001")
 * 用例名即各测试文件 TEST_ADD_CASE(...) 的第一个参数。
 *
 * 特点：
 *   - 每行相互独立，新增/删除/注释任一行都不影响其它行；
 *   - 漏写引号或拼错宏名会在编译期直接报错，不会静默失效；
 *   - 名字写错或对应用例未编译进来（被 #if 屏蔽）时，跑完会打印
 *     "[Filter] not found: <名字> (not compiled in or wrong name)"。
 *
 * ── 重编注意 ──────────────────────────────────────────────────────
 * 本仓库 lite 工具链的 cc 规则未启用头依赖跟踪，仅修改本头文件后
 * `hb build` 不会自动重编 icunit_filter.c。改完请用下面任一方式确保重编：
 *   touch kernel/liteos_m/testsuites/src/icunit_filter.c && hb build
 * 或   rm -rf out && hb build
 * ──────────────────────────────────────────────────────────────────
 */

/* ICUNIT_FILTER_ENTRY("ItLosTask001") */
/* ICUNIT_FILTER_ENTRY("ItLosTask003") */
/* ICUNIT_FILTER_ENTRY("ItLosTask057") */

#endif /* _ICUNIT_CASE_FILTER_H */
