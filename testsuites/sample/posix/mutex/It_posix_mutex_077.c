#include "It_posix_mutex.h"

static UINT32 Testcase(void)
{
    pthread_mutex_t mux;
    pthread_mutexattr_t mattr;
    int ret;
    int oldCeiling, ceiling;

    ret = pthread_mutexattr_init(&mattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_PROTECT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_setprioceiling(&mattr, 10);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&mux, &mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_mutex_getprioceiling(&mux, &ceiling);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);
    ICUNIT_GOTO_EQUAL(ceiling, 10, ceiling, EXIT_MUTEX);

    ret = pthread_mutex_setprioceiling(&mux, 5, &oldCeiling);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);
    ICUNIT_GOTO_EQUAL(oldCeiling, 10, oldCeiling, EXIT_MUTEX);

    ret = pthread_mutex_getprioceiling(&mux, &ceiling);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);
    ICUNIT_GOTO_EQUAL(ceiling, 5, ceiling, EXIT_MUTEX);

    ret = pthread_mutex_setprioceiling(NULL, 5, &oldCeiling);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_MUTEX);

    ret = pthread_mutex_setprioceiling(&mux, 99, &oldCeiling);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_MUTEX);

    ret = pthread_mutex_getprioceiling(NULL, &ceiling);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_MUTEX);

    ret = pthread_mutex_getprioceiling(&mux, NULL);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_MUTEX);

EXIT_MUTEX:
    pthread_mutex_destroy(&mux);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattr);
    return LOS_OK;
}

VOID ItPosixMux077(void)
{
    TEST_ADD_CASE("ItPosixMux077", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
