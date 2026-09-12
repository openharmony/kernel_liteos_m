#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    int ret;

    ret = timer_delete((timer_t)9999);
    ICUNIT_ASSERT_EQUAL(ret, -1, ret);
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    return LOS_OK;
}

VOID ItPosixTime007(void)
{
    TEST_ADD_CASE("ItPosixTime007", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL1, TEST_FUNCTION);
}
