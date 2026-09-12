#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    clock_t clk;
    struct tms tmsBuf = {0};

    clk = times(&tmsBuf);
    ICUNIT_ASSERT_NOT_EQUAL(clk, (clock_t)-1, clk);

    clk = times(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(clk, (clock_t)-1, clk);

    return LOS_OK;
}

VOID ItPosixTime029(void)
{
    TEST_ADD_CASE("ItPosixTime029", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
