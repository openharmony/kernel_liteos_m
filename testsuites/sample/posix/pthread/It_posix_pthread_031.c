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
#define _GNU_SOURCE 1 /* pthread_attr_setstackaddr 等声明受特性宏守卫 */
#endif

#include "It_posix_pthread.h"

/* musl porting 头未声明(POSIX.1-2008 已移除),B 侧实现存在于 pthread_attr.c:204 */
extern int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackAddr);

/* 用例简要描述: pthread_attr_getstack 未设返回 EINVAL+设置后对称回读 */
static UINT32 Testcase(VOID)
{
    pthread_attr_t a;
    int ret;
    void *stackAddr = NULL;
    size_t stackSize = 0;
    static CHAR stackBuf[PTHREAD_STACK_MIN * 2];

    ret = pthread_attr_init(&a);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    /* F-Inv: 未设置栈地址/大小时 getstack 返回 EINVAL(实现要求 stacksize_set && stackaddr_set) */
    ret = pthread_attr_getstack(&a, &stackAddr, &stackSize);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    /* F-Pos: setstacksize/setstackaddr 后 getstack 对称回读 */
    ret = pthread_attr_setstacksize(&a, sizeof(stackBuf));
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setstackaddr(&a, stackBuf);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_getstack(&a, &stackAddr, &stackSize);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_ASSERT_EQUAL(stackSize, sizeof(stackBuf), (UINT32)stackSize);
    ICUNIT_GOTO_EQUAL(stackAddr, stackBuf, (UINT32)(UINTPTR)stackAddr, EXIT);

    ret = pthread_attr_destroy(&a);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    return LOS_OK;

EXIT:
    (VOID)pthread_attr_destroy(&a);
    return LOS_OK;

}

VOID ItPosixPthread031(VOID)
{
    TEST_ADD_CASE("ItPosixPthread031", Testcase, TEST_POSIX, TEST_PTHREAD, TEST_LEVEL0, TEST_FUNCTION);
}
