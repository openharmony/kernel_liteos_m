/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_interrupt.h"
#include <stdarg.h>
#include "securec.h"
#include "los_context.h"
#include "los_arch_interrupt.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_memory.h"
#include "los_membox.h"
#include "los_arch_regs.h"

UINT32 g_intCount = 0;

/* *
 * @ingroup los_hwi
 * Hardware interrupt form mapping handling function array.
 */
STATIC HWI_PROC_FUNC __attribute__((aligned(LOSCFG_ARCH_HWI_VECTOR_ALIGN))) g_hwiForm[OS_VECTOR_CNT] = {0};

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)

typedef struct {
    HWI_PROC_FUNC pfnHandler;
    VOID *pParm;
} HWI_HANDLER_FUNC;

/* *
 * @ingroup los_hwi
 * Hardware interrupt handler form mapping handling function array.
 */
STATIC HWI_HANDLER_FUNC g_hwiHandlerForm[OS_VECTOR_CNT] = {{ (HWI_PROC_FUNC)0, (HWI_ARG_T)0 }};

/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector, VOID *arg)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalInterrupt;
        g_hwiHandlerForm[num + OS_SYS_VECTOR_CNT].pfnHandler = vector;
        g_hwiHandlerForm[num + OS_SYS_VECTOR_CNT].pParm = arg;
    }
}

#else
/* *
 * @ingroup los_hwi
 * Hardware interrupt handler form mapping handling function array.
 */
STATIC HWI_PROC_FUNC g_hwiHandlerForm[OS_VECTOR_CNT] = {0};

/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        g_hwiForm[num + OS_SYS_VECTOR_CNT] = HalInterrupt;
        g_hwiHandlerForm[num + OS_SYS_VECTOR_CNT] = vector;
    }
}
#endif

UINT32 HwiNumValid(UINT32 num)
{
    return (num >= OS_SYS_VECTOR_CNT) && (num <= OS_VECTOR_CNT);
}

/* *
 * @ingroup los_hwi
 * Lock all interrupt.
 */
UINT32 ArchIntLock(VOID)
{
    UINT32 ret;

    __asm__ volatile("rsil %0, %1" : "=r"(ret) : "i"(INT_MASK) : "memory");
    return ret;
}

/* *
 * @ingroup los_hwi
 * Restore interrupt status.
 */
VOID ArchIntRestore(UINT32 intSave)
{
    __asm__ volatile("wsr.ps %0; rsync" : : "r"(intSave) : "memory");
}

/* *
 * @ingroup los_hwi
 * Unlock interrupt.
 */
UINT32 ArchIntUnLock(VOID)
{
    UINT32 intSave;

    __asm__ volatile("rsil %0, %1" : "=r"(intSave) : "i"(0) : "memory");

    return intSave;
}

/* *
 * @ingroup los_hwi
 * Determine if the interrupt is locked
 */
STATIC INLINE UINT32 ArchIntLocked(VOID)
{
    UINT32 intSave;

    __asm__ volatile("rsr %0, ps " : "=r"(intSave) : : "memory");

    return (intSave & SPREG_PS_DI_MASK);
}

/* *
 * @ingroup los_hwi
 * Trigger the interrupt
 */
