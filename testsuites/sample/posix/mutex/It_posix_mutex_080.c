#include "It_posix_mutex.h"

static pthread_mutex_t g_mux080;
static volatile int g_mux080Step = 0;

static void *NonOwnerTask080(void *arg)
{
    int ret;

    while (g_mux080Step < 1) {
        LOS_TaskDelay(1);
    }

    ret = pthread_mutex_unlock(&g_mux080);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    g_mux080Step = 2;
EXIT:
    return NULL;
}

static UINT32 Testcase(void)
{
    int ret;
    pthread_t nonOwnerThread;
    pthread_attr_t attr;
    struct sched_param sp;

    g_mux080Step = 0;

    ret = pthread_mutex_init(&g_mux080, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    sp.sched_priority = MUTEX_TEST_LOW_PRIO;
    pthread_attr_setschedparam(&attr, &sp);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&nonOwnerThread, &attr, NonOwnerTask080, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);

    ret = pthread_mutex_lock(&g_mux080);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_THREAD);

    g_mux080Step = 1;
    LOS_TaskDelay(2);

    ICUNIT_GOTO_EQUAL(g_mux080Step, 2, g_mux080Step, EXIT_UNLOCK);

EXIT_UNLOCK:
    pthread_mutex_unlock(&g_mux080);
EXIT_THREAD:
    PosixPthreadDestroy(&attr, nonOwnerThread);
    pthread_mutex_destroy(&g_mux080);
    return LOS_OK;

EXIT_MUTEX:
    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&g_mux080);
    return LOS_NOK;
}

VOID ItPosixMux080(void)
{
    TEST_ADD_CASE("ItPosixMux080", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
