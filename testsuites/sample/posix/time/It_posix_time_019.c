#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    time_t timeVal = 0;
    struct tm *result = NULL;

    result = gmtime(&timeVal);
    ICUNIT_ASSERT_NOT_EQUAL(result, NULL, result);
    ICUNIT_ASSERT_EQUAL(result->tm_year, 70, result->tm_year);
    ICUNIT_ASSERT_EQUAL(result->tm_mon, 0, result->tm_mon);
    ICUNIT_ASSERT_EQUAL(result->tm_mday, 1, result->tm_mday);
    ICUNIT_ASSERT_EQUAL(result->tm_hour, 0, result->tm_hour);
    ICUNIT_ASSERT_EQUAL(result->tm_min, 0, result->tm_min);
    ICUNIT_ASSERT_EQUAL(result->tm_sec, 0, result->tm_sec);
    ICUNIT_ASSERT_EQUAL(result->tm_wday, 4, result->tm_wday);

    timeVal = 86400;
    result = gmtime(&timeVal);
    ICUNIT_ASSERT_NOT_EQUAL(result, NULL, result);
    ICUNIT_ASSERT_EQUAL(result->tm_year, 70, result->tm_year);
    ICUNIT_ASSERT_EQUAL(result->tm_mon, 0, result->tm_mon);
    ICUNIT_ASSERT_EQUAL(result->tm_mday, 2, result->tm_mday);
    ICUNIT_ASSERT_EQUAL(result->tm_hour, 0, result->tm_hour);
    ICUNIT_ASSERT_EQUAL(result->tm_wday, 5, result->tm_wday);

    result = gmtime(NULL);
    ICUNIT_ASSERT_EQUAL(result, NULL, result);

    return LOS_OK;
}

VOID ItPosixTime019(void)
{
    TEST_ADD_CASE("ItPosixTime019", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
