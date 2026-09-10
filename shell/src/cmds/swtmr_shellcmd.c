/* ----------------------------------------------------------------------------
 * Copyright (c) 2024. All rights reserved.
 * Description: ShellCmd Swtmr
 * --------------------------------------------------------------------------- */
#include "los_swtmr_pri.h"
#include "los_debug.h"
#include "shcmd.h"
#include "shell.h"

#if defined(LOSCFG_SHELL) && defined(LOSCFG_DEBUG_SWTMR)

LITE_OS_SEC_TEXT_MINOR UINT32 OsShellCmdSwtmrInfoGet(UINT32 argc, const CHAR **argv)
{
    UINT32 timerId;
    CHAR *endPtr = NULL;

    if (argc > 1) {
        PRINTK("\nUsage: swtmr [ID]\n");
        return OS_ERROR;
    }

    if (argc == 0) {
        timerId = OS_ALL_SWTMR_MASK;
    } else {
        timerId = strtoul((CHAR *)argv[0], &endPtr, 0);
        if ((endPtr == NULL) || (*endPtr != 0) || (timerId > LOSCFG_BASE_CORE_SWTMR_LIMIT)) {
            PRINTK("\nswtmr ID can't access %s.\n", argv[0]);
            return OS_ERROR;
        }
    }

    return OsDbgSwtmrInfoGet(timerId);
}
#endif /* LOSCFG_SHELL && LOSCFG_DEBUG_SWTMR */
