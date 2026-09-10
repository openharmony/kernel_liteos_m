
#include "It_posix_signal.h"

static volatile int g_sig084Received = 0;

static void SigHandler084(int sig)
{
    g_sig084Received = sig;
}

static UINT32 Testcase(void)
{
    int ret;
    struct sigaction sa, oldSa;

    g_sig084Received = 0;

    sa.sa_handler = SigHandler084;
    sa.sa_flags = 0;
    (void)sigemptyset(&sa.sa_mask);

    ret = sigaction(SIGUSR1, &sa, &oldSa);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = raise(SIGUSR1);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ICUNIT_GOTO_EQUAL(g_sig084Received, SIGUSR1, g_sig084Received, EXIT);

EXIT:
    sigaction(SIGUSR1, &oldSa, NULL);
    return LOS_OK;
}

VOID ItPosixSignal084(void)
{
    TEST_ADD_CASE("ItPosixSignal084", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
