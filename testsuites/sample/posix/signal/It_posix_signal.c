#include "It_posix_signal.h"

VOID ItSuitePosixSignal(void)
{
    dprintf("*********** Begin sample posix signal test ************\n");
    ItPosixSignal084();
    ItPosixSignal085();
    ItPosixSignal086();
    ItPosixSignal087();
    ItPosixSignal088();
    ItPosixSignal089();
    ItPosixSignal090();
#include "bug_isolate.h"
#ifndef PRODUCT_BUG_ISOLATE
    ItPosixSignal091();
#endif
    ItPosixSignal092();
    ItPosixSignal093();
}
