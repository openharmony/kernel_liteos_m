#include "It_posix_mutex.h"

static pthread_mutex_t g_mux054;
static volatile int g_mux054Step = 0;

static VOID *PrioProtectTask(VOID *arg)
{
    int ret;
    UINT16 prio;

    ret = pthread_mutex_lock(&g_mux054);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_HIGH_PRIO, prio, EXIT_UNLOCK);

    g_mux054Step = 1;

EXIT_UNLOCK:
    pthread_mutex_unlock(&g_mux054);

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
    pthread_mutexattr_t mattr;

    ret = pthread_mutexattr_init(&mattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_PROTECT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_setprioceiling(&mattr, MUTEX_TEST_HIGH_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&g_mux054, &mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = PosixPthreadInit(&attr, MUTEX_TEST_LOW_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);

    ret = pthread_create(&thread, &attr, PrioProtectTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR2);

    PosixPthreadDestroy(&attr, thread);

    pthread_mutex_destroy(&g_mux054);
    pthread_mutexattr_destroy(&mattr);
    return LOS_OK;

EXIT_ATTR2:
    pthread_attr_destroy(&attr);
EXIT_MUTEX:
    pthread_mutex_destroy(&g_mux054);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattr);
    return LOS_NOK;
}

VOID ItPosixMux054(void)
{
    TEST_ADD_CASE("ItPosixMux054", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
