
#include "It_posix_signal.h"

static volatile int g_sig085Count = 0;

static void SigHandler085(int sig)
{
    g_sig085Count++;
}

static UINT32 Testcase(void)
{
    int ret;
    struct sigaction sa, oldSa;

    g_sig085Count = 0;

    sa.sa_handler = SigHandler085;
    sa.sa_flags = 0;
    (void)sigemptyset(&sa.sa_mask);

    ret = sigaction(SIGUSR2, &sa, &oldSa);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = raise(SIGUSR2);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = raise(SIGUSR2);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ICUNIT_GOTO_EQUAL(g_sig085Count, 2, g_sig085Count, EXIT);

EXIT:
    sigaction(SIGUSR2, &oldSa, NULL);
    return LOS_OK;
}

VOID ItPosixSignal085(void)
{
    TEST_ADD_CASE("ItPosixSignal085", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
