#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec req = {0, 0};
    int ret;

    ret = nanosleep(NULL, NULL);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    req.tv_sec = -1;
    req.tv_nsec = 0;
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    req.tv_sec = 0;
    req.tv_nsec = -1;
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    req.tv_sec = 0;
    req.tv_nsec = 1000000000;
    ret = nanosleep(&req, NULL);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    return LOS_OK;
}

VOID ItPosixTime002(void)
{
    TEST_ADD_CASE("ItPosixTime002", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
