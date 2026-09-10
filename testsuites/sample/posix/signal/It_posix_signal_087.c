#include "It_posix_signal.h"

static volatile int g_sig087Received = 0;

static void SigHandler087(int sig)
{
    g_sig087Received = sig;
}

static UINT32 Testcase(void)
{
    int ret;
    struct sigaction sa, oldSa;
    sigset_t set, oldSet;

    g_sig087Received = 0;

    sa.sa_handler = SigHandler087;
    sa.sa_flags = 0;
    (void)sigemptyset(&sa.sa_mask);

    ret = sigaction(SIGUSR1, &sa, &oldSa);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = sigemptyset(&set);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_SIG);
    ret = sigaddset(&set, SIGUSR1);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_SIG);

    ret = sigprocmask(SIG_BLOCK, &set, &oldSet);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_SIG);

    ret = raise(SIGUSR1);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MASK);

    ICUNIT_GOTO_EQUAL(g_sig087Received, SIGUSR1, g_sig087Received, EXIT_MASK);

EXIT_MASK:
    sigprocmask(SIG_SETMASK, &oldSet, NULL);
EXIT_SIG:
    sigaction(SIGUSR1, &oldSa, NULL);
EXIT:
    return LOS_OK;
}

VOID ItPosixSignal087(void)
{
    TEST_ADD_CASE("ItPosixSignal087", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
