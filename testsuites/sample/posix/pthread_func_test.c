/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
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

#undef  _XOPEN_SOURCE
#define _XOPEN_SOURCE 600

#include <securec.h>
#include "osTest.h"
#include "pthread.h"
#include "time.h"
#include <unistd.h>

#undef TASK_PRIO_TEST
#define TASK_PRIO_TEST           LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO
#define PTHREAD_TASK_DELAY       10

static INT32 g_pthreadSem = 0;

#define TEST_STR(func) ItLos##func
#define TEST_TO_STR(x) #x
#define TEST_HEAD_TO_STR(x) TEST_TO_STR(x)
/* POSIX 兼容用例: layer=TEST_LIB, module=TEST_POSIX */
#define ADD_TEST_CASE(func) \
    TEST_ADD_CASE(TEST_HEAD_TO_STR(TEST_STR(func)), func, TEST_LIB, TEST_POSIX, TEST_LEVEL0, TEST_FUNCTION)

#define Function   0
#define MediumTest 0
#define Level1     0
#define LITE_TEST_CASE(module, function, flag) static int function(void)

static VOID *PthreadJoinF01(void *argument)
{
    g_testCount++;

    pthread_exit((void *)8); /* 8: pthread exit code */
    return (void *)9; /* 9: return val */
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_001
 * @tc.name      : event operation for join
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread001, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;
    UINTPTR uwtemp = 1;

    g_testCount = 0;

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh, &attr, PthreadJoinF01, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, (void **)&uwtemp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(uwtemp, 8, uwtemp); /* 8: pthread exit code */

    return LOS_OK;
};

static VOID *PthreadJoinF02(void *argument)
{
    g_testCount++;

    return (void *)9; /* 9: return val */
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_002
 * @tc.name      : event operation for join
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread002, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;
    UINTPTR uwtemp = 1;

    g_testCount = 0;

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh, &attr, PthreadJoinF02, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, (void **)&uwtemp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(uwtemp, 9, uwtemp); /* 9: pthread exit code */

    return LOS_OK;
};

static VOID *PthreadJoinF03(void *argument)
{
    int ret = pthread_detach(pthread_self());
    ICUNIT_GOTO_EQUAL(ret, ESRCH, ret, EXIT);

    g_testCount++;
EXIT:
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_003
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread003, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    g_testCount = 0;

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST + 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh, &attr, PthreadJoinF03, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    return LOS_OK;
};

static VOID *PthreadJoinF04(void *argument)
{
    int ret = pthread_detach(pthread_self());
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_testCount++;
EXIT:
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_004
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread004, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    g_testCount = 0;

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST - 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh, &attr, PthreadJoinF04, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    ret = pthread_join(newTh, NULL);
    ICUNIT_ASSERT_EQUAL(ret, ESRCH, ret);

    return LOS_OK;
};

static VOID *PthreadJoinF05(void *argument)
{
    int ret = pthread_detach(pthread_self());
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    usleep(100000); /* 100000: sleep 100 ms */
EXIT:
    g_testCount++;
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_005
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread005, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    g_testCount = 0;

    ret = pthread_attr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    schedParam.sched_priority = TASK_PRIO_TEST - 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_create(&newTh, &attr, PthreadJoinF05, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_join(newTh, NULL);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_ATTR);

    /* Wait for the detached thread to finish and be auto-reaped. */
    while (g_testCount == 0) {
        usleep(10000); /* 10000: poll every 10 ms */
    }
    usleep(100000); /* 100000: give the scheduler enough time to reap the exited task and free TCB/stack. */

    (VOID)pthread_attr_destroy(&attr);
    return LOS_OK;

EXIT_ATTR:
    (VOID)pthread_attr_destroy(&attr);
EXIT:
    return LOS_NOK;
};

static pthread_cond_t g_pthread_cond;
static pthread_mutex_t g_pthread_mutex;
#define TEST_THREAD_COUNT 5
static void *PthreadCondFunc001(void *arg)
{
    int ret;
    struct timespec ts;

    g_testCount++;

    ret = pthread_mutex_lock(&g_pthread_mutex);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 60; /* 60: wait 1 minute */

    ret = pthread_cond_timedwait(&g_pthread_cond, &g_pthread_mutex, &ts);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_mutex_unlock(&g_pthread_mutex);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_testCount++;
EXIT:
    return NULL;
}

