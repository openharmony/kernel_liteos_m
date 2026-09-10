#include "It_posix_time.h"

static UINT32 Testcase(VOID)
{
    int *pErrno = NULL;
    int oldErrno;

    pErrno = __errno_location();
    ICUNIT_ASSERT_NOT_EQUAL(pErrno, NULL, pErrno);

    oldErrno = *pErrno;
    *pErrno = ENOENT;
    ICUNIT_ASSERT_EQUAL(*pErrno, ENOENT, *pErrno);
    ICUNIT_ASSERT_EQUAL(errno, ENOENT, errno);

    *pErrno = oldErrno;

    errno = EINVAL;
    ICUNIT_ASSERT_EQUAL(errno, EINVAL, errno);

    errno = oldErrno;

    return LOS_OK;
}

VOID ItPosixTime030(void)
{
    TEST_ADD_CASE("ItPosixTime030", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}
