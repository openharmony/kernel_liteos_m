#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    clock_t clock1;
    clock_t clock2;

    clock1 = clock();
    ICUNIT_ASSERT_NOT_EQUAL(clock1, (clock_t)-1, clock1);

    LOS_TaskDelay(2);

    clock2 = clock();
    ICUNIT_ASSERT_NOT_EQUAL(clock2, (clock_t)-1, clock2);
    ICUNIT_ASSERT_EQUAL(clock2 >= clock1, 1, clock2);

    return LOS_OK;
}

VOID ItPosixTime031(void)
{
    TEST_ADD_CASE("ItPosixTime031", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}