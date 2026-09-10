#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct tm tmVal = {0};
    time_t result;

    /* mktime now normalizes out-of-range fields instead of failing
     * with EOVERFLOW, aligned with fbb (POSIX normalization) */

    tmVal.tm_year = 69;
    tmVal.tm_mon = 0;
    tmVal.tm_mday = 1;
    tmVal.tm_hour = 0;
    tmVal.tm_min = 0;
    tmVal.tm_sec = 0;
    tmVal.__tm_gmtoff = 0;
    tmVal.__tm_zone = NULL;
    result = mktime(&tmVal);
    ICUNIT_ASSERT_EQUAL(result, (time_t)-31564800, result);

    tmVal.tm_year = 70;
    tmVal.tm_mon = 0;
    tmVal.tm_mday = 1;
    tmVal.tm_hour = 0;
    tmVal.tm_min = 0;
    tmVal.tm_sec = 61;
    tmVal.__tm_gmtoff = 0;
    tmVal.__tm_zone = NULL;
    result = mktime(&tmVal);
    ICUNIT_ASSERT_EQUAL(result, (time_t)-28739, result);

    tmVal.tm_year = 70;
    tmVal.tm_mon = -1;
    tmVal.tm_mday = 1;
    tmVal.tm_hour = 0;
    tmVal.tm_min = 0;
    tmVal.tm_sec = 0;
    tmVal.__tm_gmtoff = 0;
    tmVal.__tm_zone = NULL;
    result = mktime(&tmVal);
    ICUNIT_ASSERT_EQUAL(result, (time_t)-2707200, result);

    tmVal.tm_year = 70;
    tmVal.tm_mon = 12;
    tmVal.tm_mday = 1;
    tmVal.tm_hour = 0;
    tmVal.tm_min = 0;
    tmVal.tm_sec = 0;
    tmVal.__tm_gmtoff = 0;
    tmVal.__tm_zone = NULL;
    result = mktime(&tmVal);
    ICUNIT_ASSERT_EQUAL(result, (time_t)31507200, result);

    return LOS_OK;
}

VOID ItPosixTime024(void)
{
    TEST_ADD_CASE("ItPosixTime024", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
