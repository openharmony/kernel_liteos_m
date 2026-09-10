#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec req = {0, 10000000};
    struct timespec rem = {0, 0};
    int ret;

    ret = clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &req, &rem);
    ICUNIT_ASSERT_EQUAL(ret, ENOTSUP, ret);

    ret = clock_nanosleep(CLOCK_MONOTONIC, 0, &req, &rem);
    ICUNIT_ASSERT_EQUAL(ret, ENOTSUP, ret);

    ret = clock_nanosleep((clockid_t)9999, 0, &req, &rem);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = clock_nanosleep(CLOCK_REALTIME, 2, &req, &rem);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    return LOS_OK;
}

VOID ItPosixTime017(void)
{
    TEST_ADD_CASE("ItPosixTime017", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
