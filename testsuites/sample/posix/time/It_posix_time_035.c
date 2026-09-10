#include "It_posix_time.h"
#include "tzdst.h"

/* 2026-06-01 00:00:00 UTC, a mid-year moment (inside any Feb..Oct DST span) */
#define DST_TEST_UTC_TIME 1780272000
#define DST_TEST_YEAR 2026
/* dst_inquire takes a tm_year-style year (years since 1900) */
#define DST_TEST_YEAR_TM (DST_TEST_YEAR - 1900)

static UINT32 Testcase(VOID)
{
    time_t timeVal = DST_TEST_UTC_TIME;
    struct tm tmLocal;
    struct tm dstStart;
    struct tm dstEnd;
    INT32 ret;

    /* inquire fails while DST is disabled */
    ret = dst_inquire(DST_TEST_YEAR_TM, &dstStart, &dstEnd);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    /* NULL configs are rejected */
    ret = dst_enable(NULL, "Oct-1st-Fri 02:59:59", 3600);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ret = dst_enable("Feb-03 03:00:00", NULL, 3600);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    /* malformed month/day/time fields are rejected */
    ret = dst_enable("Xyz-03 03:00:00", "Oct-1st-Fri 02:59:59", 3600);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ret = dst_enable("Feb-32 03:00:00", "Oct-1st-Fri 02:59:59", 3600);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ret = dst_enable("Feb-03 25:00:00", "Oct-1st-Fri 02:59:59", 3600);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    /* start >= end is rejected */
    ret = dst_enable("Feb-03 03:00:00", "Feb-03 03:00:00", 3600);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    /* forward seconds must be within [0, 24*3600) */
    ret = dst_enable("Feb-03 03:00:00", "Oct-1st-Fri 02:59:59", -1);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ret = dst_enable("Feb-03 03:00:00", "Oct-1st-Fri 02:59:59", 24 * 3600);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    /* DST stays off after every rejected configuration */
    (VOID)localtime_r(&timeVal, &tmLocal);
    ICUNIT_ASSERT_EQUAL(tmLocal.tm_isdst, 0, tmLocal.tm_isdst);

    /* a valid WDAY-format configuration is accepted */
    ret = dst_enable("Feb-3rd-Mon 03:00:00", "Oct-5th-Fri 02:59:59", 3600);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    /* 2026-02-01 is Sunday, so the 3rd Monday is Feb 16; inquire takes a
     * tm_year-style year (years since 1900). The mktime/inquire timezone
     * shifts cancel out, so the reported wall time equals the configured
     * wall time presented as UTC (same as fbb). */
    ret = dst_inquire(DST_TEST_YEAR_TM, &dstStart, &dstEnd);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    ICUNIT_ASSERT_EQUAL(dstStart.tm_year, 126, dstStart.tm_year);
    ICUNIT_ASSERT_EQUAL(dstStart.tm_mon, 1, dstStart.tm_mon);
    ICUNIT_ASSERT_EQUAL(dstStart.tm_mday, 16, dstStart.tm_mday);
    ICUNIT_ASSERT_EQUAL(dstStart.tm_hour, 3, dstStart.tm_hour);
    /* 2026-10-01 is Thursday, so the 5th Friday is Oct 30 */
    ICUNIT_ASSERT_EQUAL(dstEnd.tm_mon, 9, dstEnd.tm_mon);
    ICUNIT_ASSERT_EQUAL(dstEnd.tm_mday, 30, dstEnd.tm_mday);
    ICUNIT_ASSERT_EQUAL(dstEnd.tm_hour, 2, dstEnd.tm_hour);
    ICUNIT_ASSERT_EQUAL(dstEnd.tm_min, 59, dstEnd.tm_min);
    ICUNIT_ASSERT_EQUAL(dstEnd.tm_sec, 59, dstEnd.tm_sec);

    /* local time inside the resolved period still shifts by 1h */
    (VOID)localtime_r(&timeVal, &tmLocal);
    ICUNIT_ASSERT_EQUAL(tmLocal.tm_isdst, 1, tmLocal.tm_isdst);

    /* disable and verify the state is cleared */
    ret = dst_disable();
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    (VOID)localtime_r(&timeVal, &tmLocal);
    ICUNIT_ASSERT_EQUAL(tmLocal.tm_isdst, 0, tmLocal.tm_isdst);
    ret = dst_inquire(DST_TEST_YEAR_TM, &dstStart, &dstEnd);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);

    return LOS_OK;
}

VOID ItPosixTime035(void)
{
    TEST_ADD_CASE("ItPosixTime035", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
