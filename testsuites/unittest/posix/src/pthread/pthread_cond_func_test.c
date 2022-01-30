/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "ohos_types.h"
#include <securec.h>
#include "hctest.h"
#include "los_config.h"
#include "cmsis_os2.h"
#include "common_test.h"
#include "kernel_test.h"
#include "pthread.h"
#include "log.h"

#define PTHREAD_TASK_DELAY       10
#define TASK_PRIO_TEST           LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO
#define OS_TSK_TEST_STACK_SIZE   0x1000
static UINT32 g_testCount;

/**
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is pthread
 * @param        : module name is pthread
 * @param        : test suit name is PthreadFuncTestSuite
 */
LITE_TEST_SUIT(Posix, Pthread, PthreadFuncTestSuite);

/**
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL PthreadFuncTestSuiteSetUp(void)
{
    return TRUE;
}

/**
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL PthreadFuncTestSuiteTearDown(void)
{
    printf("+-------------------------------------------+\n");
    return TRUE;
}

static VOID *pthread_join_f01(void *argument)
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
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread001, Function | MediumTest | Level1)
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

    ret = pthread_create(&newTh, &attr, pthread_join_f01, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, (void **)&uwtemp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(uwtemp, 8, uwtemp); /* 8: pthread exit code */

    return LOS_OK;
};

static VOID *pthread_join_f02(void *argument)
{
    g_testCount++;

    return (void *)9; /* 9: return val */
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_002
 * @tc.name      : event operation for join
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread002, Function | MediumTest | Level1)
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

    ret = pthread_create(&newTh, &attr, pthread_join_f02, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, (void **)&uwtemp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(uwtemp, 9, uwtemp); /* 9: pthread exit code */

    return LOS_OK;
};

static VOID *pthread_join_f03(void *argument)
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
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread003, Function | MediumTest | Level1)
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

    ret = pthread_create(&newTh, &attr, pthread_join_f03, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
    return LOS_OK;
};

static VOID *pthread_join_f04(void *argument)
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
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread004, Function | MediumTest | Level1)
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

    ret = pthread_create(&newTh, &attr, pthread_join_f04, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    ret = pthread_join(newTh, NULL);
    ICUNIT_ASSERT_EQUAL(ret, ESRCH, ret);

    return LOS_OK;
};

