#include "It_posix_rwlock.h"

static pthread_rwlock_t g_rwl066;
static volatile int g_rwl066Step = 0;

static void *WriterTask066(void *arg)
{
    int ret;

    ret = pthread_rwlock_wrlock(&g_rwl066);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_rwl066Step = 1;
    LOS_TaskDelay(3);

    g_rwl066Step = 2;
    pthread_rwlock_unlock(&g_rwl066);
EXIT:
    return NULL;
}

static UINT32 Testcase(void)
{
    int ret;
    pthread_t writerThread;
    pthread_attr_t writerAttr;
    struct sched_param sp;
    int loopCount = 0;

    g_rwl066Step = 0;

    ret = pthread_rwlock_init(&g_rwl066, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_init(&writerAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    sp.sched_priority = RWLOCK_TEST_LOW_PRIO;
    pthread_attr_setschedparam(&writerAttr, &sp);
    pthread_attr_setinheritsched(&writerAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&writerThread, &writerAttr, WriterTask066, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    LOS_TaskDelay(1);

    ret = pthread_rwlock_tryrdlock(&g_rwl066);
    ICUNIT_GOTO_EQUAL(ret, EBUSY, ret, EXIT_JOIN);

    while (g_rwl066Step < 2 && loopCount < 100) {
        LOS_TaskDelay(1);
        loopCount++;
    }

    ret = pthread_rwlock_tryrdlock(&g_rwl066);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);
    pthread_rwlock_unlock(&g_rwl066);

EXIT_JOIN:
    PosixPthreadDestroy(&writerAttr, writerThread);
    pthread_rwlock_destroy(&g_rwl066);
    return LOS_OK;

EXIT_ATTR:
    pthread_attr_destroy(&writerAttr);
    pthread_rwlock_destroy(&g_rwl066);
    return LOS_NOK;
}

VOID ItPosixRwlock066(void)
{
    TEST_ADD_CASE("ItPosixRwlock066", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
