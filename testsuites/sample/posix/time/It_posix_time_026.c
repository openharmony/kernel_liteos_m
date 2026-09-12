#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timeval tv = {0, 0};
    struct timeval tvOld = {0, 0};
    int ret;

    ret = gettimeofday(&tvOld, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    tv.tv_sec = tvOld.tv_sec;
    tv.tv_usec = 0;
    ret = gettimeofday(&tv, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_NOT_EQUAL(tv.tv_sec, 0, tv.tv_sec);

    ret = gettimeofday(NULL, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = gettimeofday(&tv, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixTime026(void)
{
    TEST_ADD_CASE("ItPosixTime026", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
