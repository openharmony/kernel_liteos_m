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
#include <stdarg.h>
#include "securec.h"
#include "los_context.h"
#include "los_arch_interrupt.h"
#include "los_hook.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_memory.h"
#include "los_membox.h"
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
#include "los_cpup.h"
#endif

#define OS_INT_IRQ_ENABLE           (1U << 0)
#define OS_INT_FIQ_ENABLE           (1U << 1)
#define OS_INT_REG_BASE             0x00802040UL
#define OS_INT_GLOBAL_ENABLE_ADDR   (OS_INT_REG_BASE + 4)
#define OS_INT_ENABLE_ADDR          (OS_INT_REG_BASE)
#define OS_INT_STATUS_ADDR          (OS_INT_REG_BASE + 12)

#define OS_INSTR_SET_MASK           0x01000020U
#define OS_ARM_INSTR_LEN            4
#define OS_THUMB_INSTR_LEN          2

ExcInfo g_excInfo = {0};

/* ****************************************************************************
 Function    : HwiNumGet
 Description : Get an interrupt number
 Input       : None
 Output      : None
 Return      : Interrupt Indexes number
 **************************************************************************** */
STATIC UINT32 HwiNumGet(VOID)
{
    UINT32 status;

    READ_UINT32(status, OS_INT_STATUS_ADDR);

    return (31 - CLZ(status));
}

STATIC UINT32 HwiUnmask(HWI_HANDLE_T hwiNum)
{
    *((volatile UINT32 *)OS_INT_ENABLE_ADDR) |= (1U << (hwiNum));

    return LOS_OK;
}

STATIC UINT32 HwiMask(HWI_HANDLE_T hwiNum)
{
    *((volatile UINT32 *)OS_INT_ENABLE_ADDR) &= ~(1U << (hwiNum));

    return LOS_OK;
}

STATIC UINT32 HwiCreate(HWI_HANDLE_T hwiNum, HWI_PRIOR_T hwiPrio)
{
    (VOID)hwiPrio;
    HwiUnmask(hwiNum);
    return LOS_OK;
}

STATIC HwiControllerOps g_archHwiOps = {
    .enableIrq      = HwiUnmask,
    .disableIrq     = HwiMask,
    .getCurIrqNum   = HwiNumGet,
    .createIrq      = HwiCreate,
};

HwiControllerOps *ArchIntOpsGet(VOID)
{
    return &g_archHwiOps;
}

