#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec tp = {0, 0};
    int ret;

    ret = clock_getres(CLOCK_REALTIME, NULL);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    ret = clock_getres(CLOCK_PROCESS_CPUTIME_ID, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, ENOTSUP, errno);

    ret = clock_getres(CLOCK_THREAD_CPUTIME_ID, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, ENOTSUP, errno);

    ret = clock_getres(CLOCK_BOOTTIME, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, ENOTSUP, errno);

    ret = clock_getres((clockid_t)9999, &tp);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    return LOS_OK;
}

VOID ItPosixTime016(void)
{
    TEST_ADD_CASE("ItPosixTime016", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
