#include "It_posix_pthread.h"

extern int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackAddr);
extern int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackAddr);

static UINT32 Testcase(VOID)
{
    pthread_attr_t attr;
    char stackBuf[64] = {0};
    void *stackAddr = NULL;
    UINT32 ret;

    ret = pthread_attr_init(&attr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_setstackaddr(&attr, stackBuf);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = pthread_attr_getstackaddr(&attr, &stackAddr);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ICUNIT_GOTO_EQUAL(stackAddr, stackBuf, stackAddr, EXIT);

    ret = pthread_attr_setstackaddr(NULL, stackBuf);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_attr_getstackaddr(NULL, &stackAddr);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_attr_getstackaddr(&attr, NULL);
    ICUNIT_GOTO_EQUAL(ret, EINVAL, ret, EXIT);

    ret = pthread_attr_destroy(&attr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;

EXIT:
    (VOID)pthread_attr_destroy(&attr);
    return LOS_NOK;
}

/**
 * @tc.name: ItPosixPthread029
 * @tc.desc: Test interface pthread_attr_setstackaddr and pthread_attr_getstackaddr
 * @tc.type: FUNC
 */

VOID ItPosixPthread029(VOID)
{
    TEST_ADD_CASE("ItPosixPthread029", Testcase, TEST_POSIX, TEST_PTHREAD, TEST_LEVEL0, TEST_FUNCTION);
}