#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec tp = {0, 0};
    int ret;

    ret = clock_settime(CLOCK_MONOTONIC, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = clock_settime(CLOCK_PROCESS_CPUTIME_ID, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, ENOTSUP, errno);

    tp.tv_sec = -1;
    tp.tv_nsec = 0;
    ret = clock_settime(CLOCK_REALTIME, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    tp.tv_sec = 0;
    tp.tv_nsec = -1;
    ret = clock_settime(CLOCK_REALTIME, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    tp.tv_sec = 0;
    tp.tv_nsec = 1000000000;
    ret = clock_settime(CLOCK_REALTIME, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    return LOS_OK;
}

VOID ItPosixTime014(void)
{
    TEST_ADD_CASE("ItPosixTime014", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
