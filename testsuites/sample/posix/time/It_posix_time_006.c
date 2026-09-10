#include "It_posix_time.h"

static volatile int g_timerExpired = 0;

static VOID TimerCallback(union sigval sv)
{
    g_timerExpired = 1;
}

static UINT32 Testcase(VOID)
{
    timer_t timerID;
    struct sigevent evp;
    struct itimerspec value;
    struct itimerspec oldValue;
    int ret;

    g_timerExpired = 0;
    memset(&evp, 0, sizeof(evp));
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = TimerCallback;
    evp.sigev_value.sival_ptr = NULL;

    ret = timer_create(CLOCK_REALTIME, &evp, &timerID);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    value.it_value.tv_sec = 0;
    value.it_value.tv_nsec = 10000000;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_nsec = 0;

    ret = timer_settime(timerID, 0, &value, &oldValue);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);

    LOS_TaskDelay(20);

    ret = timer_delete(timerID);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    return LOS_OK;

EXIT:
    timer_delete(timerID);
    return LOS_OK;
}

VOID ItPosixTime006(void)
{
    TEST_ADD_CASE("ItPosixTime006", Testcase, TEST_POSIX, TEST_MUX, TEST_LEVEL0, TEST_FUNCTION);
}