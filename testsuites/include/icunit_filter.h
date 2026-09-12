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

#ifndef _ICUNIT_FILTER_H
#define _ICUNIT_FILTER_H

#include "iCunit.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*
 * 判断 caseName 是否应执行。
 * 返回 TRUE  = 执行该用例；
 * 返回 FALSE = 跳过（遍历但不进入执行）。
 * 过滤表为空时永远返回 TRUE（执行全量用例）。
 */
extern iUINT32 IcFilterHit(const iCHAR *caseName);

/*
 * 在所有用例跑完后调用，打印过滤表中"未找到"的条目
 * （名字写错或用例未编译进来）。过滤表为空时为 no-op。
 */
extern void IcFilterReportNotFound(void);

/*
 * 崩溃/卡死续跑判定。
 * 返回 TRUE  = 执行该用例；
 * 返回 FALSE = 跳过该用例（崩溃点之前或崩溃点本身）。
 * 未配置续跑点（ICUNIT_RESUME_CASE 为 NULL）时永远返回 TRUE。
 * 命中崩溃用例名时，该条跳过并计入 g_crashResult，之后所有用例正常执行。
 */
extern iUINT32 IcResumeHit(const iCHAR *caseName);

/*
 * 在所有用例跑完后调用，报告续跑状态：
 * 未配置续跑点、已越过续跑点、或续跑点未找到（用例名写错/未编译进来）。
 */
extern void IcResumeReport(void);

extern iUINT32 g_crashResult;

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _ICUNIT_FILTER_H */