STATIC UINT32 HwiPending(HWI_HANDLE_T hwiNum)
{
    if (!HwiNumValid(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    __asm__ __volatile__("wsr   %0, intset; rsync" : : "a"(0x1U << hwiNum));

    return LOS_OK;
}

UINT32 HwiUnmask(HWI_HANDLE_T hwiNum)
{
    UINT32 ier;

    if (!HwiNumValid(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    __asm__ __volatile__("rsr %0, intenable" : "=a"(ier) : : "memory");
    __asm__ __volatile__("wsr %0, intenable; rsync" : : "a"(ier | ((UINT32)0x1U << hwiNum)));

    return LOS_OK;
}

UINT32 HwiMask(HWI_HANDLE_T hwiNum)
{
    UINT32 ier;

    if (!HwiNumValid(hwiNum)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    __asm__ __volatile__("rsr %0, intenable" : "=a"(ier) : : "memory");
    __asm__ __volatile__("wsr %0, intenable; rsync" : : "a"(ier & ~((UINT32)0x1U << hwiNum)));

    return LOS_OK;
}

/* ****************************************************************************
 Function    : HwiNumGet
 Description : Get an interrupt number
 Input       : None
 Output      : None
 Return      : Interrupt Indexes number
 **************************************************************************** */
STATIC UINT32 HwiNumGet(VOID)
{
    UINT32 ier;
    UINT32 intenable;
    UINT32 intSave;

    __asm__ __volatile__("rsr %0, interrupt" : "=a"(ier) : : "memory");
    __asm__ __volatile__("rsr %0, intenable" : "=a"(intenable) : : "memory");

    intSave = ier & intenable;

    return  __builtin_ffs(intSave) - 1;
}

/* *
 * @ingroup los_hwi
 * Clear the interrupt
 */
STATIC UINT32 HwiClear(HWI_HANDLE_T vector)
{
    if (!HwiNumValid(vector)) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    __asm__ __volatile__("wsr   %0, intclear; rsync" : : "a"(0x1U << vector));

    return LOS_OK;
}

HwiControllerOps g_archHwiOps = {
    .triggerIrq     = HwiPending,
    .enableIrq      = HwiUnmask,
    .disableIrq     = HwiMask,
    .getCurIrqNum   = HwiNumGet,
    .clearIrq       = HwiClear,
};

INLINE UINT32 ArchIsIntActive(VOID)
{
    return (g_intCount > 0);
}

/* ****************************************************************************
 Function    : HalHwiDefaultHandler
 Description : default handler of the hardware interrupt
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
VOID HalHwiDefaultHandler(VOID)
{
    PRINT_ERR("%s irqnum:%u\n", __FUNCTION__, HwiNumGet());
    while (1) {}
}

WEAK VOID HalPreInterruptHandler(UINT32 arg)
{
    (VOID)arg;
    return;
}

WEAK VOID HalAftInterruptHandler(UINT32 arg)
{
    (VOID)arg;
    return;
}

/* ****************************************************************************
 Function    : HalInterrupt
 Description : Hardware interrupt entry function
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
VOID HalInterrupt(VOID)
{
    UINT32 hwiIndex;
    UINT32 intSave;

    intSave = LOS_IntLock();
    g_intCount++;
    LOS_IntRestore(intSave);

    hwiIndex = HwiNumGet();
    HwiClear(hwiIndex);

    OsHookCall(LOS_HOOK_TYPE_ISR_ENTER, hwiIndex);

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

    HalAftInterruptHandler(hwiIndex);

    OsHookCall(LOS_HOOK_TYPE_ISR_EXIT, hwiIndex);

    intSave = LOS_IntLock();
    g_intCount--;
    LOS_IntRestore(intSave);
    HalIrqEndCheckNeedSched();
}

/* ****************************************************************************
 Function    : ArchHwiCreate
 Description : create hardware interrupt
 Input       : hwiNum   --- hwi num to create
               hwiPrio  --- priority of the hwi
               hwiMode  --- unused
               hwiHandler --- hwi handler
               irqParam --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 **************************************************************************** */
UINT32 ArchHwiCreate(HWI_HANDLE_T hwiNum,
                     HWI_PRIOR_T hwiPrio,
                     HWI_MODE_T hwiMode,
                     HWI_PROC_FUNC hwiHandler,
                     HwiIrqParam *irqParam)
{
    (VOID)hwiMode;
    UINT32 intSave;

    if (hwiHandler == NULL) {
        return OS_ERRNO_HWI_PROC_FUNC_NULL;
    }

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (g_hwiForm[hwiNum + OS_SYS_VECTOR_CNT] != (HWI_PROC_FUNC)HalHwiDefaultHandler) {
        return OS_ERRNO_HWI_ALREADY_CREATED;
    }

    if (hwiPrio > OS_HWI_PRIO_LOWEST) {
        return OS_ERRNO_HWI_PRIO_INVALID;
    }

    intSave = LOS_IntLock();
#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)
    if (irqParam != NULL) {
        OsSetVector(hwiNum, hwiHandler, irqParam->pDevId);
    } else {
        OsSetVector(hwiNum, hwiHandler, NULL);
    }
#else
    (VOID)irqParam;
    OsSetVector(hwiNum, hwiHandler);
#endif
    HwiUnmask(hwiNum);

    LOS_IntRestore(intSave);

    return LOS_OK;
}

/* ****************************************************************************
 Function    : ArchHwiDelete
 Description : Delete hardware interrupt
 Input       : hwiNum   --- hwi num to delete
               irqParam --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT UINT32 ArchHwiDelete(HWI_HANDLE_T hwiNum, HwiIrqParam *irqParam)
{
    (VOID)irqParam;
    UINT32 intSave;

    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    HwiMask(hwiNum);

    intSave = LOS_IntLock();

    g_hwiForm[hwiNum + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalHwiDefaultHandler;

    LOS_IntRestore(intSave);

    return LOS_OK;
}

ExcInfo g_excInfo = {0};

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
        PRINTK("Task SP   = 0x%x\n", (UINTPTR)taskCB->stackPointer);
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
    INT32 index;
    PRINTK("Exception reg dump:\n");
    PRINTK("sar          = 0x%x\n", excInfo->context->sar);
    PRINTK("excCause     = 0x%x\n", excInfo->context->excCause);
    PRINTK("excVaddr     = 0x%x\n", excInfo->context->excVaddr);
    PRINTK("lbeg         = 0x%x\n", excInfo->context->lbeg);
    PRINTK("lend         = 0x%x\n", excInfo->context->lend);
    PRINTK("lcount       = 0x%x\n", excInfo->context->lcount);
    PRINTK("pc           = 0x%x\n", excInfo->context->pc);
    PRINTK("ps           = 0x%x\n", excInfo->context->ps);
    for (index = 0; index < XTENSA_LOGREG_NUM; index++) {
        PRINTK("regA%d        = 0x%x\n", index, excInfo->context->regA[index]);
    }
}

#if (LOSCFG_KERNEL_BACKTRACE == 1)
STATIC VOID OsExcBackTraceInfo(const ExcInfo *excInfo)
{
    UINTPTR LR[LOSCFG_BACKTRACE_DEPTH] = {0};
    UINT32 index;

    OsBackTraceHookCall(LR, LOSCFG_BACKTRACE_DEPTH, 0, excInfo->context->regA[1]);

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

VOID HalExcHandleEntry(UINTPTR faultAddr, EXC_CONTEXT_S *excBufAddr, UINT32 type)
{
    g_excInfo.nestCnt++;
    g_excInfo.faultAddr = faultAddr;
    g_excInfo.type = type;

    LosTaskCB *taskCB = g_losTask.runTask;

    if ((taskCB == NULL) || (taskCB == OS_TCB_FROM_TID(g_taskMaxNum))) {
        g_excInfo.phase = OS_EXC_IN_INIT;
        g_excInfo.thrdPid = OS_NULL_INT;
    } else if (HwiNumGet() != OS_NULL_INT) {
        g_excInfo.phase = OS_EXC_IN_HWI;
        g_excInfo.thrdPid = HwiNumGet();
    } else {
        g_excInfo.phase = OS_EXC_IN_TASK;
        g_excInfo.thrdPid = g_losTask.runTask->taskID;
    }

    g_excInfo.context = excBufAddr;

    OsDoExcHook(EXC_INTERRUPT);
    OsExcInfoDisplay(&g_excInfo);
    ArchSysExit();
}

/* Stack protector */
WEAK UINT32 __stack_chk_guard = 0xd00a0dff;

WEAK VOID __stack_chk_fail(VOID)
{
    /* __builtin_return_address is a builtin function, building in gcc */
    LOS_Panic("stack-protector: Kernel stack is corrupted in: 0x%x\n",
              __builtin_return_address(0));
}

/* ****************************************************************************
 Function    : HalHwiInit
 Description : initialization of the hardware interrupt
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
VOID HalHwiInit(VOID)
{
    EnableExceptionInterface();
    for (UINT32 i = 0; i < OS_HWI_MAX_NUM; i++) {
        g_hwiForm[i + OS_SYS_VECTOR_CNT] = HalHwiDefaultHandler;
        HwiMask(i);
    }
    asm volatile ("wsr %0, vecbase" : : "r"(INIT_VECTOR_START));
    return;
}
