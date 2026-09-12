
#include "It_posix_signal.h"

static UINT32 Testcase(void)
{
    int ret;
    sigset_t set, oldSet;

    ret = sigemptyset(&set);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sigaddset(&set, SIGUSR1);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sigprocmask(SIG_BLOCK, &set, &oldSet);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sigprocmask(SIG_SETMASK, &oldSet, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixSignal086(void)
{
    TEST_ADD_CASE("ItPosixSignal086", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
