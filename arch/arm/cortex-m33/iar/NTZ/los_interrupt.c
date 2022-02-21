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
#include "securec.h"
#include <stdarg.h>
#include "los_arch_interrupt.h"
#include "los_context.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_memory.h"
#include "los_membox.h"

/*lint -save -e40 -e522 -e533*/
UINT32 g_intCount = 0;

/*lint -restore*/
#pragma location = ".data.vector"
#pragma data_alignment=LOSCFG_ARCH_HWI_VECTOR_ALIGN
/* *
 * @ingroup los_hwi
 * hardware interrupt form mapping handling function array.
 */
STATIC HWI_PROC_FUNC g_hwiForm[OS_VECTOR_CNT] = {0};

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)

typedef struct {
    HWI_PROC_FUNC pfnHandler;
    VOID *pParm;
} HWI_HANDLER_FUNC;

/* *
 * @ingroup los_hwi
 * hardware interrupt handler form mapping handling function array.
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
 * hardware interrupt handler form mapping handling function array.
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

WEAK VOID SysTick_Handler(VOID)
{
    return;
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
    return __get_IPSR();
}

STATIC UINT32 HwiUnmask(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    NVIC_EnableIRQ((IRQn_Type)hwiNum);

    return LOS_OK;
}

STATIC UINT32 HwiMask(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    NVIC_DisableIRQ((IRQn_Type)hwiNum);

    return LOS_OK;
}

STATIC UINT32 HwiSetPriority(HWI_HANDLE_T hwiNum, UINT8 priority)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    if (priority > OS_HWI_PRIO_LOWEST) {
        return OS_ERRNO_HWI_PRIO_INVALID;
    }

    NVIC_SetPriority((IRQn_Type)hwiNum, priority);

    return LOS_OK;
}

STATIC UINT32 HwiPending(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    NVIC_SetPendingIRQ((IRQn_Type)hwiNum);

    return LOS_OK;
}

STATIC UINT32 HwiClear(HWI_HANDLE_T hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return OS_ERRNO_HWI_NUM_INVALID;
    }

    NVIC_ClearPendingIRQ((IRQn_Type)hwiNum);

    return LOS_OK;
}

HwiControllerOps g_archHwiOps = {
    .enableIrq      = HwiUnmask,
    .disableIrq     = HwiMask,
    .setIrqPriority = HwiSetPriority,
    .getCurIrqNum   = HwiNumGet,
    .triggerIrq     = HwiPending,
    .clearIrq       = HwiClear,
};

inline UINT32 ArchIsIntActive(VOID)
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
/*lint -e529*/
LITE_OS_SEC_TEXT_MINOR VOID HalHwiDefaultHandler(VOID)
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
LITE_OS_SEC_TEXT VOID HalInterrupt(VOID)
{
    UINT32 hwiIndex;
    UINT32 intSave;

#if (LOSCFG_KERNEL_RUNSTOP == 1)
    SCB->SCR &= (UINT32) ~((UINT32)SCB_SCR_SLEEPDEEP_Msk);
#endif

    intSave = LOS_IntLock();

    g_intCount++;

    LOS_IntRestore(intSave);

    hwiIndex = HwiNumGet();

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
LITE_OS_SEC_TEXT_INIT UINT32 ArchHwiCreate(HWI_HANDLE_T hwiNum,
                                           HWI_PRIOR_T hwiPrio,
                                           HWI_MODE_T hwiMode,
                                           HWI_PROC_FUNC hwiHandler,
                                           HwiIrqParam *irqParam)
{
    (VOID)hwiMode;
    UINTPTR intSave;

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
    HwiUnmask((IRQn_Type)hwiNum);
    HwiSetPriority((IRQn_Type)hwiNum, hwiPrio);

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

    NVIC_DisableIRQ((IRQn_Type)hwiNum);

    intSave = LOS_IntLock();

    g_hwiForm[hwiNum + OS_SYS_VECTOR_CNT] = (HWI_PROC_FUNC)HalHwiDefaultHandler;

    LOS_IntRestore(intSave);

    return LOS_OK;
}

#define FAULT_STATUS_REG_BIT            32
#define USGFAULT                        (1 << 18)
#define BUSFAULT                        (1 << 17)
#define MEMFAULT                        (1 << 16)
#define DIV0FAULT                       (1 << 4)
#define HARDFAULT_IRQN                  (-13)

ExcInfo g_excInfo = {0};

UINT8 g_uwExcTbl[FAULT_STATUS_REG_BIT] = {
    0, 0, 0, 0, 0, 0, OS_EXC_UF_DIVBYZERO, OS_EXC_UF_UNALIGNED,
    0, 0, 0, 0, OS_EXC_UF_NOCP, OS_EXC_UF_INVPC, OS_EXC_UF_INVSTATE, OS_EXC_UF_UNDEFINSTR,
    0, 0, 0, OS_EXC_BF_STKERR, OS_EXC_BF_UNSTKERR, OS_EXC_BF_IMPRECISERR, OS_EXC_BF_PRECISERR, OS_EXC_BF_IBUSERR,
    0, 0, 0, OS_EXC_MF_MSTKERR, OS_EXC_MF_MUNSTKERR, 0, OS_EXC_MF_DACCVIOL, OS_EXC_MF_IACCVIOL
};

#if (LOSCFG_KERNEL_PRINTF != 0)
STATIC VOID OsExcNvicDump(VOID)
{
#define OS_NR_NVIC_EXC_DUMP_TYPES   7
    UINT32 *base = NULL;
    UINT32 len, i, j;
    UINT32 rgNvicBases[OS_NR_NVIC_EXC_DUMP_TYPES] = {
        OS_NVIC_SETENA_BASE, OS_NVIC_SETPEND_BASE, OS_NVIC_INT_ACT_BASE,
        OS_NVIC_PRI_BASE, OS_NVIC_EXCPRI_BASE, OS_NVIC_SHCSR, OS_NVIC_INT_CTRL
    };
    UINT32 rgNvicLens[OS_NR_NVIC_EXC_DUMP_TYPES] = {
        OS_NVIC_INT_ENABLE_SIZE, OS_NVIC_INT_PEND_SIZE, OS_NVIC_INT_ACT_SIZE,
        OS_NVIC_INT_PRI_SIZE, OS_NVIC_EXCPRI_SIZE, OS_NVIC_SHCSR_SIZE,
        OS_NVIC_INT_CTRL_SIZE
    };
    CHAR strRgEnable[] = "enable";
    CHAR strRgPending[] = "pending";
    CHAR strRgActive[] = "active";
    CHAR strRgPriority[] = "priority";
    CHAR strRgException[] = "exception";
    CHAR strRgShcsr[] = "shcsr";
    CHAR strRgIntCtrl[] = "control";
    CHAR *strRgs[] = {
        strRgEnable, strRgPending, strRgActive, strRgPriority,
        strRgException, strRgShcsr, strRgIntCtrl
    };

    PRINTK("\r\nOS exception NVIC dump:\n");
    for (i = 0; i < OS_NR_NVIC_EXC_DUMP_TYPES; i++) {
        base = (UINT32 *)rgNvicBases[i];
        len = rgNvicLens[i];
        PRINTK("interrupt %s register, base address: %p, size: 0x%x\n", strRgs[i], base, len);
        len = (len >> 2); /* 2: Gets the next register offset */
        for (j = 0; j < len; j++) {
            PRINTK("0x%x ", *(base + j));
            if ((j != 0) && ((j % 16) == 0)) { /* 16: print wrap line */
                PRINTK("\n");
            }
        }
        PRINTK("\n");
    }
}

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
        PRINTK("Task SP   = %p\n", taskCB->stackPointer);
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
    PRINTK("PC        = 0x%x\n", excInfo->context->uwPC);
    PRINTK("LR        = 0x%x\n", excInfo->context->uwLR);
    PRINTK("SP        = 0x%x\n", excInfo->context->uwSP);
    PRINTK("R0        = 0x%x\n", excInfo->context->uwR0);
    PRINTK("R1        = 0x%x\n", excInfo->context->uwR1);
    PRINTK("R2        = 0x%x\n", excInfo->context->uwR2);
    PRINTK("R3        = 0x%x\n", excInfo->context->uwR3);
    PRINTK("R4        = 0x%x\n", excInfo->context->uwR4);
    PRINTK("R5        = 0x%x\n", excInfo->context->uwR5);
    PRINTK("R6        = 0x%x\n", excInfo->context->uwR6);
    PRINTK("R7        = 0x%x\n", excInfo->context->uwR7);
    PRINTK("R8        = 0x%x\n", excInfo->context->uwR8);
    PRINTK("R9        = 0x%x\n", excInfo->context->uwR9);
    PRINTK("R10       = 0x%x\n", excInfo->context->uwR10);
    PRINTK("R11       = 0x%x\n", excInfo->context->uwR11);
    PRINTK("R12       = 0x%x\n", excInfo->context->uwR12);
    PRINTK("PriMask   = 0x%x\n", excInfo->context->uwPriMask);
    PRINTK("xPSR      = 0x%x\n", excInfo->context->uwxPSR);
}