static VOID *PthreadFunc06(void *argument)
{
    int policy;
    int ret;
    int i;
    int createdCount = 0;
    pthread_attr_t attr;
    struct sched_param schedParam = { 0 };
    pthread_t thread[TEST_THREAD_COUNT];

    g_testCount = 0;

    ret = pthread_attr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_getschedparam(pthread_self(), &policy, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    schedParam.sched_priority -= 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_create(&thread[i], &attr, PthreadCondFunc001, NULL);
        if (ret != 0) {
            ICunitSaveErr(__LINE__, (iiUINT32)ret);
            goto EXIT;
        }
        createdCount++;
    }

    ICUNIT_GOTO_EQUAL(g_testCount, 5, g_testCount, EXIT); /* 5: Five threads */

    ret = pthread_mutex_lock(&g_pthread_mutex);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_cond_broadcast(&g_pthread_cond);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_mutex_unlock(&g_pthread_mutex);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ICUNIT_GOTO_EQUAL(g_testCount, 10, g_testCount, EXIT); /* 10: Twice per thread */

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_join(thread[i], NULL);
        ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    }

EXIT:
    for (i = 0; i < createdCount; i++) {
        pthread_join(thread[i], NULL);
    }
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_006
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread006, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    ret = pthread_mutex_init(&g_pthread_mutex, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_cond_init(&g_pthread_cond, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);

    ret = pthread_attr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_COND);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    schedParam.sched_priority = TASK_PRIO_TEST - 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_create(&newTh, &attr, PthreadFunc06, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_join(newTh, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_cond_destroy(&g_pthread_cond);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);

    ret = pthread_mutex_destroy(&g_pthread_mutex);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);

    return LOS_OK;

EXIT_ATTR:
    (VOID)pthread_attr_destroy(&attr);
EXIT_COND:
    (VOID)pthread_cond_destroy(&g_pthread_cond);
EXIT_MUTEX:
    (VOID)pthread_mutex_destroy(&g_pthread_mutex);
EXIT:
    return LOS_NOK;
};

static void *PthreadCondFunc002(void *arg)
{
    int ret;
    struct timespec ts;
    g_testCount++;

    ret = pthread_mutex_lock(&g_pthread_mutex);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 1; /* 1: wait 1 s */

    ret = pthread_cond_timedwait(&g_pthread_cond, &g_pthread_mutex, &ts);
    ICUNIT_GOTO_EQUAL(ret, ETIMEDOUT, ret, EXIT);

    ret = pthread_mutex_unlock(&g_pthread_mutex);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_testCount++;

EXIT:
    return NULL;
}

static VOID *PthreadFunc07(void *argument)
{
    int policy;
    int ret;
    int i;
    int createdCount = 0;
    pthread_attr_t attr;
    struct sched_param schedParam = { 0 };
    pthread_t thread[TEST_THREAD_COUNT];

    g_testCount = 0;

    ret = pthread_mutex_init(&g_pthread_mutex, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_getschedparam(pthread_self(), &policy, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    schedParam.sched_priority -= 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_create(&thread[i], &attr, PthreadCondFunc002, NULL);
        if (ret != 0) {
            ICunitSaveErr(__LINE__, (iiUINT32)ret);
            goto EXIT;
        }
        createdCount++;
    }

    ICUNIT_GOTO_EQUAL(g_testCount, 5, g_testCount, EXIT); /* 5: Five threads */

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_join(thread[i], NULL);
        ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    }

    ICUNIT_GOTO_EQUAL(g_testCount, 10, g_testCount, EXIT); /* 10: Twice per thread */

EXIT:
    for (i = 0; i < createdCount; i++) {
        pthread_join(thread[i], NULL);
    }
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_007
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread007, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    ret = pthread_cond_init(&g_pthread_cond, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_COND);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    schedParam.sched_priority = TASK_PRIO_TEST - 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_create(&newTh, &attr, PthreadFunc07, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_join(newTh, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_cond_destroy(&g_pthread_cond);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);

    ret = pthread_mutex_destroy(&g_pthread_mutex);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);

    return LOS_OK;

EXIT_ATTR:
    (VOID)pthread_attr_destroy(&attr);
EXIT_COND:
    (VOID)pthread_cond_destroy(&g_pthread_cond);
EXIT:
    return LOS_NOK;
};

