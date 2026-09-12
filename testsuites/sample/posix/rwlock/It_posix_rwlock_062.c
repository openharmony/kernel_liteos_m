
#include "It_posix_rwlock.h"

static UINT32 Testcase(void)
{
    pthread_rwlock_t rwl;
    int ret;

    ret = pthread_rwlock_init(&rwl, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_rwlock_init(&rwl, NULL);
    ICUNIT_GOTO_EQUAL(ret, EBUSY, ret, EXIT);

    ret = pthread_rwlock_destroy(&rwl);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

EXIT:
    pthread_rwlock_destroy(&rwl);
    return LOS_OK;
}

VOID ItPosixRwlock062(void)
{
    TEST_ADD_CASE("ItPosixRwlock062", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL2, TEST_FUNCTION);
}
