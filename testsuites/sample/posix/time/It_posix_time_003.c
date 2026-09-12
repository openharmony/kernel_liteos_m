#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec req = {0, 1};
    int ret;

    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    req.tv_sec = 0;
    req.tv_nsec = 0;
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixTime003(void)
{
    TEST_ADD_CASE("ItPosixTime003", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
