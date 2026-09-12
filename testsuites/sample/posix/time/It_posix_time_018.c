#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    time_t t;
    time_t *tp = &t;

    t = time(tp);
    ICUNIT_ASSERT_NOT_EQUAL(t, (time_t)-1, t);
    ICUNIT_ASSERT_NOT_EQUAL(*tp, (time_t)-1, *tp);
    ICUNIT_ASSERT_EQUAL(t, *tp, t);

    t = time(NULL);
    ICUNIT_ASSERT_NOT_EQUAL(t, (time_t)-1, t);

    return LOS_OK;
}

VOID ItPosixTime018(void)
{
    TEST_ADD_CASE("ItPosixTime018", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
