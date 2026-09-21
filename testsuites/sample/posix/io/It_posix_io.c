#include "It_posix_io.h"

VOID ItSuitePosixIo(void)
{
    dprintf("*********** Begin sample posix pipe test ************\n");
    ItPosixPipe001();
    ItPosixPipe002();
    ItPosixPipe004();
    ItPosixPipe005();
    ItPosixPipe006();
    ItPosixPipe007();
}