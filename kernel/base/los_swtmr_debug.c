/* ----------------------------------------------------------------------------
 * Copyright (c) 2024. All rights reserved.
 * Description: Debug Swtmr
 * --------------------------------------------------------------------------- */

#include "los_swtmr_pri.h"
#include "los_debug.h"

#ifdef LOSCFG_DEBUG_SWTMR

#define SWTMR_STRLEN  12

STATIC CHAR g_shellSwtmrMode[][SWTMR_STRLEN] = {
    "Once",
    "Period",
    "NSD",
    "OPP",
};

STATIC CHAR g_shellSwtmrStatus[][SWTMR_STRLEN] = {
    "UnUsed",
    "Created",
    "Ticking",
    "Deleting",
};

STATIC VOID OsPrintSwtmrMsg(const SWTMR_CTRL_S *swtmr)
{
    PRINTK("0x%08x  "
           "%-7s  "
           "%-6s  "
           "%-6u    "
           "0x%-08x          "
           "0x%lx\n",
           swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT,
           g_shellSwtmrStatus[swtmr->ucState],
           g_shellSwtmrMode[swtmr->ucMode],
           swtmr->uwInterval,
           swtmr->uwArg,
           (unsigned long)swtmr->pfnHandler);
}

STATIC INLINE VOID OsPrintSwtmrMsgHead(VOID)
{
    PRINTK("\r\nSwTmrID     State    Mode    Interval  Arg                 handlerAddr\n");
    PRINTK("----------  -------  ------- --------- ----------          --------\n");
}

LITE_OS_SEC_TEXT_MINOR UINT32 OsDbgSwtmrInfoGet(UINT32 timerId)
{
    SWTMR_CTRL_S *swtmr = g_swtmrCBArray;
    UINT16 index;
    UINT16 num = 0;

    OsPrintSwtmrMsgHead();
    if (timerId == OS_ALL_SWTMR_MASK) {
        for (index = 0; index < LOSCFG_BASE_CORE_SWTMR_LIMIT; ++index, ++swtmr) {
            if (swtmr->ucState != 0) {
                OsPrintSwtmrMsg(swtmr);
                num++;
            }
        }

        if (num == 0) {
            PRINTK("\r\nThere is no swtmr was created!\n");
            return OS_ERROR;
        }
    } else {
        for (index = 0; index < LOSCFG_BASE_CORE_SWTMR_LIMIT; ++index, ++swtmr) {
            if ((timerId == (swtmr->usTimerID % LOSCFG_BASE_CORE_SWTMR_LIMIT)) && (swtmr->ucState != 0)) {
                OsPrintSwtmrMsg(swtmr);
                return LOS_OK;
            }
        }
        PRINTK("\r\nThe SwTimerID is not exist.\n");
    }
    return LOS_OK;
}
#endif /* LOSCFG_DEBUG_SWTMR */
