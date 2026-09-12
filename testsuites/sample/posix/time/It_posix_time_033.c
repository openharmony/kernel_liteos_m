#include "It_posix_time.h"
#include "tzdst.h"

/* 2026-06-01 00:00:00 UTC, inside the DST period configured below */
#define DST_TEST_UTC_TIME 1780272000
#define DST_TEST_YEAR 2026
/* dst_inquire takes a tm_year-style year (years since 1900) */
#define DST_TEST_YEAR_TM (DST_TEST_YEAR - 1900)

static UINT32 Testcase(VOID)
{
    time_t timeVal = DST_TEST_UTC_TIME;
    struct tm tmLocal;
    struct tm tmGmt;
    struct tm dstStart;
    struct tm dstEnd;
    INT32 ret;

    /* no DST by default: local = UTC + 8h */
    (VOID)localtime_r(&timeVal, &tmLocal);
    (VOID)gmtime_r(&timeVal, &tmGmt);
    ICUNIT_ASSERT_EQUAL((INT32)(tmLocal.tm_hour - tmGmt.tm_hour), 8, tmLocal.tm_hour);

    /* enable DST: +1h between "Feb-03 03:00:00" and the 1st Friday of Oct 02:59:59 */
    ret = dst_enable("Feb-03 03:00:00", "Oct-1st-Fri 02:59:59", 3600);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    /* inside the DST period: one extra hour */
    (VOID)localtime_r(&timeVal, &tmLocal);
    ICUNIT_ASSERT_EQUAL(tmLocal.tm_isdst, 1, tmLocal.tm_isdst);
    ICUNIT_ASSERT_EQUAL((INT32)(tmLocal.tm_hour - tmGmt.tm_hour), 9, tmLocal.tm_hour);

    /* inquire the DST period of 2026 (local wall time); the year argument
     * is tm_year-style, i.e. years since 1900 */
    ret = dst_inquire(DST_TEST_YEAR_TM, &dstStart, &dstEnd);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(dstStart.tm_mon, 1, dstStart.tm_mon);
    ICUNIT_ASSERT_EQUAL(dstStart.tm_mday, 3, dstStart.tm_mday);
    ICUNIT_ASSERT_EQUAL(dstStart.tm_hour, 3, dstStart.tm_hour);
    ICUNIT_ASSERT_EQUAL(dstEnd.tm_mon, 9, dstEnd.tm_mon);
    ICUNIT_ASSERT_EQUAL(dstEnd.tm_hour, 2, dstEnd.tm_hour);
    ICUNIT_ASSERT_EQUAL(dstEnd.tm_min, 59, dstEnd.tm_min);

    /* disable DST again */
    ret = dst_disable();
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    (VOID)localtime_r(&timeVal, &tmLocal);
    ICUNIT_ASSERT_EQUAL((INT32)(tmLocal.tm_hour - tmGmt.tm_hour), 8, tmLocal.tm_hour);

    return LOS_OK;
}

VOID ItPosixTime033(void)
{
    TEST_ADD_CASE("ItPosixTime033", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
