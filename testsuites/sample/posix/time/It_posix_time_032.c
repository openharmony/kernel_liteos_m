#include "It_posix_time.h"
#include "time64.h"

static UINT32 Testcase(VOID)
{
    time64_t timeVal = 0;
    time64_t timeZero = 0;
    time64_t beyond2038 = (time64_t)2147483648LL; /* 0x80000000, out of 32-bit time_t range */
    struct tm result;
    struct tm *ret = NULL;

    /* localtime64_r: aligned with localtime_r, timezone default CST(UTC+8) */
    ret = localtime64_r(&timeVal, &result);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(result.tm_year, 70, result.tm_year);
    ICUNIT_ASSERT_EQUAL(result.tm_mon, 0, result.tm_mon);
    ICUNIT_ASSERT_EQUAL(result.tm_mday, 1, result.tm_mday);
    ICUNIT_ASSERT_EQUAL(result.tm_hour, 8, result.tm_hour);
    ICUNIT_ASSERT_EQUAL(result.tm_min, 0, result.tm_min);
    ICUNIT_ASSERT_EQUAL(result.tm_sec, 0, result.tm_sec);

    /* mktime64: round trip of the localtime result (local fields) */
    timeVal = mktime64(&result);
    ICUNIT_ASSERT_EQUAL(timeVal, 0, timeVal);

    /* gmtime64_r: UTC */
    ret = gmtime64_r(&timeZero, &result);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(result.tm_year, 70, result.tm_year);
    ICUNIT_ASSERT_EQUAL(result.tm_mday, 1, result.tm_mday);
    ICUNIT_ASSERT_EQUAL(result.tm_hour, 0, result.tm_hour);

    /* 64-bit range: 2038-01-19 11:14:08 local (UTC+8) => 0x80000000 seconds */
    ret = localtime64_r(&beyond2038, &result);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(result.tm_year, 138, result.tm_year);
    ICUNIT_ASSERT_EQUAL(result.tm_hour, 11, result.tm_hour);
    ICUNIT_ASSERT_EQUAL(result.tm_min, 14, result.tm_min);
    ICUNIT_ASSERT_EQUAL(result.tm_sec, 8, result.tm_sec);

    timeVal = mktime64(&result);
    ICUNIT_ASSERT_EQUAL(timeVal, beyond2038, timeVal);

    return LOS_OK;
}

VOID ItPosixTime032(void)
{
    TEST_ADD_CASE("ItPosixTime032", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