static int g_pthreadKey1;
static int g_pthreadKey2;
static void pthreadKeyFree(void *data)
{
    if (data != NULL) {
        free(data);
    }
}

static void *PthreadFunc08(void *arg)
{
#define TEST_KEY_SIZE 0x100
    int *data = NULL;
    int ret;
    int *result = NULL;

    data = (int *)malloc(TEST_KEY_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(data, NULL, data, EXIT);

    (void)memset_s(data, TEST_KEY_SIZE, 0, TEST_KEY_SIZE);
    *data = 100 + (int)pthread_self(); /* 100: test data */
    ret = pthread_setspecific(g_pthreadKey1, (void *)data);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    data = (int *)malloc(TEST_KEY_SIZE);
    ICUNIT_GOTO_NOT_EQUAL(data, NULL, data, EXIT);

    (void)memset_s(data, TEST_KEY_SIZE, 0, TEST_KEY_SIZE);
    *data = 200 + (int)pthread_self(); /* 200: test data */
    ret = pthread_setspecific(g_pthreadKey2, (void *)data);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    result = (int *)pthread_getspecific(g_pthreadKey1);
    ICUNIT_GOTO_NOT_EQUAL(result, NULL, result, EXIT);
    ICUNIT_GOTO_EQUAL(*result, 100 + (int)pthread_self(), *result, EXIT); /* 100: test data */

    result = (int *)pthread_getspecific(g_pthreadKey2);
    ICUNIT_GOTO_NOT_EQUAL(result, NULL, result, EXIT);
    ICUNIT_GOTO_EQUAL(*result, 200 + (int)pthread_self(), *result, EXIT); /* 200: test data */

    return NULL;
EXIT:
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_008
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread008, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh1, newTh2;
    struct sched_param schedParam = { 0 };
    int result = 0;
    UINT32 ret;

    ret = pthread_key_create((pthread_key_t *)&g_pthreadKey1, pthreadKeyFree);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_key_create((pthread_key_t *)&g_pthreadKey2, pthreadKeyFree);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST - 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh1, &attr, PthreadFunc08, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh2, &attr, PthreadFunc08, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh1, (void **)&result);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(result, 0, result);

    ret = pthread_join(newTh2, (void **)&result);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(result, 0, result);

    ret = pthread_key_delete(g_pthreadKey1);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_key_delete(g_pthreadKey2);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return LOS_OK;
};

static VOID *PthreadPrioFunc01(void *argument)
{
    g_testCount++;
    ICUNIT_TRACK_EQUAL(g_testCount, (UINT32)argument, g_testCount);
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_009
 * @tc.name      : event operation for set priority
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread009, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t thread[TEST_THREAD_COUNT];
    struct sched_param schedParam = { 0 };
    UINT32 ret;
    UINT32 i = 0;
    g_testCount = 0;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST + 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_create(&thread[i], &attr, PthreadPrioFunc01, (void *)(TEST_THREAD_COUNT - i));
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_setschedprio(thread[i], TASK_PRIO_TEST + TEST_THREAD_COUNT - i);
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_join(thread[i], NULL);
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }

    return LOS_OK;
};

static VOID PthreadOnceFunc01(void)
{
    g_testCount++;
    ICUNIT_TRACK_EQUAL(g_testCount, 1, g_testCount);
    return;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_010
 * @tc.name      : event operation for pthread_once
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread010, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    struct sched_param schedParam = { 0 };
    UINT32 ret;
    UINT32 i = 0;
    pthread_once_t onceControl = 0;
    g_testCount = 0;
    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST + 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_once(&onceControl, PthreadOnceFunc01);
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    return LOS_OK;
};

static VOID *PthreadCancelFunc01(void *argument)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    while (1) {
        g_testCount++;
        ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT);
        LOS_TaskDelay(PTHREAD_TASK_DELAY);
    }

EXIT:
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_011
 * @tc.name      : event operation for pthread_cancel
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread011, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t thread;
    struct sched_param schedParam = { 0 };
    UINT32 ret;
    g_testCount = 0;
    ret = pthread_attr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    schedParam.sched_priority = TASK_PRIO_TEST - 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_create(&thread, &attr, PthreadCancelFunc01, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_cancel(thread);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_THREAD);

    ICUNIT_GOTO_EQUAL(g_testCount, 1, g_testCount, EXIT_THREAD);

    ret = pthread_join(thread, NULL);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);

    (VOID)pthread_attr_destroy(&attr);
    return LOS_OK;

