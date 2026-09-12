#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timeval tv = {0, 0};
    int ret;

    ret = gettimeofday(&tv, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_NOT_EQUAL(tv.tv_sec, 0, tv.tv_sec);

    ret = gettimeofday(&tv, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixTime025(void)
{
    TEST_ADD_CASE("ItPosixTime025", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
