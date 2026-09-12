
#include "It_posix_rwlock.h"

static UINT32 Testcase(void)
{
    pthread_rwlock_t rwl;
    int ret;

    ret = pthread_rwlock_init(&rwl, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_rwlock_destroy(&rwl);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_rwlock_init(NULL, NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_rwlock_destroy(NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    return LOS_OK;
}

VOID ItPosixRwlock061(void)
{
    TEST_ADD_CASE("ItPosixRwlock061", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
