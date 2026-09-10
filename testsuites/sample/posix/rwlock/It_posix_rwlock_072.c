
#include "It_posix_rwlock.h"
#include "string.h"

static UINT32 Testcase(void)
{
    pthread_rwlock_t rwl;
    struct timespec ts;
    int ret;

    ts.tv_sec = 1;
    ts.tv_nsec = 0;

    ret = pthread_rwlock_timedwrlock(NULL, &ts);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_rwlock_timedrdlock(NULL, &ts);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_rwlock_wrlock(NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_rwlock_rdlock(NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_rwlock_trywrlock(NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_rwlock_tryrdlock(NULL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    memset(&rwl, 0, sizeof(pthread_rwlock_t));
    ret = pthread_rwlock_timedwrlock(&rwl, &ts);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_rwlock_timedrdlock(&rwl, &ts);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    return LOS_OK;
}

VOID ItPosixRwlock072(void)
{
    TEST_ADD_CASE("ItPosixRwlock072", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
