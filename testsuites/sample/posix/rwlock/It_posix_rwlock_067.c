
#include "It_posix_rwlock.h"

static UINT32 Testcase(void)
{
    pthread_rwlock_t rwl;
    int ret;

    ret = pthread_rwlock_init(&rwl, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlock_rdlock(&rwl);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_UNLOCK);

    ret = pthread_rwlock_trywrlock(&rwl);
    ICUNIT_GOTO_EQUAL(ret, EDEADLK, ret, EXIT_UNLOCK);

    ret = pthread_rwlock_unlock(&rwl);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlock_trywrlock(&rwl);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_UNLOCK);

    ret = pthread_rwlock_unlock(&rwl);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlock_destroy(&rwl);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

EXIT_UNLOCK:
    pthread_rwlock_unlock(&rwl);
EXIT:
    pthread_rwlock_destroy(&rwl);
    return LOS_OK;
}

VOID ItPosixRwlock067(void)
{
    TEST_ADD_CASE("ItPosixRwlock067", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
