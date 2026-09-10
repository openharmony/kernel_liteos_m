#include "It_posix_mutex.h"

static UINT32 Testcase(void)
{
    int ret;
    int protocol;
    pthread_mutexattr_t attr;

    ret = pthread_mutexattr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_mutexattr_getprotocol(&attr, &protocol);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(protocol, PTHREAD_PRIO_INHERIT, protocol, EXIT);

    ret = pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_NONE);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_mutexattr_getprotocol(&attr, &protocol);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(protocol, PTHREAD_PRIO_NONE, protocol, EXIT);

    ret = pthread_mutexattr_setprotocol(&attr, 999);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

EXIT:
    (VOID)pthread_mutexattr_destroy(&attr);
    return LOS_OK;
}

VOID ItPosixMux083(void)
{
    TEST_ADD_CASE("ItPosixMux083", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
