#include "It_posix_signal.h"

static volatile int g_sig088Received = 0;

static void SigHandler088(int sig)
{
    g_sig088Received = sig;
}

static UINT32 Testcase(void)
{
    int ret;
    struct sigaction sa, oldSa, ignoreSa;

    g_sig088Received = 0;

    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    (void)sigemptyset(&sa.sa_mask);

    ret = sigaction(SIGUSR1, &sa, &oldSa);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    raise(SIGUSR1);

    ICUNIT_GOTO_EQUAL(g_sig088Received, 0, g_sig088Received, EXIT);

    ignoreSa = oldSa;
    ignoreSa.sa_handler = SigHandler088;
    ignoreSa.sa_flags = 0;
    (void)sigemptyset(&ignoreSa.sa_mask);

    ret = sigaction(SIGUSR1, &ignoreSa, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

EXIT:
    sigaction(SIGUSR1, &oldSa, NULL);
    return LOS_OK;
}

VOID ItPosixSignal088(void)
{
    TEST_ADD_CASE("ItPosixSignal088", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
