#include "It_posix_mutex.h"

static pthread_mutex_t g_mux056a;
static pthread_mutex_t g_mux056b;
static volatile int g_mux056Step = 0;

static VOID *PrioProtectNestedTask(VOID *arg)
{
    int ret;
    UINT16 prio;

    ret = pthread_mutex_lock(&g_mux056a);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_HIGH_PRIO - 2, prio, EXIT_UNLOCK_A);

    ret = pthread_mutex_lock(&g_mux056b);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_UNLOCK_A);

    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_HIGH_PRIO - 2, prio, EXIT_UNLOCK_ALL);

    g_mux056Step = 1;

EXIT_UNLOCK_ALL:
    pthread_mutex_unlock(&g_mux056b);
    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_HIGH_PRIO - 2, prio, EXIT_UNLOCK_A);

EXIT_UNLOCK_A:
    pthread_mutex_unlock(&g_mux056a);
    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_LOW_PRIO, prio, EXIT);

EXIT:
    return NULL;
}

static UINT32 Testcase(VOID)
{
    int ret;
    pthread_t thread;
    pthread_attr_t attr;
    pthread_mutexattr_t mattrA, mattrB;

    g_mux056Step = 0;

    ret = pthread_mutexattr_init(&mattrA);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprotocol(&mattrA, PTHREAD_PRIO_PROTECT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprioceiling(&mattrA, MUTEX_TEST_HIGH_PRIO - 2);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_init(&mattrB);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprotocol(&mattrB, PTHREAD_PRIO_PROTECT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprioceiling(&mattrB, MUTEX_TEST_HIGH_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&g_mux056a, &mattrA);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);
    ret = pthread_mutex_init(&g_mux056b, &mattrB);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUX_A);

    ret = PosixPthreadInit(&attr, MUTEX_TEST_LOW_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUX);

    ret = pthread_create(&thread, &attr, PrioProtectNestedTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR2);

    PosixPthreadDestroy(&attr, thread);

    pthread_mutex_destroy(&g_mux056b);
    pthread_mutex_destroy(&g_mux056a);
    pthread_mutexattr_destroy(&mattrB);
    pthread_mutexattr_destroy(&mattrA);
    return LOS_OK;

EXIT_ATTR2:
    pthread_attr_destroy(&attr);
EXIT_MUX:
    pthread_mutex_destroy(&g_mux056b);
EXIT_MUX_A:
    pthread_mutex_destroy(&g_mux056a);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattrB);
    pthread_mutexattr_destroy(&mattrA);
    return LOS_NOK;
}

VOID ItPosixMux056(void)
{
    TEST_ADD_CASE("ItPosixMux056", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
