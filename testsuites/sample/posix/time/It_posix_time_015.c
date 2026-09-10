#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec tp = {0, 0};
    int ret;

    ret = clock_getres(CLOCK_REALTIME, &tp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(tp.tv_sec, 0, tp.tv_sec);
    ICUNIT_ASSERT_NOT_EQUAL(tp.tv_nsec, 0, tp.tv_nsec);

    ret = clock_getres(CLOCK_MONOTONIC, &tp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = clock_getres(CLOCK_MONOTONIC_RAW, &tp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = clock_getres(CLOCK_REALTIME_COARSE, &tp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = clock_getres(CLOCK_MONOTONIC_COARSE, &tp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixTime015(void)
{
    TEST_ADD_CASE("ItPosixTime015", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
