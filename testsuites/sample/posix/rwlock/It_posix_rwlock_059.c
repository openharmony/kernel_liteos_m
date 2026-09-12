
#include "It_posix_rwlock.h"

static UINT32 Testcase(void)
{
    pthread_rwlockattr_t attr;
    int ret;

    ret = pthread_rwlockattr_init(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_rwlockattr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_rwlockattr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;
}

VOID ItPosixRwlock059(void)
{
    TEST_ADD_CASE("ItPosixRwlock059", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