/* ****************************************************************************
 Function    : HalInterrupt
 Description : Hardware interrupt entry function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT VOID HalInterrupt(VOID)
{
    UINT32 intSave;
    UINT32 hwiIndex;

    intSave = LOS_IntLock();
    g_intCount++;
    LOS_IntRestore(intSave);

#if (LOSCFG_BASE_CORE_SCHED_SLEEP == 1)
    OsSchedUpdateSleepTime();
#endif

    hwiIndex = HwiNumGet();

    OsHookCall(LOS_HOOK_TYPE_ISR_ENTER, hwiIndex);
#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    OsCpupIrqStart(hwiIndex);
#endif

    HalPreInterruptHandler(hwiIndex);

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
    if (g_hwiHandlerForm[hwiIndex].pfnHandler != 0) {
        g_hwiHandlerForm[hwiIndex].pfnHandler((VOID *)g_hwiHandlerForm[hwiIndex].pParm);
    }
#else
    if (g_hwiHandlerForm[hwiIndex] != 0) {
        g_hwiHandlerForm[hwiIndex]();
    }
#endif

#if (LOSCFG_DEBUG_TOOLS == 1)
    ++g_hwiFormCnt[hwiIndex];
#endif

    HalAftInterruptHandler(hwiIndex);

#if (LOSCFG_CPUP_INCLUDE_IRQ == 1)
    OsCpupIrqEnd(hwiIndex);
#endif

    OsHookCall(LOS_HOOK_TYPE_ISR_EXIT, hwiIndex);

    intSave = LOS_IntLock();
    g_intCount--;
    LOS_IntRestore(intSave);
}

#if (LOSCFG_KERNEL_PRINTF != 0)
STATIC VOID OsExcTypeInfo(const ExcInfo *excInfo)
{
    CHAR *phaseStr[] = {"exc in init", "exc in task", "exc in hwi"};

    PRINTK("Type      = %d\n", excInfo->type);
    PRINTK("ThrdPid   = %d\n", excInfo->thrdPid);
    PRINTK("Phase     = %s\n", phaseStr[excInfo->phase]);
    PRINTK("FaultAddr = 0x%x\n", excInfo->faultAddr);
}

STATIC VOID OsExcCurTaskInfo(const ExcInfo *excInfo)
{
    PRINTK("Current task info:\n");
    if (excInfo->phase == OS_EXC_IN_TASK) {
        LosTaskCB *taskCB = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
        PRINTK("Task name = %s\n", taskCB->taskName);
        PRINTK("Task ID   = %d\n", taskCB->taskID);
        PRINTK("Task SP   = 0x%x\n", taskCB->stackPointer);
        PRINTK("Task ST   = 0x%x\n", taskCB->topOfStack);
        PRINTK("Task SS   = 0x%x\n", taskCB->stackSize);
    } else if (excInfo->phase == OS_EXC_IN_HWI) {
        PRINTK("Exception occur in interrupt phase!\n");
    } else {
        PRINTK("Exception occur in system init phase!\n");
    }
}

STATIC VOID OsExcRegInfo(const ExcInfo *excInfo)
{
    PRINTK("Exception reg dump:\n");
    PRINTK("PC        = 0x%x\n", excInfo->context->pc);
    PRINTK("LR        = 0x%x\n", excInfo->context->lr);
    PRINTK("R0        = 0x%x\n", excInfo->context->r0);
    PRINTK("R1        = 0x%x\n", excInfo->context->r1);
    PRINTK("R2        = 0x%x\n", excInfo->context->r2);
    PRINTK("R3        = 0x%x\n", excInfo->context->r3);
    PRINTK("R4        = 0x%x\n", excInfo->context->r4);
    PRINTK("R5        = 0x%x\n", excInfo->context->r5);
    PRINTK("R6        = 0x%x\n", excInfo->context->r6);
    PRINTK("R7        = 0x%x\n", excInfo->context->r7);
    PRINTK("R8        = 0x%x\n", excInfo->context->r8);
    PRINTK("R9        = 0x%x\n", excInfo->context->r9);
    PRINTK("R10       = 0x%x\n", excInfo->context->r10);
    PRINTK("R11       = 0x%x\n", excInfo->context->r11);
    PRINTK("R12       = 0x%x\n", excInfo->context->r12);
    PRINTK("xPSR      = 0x%x\n", excInfo->context->spsr);
}

#if (LOSCFG_KERNEL_BACKTRACE == 1)
STATIC VOID OsExcBackTraceInfo(const ExcInfo *excInfo)
{
    UINTPTR LR[LOSCFG_BACKTRACE_DEPTH] = {0};
    UINT32 index;

    OsBackTraceHookCall(LR, LOSCFG_BACKTRACE_DEPTH, 0, excInfo->context->sp);

    PRINTK("----- backtrace start -----\n");
    for (index = 0; index < LOSCFG_BACKTRACE_DEPTH; index++) {
        if (LR[index] == 0) {
            break;
        }
        PRINTK("backtrace %d -- lr = 0x%x\n", index, LR[index]);
    }
    PRINTK("----- backtrace end -----\n");
}
#endif

STATIC VOID OsExcMemPoolCheckInfo(VOID)
{
    PRINTK("\r\nmemory pools check:\n");
#if (LOSCFG_PLATFORM_EXC == 1)
    MemInfoCB memExcInfo[OS_SYS_MEM_NUM];
    UINT32 errCnt;
    UINT32 i;

    (VOID)memset_s(memExcInfo, sizeof(memExcInfo), 0, sizeof(memExcInfo));

    errCnt = OsMemExcInfoGet(OS_SYS_MEM_NUM, memExcInfo);
    if (errCnt < OS_SYS_MEM_NUM) {
        errCnt += OsMemboxExcInfoGet(OS_SYS_MEM_NUM - errCnt, memExcInfo + errCnt);
    }

    if (errCnt == 0) {
        PRINTK("all memory pool check passed!\n");
        return;
    }

    for (i = 0; i < errCnt; i++) {
        PRINTK("pool num    = %d\n", i);
        PRINTK("pool type   = %d\n", memExcInfo[i].type);
        PRINTK("pool addr   = 0x%x\n", memExcInfo[i].startAddr);
        PRINTK("pool size   = 0x%x\n", memExcInfo[i].size);
        PRINTK("pool free   = 0x%x\n", memExcInfo[i].free);
        PRINTK("pool blkNum = %d\n", memExcInfo[i].blockSize);
        PRINTK("pool error node addr  = 0x%x\n", memExcInfo[i].errorAddr);
        PRINTK("pool error node len   = 0x%x\n", memExcInfo[i].errorLen);
        PRINTK("pool error node owner = %d\n", memExcInfo[i].errorOwner);
    }
#endif
    UINT32 ret = LOS_MemIntegrityCheck(LOSCFG_SYS_HEAP_ADDR);
    if (ret == LOS_OK) {
        PRINTK("system heap memcheck over, all passed!\n");
    }

    PRINTK("memory pool check end!\n");
}
#endif

STATIC VOID OsExcInfoDisplay(const ExcInfo *excInfo)
{
#if (LOSCFG_KERNEL_PRINTF != 0)
    PRINTK("*************Exception Information**************\n");
    OsExcTypeInfo(excInfo);
    OsExcCurTaskInfo(excInfo);
    OsExcRegInfo(excInfo);
#if (LOSCFG_KERNEL_BACKTRACE == 1)
    OsExcBackTraceInfo(excInfo);
#endif
    OsGetAllTskInfo();
    OsExcMemPoolCheckInfo();
#endif
}

LITE_OS_SEC_TEXT_INIT VOID HalExcHandleEntry(UINT32 excType, UINT32 faultAddr, UINT32 pid, EXC_CONTEXT_S *excBufAddr)
{
    g_intCount++;
    g_excInfo.nestCnt++;

    g_excInfo.type = excType;

    if ((excType == OS_EXCEPT_UNDEF_INSTR) || (excType == OS_EXCEPT_SWI)) {
        if ((excBufAddr->spsr & OS_INSTR_SET_MASK) == 0) { /* Work status: ARM */
            excBufAddr->pc -= OS_ARM_INSTR_LEN;
        } else if ((excBufAddr->spsr & OS_INSTR_SET_MASK) == 0x20) { /* Work status: Thumb */
            excBufAddr->pc -= OS_THUMB_INSTR_LEN;
        }
    }
    g_excInfo.faultAddr = OS_EXC_IMPRECISE_ACCESS_ADDR;

    if (g_losTask.runTask != NULL) {
        g_excInfo.phase = OS_EXC_IN_TASK;
        g_excInfo.thrdPid = g_losTask.runTask->taskID;
    } else {
        g_excInfo.phase = OS_EXC_IN_INIT;
        g_excInfo.thrdPid = OS_NULL_INT;
    }
    g_excInfo.context = excBufAddr;

    OsDoExcHook(EXC_INTERRUPT);
    OsExcInfoDisplay(&g_excInfo);
    ArchSysExit();
}