#if (LOSCFG_KERNEL_BACKTRACE == 1)
STATIC VOID OsExcBackTraceInfo(const ExcInfo *excInfo)
{
    UINTPTR LR[LOSCFG_BACKTRACE_DEPTH] = {0};
    UINT32 index;

    OsBackTraceHookCall(LR, LOSCFG_BACKTRACE_DEPTH, 0, excInfo->context->uwSP);

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
    OsExcNvicDump();
    OsExcMemPoolCheckInfo();
#endif
}

LITE_OS_SEC_TEXT_INIT VOID HalExcHandleEntry(UINT32 excType, UINT32 faultAddr, UINT32 pid, EXC_CONTEXT_S *excBufAddr)
{
    UINT16 tmpFlag = (excType >> 16) & OS_NULL_SHORT; /* 16: Get Exception Type */
    g_intCount++;
    g_excInfo.nestCnt++;

    g_excInfo.type = excType & OS_NULL_SHORT;

    if (tmpFlag & OS_EXC_FLAG_FAULTADDR_VALID) {
        g_excInfo.faultAddr = faultAddr;
    } else {
        g_excInfo.faultAddr = OS_EXC_IMPRECISE_ACCESS_ADDR;
    }
    if (g_losTask.runTask != NULL) {
        if (tmpFlag & OS_EXC_FLAG_IN_HWI) {
            g_excInfo.phase = OS_EXC_IN_HWI;
            g_excInfo.thrdPid = pid;
        } else {
            g_excInfo.phase = OS_EXC_IN_TASK;
            g_excInfo.thrdPid = g_losTask.runTask->taskID;
        }
    } else {
        g_excInfo.phase = OS_EXC_IN_INIT;
        g_excInfo.thrdPid = OS_NULL_INT;
    }
    if (excType & OS_EXC_FLAG_NO_FLOAT) {
        g_excInfo.context = (EXC_CONTEXT_S *)((CHAR *)excBufAddr - LOS_OFF_SET_OF(EXC_CONTEXT_S, uwR4));
    } else {
        g_excInfo.context = excBufAddr;
    }

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
    UINT32 index;
    g_hwiForm[0] = 0;             /* [0] Top of Stack */
    g_hwiForm[1] = Reset_Handler; /* [1] reset */
    for (index = 2; index < OS_VECTOR_CNT; index++) { /* 2: The starting position of the interrupt */
        g_hwiForm[index] = (HWI_PROC_FUNC)HalHwiDefaultHandler;
    }
    /* Exception handler register */
    g_hwiForm[NonMaskableInt_IRQn + OS_SYS_VECTOR_CNT]   = HalExcNMI;
    g_hwiForm[HARDFAULT_IRQN + OS_SYS_VECTOR_CNT]        = HalExcHardFault;
    g_hwiForm[MemoryManagement_IRQn + OS_SYS_VECTOR_CNT] = HalExcMemFault;
    g_hwiForm[BusFault_IRQn + OS_SYS_VECTOR_CNT]         = HalExcBusFault;
    g_hwiForm[UsageFault_IRQn + OS_SYS_VECTOR_CNT]       = HalExcUsageFault;
    g_hwiForm[SVCall_IRQn + OS_SYS_VECTOR_CNT]           = HalSVCHandler;
    g_hwiForm[PendSV_IRQn + OS_SYS_VECTOR_CNT]           = HalPendSV;
    g_hwiForm[SysTick_IRQn + OS_SYS_VECTOR_CNT]          = SysTick_Handler;

    /* Interrupt vector table location */
    SCB->VTOR = (UINT32)(UINTPTR)g_hwiForm;
#endif
#if (__CORTEX_M >= 0x03U) /* only for Cortex-M3 and above */
    NVIC_SetPriorityGrouping(OS_NVIC_AIRCR_PRIGROUP);
#endif

    /* Enable USGFAULT, BUSFAULT, MEMFAULT */
    *(volatile UINT32 *)OS_NVIC_SHCSR |= (USGFAULT | BUSFAULT | MEMFAULT);
    /* Enable DIV 0 and unaligned exception */
    *(volatile UINT32 *)OS_NVIC_CCR |= DIV0FAULT;

    return;
}

