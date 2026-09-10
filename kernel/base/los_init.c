/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2023 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "stdarg.h"
#include "los_arch.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_memory_pri.h"
#include "los_mux.h"
#include "los_queue.h"
#include "los_sem.h"
#include "los_rwsem_pri.h"
#include "los_init.h"
#include "los_printf_pri.h"
#include "los_sched.h"
#include "securec.h"

#if (LOSCFG_PLATFORM_HWI == 1)
#include "los_interrupt.h"
#endif

#if (LOSCFG_BASE_CORE_SWTMR == 1)
#include "los_swtmr.h"
#endif

#if (LOSCFG_BASE_CORE_CPUP == 1)
#include "los_cpup_pri.h"
#endif
#include "los_sched_pri.h"   /* LOS_Start 无条件调 OsSchedStart;上游只在 CPUP 开时引,A7 CPUP 关则未声明 */

#ifdef LOSCFG_MEM_TASK_STAT
#include "los_memstat_pri.h"
#endif

#if (LOSCFG_PLATFORM_EXC == 1)
#include "los_exc_info.h"
#endif

#if (LOSCFG_BACKTRACE_TYPE != 0)
#include "los_backtrace.h"
#endif

#if (LOSCFG_KERNEL_PM == 1)
#include "los_pm.h"
#endif

#if (LOSCFG_DYNLINK == 1)
#include "los_dynlink.h"
#endif

#ifdef LOSCFG_KERNEL_LMS
#include "los_lms_pri.h"
#endif

#if (LOSCFG_KERNEL_LMK == 1)
#include "los_lmk.h"
#endif

#if (LOSCFG_POSIX_PIPE_API == 1)
#include "pipe_impl.h"
#endif

#if (LOSCFG_KERNEL_SIGNAL == 1)
#include "los_signal.h"
#endif

#if (LOSCFG_SECURE == 1)
#include "los_syscall.h"
#include "los_box.h"
#endif

#if (LOSCFG_FS_VFS == 1)
#include "vfs_operations.h"
#endif

#if (LOSCFG_KERNEL_TRACE == 1)
#include "los_trace_pri.h"
#endif

/*****************************************************************************
 Function    : LOS_Reboot
 Description : system exception, die in here, wait for watchdog.
 Input       : None
 Output      : None
 Return      : None
 *****************************************************************************/
LITE_OS_SEC_TEXT_INIT VOID LOS_Reboot(VOID)
{
    OsDoExcHook(EXC_REBOOT);
    ArchTaskExit();
}

LITE_OS_SEC_TEXT_INIT VOID LOS_Panic(const CHAR *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
#if (LOSCFG_KERNEL_PRINTF == 1)
    UartVprintf(fmt, ap);
#elif (LOSCFG_KERNEL_PRINTF > 1)
    {
        CHAR buf[256];
        va_list apCopy;
        va_copy(apCopy, ap);
        INT32 len = vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, fmt, apCopy);
        va_end(apCopy);
        if (len > 0) {
            HalConsoleOutput(LOG_MODULE_KERNEL, LOG_ERR_LEVEL, "%s", buf);
        }
    }
#endif
    va_end(ap);
    OsDoExcHook(EXC_PANIC);
#if (LOSCFG_BACKTRACE_TYPE != 0)
    LOS_BackTrace();
#endif
    ArchTaskExit();
}

LITE_OS_SEC_TEXT_INIT UINT32 LOS_Start(VOID)
{
    OsSchedStart();
    return LOS_OK;
}

/* ========================================================================== */
/* Initcall mechanism                                                         */
/* ========================================================================== */

#ifdef LOSCFG_MEM_TASK_STAT
/* extra 1 blocks is for extra temporary task */
TaskMemUsedInfo g_taskMemstats[LOSCFG_BASE_CORE_TSK_LIMIT + 1] = {0};
#endif

LITE_OS_SEC_TEXT_INIT STATIC UINT32 OsRegister(VOID)
{
#ifdef LOSCFG_MEM_TASK_STAT
    g_osTaskMemstats = g_taskMemstats;
#endif
    return LOS_OK;
}

/* OsIpcInit: consolidate sem/mux/queue/rwsem init into one initcall */
LITE_OS_SEC_TEXT_INIT STATIC UINT32 OsIpcInit(VOID)
{
    UINT32 ret;

#if (LOSCFG_BASE_IPC_SEM == 1)
    ret = OsSemInit();
    if (ret != LOS_OK) {
        return ret;
    }
#endif

#if (LOSCFG_BASE_IPC_MUX == 1)
    ret = OsMuxInit();
    if (ret != LOS_OK) {
        return ret;
    }
#endif

#if (LOSCFG_BASE_IPC_QUEUE == 1)
    ret = OsQueueInit();
    if (ret != LOS_OK) {
        PRINT_ERR("OsQueueInit error\n");
        return ret;
    }
#endif

#if (LOSCFG_BASE_IPC_RWSEM == 1)
    ret = OsRwsemInit();
    if (ret != LOS_OK) {
        PRINT_ERR("OsRwsemInit error\n");
        return ret;
    }
#endif

    return LOS_OK;
}
LOS_SYS_INIT(OsIpcInit, SYS_INIT_LEVEL_KERNEL, SYS_INIT_SYNC_2);

