#include "It_posix_io.h"

static UINT32 Testcase(VOID)
{
    int fds[2];
    char writeBuf[16] = "double write";
    char readBuf[16] = {0};
    ssize_t ret;

    ret = pipe(fds);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = write(fds[1], writeBuf, strlen(writeBuf));
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)strlen(writeBuf), ret, EXIT);

    ret = write(fds[1], writeBuf, strlen(writeBuf));
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)strlen(writeBuf), ret, EXIT);

    ret = read(fds[0], readBuf, strlen(writeBuf));
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)strlen(writeBuf), ret, EXIT);
    ICUNIT_GOTO_EQUAL(memcmp(readBuf, writeBuf, strlen(writeBuf)), 0, readBuf[0], EXIT);

    (void)memset_s(readBuf, sizeof(readBuf), 0, sizeof(readBuf));
    ret = read(fds[0], readBuf, strlen(writeBuf));
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)strlen(writeBuf), ret, EXIT);
    ICUNIT_GOTO_EQUAL(memcmp(readBuf, writeBuf, strlen(writeBuf)), 0, readBuf[0], EXIT);

    (void)close(fds[0]);
    (void)close(fds[1]);
    return LOS_OK;

EXIT:
    (void)close(fds[0]);
    (void)close(fds[1]);
    return LOS_NOK;
}

VOID ItPosixPipe002(void)
{
    TEST_ADD_CASE("ItPosixPipe002", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}