/* ****************************************************************************
 Function    : HalHwiInit
 Description : initialization of the hardware interrupt
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID HalHwiInit(VOID)
{
#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
    UINT32 reg;
    UINT32 val;
    HWI_PROC_FUNC *hwiForm = (HWI_PROC_FUNC *)ArchGetHwiFrom();

    for (val = OS_SYS_VECTOR_CNT; val < OS_VECTOR_CNT; val++) {
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
        hwiForm[val].pfnHook = HalHwiDefaultHandler;
        hwiForm[val].uwParam = 0;
#else
        hwiForm[val] = (HWI_PROC_FUNC)HalHwiDefaultHandler;
#endif
    }

    val = OS_INT_IRQ_ENABLE | OS_INT_FIQ_ENABLE;
    READ_UINT32(reg, OS_INT_GLOBAL_ENABLE_ADDR);
    reg |= val;
    WRITE_UINT32(reg, OS_INT_GLOBAL_ENABLE_ADDR);
#endif
    return;
}

UINT32 ArchIntLock(VOID)
{
    UINT32 ret;
    UINT32 temp;

    __asm__ __volatile__("MRS %0, CPSR\n"
                         "ORR %1, %0, #0xC0\n"
                         "MSR CPSR_c, %1"
                         : "=r"(ret), "=r"(temp)
                         :
                         : "memory");
    return ret;
}

VOID ArchIntRestore(UINT32 intSave)
{
    __asm__ __volatile__("MSR CPSR_c, %0" : : "r"(intSave));
}

UINT32 ArchIntUnLock(VOID)
{
    UINT32 intSave;

    __asm__ __volatile__("MRS %0, CPSR\n"
                         "BIC %0, %0, #0xC0\n"
                         "MSR CPSR_c, %0"
                         : "=r"(intSave)
                         :
                         : "memory");
    return intSave;
}
