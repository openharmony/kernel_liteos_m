#include "It_posix_rwlock.h"

static pthread_rwlock_t g_rwl075;
static volatile int g_rwl075Step = 0;

static void *PendingWriter075(void *arg)
{
    int ret;

    while (g_rwl075Step < 1) {
        LOS_TaskDelay(1);
    }

    ret = pthread_rwlock_wrlock(&g_rwl075);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    g_rwl075Step = 3;
    pthread_rwlock_unlock(&g_rwl075);
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

    g_rwl075Step = 0;

    ret = pthread_rwlock_init(&g_rwl075, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_rwlock_rdlock(&g_rwl075);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_DESTROY);

    g_rwl075Step = 1;

    ret = pthread_attr_init(&writerAttr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_RD);
    sp.sched_priority = RWLOCK_TEST_LOW_PRIO;
    pthread_attr_setschedparam(&writerAttr, &sp);
    pthread_attr_setinheritsched(&writerAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&writerThread, &writerAttr, PendingWriter075, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    LOS_TaskDelay(2);
    g_rwl075Step = 2;

    LOS_TaskDelay(3);

    pthread_rwlock_unlock(&g_rwl075);

    LOS_TaskDelay(3);

    while (g_rwl075Step < 3 && loopCount < 100) {
        LOS_TaskDelay(1);
        loopCount++;
    }

    ICUNIT_GOTO_EQUAL(g_rwl075Step, 3, g_rwl075Step, EXIT_JOIN);

EXIT_JOIN:
    PosixPthreadDestroy(&writerAttr, writerThread);
EXIT_ATTR:
    pthread_attr_destroy(&writerAttr);
EXIT_RD:
    pthread_rwlock_unlock(&g_rwl075);
EXIT_DESTROY:
    pthread_rwlock_destroy(&g_rwl075);
    return LOS_OK;
}

VOID ItPosixRwlock075(void)
{
    TEST_ADD_CASE("ItPosixRwlock075", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
