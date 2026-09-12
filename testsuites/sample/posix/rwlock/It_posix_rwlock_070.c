#include "It_posix_rwlock.h"
#include "time.h"

static pthread_rwlock_t g_rwl070;
static volatile int g_rwl070Step = 0;

static void *BlockingWriter070(void *arg)
{
    int ret;

    ret = pthread_rwlock_wrlock(&g_rwl070);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_rwl070Step = 1;
    LOS_TaskDelay(10);

    g_rwl070Step = 2;
    pthread_rwlock_unlock(&g_rwl070);
EXIT:
    return NULL;
}

static UINT32 Testcase(void)
{
    int ret;
    pthread_t writerThread;
    pthread_attr_t writerAttr;
    struct sched_param sp;
    struct timespec ts;
    int loopCount = 0;

    g_rwl070Step = 0;

    ret = pthread_rwlock_init(&g_rwl070, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_init(&writerAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    sp.sched_priority = RWLOCK_TEST_HIGH_PRIO;
    pthread_attr_setschedparam(&writerAttr, &sp);
    pthread_attr_setinheritsched(&writerAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&writerThread, &writerAttr, BlockingWriter070, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    LOS_TaskDelay(1);

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += 10000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }
    ret = pthread_rwlock_timedrdlock(&g_rwl070, &ts);
    ICUNIT_GOTO_EQUAL(ret, ETIMEDOUT, ret, EXIT_JOIN);

    while (g_rwl070Step < 2 && loopCount < 100) {
        LOS_TaskDelay(1);
        loopCount++;
    }

EXIT_JOIN:
    PosixPthreadDestroy(&writerAttr, writerThread);
    pthread_rwlock_destroy(&g_rwl070);
    return LOS_OK;

EXIT_ATTR:
    pthread_attr_destroy(&writerAttr);
    pthread_rwlock_destroy(&g_rwl070);
    return LOS_NOK;
}

VOID ItPosixRwlock070(void)
{
    TEST_ADD_CASE("ItPosixRwlock070", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
