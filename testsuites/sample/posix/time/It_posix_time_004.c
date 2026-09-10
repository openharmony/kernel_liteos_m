#include "It_posix_time.h"

static volatile int g_timer004Fired = 0;

static VOID Timer004Callback(union sigval sv)
{
    g_timer004Fired = 1;
}

static UINT32 Testcase(VOID)
{
    timer_t timerID;
    struct sigevent evp;
    int ret;

    g_timer004Fired = 0;
    memset(&evp, 0, sizeof(evp));
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = Timer004Callback;
    evp.sigev_value.sival_ptr = NULL;

    ret = timer_create(CLOCK_REALTIME, &evp, &timerID);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    ret = timer_delete(timerID);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;

EXIT:
    timer_delete(timerID);
    return LOS_OK;
}

VOID ItPosixTime004(void)
{
    TEST_ADD_CASE("ItPosixTime004", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}