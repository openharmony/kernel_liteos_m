#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec req = {0, 10000000};
    struct timespec rem = {0, 0};
    int ret;

    ret = nanosleep(&req, &rem);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(rem.tv_sec, 0, rem.tv_sec);
    ICUNIT_ASSERT_EQUAL(rem.tv_nsec, 0, rem.tv_nsec);

    return LOS_OK;
}

VOID ItPosixTime028(void)
{
    TEST_ADD_CASE("ItPosixTime028", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}