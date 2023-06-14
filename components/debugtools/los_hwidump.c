/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_debugtools.h"
#include "securec.h"
#include "los_debug.h"
#include "los_memory.h"
#include "los_arch.h"
#include "los_interrupt.h"
#include "los_arch_interrupt.h"

#if (LOSCFG_DEBUG_TOOLS == 1)

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1) && (LOSCFG_BASE_CORE_SWTMR == 1)
#include "los_cpup.h"

#define IRQ_CPUP_INFO_SIZE     (sizeof(CPUP_INFO_S) * LOSCFG_PLATFORM_HWI_LIMIT)
#define IRQ_CPUP_ALL_INFO_SIZE (IRQ_CPUP_INFO_SIZE + IRQ_CPUP_INFO_SIZE)
#define IRQ_DATA_SIZE          sizeof(OsIrqCpupCB)
#define CPUP_PRECISION_MULT    LOS_CPUP_PRECISION_MULT

STATIC VOID ShellCmdHwiInfoShow(OsIrqCpupCB *irqData, CPUP_INFO_S *hwiCpup1s,
                                CPUP_INFO_S *hwiCpup10s)
{
    UINT32 i;
    UINT32 intSave;
    UINT32 count;
    UINT64 cycles = 0;
    UINT64 timeMax = 0;
    CHAR *irqName = NULL;

    OsIrqCpupCB *irqDataBase = OsGetIrqCpupArrayBase();
    if (irqDataBase == NULL) {
        PRINT_ERR("get hwi info error\n");
        return;
    }

    for (i = 0; i < LOSCFG_PLATFORM_HWI_LIMIT; i++) {
        if ((OsHwiIsCreated(i) != TRUE) || (OsGetHwiFormCnt(i) == 0)) {
            continue;
        }

        intSave = LOS_IntLock();
        (VOID)memcpy_s(irqData, IRQ_DATA_SIZE, &irqDataBase[i], IRQ_DATA_SIZE);
        LOS_IntRestore(intSave);

        if (irqData->status == 0) {
            continue;
        }

        count = OsGetHwiFormCnt(i);
        if (count != 0) {
            if (irqData->count != 0) {
                cycles = (irqData->allTime * OS_NS_PER_CYCLE) / (irqData->count * OS_SYS_NS_PER_US);
            }
            timeMax = (irqData->timeMax * OS_NS_PER_CYCLE) / OS_SYS_NS_PER_US;
        }
        irqName = OsGetHwiFormName(i);

        PRINTK(" %10d:%11u%11llu%10llu%9u.%-2u%9u.%-2u %-12s\n", i - OS_SYS_VECTOR_CNT, count, cycles, timeMax,
               hwiCpup1s[i].uwUsage / CPUP_PRECISION_MULT, hwiCpup1s[i].uwUsage % CPUP_PRECISION_MULT,
               hwiCpup10s[i].uwUsage / CPUP_PRECISION_MULT, hwiCpup10s[i].uwUsage % CPUP_PRECISION_MULT,
               (irqName != NULL) ? irqName : NULL);
    }
}

STATIC VOID HwiInfoDump(VOID)
{
    UINT32 size;

    size = IRQ_CPUP_ALL_INFO_SIZE + IRQ_DATA_SIZE;
    CHAR *irqCpup = LOS_MemAlloc(m_aucSysMem0, size);
    if (irqCpup == NULL) {
        return;
    }

    CPUP_INFO_S *hwiCpup10s = (CPUP_INFO_S *)(irqCpup);
    CPUP_INFO_S *hwiCpup1s = (CPUP_INFO_S *)(hwiCpup10s + IRQ_CPUP_INFO_SIZE);
    OsIrqCpupCB *irqData = (OsIrqCpupCB *)(irqCpup + IRQ_CPUP_ALL_INFO_SIZE);

    (VOID)LOS_GetAllIrqCpuUsage(CPUP_IN_1S, hwiCpup1s);
    (VOID)LOS_GetAllIrqCpuUsage(CPUP_IN_10S, hwiCpup10s);

    PRINTK(" InterruptNo      Count  ATime(us)  MTime(us)  CPUUSE1s   CPUUSE10s  Name\n");
    ShellCmdHwiInfoShow(irqData, hwiCpup1s, hwiCpup10s);
    (VOID)LOS_MemFree(m_aucSysMem0, irqCpup);
    return;
}
#else
STATIC VOID HwiInfoDump(VOID)
{
    INT32 i;

    PRINTK(" InterruptNo     Count     Name\n");
    for (i = 0; i < LOSCFG_PLATFORM_HWI_LIMIT; i++) {
        if ((OsHwiIsCreated(i) != TRUE) || (OsGetHwiFormCnt(i) == 0)) {
            continue;
        }

        if (OsGetHwiFormName(i) != NULL) {
            PRINTK(" %8d:%10d      %-s\n", i - OS_SYS_VECTOR_CNT, OsGetHwiFormCnt(i), OsGetHwiFormName(i));
        } else {
            PRINTK(" %8d:%10d\n", i - OS_SYS_VECTOR_CNT, OsGetHwiFormCnt(i));
        }
    }
    return;
}
#endif

UINT32 OsShellCmdHwiDump(INT32 argc, const CHAR **argv)
{
    (VOID)argv;

    if (argc > 1) {
        PRINT_ERR("\nUsage:hwi\n");
        return LOS_NOK;
    }

    HwiInfoDump();
    return LOS_OK;
}
#endif /* LOSCFG_STACK_DUMP == 1 */
