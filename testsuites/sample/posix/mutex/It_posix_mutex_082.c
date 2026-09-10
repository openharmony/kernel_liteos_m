
#include "It_posix_mutex.h"

static UINT32 Testcase(void)
{
    pthread_mutexattr_t mattr;
    int ret;
    int type;
    int protocol;

    ret = pthread_mutexattr_gettype(NULL, &type);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_mutexattr_getprotocol(NULL, &protocol);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_mutexattr_settype(NULL, PTHREAD_MUTEX_NORMAL);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_mutexattr_setprotocol(NULL, PTHREAD_PRIO_INHERIT);
    ICUNIT_ASSERT_EQUAL(ret, EINVAL, ret);

    ret = pthread_mutexattr_init(&mattr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_mutexattr_gettype(&mattr, &type);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(type, PTHREAD_MUTEX_NORMAL, type, EXIT);

    ret = pthread_mutexattr_getprotocol(&mattr, &protocol);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(protocol, PTHREAD_PRIO_NONE, protocol, EXIT);

    ret = pthread_mutexattr_settype(&mattr, 999);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_mutexattr_setprotocol(&mattr, 999);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

EXIT:
    pthread_mutexattr_destroy(&mattr);
    return LOS_OK;
}

VOID ItPosixMux082(void)
{
    TEST_ADD_CASE("ItPosixMux082", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
