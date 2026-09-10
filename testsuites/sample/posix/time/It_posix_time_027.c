#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    struct timespec start = {0, 0};
    struct timespec end = {0, 0};

    clock_gettime(CLOCK_MONOTONIC, &start);
    sleep(1);
    clock_gettime(CLOCK_MONOTONIC, &end);

    ICUNIT_ASSERT_EQUAL(end.tv_sec > start.tv_sec, 1, end.tv_sec - start.tv_sec);

    return LOS_OK;
}

VOID ItPosixTime027(void)
{
    TEST_ADD_CASE("ItPosixTime027", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
