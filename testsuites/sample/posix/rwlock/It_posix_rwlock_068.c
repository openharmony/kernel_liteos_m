#include "It_posix_rwlock.h"

static UINT32 Testcase(void)
{
    pthread_rwlock_t rwl;
    int ret;

    ret = pthread_rwlock_init(&rwl, NULL);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_rwlock_unlock(&rwl);
    ICUNIT_GOTO_NOT_EQUAL(ret, 0, ret, EXIT);

EXIT:
    ret = pthread_rwlock_unlock(NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    pthread_rwlock_destroy(&rwl);
    return LOS_OK;
}

VOID ItPosixRwlock068(void)
{
    TEST_ADD_CASE("ItPosixRwlock068", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