static VOID *pthread_join_f05(void *argument)
{
    int ret = pthread_detach(pthread_self());
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    usleep(100000); /* 100000: sleep 100 ms */
EXIT:
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_005
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread005, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setstacksize(&attr, OS_TSK_TEST_STACK_SIZE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    schedParam.sched_priority = TASK_PRIO_TEST - 1;
    ret = pthread_attr_setschedparam(&attr, &schedParam);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh, &attr, pthread_join_f05, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    return LOS_OK;
};

static pthread_cond_t g_pthread_cond;
static pthread_mutex_t g_pthread_mutex;
#define TEST_THREAD_COUNT 5
static void *pthread_cond_func001(void *arg)
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

static VOID *pthread_f06(void *argument)
{
    int policy;
    int ret;
    int i;
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
        ret = pthread_create(&thread[i], &attr, pthread_cond_func001, NULL);
        ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
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
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_006
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread006, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    ret = pthread_mutex_init(&g_pthread_mutex, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_cond_init(&g_pthread_cond, NULL);
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

    ret = pthread_create(&newTh, &attr, pthread_f06, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
};

static void *pthread_cond_func002(void *arg)
{
    int ret;
    struct timespec ts;

    g_testCount++;

    ret = pthread_mutex_lock(&g_pthread_mutex);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 60; /* 60: wait 1 minute */

    ret = pthread_cond_timedwait(&g_pthread_cond, &g_pthread_mutex, &ts);
    ICUNIT_GOTO_EQUAL(ret, ETIMEDOUT, ret, EXIT);

    ret = pthread_mutex_unlock(&g_pthread_mutex);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_testCount++;

EXIT:
    return NULL;
}

static VOID *pthread_f07(void *argument)
{
    int policy;
    int ret;
    int i;
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
        ret = pthread_create(&thread[i], &attr, pthread_cond_func002, NULL);
        ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    }

    ICUNIT_GOTO_EQUAL(g_testCount, 5, g_testCount, EXIT); /* 5: Five threads */

    for (i = 0; i < TEST_THREAD_COUNT; i++) {
        ret = pthread_join(thread[i], NULL);
        ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    }

    ICUNIT_GOTO_EQUAL(g_testCount, 10, g_testCount, EXIT); /* 10: Twice per thread */

EXIT:
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_007
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread007, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh;
    struct sched_param schedParam = { 0 };
    UINT32 ret;

    ret = pthread_mutex_init(&g_pthread_mutex, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_cond_init(&g_pthread_cond, NULL);
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

    ret = pthread_create(&newTh, &attr, pthread_f07, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_join(newTh, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
};

static int g_pthreadKey1;
static int g_pthreadKey2;
static void pthreadKeyFree(void *data)
{
    if (data != NULL) {
        free(data);
    }
}

static void *pthread_f08(void *arg)
{
#define TEST_KEY_SIZE 0x100
    int *data = (int *)malloc(TEST_KEY_SIZE);
    if (data == NULL) {
        return (void *)ENOMEM;
    }

    (void)memset_s(data, TEST_KEY_SIZE, 0, TEST_KEY_SIZE);
    *data = 100 + (int)pthread_self(); /* 100: test data */
    int ret = pthread_setspecific(g_pthreadKey1, (void *)data);
    if (ret != 0) {
        return (void *)ret;
    }

    data = (int *)malloc(TEST_KEY_SIZE);
    if (data == NULL) {
        return (void *)ENOMEM;
    }

    (void)memset_s(data, TEST_KEY_SIZE, 0, TEST_KEY_SIZE);
    *data = 200 + (int)pthread_self(); /* 200: test data */
    ret = pthread_setspecific(g_pthreadKey2, (void *)data);
    if (ret != 0) {
        return (void *)ret;
    }

    int *result = (int *)pthread_getspecific(g_pthreadKey1);
    if (result == NULL) {
        return (void *)EINVAL;
    }

    if (*result != (100 + (int)pthread_self())) { /* 100: test data */
        return (void *)EDEADLK;
    }

    result = (int *)pthread_getspecific(g_pthreadKey2);
    if (result == NULL) {
        return (void *)EINVAL;
    }

    if (*result != (200 + (int)pthread_self())) { /* 200: test data */
        return (void *)EDEADLK;
    }

    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_008
 * @tc.name      : event operation for deatch
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread008, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t newTh1, newTh2;
    struct sched_param schedParam = { 0 };
    int result = 0;
    UINT32 ret;

    ret = pthread_key_create(&g_pthreadKey1, pthreadKeyFree);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_key_create(&g_pthreadKey2, pthreadKeyFree);
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

    ret = pthread_create(&newTh1, &attr, pthread_f08, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_create(&newTh2, &attr, pthread_f08, NULL);
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

static VOID *pthread_prio_f01(void *argument)
{
    g_testCount++;
    ICUNIT_ASSERT_EQUAL(g_testCount, (UINT32)argument, g_testCount);
    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_009
 * @tc.name      : event operation for set priority
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread009, Function | MediumTest | Level1)
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
        ret = pthread_create(&thread[i], &attr, pthread_prio_f01, TEST_THREAD_COUNT - i);
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

static VOID pthread_once_f01(void)
{
    g_testCount++;
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_010
 * @tc.name      : event operation for pthread_once
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread010, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t thread[TEST_THREAD_COUNT];
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
        ret = pthread_once(&onceControl, pthread_once_f01);
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }
    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    return LOS_OK;
};

static VOID *pthread_cancel_f01(void *argument)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    while (1) {
        g_testCount++;
        ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);
        LOS_TaskDelay(PTHREAD_TASK_DELAY);
    }

    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_011
 * @tc.name      : event operation for pthread_cancel
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread011, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t thread;
    struct sched_param schedParam = { 0 };
    UINT32 ret;
    pthread_once_t onceControl = 0;
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
 
    ret = pthread_create(&thread, &attr, pthread_cancel_f01, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_cancel(thread);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(g_testCount, 1, g_testCount);

    return LOS_OK;
};

static VOID *pthread_testcancel_f01(void *argument)
{
    INT32 ret = 0;

    ret = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);

    ret = pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    g_testCount = 1;
    g_pthreadSem = 1;
    while (g_pthreadSem == 1) {
        LOS_TaskDelay(PTHREAD_TASK_DELAY);
    }

    LOS_TaskDelay(PTHREAD_TASK_DELAY);
    pthread_testcancel();
    g_testCount = -1;

    return NULL;
}

/**
 * @tc.number    : SUB_KERNEL_PTHREAD_OPERATION_012
 * @tc.name      : event operation for testcancel
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread012, Function | MediumTest | Level1)
{
    pthread_attr_t attr;
    pthread_t thread;
    struct sched_param schedParam = { 0 };
    UINT32 ret;
    pthread_once_t onceControl = 0;

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
    
    ret = pthread_create(&thread, &attr, pthread_testcancel_f01, NULL);
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
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread013, Function | MediumTest | Level1)
{
    INT32 ret;
    clockid_t clk;
    const int invalidClock = -100;
    pthread_condattr_t condattr;
    ret = pthread_condattr_init(&condattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_condattr_getclock(&condattr, &clk);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(clk, 0, clk);

    ret = pthread_condattr_setclock(&condattr, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_condattr_getclock(&condattr, &clk);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(clk, 0, clk);

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
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread014, Function | MediumTest | Level1)
{
    INT32 ret;
    pthread_condattr_t attr;

    /* Initialize a cond attributes object */
    ret = pthread_condattr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    /* Set 'pshared' to INVALID_PSHARED_VALUE. */
    ret = pthread_condattr_setpshared(&attr, (-100));
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
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread015, Function | MediumTest | Level1)
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
LITE_TEST_CASE(PthreadFuncTestSuite, testPthread016, Function | MediumTest | Level1)
{
    pthread_mutexattr_t mutex_attr;
    int mutex_type;
    int ret;
    pthread_mutexattr_init(&mutex_attr);
    ret = pthread_mutexattr_settype(NULL, PTHREAD_MUTEX_ERRORCHECK);
    if (ret == 0)
    {
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }
    pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
    pthread_mutexattr_gettype(&mutex_attr, &mutex_type);
    if (mutex_type != PTHREAD_MUTEX_ERRORCHECK) {
        ICUNIT_ASSERT_EQUAL(mutex_type, 0, mutex_type);
    }
    ret = pthread_mutexattr_gettype(NULL, &mutex_type);
    if (ret == 0) {
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }
    mutex_attr.type = 3;
    ret = pthread_mutexattr_gettype(&mutex_attr, &mutex_type);
    if (ret == 0) {
        ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    }
    return LOS_OK;
}

RUN_TEST_SUITE(PthreadFuncTestSuite);

