#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct tm tmVal = {0};
    time_t result;

    tmVal.tm_year = 70;
    tmVal.tm_mon = 0;
    tmVal.tm_mday = 1;
    tmVal.tm_hour = 8;
    tmVal.tm_min = 0;
    tmVal.tm_sec = 0;
    tmVal.__tm_gmtoff = 0;
    tmVal.__tm_zone = NULL;
    result = mktime(&tmVal);
    ICUNIT_ASSERT_NOT_EQUAL(result, (time_t)-1, result);

    result = mktime(NULL);
    ICUNIT_ASSERT_EQUAL(result, (time_t)-1, result);
    ICUNIT_ASSERT_EQUAL(errno, EFAULT, errno);

    return LOS_OK;
}

VOID ItPosixTime023(void)
{
    TEST_ADD_CASE("ItPosixTime023", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
