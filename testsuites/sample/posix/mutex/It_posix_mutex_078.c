#include "It_posix_mutex.h"

static UINT32 Testcase(void)
{
    pthread_mutex_t mux;
    pthread_mutexattr_t mattr;
    int ret;
    int oldCeiling;

    ret = pthread_mutexattr_init(&mattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_PROTECT);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_setprioceiling(&mattr, MUTEX_TEST_HIGH_PRIO);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&mux, &mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_mutex_setprioceiling(&mux, MUTEX_TEST_HIGH_PRIO - 2, &oldCeiling);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);
    ICUNIT_GOTO_EQUAL(oldCeiling, MUTEX_TEST_HIGH_PRIO, oldCeiling, EXIT_MUTEX);

EXIT_MUTEX:
    pthread_mutex_destroy(&mux);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattr);
    return LOS_OK;
}

VOID ItPosixMux078(void)
{
    TEST_ADD_CASE("ItPosixMux078", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
