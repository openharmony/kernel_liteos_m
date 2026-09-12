#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    time_t timeVal = 0;
    struct tm result = {0};
    struct tm *ret = NULL;

    ret = gmtime_r(&timeVal, &result);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(result.tm_year, 70, result.tm_year);
    ICUNIT_ASSERT_EQUAL(result.tm_mon, 0, result.tm_mon);
    ICUNIT_ASSERT_EQUAL(result.tm_mday, 1, result.tm_mday);
    ICUNIT_ASSERT_EQUAL(result.tm_hour, 0, result.tm_hour);
    ICUNIT_ASSERT_EQUAL(result.tm_min, 0, result.tm_min);
    ICUNIT_ASSERT_EQUAL(result.tm_sec, 0, result.tm_sec);
    ICUNIT_ASSERT_EQUAL(result.tm_wday, 4, result.tm_wday);

    ret = gmtime_r(NULL, &result);
    ICUNIT_ASSERT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(errno, EFAULT, errno);

    ret = gmtime_r(&timeVal, NULL);
    ICUNIT_ASSERT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(errno, EFAULT, errno);

    return LOS_OK;
}

VOID ItPosixTime020(void)
{
    TEST_ADD_CASE("ItPosixTime020", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
