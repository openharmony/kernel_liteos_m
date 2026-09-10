#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec tp = {0, 0};
    struct timespec tpOld = {0, 0};
    int ret;

    ret = clock_gettime(CLOCK_REALTIME, &tpOld);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    tp.tv_sec = tpOld.tv_sec - 1;
    tp.tv_nsec = 1;
    ret = clock_settime(CLOCK_REALTIME, &tp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = clock_gettime(CLOCK_REALTIME, &tp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    tp.tv_sec = tpOld.tv_sec;
    tp.tv_nsec = 0;
    ret = clock_settime(CLOCK_REALTIME, &tp);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixTime013(void)
{
    TEST_ADD_CASE("ItPosixTime013", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
