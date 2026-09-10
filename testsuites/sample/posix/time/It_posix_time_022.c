#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    time_t timeVal = 0;
    struct tm result = {0};
    struct tm *ret = NULL;

    ret = localtime_r(&timeVal, &result);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(result.tm_year, 70, result.tm_year);
    ICUNIT_ASSERT_EQUAL(result.tm_mon, 0, result.tm_mon);
    ICUNIT_ASSERT_EQUAL(result.tm_mday, 1, result.tm_mday);
    ICUNIT_ASSERT_EQUAL(result.tm_hour, 8, result.tm_hour);
    ICUNIT_ASSERT_EQUAL(result.tm_min, 0, result.tm_min);
    ICUNIT_ASSERT_EQUAL(result.tm_sec, 0, result.tm_sec);

    ret = localtime_r(NULL, &result);
    ICUNIT_ASSERT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(errno, EFAULT, errno);

    ret = localtime_r(&timeVal, NULL);
    ICUNIT_ASSERT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(errno, EFAULT, errno);

    return LOS_OK;
}

VOID ItPosixTime022(void)
{
    TEST_ADD_CASE("ItPosixTime022", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