EXIT_THREAD:
    (VOID)pthread_join(thread, NULL);
EXIT_ATTR:
    (VOID)pthread_attr_destroy(&attr);
EXIT:
    return LOS_NOK;
};

static VOID *PthreadTestcancelFunc01(void *argument)
{
    INT32 ret = 0;

    ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);

    ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_testCount = 1;
    g_pthreadSem = 1;
    while (g_pthreadSem == 1) {
        LOS_TaskDelay(PTHREAD_TASK_DELAY);
    }

    LOS_TaskDelay(PTHREAD_TASK_DELAY);
    pthread_testcancel();
    g_testCount = -1;

EXIT:
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_012
 * @tc.name      : event operation for testcancel
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread012, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t thread;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    g_testCount = 0;
    g_pthreadSem = 0;

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST - 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&thread, &attr, PthreadTestcancelFunc01, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    while (g_pthreadSem == 0) {
        LOS_TaskDelay(PTHREAD_TASK_DELAY);
    }

    ret = pthread_cancel(thread);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    g_pthreadSem = 0;

    ret = pthread_join(thread, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    return LOS_OK;
};

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_013
 * @tc.name      : event operation for set/get clock
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread013, Function | MediumTest | Level1)
{
    INT32 ret;
    clockid_t clk;
    const int invalidClock = -100;
    pthread_condattr_t condattr;
    ret = pthread_condattr_init(&condattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_condattr_getclock(&condattr, &clk);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(clk, CLOCK_REALTIME, clk);

    ret = pthread_condattr_setclock(&condattr, CLOCK_REALTIME);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_condattr_getclock(&condattr, &clk);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(clk, CLOCK_REALTIME, clk);

    struct timespec ts = {0};
    ret = clock_getres(CLOCK_MONOTONIC, &ts);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_condattr_setclock(&condattr, CLOCK_MONOTONIC);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_condattr_getclock(&condattr, &clk);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(clk, CLOCK_MONOTONIC, clk);

    ret = pthread_condattr_setclock(&condattr, invalidClock);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_014
 * @tc.name      : event operation for setpshared
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread014, Function | MediumTest | Level1)
{
    INT32 ret;
    pthread_condattr_t attr;

    /* Initialize a cond attributes object */
    ret = pthread_condattr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_condattr_setpshared(&attr, (-100)); /* -100: Set 'pshared' to INVALID_PSHARED_VALUE. */
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    /* Destroy the cond attributes object */
    ret = pthread_condattr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_015
 * @tc.name      : event operation for getpshared
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread015, Function | MediumTest | Level1)
{
    INT32 ret;
    INT32 pshared;
    pthread_condattr_t attr;

    /* Initialize a cond attributes object */
    ret = pthread_condattr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    /* Set 'pshared' to INVALID_PSHARED_VALUE. */
    ret = pthread_condattr_getpshared(&attr, &pshared);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(pshared, PTHREAD_PROCESS_PRIVATE, pshared);

    /* Destroy the cond attributes object */
    ret = pthread_condattr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_016
 * @tc.name      : event operation for get/set mutex attr
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread016, Function | MediumTest | Level1)
{
    pthread_mutexattr_t mutexAttr;
    int mutexType;
    int ret;
    pthread_mutexattr_init(&mutexAttr);
    ret = pthread_mutexattr_settype(NULL, PTHREAD_MUTEX_ERRORCHECK);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_ERRORCHECK);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_gettype(&mutexAttr, &mutexType);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(mutexType, PTHREAD_MUTEX_ERRORCHECK, mutexType);

    ret = pthread_mutexattr_gettype(NULL, &mutexType);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);

    mutexAttr.type = 3; /* 3: Invalid type */
    ret = pthread_mutexattr_gettype(&mutexAttr, &mutexType);
    ICUNIT_ASSERT_NOT_EQUAL(ret, 0, ret);
    return LOS_OK;
}

