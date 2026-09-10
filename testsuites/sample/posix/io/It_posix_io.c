#include "It_posix_io.h"

VOID ItSuitePosixIo(void)
{
    dprintf("*********** Begin sample posix pipe test ************\n");
    ItPosixPipe001();
    ItPosixPipe002();
#include "bug_isolate.h"
#ifndef PRODUCT_BUG_ISOLATE
    ItPosixPipe003();
#endif
}