#include "It_posix_io.h"

#define PIPE_TEST_BUF_LEN 32

static UINT32 Testcase(VOID)
{
    int fds[2];
    char writeBuf[PIPE_TEST_BUF_LEN] = "pipe test data";
    char readBuf[PIPE_TEST_BUF_LEN] = {0};
    ssize_t ret;

    ret = pipe(fds);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = write(fds[1], writeBuf, strlen(writeBuf));
    ICUNIT_GOTO_EQUAL(ret, (ssize_t)strlen(writeBuf), ret, EXIT);

    ret = read(fds[0], readBuf, PIPE_TEST_BUF_LEN);
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

VOID ItPosixPipe001(void)
{
    TEST_ADD_CASE("ItPosixPipe001", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}