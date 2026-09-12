#include "It_posix_mutex.h"
#include "time.h"

static pthread_mutex_t g_mux076;
static volatile int g_mux076Step = 0;

static void *HolderTask076(void *arg)
{
    int ret;

    ret = pthread_mutex_lock(&g_mux076);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_mux076Step = 1;
    LOS_TaskDelay(10);

    g_mux076Step = 2;
    pthread_mutex_unlock(&g_mux076);
EXIT:
    return NULL;
}

static UINT32 Testcase(void)
{
    int ret;
    pthread_t holderThread;
    pthread_attr_t holderAttr;
    struct sched_param sp;
    struct timespec ts;

    g_mux076Step = 0;

    ret = pthread_mutex_init(&g_mux076, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_init(&holderAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    sp.sched_priority = MUTEX_TEST_LOW_PRIO;
    pthread_attr_setschedparam(&holderAttr, &sp);
    pthread_attr_setinheritsched(&holderAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&holderThread, &holderAttr, HolderTask076, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);

    LOS_TaskDelay(1);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 5000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }
    ret = pthread_mutex_timedlock(&g_mux076, &ts);
    ICUNIT_GOTO_EQUAL(ret, ETIMEDOUT, ret, EXIT_JOIN);

    while (g_mux076Step < 2) {
        LOS_TaskDelay(1);
    }

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += 2;
    ret = pthread_mutex_timedlock(&g_mux076, &ts);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);
    pthread_mutex_unlock(&g_mux076);

EXIT_JOIN:
    PosixPthreadDestroy(&holderAttr, holderThread);
    pthread_mutex_destroy(&g_mux076);
    return LOS_OK;

EXIT_MUTEX:
    pthread_attr_destroy(&holderAttr);
    pthread_mutex_destroy(&g_mux076);
    return LOS_NOK;
}

VOID ItPosixMux076(void)
{
    TEST_ADD_CASE("ItPosixMux076", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
