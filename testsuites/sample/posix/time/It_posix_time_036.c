#include "It_posix_time.h"
#include "time64.h"
#include "tzdst.h"

extern long timezone;

static UINT32 Testcase(VOID)
{
    time64_t timeVal = 0;
    time64_t ret64;
    struct tm tmBuf;
    struct timeval64 tv64;
    char *strRet = NULL;
    char buf[64];
    INT32 err;
    INT32 ret32;
    long tzSaved = timezone;

    /* NULL guards of the time64 family */
    ICUNIT_ASSERT_EQUAL(localtime64_r(NULL, &tmBuf), NULL, 0);
    ICUNIT_ASSERT_EQUAL(localtime64_r(&timeVal, NULL), NULL, 0);
    ICUNIT_ASSERT_EQUAL(gmtime64_r(NULL, &tmBuf), NULL, 0);
    ICUNIT_ASSERT_EQUAL(gmtime64_r(&timeVal, NULL), NULL, 0);

    ret64 = mktime64(NULL);
    err = errno;
    ICUNIT_ASSERT_EQUAL(ret64, (time64_t)-1, ret64);
    ICUNIT_ASSERT_EQUAL(err, EFAULT, err);

    strRet = ctime64(NULL);
    ICUNIT_ASSERT_EQUAL(strRet, NULL, 0);
    strRet = ctime64_r(NULL, buf);
    ICUNIT_ASSERT_EQUAL(strRet, NULL, 0);
    strRet = ctime64_r(&timeVal, NULL);
    ICUNIT_ASSERT_EQUAL(strRet, NULL, 0);

    /* asctime64_r rejects an out-of-range tm_mon */
    (VOID)gmtime64_r(&timeVal, &tmBuf);
    tmBuf.tm_mon = 12;
    strRet = asctime64_r(&tmBuf, buf);
    ICUNIT_ASSERT_EQUAL(strRet, NULL, 0);

    /* settimeofday64 rejects a negative seconds field */
    tv64.tv_sec = -1;
    tv64.tv_usec = 0;
    ret32 = settimeofday64(&tv64, NULL);
    ICUNIT_ASSERT_EQUAL(ret32, -1, ret32);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    /* settimezone: malformed strings keep the current timezone */
    settimezone("CST");
    ICUNIT_ASSERT_EQUAL(timezone, tzSaved, timezone);
    settimezone("CST-25:00:00");
    ICUNIT_ASSERT_EQUAL(timezone, tzSaved, timezone);
    settimezone("UTC+15:00:00");
    ICUNIT_ASSERT_EQUAL(timezone, tzSaved, timezone);

    /* a valid string applies the offset (east-positive input) */
    settimezone("UTC+09:30:00");
    ICUNIT_ASSERT_EQUAL(timezone, -9 * 3600 - 1800, timezone);

    /* restore the default CST(UTC+8) */
    settimezone("CST+08:00:00");
    ICUNIT_ASSERT_EQUAL(timezone, -8 * 3600, timezone);

    return LOS_OK;
}

VOID ItPosixTime036(void)
{
    TEST_ADD_CASE("ItPosixTime036", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
