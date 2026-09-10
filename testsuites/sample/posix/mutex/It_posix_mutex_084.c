#include "It_posix_mutex.h"

static pthread_mutex_t g_mux084;
static volatile int g_mux084Step = 0;
static volatile int g_mux084HolderPrio = -1;
static volatile int g_mux084WaiterPending = 0;

static void *HighPrioWaiter084(void *arg)
{
    int ret;
    g_mux084WaiterPending = 1;

    ret = pthread_mutex_lock(&g_mux084);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    pthread_mutex_unlock(&g_mux084);
EXIT:
    return NULL;
}

static void *LowPrioHolder084(void *arg)
{
    int ret;
    UINT32 myTaskID = LOS_CurTaskIDGet();

    ret = pthread_mutex_lock(&g_mux084);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_mux084Step = 1;

    TEST_DELAY(g_mux084WaiterPending, 1, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_mux084WaiterPending, 1, g_mux084WaiterPending, EXIT);

    g_mux084HolderPrio = (int)LOS_TaskPriGet(myTaskID);

    g_mux084Step = 2;
    pthread_mutex_unlock(&g_mux084);
EXIT:
    return NULL;
}

static UINT32 Testcase(void)
{
    int ret;
    pthread_t holderThread, waiterThread;
    pthread_attr_t holderAttr, waiterAttr;
    struct sched_param sp;
    pthread_mutexattr_t muxAttr;

    g_mux084Step = 0;
    g_mux084HolderPrio = -1;
    g_mux084WaiterPending = 0;

    ret = pthread_mutexattr_init(&muxAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprotocol(&muxAttr, PTHREAD_PRIO_INHERIT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutex_init(&g_mux084, &muxAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    (VOID)pthread_mutexattr_destroy(&muxAttr);

    ret = pthread_attr_init(&holderAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    sp.sched_priority = MUTEX_TEST_LOW_PRIO;
    pthread_attr_setschedparam(&holderAttr, &sp);
    pthread_attr_setinheritsched(&holderAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&holderThread, &holderAttr, LowPrioHolder084, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    TEST_DELAY(g_mux084Step, 1, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_mux084Step, 1, g_mux084Step, EXIT_HOLDER);

    LOS_TaskDelay(1);

    ret = pthread_attr_init(&waiterAttr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_HOLDER);
    sp.sched_priority = MUTEX_TEST_HIGH_PRIO;
    pthread_attr_setschedparam(&waiterAttr, &sp);
    pthread_attr_setinheritsched(&waiterAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&waiterThread, &waiterAttr, HighPrioWaiter084, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_WAITER_ATTR);

    TEST_DELAY(g_mux084Step, 2, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_mux084Step, 2, g_mux084Step, EXIT_JOIN);

    ICUNIT_GOTO_EQUAL(g_mux084HolderPrio, MUTEX_TEST_HIGH_PRIO, g_mux084HolderPrio, EXIT_JOIN);

EXIT_JOIN:
    PosixPthreadDestroy(&waiterAttr, waiterThread);
EXIT_WAITER_ATTR:
    pthread_attr_destroy(&waiterAttr);
EXIT_HOLDER:
    PosixPthreadDestroy(&holderAttr, holderThread);
EXIT_ATTR:
    pthread_attr_destroy(&holderAttr);
    pthread_mutex_destroy(&g_mux084);
    return LOS_OK;
}

VOID ItPosixMux084(void)
{
    TEST_ADD_CASE("ItPosixMux084", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
