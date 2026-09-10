#include "It_posix_mutex.h"

static UINT32 Testcase(void)
{
    pthread_mutex_t mux;
    pthread_mutexattr_t mattr;
    int ret;

    ret = pthread_mutexattr_init(&mattr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_ERRORCHECK);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = pthread_mutex_init(&mux, &mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    ret = pthread_mutex_lock(&mux);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);

    ret = pthread_mutex_lock(&mux);
    ICUNIT_GOTO_EQUAL(ret, EDEADLK, ret, EXIT_MUTEX);

    ret = pthread_mutex_unlock(&mux);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_MUTEX);

    ret = pthread_mutex_unlock(&mux);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT_MUTEX);

EXIT_MUTEX:
    pthread_mutex_destroy(&mux);
EXIT_ATTR:
    pthread_mutexattr_destroy(&mattr);
    return LOS_OK;
}

VOID ItPosixMux079(void)
{
    TEST_ADD_CASE("ItPosixMux079", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
