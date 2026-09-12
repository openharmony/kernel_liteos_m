#include "It_posix_signal.h"
#include "pthread.h"

static volatile int g_sig090Received = 0;

static void SigHandler090(int sig)
{
    g_sig090Received = sig;
}

static void *SenderTask090(void *arg)
{
    int ret;
    UINT32 targetTid = *(UINT32 *)arg;

    LOS_TaskDelay(2);

    ret = pthread_kill((pthread_t)targetTid, SIGUSR2);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

EXIT:
    return NULL;
}

static UINT32 Testcase(void)
{
    int ret;
    pthread_t senderThread;
    pthread_attr_t senderAttr;
    struct sched_param sp;
    struct sigaction sa, oldSa;
    UINT32 targetTid;

    g_sig090Received = 0;
    targetTid = LOS_CurTaskIDGet();

    sa.sa_handler = SigHandler090;
    sa.sa_flags = 0;
    (void)sigemptyset(&sa.sa_mask);

    ret = sigaction(SIGUSR2, &sa, &oldSa);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_attr_init(&senderAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    sp.sched_priority = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    pthread_attr_setschedparam(&senderAttr, &sp);
    pthread_attr_setinheritsched(&senderAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&senderThread, &senderAttr, SenderTask090, &targetTid);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_SA);

    LOS_TaskDelay(4);

    ICUNIT_GOTO_EQUAL(g_sig090Received, SIGUSR2, g_sig090Received, EXIT_JOIN);

EXIT_JOIN:
    pthread_join(senderThread, NULL);
    pthread_attr_destroy(&senderAttr);
    sigaction(SIGUSR2, &oldSa, NULL);
    return LOS_OK;

EXIT_SA:
    pthread_attr_destroy(&senderAttr);
    sigaction(SIGUSR2, &oldSa, NULL);
    return LOS_NOK;
}

VOID ItPosixSignal090(void)
{
    TEST_ADD_CASE("ItPosixSignal090", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
