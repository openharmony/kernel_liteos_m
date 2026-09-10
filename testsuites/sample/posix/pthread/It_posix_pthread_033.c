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
#define _GNU_SOURCE 1 /* affinity_np/cpuset 声明受 _GNU_SOURCE 守卫 */
#endif

#include "It_posix_pthread.h"
#include <sched.h>

/* 用例简要描述: pthread_attr_setaffinity/getaffinity_np 单核 no-op 契约 */
/* 用例简要描述: attr affinity_np 单核 no-op 契约 */
static UINT32 Testcase(VOID)
{
    pthread_attr_t a;
    cpu_set_t set;
    int ret;

    ret = pthread_attr_init(&a);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    (VOID)memset_s(&set, sizeof(set), 0, sizeof(set));
    set.__bits[0] = 1; /* cpu0 */

    /* F-Pos: 单核(非 SMP)下 setaffinity_np 为 no-op 恒成功(pthread_attr.c #else 分支) */
    ret = pthread_attr_setaffinity_np(&a, sizeof(cpu_set_t), &set);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    /* F-Pos: getaffinity_np 同为 no-op 成功 */
    (VOID)memset_s(&set, sizeof(set), 0, sizeof(set));
    ret = pthread_attr_getaffinity_np(&a, sizeof(cpu_set_t), &set);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_destroy(&a);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    return LOS_OK;

EXIT:
    (VOID)pthread_attr_destroy(&a);
    return LOS_OK;

}

VOID ItPosixPthread033(VOID)
{
    TEST_ADD_CASE("ItPosixPthread033", Testcase, TEST_POSIX, TEST_PTHREAD, TEST_LEVEL0, TEST_FUNCTION);
}
