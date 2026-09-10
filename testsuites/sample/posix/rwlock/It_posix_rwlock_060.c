
#include "It_posix_rwlock.h"

static UINT32 Testcase(void)
{
    pthread_rwlockattr_t attr;
    int ret;
    int pshared;

    ret = pthread_rwlockattr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_PRIVATE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlockattr_getpshared(&attr, &pshared);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlockattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    ICUNIT_GOTO_EQUAL(ret, ENOSYS, ret, EXIT);

    ret = pthread_rwlockattr_setpshared(&attr, 999);
    ICUNIT_GOTO_EQUAL(ret, ENOSYS, ret, EXIT);

    ret = pthread_rwlockattr_getpshared(NULL, &pshared);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_rwlockattr_getpshared(&attr, NULL);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

EXIT:
    pthread_rwlockattr_destroy(&attr);
    return LOS_OK;
}

VOID ItPosixRwlock060(void)
{
    TEST_ADD_CASE("ItPosixRwlock060", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