static void *PthreadStackTest(void *argument)
{
    (void)argument;
    g_testCount++;
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_017
 * @tc.name      : set thread stack
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread017, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t thread;
    UINT32 ret;
    void *stackAddr = NULL;
    g_testCount = 0;

    stackAddr = LOS_MemAllocAlign(OS_TASK_STACK_ADDR, OS_TSK_TEST_STACK_SIZE, 8);
    ICUNIT_ASSERT_NOT_EQUAL(stackAddr, NULL, stackAddr);

    ret = pthread_attr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setstack(&attr, stackAddr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_create(&thread, &attr, PthreadStackTest, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_join(thread, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

EXIT:
    (VOID)LOS_MemFree(OS_TASK_STACK_ADDR, stackAddr);
    return LOS_OK;
};

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_018
 * @tc.name      : event operation for condattr setpshared parameter validation
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: pthread_condattr_setpshared 成功路径/SHARED 不支持/NULL attr 行为
 * 覆盖目标: lib/posix/src/pthread_cond.c:63-75
 *   :74      shared == PTHREAD_PROCESS_PRIVATE → 0(成功路径)
 *   :70-71   shared == PTHREAD_PROCESS_SHARED → ENOSYS(不支持进程间共享)
 *   NULL attr: 实现不解引用 attr(:65 (VOID)attr), 按 shared 值返回, 不崩溃
 *   (:66-68 的 EINVAL 分支已由 TestPthread014 覆盖)
 * getpshared 往返: setpshared(PRIVATE) 后 getpshared == PRIVATE(:52-61) */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread018, Function | MediumTest | Level1)
{
    INT32 ret;
    INT32 pshared = -1;
    pthread_condattr_t attr;

    /* 成功路径: PRIVATE */
    ret = pthread_condattr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    /* getpshared 往返 == PRIVATE */
    ret = pthread_condattr_getpshared(&attr, &pshared);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(pshared, PTHREAD_PROCESS_PRIVATE, pshared, EXIT);

    /* SHARED: 进程间共享不支持 → ENOSYS */
    ret = pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    ICUNIT_GOTO_EQUAL(ret, ENOSYS, ret, EXIT);

    /* NULL attr: 实现忽略 attr 指针, 按 shared 值返回, 不解引用不崩溃 */
    ret = pthread_condattr_setpshared(NULL, PTHREAD_PROCESS_PRIVATE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ret = pthread_condattr_setpshared(NULL, PTHREAD_PROCESS_SHARED);
    ICUNIT_GOTO_EQUAL(ret, ENOSYS, ret, EXIT);

EXIT:
    /* destroy 收尾: 任意断言失败均跳转至此, 确保 attr 资源释放 */
    ret = pthread_condattr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return 0;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_019
 * @tc.name      : pthread parameter validation for schedparam specific key cancelstate
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: getschedparam/getspecific/key_delete/setcancelstate 参数校验集+正常路径回读
 * 覆盖目标: lib/posix/src/pthread.c
 *   :415-418  getschedparam 非法线程 id(超任务数上限, IsPthread 判假) → EINVAL
 *   :420-422  getschedparam policy 或 param 传 NULL → EINVAL
 *   :429-432  getschedparam 正常路径: 策略恒 SCHED_RR, 优先级回写
 *   :815-817  getspecific key 越界(>= PTHREAD_KEYS_MAX=128) → 返回 NULL(不置 errno)
 *   :822-824  getspecific 范围内未使用 key(本线程未调过 setspecific, key 数组
 *             未分配/槽位 UNUSED) → 返回 NULL
 *   :733-735  key_delete key 越界 → EINVAL
 *   :738-741  key_delete 重复删除(槽位已 UNUSED) → EAGAIN
 *   :743-757  key_delete 正常路径: 置 UNUSED 并遍历清理各线程槽位
 *   :356-358  setcancelstate 非法状态值(5 非 ENABLE/DISABLE) → EINVAL
 *   :368-372  setcancelstate 正常路径: oldState 回读改前状态 */
LITE_TEST_CASE(PthreadFuncTestSuite, TestPthread019, Function | MediumTest | Level1)
{
    pthread_key_t key = (pthread_key_t)128; /* 128: PTHREAD_KEYS_MAX, 越界哨兵值, EXIT 补删无害 */
    pthread_t invalidThread;
    struct sched_param schedParam = { 0 };
    void *specific = NULL;
    int policy = 0;
    int oldState = -1;
    int ret;

    /* 1. getschedparam: 非法线程 id(超过任务数上限, IsPthread 判假, pthread.c:415-418) → EINVAL */
    invalidThread = (pthread_t)(LOSCFG_BASE_CORE_TSK_LIMIT + 1);
    ret = pthread_getschedparam(invalidThread, &policy, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    /* 2. getschedparam: policy 传 NULL(pthread.c:420-422) → EINVAL */
    ret = pthread_getschedparam(pthread_self(), NULL, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    /* 3. getschedparam 正常路径(pthread.c:429-432): 调度策略恒 SCHED_RR */
    ret = pthread_getschedparam(pthread_self(), &policy, &schedParam);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(policy, SCHED_RR, policy, EXIT);

    /* 4. getspecific: key 越界(200 >= PTHREAD_KEYS_MAX=128, pthread.c:815-817)
     *    → 返回 NULL, POSIX 语义该函数无错误返回不置 errno */
    specific = pthread_getspecific((pthread_key_t)200);
    ICUNIT_GOTO_EQUAL((UINTPTR)specific, (UINTPTR)NULL, 0, EXIT);

    /* 5. getspecific: 范围内未使用 key 99(本线程未调过 setspecific,
     *    key 数组未分配, pthread.c:822-824) → 返回 NULL */
    specific = pthread_getspecific((pthread_key_t)99);
    ICUNIT_GOTO_EQUAL((UINTPTR)specific, (UINTPTR)NULL, 0, EXIT);

    /* 6. key_delete: key 越界(pthread.c:733-735) → EINVAL */
    ret = pthread_key_delete((pthread_key_t)200);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    /* 7. key 正常创建后删除(pthread.c:743-757 正常路径, 无析构器不触发回调) */
    ret = pthread_key_create(&key, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ret = pthread_key_delete(key);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    /* 8. key_delete 重复删除: 槽位已 UNUSED(pthread.c:738-741) → EAGAIN */
    ret = pthread_key_delete(key);
    ICUNIT_GOTO_EQUAL(ret, EAGAIN, ret, EXIT);

    /* 9. setcancelstate: 非法状态值 5(非 ENABLE/DISABLE, pthread.c:356-358) → EINVAL */
    ret = pthread_setcancelstate(5, NULL);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    /* 10. setcancelstate 正常路径(pthread.c:368-372): 先恢复默认 ENABLE 保证基线,
     *     置 DISABLE 回读旧值应为 ENABLE, 再恢复 ENABLE 回读旧值应为 DISABLE */
    ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ret = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(oldState, PTHREAD_CANCEL_ENABLE, oldState, EXIT);
    ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(oldState, PTHREAD_CANCEL_DISABLE, oldState, EXIT);

    return LOS_OK;

EXIT:
    /* 恢复: 取消状态回默认 ENABLE; key 哨兵值越界补删返回 EINVAL 无害, 已建未删则真删 */
    (VOID)pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    (VOID)pthread_key_delete(key);
    return LOS_NOK;
}

static void PosixTestCase(void)
{
    ADD_TEST_CASE(TestPthread001);
    ADD_TEST_CASE(TestPthread002);
    ADD_TEST_CASE(TestPthread003);
    ADD_TEST_CASE(TestPthread004);
    ADD_TEST_CASE(TestPthread005);
    ADD_TEST_CASE(TestPthread006);
    ADD_TEST_CASE(TestPthread007);
    ADD_TEST_CASE(TestPthread008);
    ADD_TEST_CASE(TestPthread009);
    ADD_TEST_CASE(TestPthread010);
    ADD_TEST_CASE(TestPthread011);
    ADD_TEST_CASE(TestPthread012);
    ADD_TEST_CASE(TestPthread013);
    ADD_TEST_CASE(TestPthread014);
    ADD_TEST_CASE(TestPthread015);
    ADD_TEST_CASE(TestPthread016);
    ADD_TEST_CASE(TestPthread017);
    ADD_TEST_CASE(TestPthread018);
    ADD_TEST_CASE(TestPthread019);
    return;
}

static void *PosixTestThread(void *arg)
{
    PosixTestCase();
    return NULL;
}
int PthreadFuncTestSuite(void)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh, &attr, PosixTestThread, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    pthread_join(newTh, NULL);
    return 0;
}

