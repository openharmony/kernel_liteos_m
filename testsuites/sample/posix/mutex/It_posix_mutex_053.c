#include "It_posix_mutex.h"

static pthread_mutex_t g_mux053;
static volatile int g_mux053Step = 0;

static VOID *PrioInheritLowTask(VOID *arg)
{
    int ret;
    UINT16 prio;

    ret = pthread_mutex_lock(&g_mux053);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_mux053Step = 1;
    LOS_TaskDelay(20);

    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_HIGH_PRIO, prio, EXIT_UNLOCK);

    g_mux053Step = 3;

EXIT_UNLOCK:
    pthread_mutex_unlock(&g_mux053);
EXIT:
    return NULL;
}

static VOID *PrioInheritHighTask(VOID *arg)
{
    int ret;

    while (g_mux053Step < 1) {
        LOS_TaskDelay(1);
    }

    ret = pthread_mutex_lock(&g_mux053);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_mux053Step = 4;

EXIT:
    pthread_mutex_unlock(&g_mux053);
    return NULL;
}

static UINT32 Testcase(VOID)
{
    int ret;
    pthread_t lowThread, highThread;
    pthread_attr_t lowAttr, highAttr;
    pthread_mutexattr_t mattr;

    g_mux053Step = 0;

    ret = pthread_mutexattr_init(&mattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_INHERIT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&g_mux053, &mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = PosixPthreadInit(&lowAttr, MUTEX_TEST_LOW_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);

    ret = PosixPthreadInit(&highAttr, MUTEX_TEST_HIGH_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_LOW_ATTR);

    ret = pthread_create(&lowThread, &lowAttr, PrioInheritLowTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_HIGH_ATTR);

    LOS_TaskDelay(1);

    ret = pthread_create(&highThread, &highAttr, PrioInheritHighTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_HIGH_ATTR);

    PosixPthreadDestroy(&highAttr, highThread);
    PosixPthreadDestroy(&lowAttr, lowThread);

    pthread_mutex_destroy(&g_mux053);
    pthread_mutexattr_destroy(&mattr);
    return LOS_OK;

EXIT_HIGH_ATTR:
    pthread_attr_destroy(&highAttr);
EXIT_LOW_ATTR:
    pthread_attr_destroy(&lowAttr);
EXIT_MUTEX:
    pthread_mutex_destroy(&g_mux053);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattr);
    return LOS_NOK;
}

VOID ItPosixMux053(void)
{
    TEST_ADD_CASE("ItPosixMux053", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
