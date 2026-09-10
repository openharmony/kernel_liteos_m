#include "It_posix_pthread.h"

static UINT32 Testcase(VOID)
{
    int ret;

    ret = sched_yield();
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = sched_yield();
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

/**
 * @tc.name: ItPosixPthread030
 * @tc.desc: Test interface sched_yield
 * @tc.type: FUNC
 */

VOID ItPosixPthread030(VOID)
{
    TEST_ADD_CASE("ItPosixPthread030", Testcase, TEST_POSIX, TEST_PTHREAD, TEST_LEVEL0, TEST_FUNCTION);
}