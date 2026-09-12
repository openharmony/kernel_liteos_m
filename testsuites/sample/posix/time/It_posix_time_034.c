#include "It_posix_time.h"
#include "time64.h"
#include "tzdst.h"

static UINT32 Testcase(VOID)
{
    struct timeval64 tv64 = {0};
    time_t timeZero = 0;
    time_t now;
    time_t before;
    struct tm result;
    struct tm *ret = NULL;
    INT32 ret32;

    /* settimezone: "XXX+8" means UTC+8 in the fbb string format,
     * stored with POSIX sign convention (west positive) => -28800 */
    settimezone("XXX+8");
    ret = localtime_r(&timeZero, &result);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(result.tm_year, 70, result.tm_year);
    ICUNIT_ASSERT_EQUAL(result.tm_hour, 8, result.tm_hour);

    /* "CST-8" means UTC-8 here => +28800 */
    settimezone("CST-8");
    ret = localtime_r(&timeZero, &result);
    ICUNIT_ASSERT_NOT_EQUAL(ret, NULL, ret);
    ICUNIT_ASSERT_EQUAL(result.tm_year, 69, result.tm_year);
    ICUNIT_ASSERT_EQUAL(result.tm_mday, 31, result.tm_mday);
    ICUNIT_ASSERT_EQUAL(result.tm_hour, 16, result.tm_hour);

    /* restore the default CST(UTC+8) */
    settimezone("XXX+8");

    /* gettimeofday64 */
    ret32 = gettimeofday64(NULL, NULL);
    ICUNIT_ASSERT_EQUAL(ret32, -1, ret32);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    now = time(NULL);
    ret32 = gettimeofday64(&tv64, NULL);
    ICUNIT_ASSERT_EQUAL(ret32, 0, ret32);
    ICUNIT_ASSERT_EQUAL((now >= (time_t)tv64.tv_sec - 2) && (now <= (time_t)tv64.tv_sec + 2), 1, now);

    /* setlocalseconds: set the wall clock, then restore */
    before = time(NULL);
    ret32 = setlocalseconds((int)before + 100);
    ICUNIT_ASSERT_EQUAL(ret32, 0, ret32);
    now = time(NULL);
    ICUNIT_ASSERT_EQUAL((now >= before + 100) && (now <= before + 102), 1, now);
    ret32 = setlocalseconds((int)before);
    ICUNIT_ASSERT_EQUAL(ret32, 0, ret32);
    now = time(NULL);
    ICUNIT_ASSERT_EQUAL((now >= before) && (now <= before + 2), 1, now);

    /* settimeofday64: reject invalid input, accept valid time */
    tv64.tv_sec = (int64_t)before;
    tv64.tv_usec = 1000001;
    ret32 = settimeofday64(&tv64, NULL);
    ICUNIT_ASSERT_EQUAL(ret32, -1, ret32);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    tv64.tv_usec = 0;
    ret32 = settimeofday64(&tv64, NULL);
    ICUNIT_ASSERT_EQUAL(ret32, 0, ret32);
    now = time(NULL);
    ICUNIT_ASSERT_EQUAL((now >= before) && (now <= before + 2), 1, now);

    return LOS_OK;
}

VOID ItPosixTime034(void)
{
    TEST_ADD_CASE("ItPosixTime034", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
