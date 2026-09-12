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


#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1 /* sched affinity 族声明受 _GNU_SOURCE 守卫(musl sched.h) */
#endif

#include "It_posix_pthread.h"
#include <sched.h>

/* 用例简要描述: sched_setaffinity/getaffinity 单核 no-op 契约 */
static UINT32 Testcase(VOID)
{
    cpu_set_t set;
    int ret;

    (VOID)memset_s(&set, sizeof(set), 0, sizeof(set));
    set.__bits[0] = 1; /* cpu0 */

    /* F-Pos: 单核(非 SMP)下 setaffinity 为 no-op 恒成功(sched.c:73-105 #else 分支) */
    ret = sched_setaffinity(0, sizeof(cpu_set_t), &set);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    /* F-Pos: 单核下 getaffinity 同为 no-op 成功 */
    ret = sched_getaffinity(0, sizeof(cpu_set_t), &set);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixPthread032(VOID)
{
    TEST_ADD_CASE("ItPosixPthread032", Testcase, TEST_POSIX, TEST_PTHREAD, TEST_LEVEL0, TEST_FUNCTION);
}