UINT32 OsArchInit(VOID)
{
    ArchInit();
    return LOS_OK;
}
LOS_SYS_INIT(OsArchInit, SYS_INIT_LEVEL_ARCH, SYS_INIT_SYNC_0);

#if (LOSCFG_KERNEL_INITCALL == 1)
/*
 * System init as follow level:
 *   level0: SYS_INIT_LEVEL_EARLY:  NONE
 *
 *   level1: SYS_INIT_LEVEL_AHEAD
 *      sync0: OsMemSystemInit
 *
 *   level2: SYS_INIT_LEVEL_ARCH:
 *      sync0: OsArchInit
 *      sync1: OsTickTimerInit
 *
 *   level3: SYS_INIT_LEVEL_KERNEL:
 *      sync0: OsTaskInit, OsTaskMonInit
 *      sync1: OsCpupInit,
 *      sync2: OsIpcInit
 *      sync3: OsSwtmrInit, OsIdleTaskCreate
 *
 *   level4: SYS_INIT_LEVEL_KERNEL_ADDITION:  NONE
 *
 *   level5: SYS_INIT_LEVEL_COMPONENT:  NONE
 *
 *   level6: SYS_INIT_LEVEL_APP:  NONE
 *
 *   level7: SYS_INIT_LEVEL_RESERVE:  NONE
 */
LITE_OS_SEC_TEXT_INIT UINT32 OsMain(VOID)
{
    UINT32 ret;
    SysInitcallFunc *func = NULL;

    PRINTK("Init using initcall.\n");

    for (func = __sysinitcall0_start; func < __sysinitcall_end; func++) {
        ret = (UINT32)(*func)();
        if (ret != LOS_OK) {
            PRINT_ERR("OsMain init err %u.\n", ret);
            return ret;
        }
    }
    return LOS_OK;
}
#else
LITE_OS_SEC_TEXT_INIT UINT32 OsMain(VOID)
{
    UINT32 ret;

    ret = OsMemSystemInit();
    if (ret != LOS_OK) {
        PRINT_ERR("OsMemSystemInit error %d\n", ret);
        return ret;
    }

    ArchInit();

    ret = OsTickTimerInit();
    if (ret != LOS_OK) {
        PRINT_ERR("OsTickTimerInit error! 0x%x\n", ret);
        return ret;
    }

    ret = OsTaskInit();
    if (ret != LOS_OK) {
        PRINT_ERR("OsTaskInit error\n");
        return ret;
    }

#if (LOSCFG_BASE_CORE_TSK_MONITOR == 1)
    ret = OsTaskMonInit();
    if (ret != LOS_OK) {
        return ret;
    }
#endif

#if (LOSCFG_BASE_CORE_CPUP == 1)
    ret = OsCpupInit();
    if (ret != LOS_OK) {
        PRINT_ERR("OsCpupInit error\n");
        return ret;
    }
#endif

    ret = OsIpcInit();
    if (ret != LOS_OK) {
        return ret;
    }

#if (LOSCFG_BASE_CORE_SWTMR == 1)
    ret = OsSwtmrInit();
    if (ret != LOS_OK) {
        PRINT_ERR("OsSwtmrInit error\n");
        return ret;
    }
#endif

    ret = OsIdleTaskCreate();
    if (ret != LOS_OK) {
        return ret;
    }

    return LOS_OK;
}
#endif

LITE_OS_SEC_TEXT_INIT UINT32 LOS_KernelInit(VOID)
{
    UINT32 ret;
    PRINTK("entering kernel init...\n");

    ret = OsRegister();
    if (ret != LOS_OK) {
        return ret;
    }

#if (LOSCFG_BACKTRACE_TYPE != 0)
    OsBackTraceInit();
#endif

#ifdef LOSCFG_KERNEL_LMS
    OsLmsInit();
#endif

    /* Run all initcall-registered functions in order */
    ret = OsMain();
    if (ret != LOS_OK) {
        return ret;
    }

    /* Non-kernel-base modules not migrated to initcall yet */
#if (LOSCFG_FS_VFS == 1)
    ret = OsVfsInit();
    if (ret != LOS_OK) {
        PRINT_ERR("OsVfsInit error\n");
        return ret;
    }
#endif

#if (LOSCFG_KERNEL_PM == 1)
    ret = OsPmInit();
    if (ret != LOS_OK) {
        PRINT_ERR("Pm init failed!\n");
        return ret;
    }
#endif

#if (LOSCFG_KERNEL_LMK == 1)
    OsLmkInit();
#endif

#if (LOSCFG_PLATFORM_EXC == 1)
    OsExcMsgDumpInit();
#endif

#if (LOSCFG_DYNLINK == 1)
    ret = LOS_DynlinkInit();
    if (ret != LOS_OK) {
        return ret;
    }
#endif

#if (LOSCFG_POSIX_PIPE_API == 1)
    ret = OsPipeInit();
    if (ret != LOS_OK) {
        PRINT_ERR("Pipe init failed!\n");
        return ret;
    }
#endif

#if (LOSCFG_KERNEL_SIGNAL == 1)
    ret = OsSignalInit();
    if (ret != LOS_OK) {
        PRINT_ERR("Signal init failed!\n");
        return ret;
    }
#endif

#if (LOSCFG_SECURE == 1)
    OsSyscallHandleInit();
    LOS_BoxStart();
#endif

    return LOS_OK;
}
