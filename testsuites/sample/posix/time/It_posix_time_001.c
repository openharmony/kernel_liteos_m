#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec req = {0, 10000000};
    struct timespec start = {0, 0};
    struct timespec end = {0, 0};
    int ret;

    clock_gettime(CLOCK_MONOTONIC, &start);
    ret = nanosleep(&req, NULL);
    clock_gettime(CLOCK_MONOTONIC, &end);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ICUNIT_ASSERT_EQUAL(end.tv_sec >= start.tv_sec, 1, end.tv_sec - start.tv_sec);

    return LOS_OK;
}

VOID ItPosixTime001(void)
{
    TEST_ADD_CASE("ItPosixTime001", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
