#include "It_posix_mutex.h"

static pthread_mutex_t g_mux057;
static volatile int g_mux057Step = 0;
static volatile UINT16 g_mux057LowPrioWhenHighWaits = 0;

static VOID *PrioInheritHighTask(VOID *arg)
{
    int ret;

    while (g_mux057Step < 1) {
        LOS_TaskDelay(1);
    }

    ret = pthread_mutex_lock(&g_mux057);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_mux057Step = 3;

EXIT:
    pthread_mutex_unlock(&g_mux057);
    return NULL;
}

static VOID *PrioInheritMidTask(VOID *arg)
{
    int ret;

    while (g_mux057Step < 1) {
        LOS_TaskDelay(1);
    }

    ret = pthread_mutex_lock(&g_mux057);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_mux057Step = 4;

EXIT:
    pthread_mutex_unlock(&g_mux057);
    return NULL;
}

static VOID *PrioInheritLowTask(VOID *arg)
{
    int ret;

    ret = pthread_mutex_lock(&g_mux057);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_mux057Step = 1;
    LOS_TaskDelay(20);

    g_mux057LowPrioWhenHighWaits = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(g_mux057LowPrioWhenHighWaits, MUTEX_TEST_HIGH_PRIO, g_mux057LowPrioWhenHighWaits, EXIT_UNLOCK);

    g_mux057Step = 2;

EXIT_UNLOCK:
    pthread_mutex_unlock(&g_mux057);
EXIT:
    return NULL;
}

static UINT32 Testcase(VOID)
{
    int ret;
    pthread_t lowThread, midThread, highThread;
    pthread_attr_t lowAttr, midAttr, highAttr;
    pthread_mutexattr_t mattr;

    g_mux057Step = 0;
    g_mux057LowPrioWhenHighWaits = 0;

    ret = pthread_mutexattr_init(&mattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&g_mux057, &mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = PosixPthreadInit(&lowAttr, MUTEX_TEST_LOW_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);
    ret = PosixPthreadInit(&midAttr, MUTEX_TEST_DEFAULT_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_LOW_ATTR);
    ret = PosixPthreadInit(&highAttr, MUTEX_TEST_HIGH_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MID_ATTR);

    ret = pthread_create(&lowThread, &lowAttr, PrioInheritLowTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_HIGH_ATTR);
    LOS_TaskDelay(1);

    ret = pthread_create(&highThread, &highAttr, PrioInheritHighTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_HIGH_ATTR);

    ret = pthread_create(&midThread, &midAttr, PrioInheritMidTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_HIGH_ATTR);

    PosixPthreadDestroy(&highAttr, highThread);
    PosixPthreadDestroy(&midAttr, midThread);
    PosixPthreadDestroy(&lowAttr, lowThread);

    ICUNIT_GOTO_EQUAL(g_mux057LowPrioWhenHighWaits, MUTEX_TEST_HIGH_PRIO, g_mux057LowPrioWhenHighWaits, EXIT_MUTEX);

    ret = pthread_mutex_destroy(&g_mux057);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_destroy(&mattr);
    ICUNIT_TRACK_EQUAL(ret, 0, ret);
    return LOS_OK;

EXIT_HIGH_ATTR:
    pthread_attr_destroy(&highAttr);
EXIT_MID_ATTR:
    pthread_attr_destroy(&midAttr);
EXIT_LOW_ATTR:
    pthread_attr_destroy(&lowAttr);
EXIT_MUTEX:
    pthread_mutex_destroy(&g_mux057);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattr);
    return LOS_NOK;
}

VOID ItPosixMux057(void)
{
    TEST_ADD_CASE("ItPosixMux057", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
