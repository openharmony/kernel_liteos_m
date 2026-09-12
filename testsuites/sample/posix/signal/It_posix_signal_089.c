
#include "It_posix_signal.h"

static UINT32 Testcase(void)
{
    int ret;
    sigset_t set;

    ret = sigemptyset(&set);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sigaddset(&set, SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sigaddset(&set, SIGUSR2);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sigismember(&set, SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, 1, ret);

    ret = sigismember(&set, SIGUSR2);
    ICUNIT_ASSERT_EQUAL(ret, 1, ret);

    ret = sigismember(&set, SIGPIPE);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sigdelset(&set, SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sigismember(&set, SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixSignal089(void)
{
    TEST_ADD_CASE("ItPosixSignal089", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
