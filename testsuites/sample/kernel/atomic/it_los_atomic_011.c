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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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


#include "it_los_atomic.h"

static VOID TaskF01(VOID)
{
    Atomic atomicTestCounter = 0;
    INT32 readCounter = 0;
    INT32 atomicRet = 0;

    Atomic64 atomic64TestCounter = 0;
    INT64 read64Counter = 0;
    INT64 atomic64Ret = 0;

    ICUNIT_ASSERT_EQUAL_VOID(g_testCount, 0, g_testCount);

    atomicTestCounter = 0x88888888;
    LOS_AtomicSet(&atomicTestCounter, 0x88888888);
    atomicRet = LOS_AtomicAdd(&atomicTestCounter, 0x33333333);
    ICUNIT_ASSERT_EQUAL_VOID(atomicRet, 0xbbbbbbbb, atomicRet);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0xbbbbbbbb, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0xaaaaaaaa);
    atomicRet = LOS_AtomicAdd(&atomicTestCounter, 0x77777777);
    ICUNIT_ASSERT_EQUAL_VOID(atomicRet, 0x22222221, atomicRet);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL(readCounter, 0x22222221, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0xdddddddd);
    atomicRet = LOS_AtomicSub(&atomicTestCounter, 0x99999999);
    ICUNIT_ASSERT_EQUAL_VOID(atomicRet, 0x44444444, atomicRet);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0x44444444, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0x88888888);
    atomicRet = LOS_AtomicSub(&atomicTestCounter, 0xdddddddd);
    ICUNIT_ASSERT_EQUAL_VOID(atomicRet, 0xaaaaaaab, atomicRet);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0xaaaaaaab, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0x11111111);
    LOS_AtomicInc(&atomicTestCounter);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0x11111112, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0xffffffff);
    LOS_AtomicInc(&atomicTestCounter);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0x00000000, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0x88888888);
    atomicRet = LOS_AtomicIncRet(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomicRet, 0x88888889, atomicRet);

    LOS_AtomicSet(&atomicTestCounter, 0xffffffff);
    atomicRet = LOS_AtomicIncRet(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomicRet, 0x00000000, atomicRet);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0x00000000, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0x66666666);
    LOS_AtomicDec(&atomicTestCounter);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0x66666665, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0x00000000);
    LOS_AtomicDec(&atomicTestCounter);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0xffffffff, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0x88888888);
    atomicRet = LOS_AtomicDecRet(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomicRet, 0x88888887, atomicRet);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0x88888887, readCounter);

    LOS_AtomicSet(&atomicTestCounter, 0x00000000);
    atomicRet = LOS_AtomicDecRet(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomicRet, 0xffffffff, atomicRet);
    readCounter = LOS_AtomicRead(&atomicTestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(readCounter, 0xffffffff, readCounter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    atomic64Ret = LOS_Atomic64Add(&atomic64TestCounter, 0x0000000033333333);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x5aaaaaaabbbbbbbb, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaaabbbbbbbb, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    atomic64Ret = LOS_Atomic64Add(&atomic64TestCounter, 0x0000123456789abc);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x5aaabcdedf012344, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaabcdedf012344, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0xaaaaaaaa88888888);
    atomic64Ret = LOS_Atomic64Add(&atomic64TestCounter, 0x88888888aaaaaaaa);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x3333333333333332, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x3333333333333332, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    atomic64Ret = LOS_Atomic64Sub(&atomic64TestCounter, 0x0000000033333333);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x5aaaaaaa55555555, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaaa55555555, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    atomic64Ret = LOS_Atomic64Sub(&atomic64TestCounter, 0x00001234cba98765);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x5aaa9875bcdf0123, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaa9875bcdf0123, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    atomic64Ret = LOS_Atomic64Sub(&atomic64TestCounter, 0x60001234cba98765);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0xfaaa9875bcdf0123, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0xfaaa9875bcdf0123, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    LOS_Atomic64Inc(&atomic64TestCounter);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaaa88888889, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaaffffffff);
    LOS_Atomic64Inc(&atomic64TestCounter);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaab00000000, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0xffffffffffffffff);
    LOS_Atomic64Inc(&atomic64TestCounter);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x0000000000000000, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    atomic64Ret = LOS_Atomic64IncRet(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x5aaaaaaa88888889, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaaa88888889, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaaffffffff);
    atomic64Ret = LOS_Atomic64IncRet(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x5aaaaaab00000000, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaab00000000, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0xffffffffffffffff);
    atomic64Ret = LOS_Atomic64IncRet(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x0000000000000000, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x0000000000000000, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    LOS_Atomic64Dec(&atomic64TestCounter);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaaa88888887, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa00000000);
    LOS_Atomic64Dec(&atomic64TestCounter);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaa9ffffffff, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x0000000000000000);
    LOS_Atomic64Dec(&atomic64TestCounter);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0xffffffffffffffff, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa88888888);
    atomic64Ret = LOS_Atomic64DecRet(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x5aaaaaaa88888887, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaaa88888887, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x5aaaaaaa00000000);
    atomic64Ret = LOS_Atomic64DecRet(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0x5aaaaaa9ffffffff, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0x5aaaaaa9ffffffff, read64Counter);

    LOS_Atomic64Set(&atomic64TestCounter, 0x0000000000000000);
    atomic64Ret = LOS_Atomic64DecRet(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(atomic64Ret, 0xffffffffffffffff, atomic64Ret);
    read64Counter = LOS_Atomic64Read(&atomic64TestCounter);
    ICUNIT_ASSERT_EQUAL_VOID(read64Counter, 0xffffffffffffffff, read64Counter);

    ++g_testCount;
}

static UINT32 TestCase(VOID)
{
    UINT32 ret;
    g_testCount = 0;

    TSK_INIT_PARAM_S stTask1 = {0};
    stTask1.pfnTaskEntry = (TSK_ENTRY_FUNC)TaskF01;
    stTask1.pcName       = "Atomic_011";
    stTask1.uwStackSize  = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    stTask1.usTaskPrio   = TASK_PRIO_TEST - 2; // TASK_PRIO_TEST - 2 has higher priority than TASK_PRIO_TEST
    stTask1.uwResved     = LOS_TASK_STATUS_DETACHED;

    ret = LOS_TaskCreate(&g_testTaskID01, &stTask1);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    LOS_TaskDelay(20); // delay 20 ticks.

    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);
EXIT:
    return LOS_OK;
}

VOID ItLosAtomic011(VOID)
{
    TEST_ADD_CASE("ItLosAtomic011", TestCase, TEST_LOS, TEST_ATO, TEST_LEVEL0, TEST_FUNCTION);
}
