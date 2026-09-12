#include "It_posix_mutex.h"

static pthread_mutex_t g_mux055;

static VOID *PrioNoneTask(VOID *arg)
{
    int ret;
    UINT16 prio;

    ret = pthread_mutex_lock(&g_mux055);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_LOW_PRIO, prio, EXIT_UNLOCK);

    TestExtraTaskDelay(2);

EXIT_UNLOCK:
    pthread_mutex_unlock(&g_mux055);
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

    ret = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_NONE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&g_mux055, &mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = PosixPthreadInit(&attr, MUTEX_TEST_LOW_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);

    ret = pthread_create(&thread, &attr, PrioNoneTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR2);

    PosixPthreadDestroy(&attr, thread);

    pthread_mutex_destroy(&g_mux055);
    pthread_mutexattr_destroy(&mattr);
    return LOS_OK;

EXIT_ATTR2:
    pthread_attr_destroy(&attr);
EXIT_MUTEX:
    pthread_mutex_destroy(&g_mux055);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattr);
    return LOS_NOK;
}

VOID ItPosixMux055(void)
{
    TEST_ADD_CASE("ItPosixMux055", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
