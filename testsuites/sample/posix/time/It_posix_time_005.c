#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    timer_t timerID;
    struct sigevent evp;
    int ret;

    ret = timer_create(CLOCK_REALTIME, NULL, &timerID);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = timer_create(CLOCK_MONOTONIC, &evp, &timerID);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    memset(&evp, 0, sizeof(evp));
    evp.sigev_notify = SIGEV_SIGNAL;
    ret = timer_create(CLOCK_REALTIME, &evp, &timerID);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, ENOTSUP, errno);

    return LOS_OK;
}

VOID ItPosixTime005(void)
{
    TEST_ADD_CASE("ItPosixTime005", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
