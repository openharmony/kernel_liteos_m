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

#include "icunit_filter.h"
#include "icunit_resume.h"
#include <string.h>

iUINT32 g_crashResult = 0;

static const iCHAR *g_resumeFromCase = ICUNIT_RESUME_CASE;
static iUINT32 g_resumePassed = 0;
static iUINT32 g_resumeFound = 0;

#define ICUNIT_RESUME_ENABLED (g_resumeFromCase != NULL)

iUINT32 IcResumeHit(const iCHAR *caseName)
{
    if (!ICUNIT_RESUME_ENABLED) {
        return TRUE;
    }
    if (g_resumePassed) {
        return TRUE;
    }
    if (strcmp(caseName, g_resumeFromCase) == 0) {
        g_resumePassed = 1;
        g_resumeFound = 1;
        g_crashResult++;
        dprintf("  [Crash-Skip]-%s (resume point, skipped)\n", caseName);
        return FALSE;
    }
    return FALSE;
}

void IcResumeReport(void)
{
    if (!ICUNIT_RESUME_ENABLED) {
        return;
    }
    if (!g_resumeFound) {
        dprintf("[Resume] point '%s' not found (wrong name or not compiled in)\n",
                g_resumeFromCase);
    } else {
        dprintf("[Resume] resumed from after '%s'\n", g_resumeFromCase);
    }
}

/* X-macro：每条 ICUNIT_FILTER_ENTRY(name) 展开成 "name",，
 * 由 #include 把 icunit_case_filter.h 里的名表直接喂进数组初始化。
 * 头文件留空 → 数组为 {NULL} → 过滤项数 0 → 跑全量。 */
#define ICUNIT_FILTER_ENTRY(name) name,
static const iCHAR *g_filterList[] = {
#include "icunit_case_filter.h"
    NULL
};
#undef ICUNIT_FILTER_ENTRY

/* 编译期算出过滤项数（不含 NULL 哨兵）。 */
#define ICUNIT_FILTER_COUNT (sizeof(g_filterList) / sizeof(g_filterList[0]) - 1)

/* 命中标记，BSS 零初始化；尺寸 = 项数 + 1，保证空表时也为合法数组。 */
static iUINT32 g_filterMatchedFlag[ICUNIT_FILTER_COUNT + 1];

iUINT32 IcFilterHit(const iCHAR *caseName)
{
    iUINT16 i;

    /* 过滤表为空 → 执行全量 */
    if (ICUNIT_FILTER_COUNT == 0) {
        return TRUE;
    }
    /* 线性 strcmp：命中即执行，否则跳过（遍历但不进入） */
    for (i = 0; i < (iUINT16)ICUNIT_FILTER_COUNT; i++) {
        if (strcmp(g_filterList[i], caseName) == 0) {
            g_filterMatchedFlag[i] = TRUE;
            return TRUE;
        }
    }
    return FALSE;
}

void IcFilterReportNotFound(void)
{
    iUINT16 i;

    for (i = 0; i < (iUINT16)ICUNIT_FILTER_COUNT; i++) {
        if (!g_filterMatchedFlag[i]) {
            dprintf("[Filter] not found: %s (not compiled in or wrong name)\n",
                    g_filterList[i]);
        }
    }
}
