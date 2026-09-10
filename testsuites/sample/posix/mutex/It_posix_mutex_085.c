#include "It_posix_mutex.h"

static pthread_mutex_t g_mux085;
static volatile int g_mux085Step = 0;
static volatile int g_mux085HolderPrioBoosted = -1;
static volatile int g_mux085HolderPrioRestored = -1;
static volatile int g_mux085WaiterPending = 0;

static void *HighPrioWaiter085(void *arg)
{
    int ret;
    g_mux085WaiterPending = 1;

    ret = pthread_mutex_lock(&g_mux085);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    pthread_mutex_unlock(&g_mux085);
EXIT:
    return NULL;
}

static void *LowPrioHolder085(void *arg)
{
    int ret;

    ret = pthread_mutex_lock(&g_mux085);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_mux085Step = 1;

    TEST_DELAY(g_mux085WaiterPending, 1, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_mux085WaiterPending, 1, g_mux085WaiterPending, EXIT);

    g_mux085HolderPrioBoosted = (int)LOS_TaskPriGet(LOS_CurTaskIDGet());

    g_mux085Step = 2;
    pthread_mutex_unlock(&g_mux085);

    LOS_TaskDelay(3);

    g_mux085HolderPrioRestored = (int)LOS_TaskPriGet(LOS_CurTaskIDGet());

    g_mux085Step = 3;
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

    g_mux085Step = 0;
    g_mux085HolderPrioBoosted = -1;
    g_mux085HolderPrioRestored = -1;
    g_mux085WaiterPending = 0;

    ret = pthread_mutexattr_init(&muxAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprotocol(&muxAttr, PTHREAD_PRIO_INHERIT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutex_init(&g_mux085, &muxAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    (VOID)pthread_mutexattr_destroy(&muxAttr);

    ret = pthread_attr_init(&holderAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    sp.sched_priority = MUTEX_TEST_LOW_PRIO;
    pthread_attr_setschedparam(&holderAttr, &sp);
    pthread_attr_setinheritsched(&holderAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&holderThread, &holderAttr, LowPrioHolder085, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    TEST_DELAY(g_mux085Step, 1, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_mux085Step, 1, g_mux085Step, EXIT_HOLDER);

    LOS_TaskDelay(1);

    ret = pthread_attr_init(&waiterAttr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_HOLDER);
    sp.sched_priority = MUTEX_TEST_HIGH_PRIO;
    pthread_attr_setschedparam(&waiterAttr, &sp);
    pthread_attr_setinheritsched(&waiterAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&waiterThread, &waiterAttr, HighPrioWaiter085, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_WAITER_ATTR);

    TEST_DELAY(g_mux085Step, 3, TEST_WAIT_TIMEOUT);
    ICUNIT_GOTO_EQUAL(g_mux085Step, 3, g_mux085Step, EXIT_JOIN);

    ICUNIT_GOTO_EQUAL(g_mux085HolderPrioBoosted, MUTEX_TEST_HIGH_PRIO, g_mux085HolderPrioBoosted, EXIT_JOIN);
    ICUNIT_GOTO_EQUAL(g_mux085HolderPrioRestored, MUTEX_TEST_LOW_PRIO, g_mux085HolderPrioRestored, EXIT_JOIN);

EXIT_JOIN:
    PosixPthreadDestroy(&waiterAttr, waiterThread);
EXIT_WAITER_ATTR:
    pthread_attr_destroy(&waiterAttr);
EXIT_HOLDER:
    PosixPthreadDestroy(&holderAttr, holderThread);
EXIT_ATTR:
    pthread_attr_destroy(&holderAttr);
    pthread_mutex_destroy(&g_mux085);
    return LOS_OK;
}

VOID ItPosixMux085(void)
{
    TEST_ADD_CASE("ItPosixMux085", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
