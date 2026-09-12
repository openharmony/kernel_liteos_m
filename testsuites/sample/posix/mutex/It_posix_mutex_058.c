#include "It_posix_mutex.h"

static pthread_mutex_t g_mux058;
static volatile int g_mux058Step = 0;
static volatile int g_mux058MidRanBeforeUnlock = 0;

static VOID *PrioProtectSchedMidTask(VOID *arg)
{
    if (g_mux058Step < 2) {
        g_mux058MidRanBeforeUnlock = 1;
    }
    return NULL;
}

static VOID *PrioProtectSchedLowTask(VOID *arg)
{
    int ret;
    UINT16 prio;

    ret = pthread_mutex_lock(&g_mux058);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_HIGH_PRIO, prio, EXIT_UNLOCK);

    g_mux058Step = 1;
    LOS_TaskDelay(1);

    g_mux058Step = 2;
    pthread_mutex_unlock(&g_mux058);
    LOS_TaskDelay(1);

    ICUNIT_GOTO_EQUAL(g_mux058MidRanBeforeUnlock, 0, g_mux058MidRanBeforeUnlock, EXIT);

    prio = LOS_TaskPriGet(LOS_CurTaskIDGet());
    ICUNIT_GOTO_EQUAL(prio, MUTEX_TEST_LOW_PRIO, prio, EXIT);

EXIT:
    return NULL;

EXIT_UNLOCK:
    pthread_mutex_unlock(&g_mux058);
    goto EXIT;
}

static UINT32 Testcase(VOID)
{
    int ret;
    pthread_t lowThread, midThread;
    pthread_attr_t lowAttr, midAttr;
    pthread_mutexattr_t mattr;

    g_mux058Step = 0;
    g_mux058MidRanBeforeUnlock = 0;

    ret = pthread_mutexattr_init(&mattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_PROTECT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ret = pthread_mutexattr_setprioceiling(&mattr, MUTEX_TEST_HIGH_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&g_mux058, &mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = PosixPthreadInit(&lowAttr, MUTEX_TEST_LOW_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);
    ret = PosixPthreadInit(&midAttr, MUTEX_TEST_DEFAULT_PRIO);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_LOW_ATTR);

    ret = pthread_create(&lowThread, &lowAttr, PrioProtectSchedLowTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MID_ATTR);

    LOS_TaskDelay(1);

    ret = pthread_create(&midThread, &midAttr, PrioProtectSchedMidTask, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MID_ATTR);

    PosixPthreadDestroy(&midAttr, midThread);
    PosixPthreadDestroy(&lowAttr, lowThread);

    pthread_mutex_destroy(&g_mux058);
    pthread_mutexattr_destroy(&mattr);
    return LOS_OK;

EXIT_MID_ATTR:
    pthread_attr_destroy(&midAttr);
EXIT_LOW_ATTR:
    pthread_attr_destroy(&lowAttr);
EXIT_MUTEX:
    pthread_mutex_destroy(&g_mux058);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattr);
    return LOS_NOK;
}

VOID ItPosixMux058(void)
{
    TEST_ADD_CASE("ItPosixMux058", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
