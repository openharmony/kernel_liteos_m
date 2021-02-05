/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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
#include "los_context.h"
#include "los_arch_interrupt.h"
#include <stdarg.h>
#include "los_debug.h"
#include "los_task.h"
#include "los_tick.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*lint -save -e40 -e522 -e533*/

UINT32 g_intCount = 0;
/*lint -restore*/
#ifdef __ICCARM__
#pragma location = ".data.vector"
#pragma data_alignment=0x100
#elif defined(__CC_ARM) || defined(__GNUC__)
LITE_OS_SEC_VEC
#endif
HWI_PROC_FUNC g_hwiForm[OS_VECTOR_CNT] = {0};

#if (OS_HWI_WITH_ARG == 1)
HWI_SLAVE_FUNC g_hwiSlaveForm[OS_VECTOR_CNT] = {{ (HWI_PROC_FUNC)0, (HWI_ARG_T)0 }};
#else
HWI_PROC_FUNC g_hwiSlaveForm[OS_VECTOR_CNT] = {0};
#endif

/* ****************************************************************************
 Function    : HalIntNumGet
 Description : Get a interrupt number
 Input       : None
 Output      : None
 Return      : Interrupt Indexes number
 **************************************************************************** */
LITE_OS_SEC_TEXT_MINOR UINT32 HalIntNumGet(VOID)
{
    return __get_IPSR();
}

inline UINT32 HalIsIntAcvive(VOID)
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
    UINT32 irqNum = HalIntNumGet();
    PRINT_ERR("%s irqnum:%d\n", __FUNCTION__, irqNum);
    while (1) {}
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

    hwiIndex = HalIntNumGet();

#if (OS_HWI_WITH_ARG == 1)
    if (g_hwiSlaveForm[hwiIndex].pfnHandler != 0) {
        g_hwiSlaveForm[hwiIndex].pfnHandler((VOID *)g_hwiSlaveForm[hwiIndex].pParm);
    }
#else
    if (g_hwiSlaveForm[hwiIndex] != 0) {
        g_hwiSlaveForm[hwiIndex]();
    }
#endif
    intSave = LOS_IntLock();
    g_intCount--;
    LOS_IntRestore(intSave);
}

/* ****************************************************************************
 Function    : HalHwiCreate
 Description : create hardware interrupt
 Input       : hwiNum   --- hwi num to create
               hwiPrio  --- priority of the hwi
               mode     --- unused
               handler --- hwi handler
               arg      --- param of the hwi handler
 Output      : None
 Return      : LOS_OK on success or error code on failure
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT UINT32 HalHwiCreate(HWI_HANDLE_T hwiNum,
                                           HWI_PRIOR_T hwiPrio,
                                           HWI_MODE_T mode,
                                           HWI_PROC_FUNC handler,
                                           HWI_ARG_T arg)
{
    UINTPTR intSave;

    if (handler == NULL) {
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
#if (OS_HWI_WITH_ARG == 1)
    OsSetVector(hwiNum, handler, arg);
#else
    OsSetVector(hwiNum, handler);
#endif
    NVIC_EnableIRQ((IRQn_Type)hwiNum);
    NVIC_SetPriority((IRQn_Type)hwiNum, hwiPrio);

    LOS_IntRestore(intSave);

    return LOS_OK;
}

/* ****************************************************************************
 Function    : HalHwiDelete
 Description : Delete hardware interrupt
 Input       : hwiNum   --- hwi num to delete
 Output      : None
 Return      : LOS_OK on success or error code on failure
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT UINT32 HalHwiDelete(HWI_HANDLE_T hwiNum)
{
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

#define OS_NVIC_INT_CTRL_SIZE           4
#define OS_NVIC_SHCSR_SIZE              4
#define FAULT_STATUS_REG_BIT            32
#define USGFAULT                        (1 << 18)
#define BUSFAULT                        (1 << 17)
#define MEMFAULT                        (1 << 16)
#define DIV0FAULT                       (1 << 4)
#define HARDFAULT_IRQN                  (-13)

static ExcInfoArray g_excArray[OS_EXC_TYPE_MAX];

static ExcInfo g_excInfo = {0};

UINT8 g_uwExcTbl[FAULT_STATUS_REG_BIT] = {
    0, 0, 0, 0, 0, 0, OS_EXC_UF_DIVBYZERO, OS_EXC_UF_UNALIGNED,
    0, 0, 0, 0, OS_EXC_UF_NOCP, OS_EXC_UF_INVPC, OS_EXC_UF_INVSTATE, OS_EXC_UF_UNDEFINSTR,
    0, 0, 0, OS_EXC_BF_STKERR, OS_EXC_BF_UNSTKERR, OS_EXC_BF_IMPRECISERR, OS_EXC_BF_PRECISERR, OS_EXC_BF_IBUSERR,
    0, 0, 0, OS_EXC_MF_MSTKERR, OS_EXC_MF_MUNSTKERR, 0, OS_EXC_MF_DACCVIOL, OS_EXC_MF_IACCVIOL
};

UINT32 HalExcNvicDump(UINT32 index, UINT32 *excContent)
{
    UINT32 *base = NULL;
    UINT32 len = 0, i, j;
#define OS_NR_NVIC_EXC_DUMP_Types      7
    UINT32 rgNvicBases[OS_NR_NVIC_EXC_DUMP_Types] = {OS_NVIC_SETENA_BASE, OS_NVIC_SETPEND_BASE,
        OS_NVIC_INT_ACT_BASE, OS_NVIC_PRI_BASE, OS_NVIC_EXCPRI_BASE, OS_NVIC_SHCSR, OS_NVIC_INT_CTRL};
    UINT32 rgNvicLens[OS_NR_NVIC_EXC_DUMP_Types] = {OS_NVIC_INT_ENABLE_SIZE, OS_NVIC_INT_PEND_SIZE,
        OS_NVIC_INT_ACT_SIZE, OS_NVIC_INT_PRI_SIZE, OS_NVIC_EXCPRI_SIZE, OS_NVIC_SHCSR_SIZE, OS_NVIC_INT_CTRL_SIZE};
    char strRgEnable[] = "enable";
    char strRgPending[] = "pending";
    char strRgActive[] = "active";
    char strRgPriority[] = "priority";
    char strRgException[] = "exception";
    char strRgShcsr[] = "shcsr";
    char strRgIntCtrl[] = "control";
    char *strRgs[] = {strRgEnable, strRgPending, strRgActive, strRgPriority, strRgException, strRgShcsr, strRgIntCtrl};
    (VOID)index;
    (VOID)excContent;

    PRINTK("OS exception NVIC dump: \n");
    for (i = 0; i < OS_NR_NVIC_EXC_DUMP_Types; i++) {
        base = (UINT32 *)rgNvicBases[i];
        len = rgNvicLens[i];
        PRINTK("interrupt %s register, base address: 0x%x, size: 0x%x\n", strRgs[i], base, len);
        len = (len >> 2);
        for (j = 0; j < len; j++) {
            PRINTK("0x%x ", *(base + j));
        }
        PRINTK("\n");
    }
    return 0;
}

UINT32 HalExcContextDump(UINT32 index, UINT32 *excContent)
{
    (VOID)index;
    (VOID)excContent;
    PRINTK("OS exception context dump:\n");
    PRINTK("Phase      = 0x%x\n", g_excInfo.phase);
    PRINTK("Type       = 0x%x\n", g_excInfo.type);
    PRINTK("FaultAddr  = 0x%x\n", g_excInfo.faultAddr);
    PRINTK("ThrdPid    = 0x%x\n", g_excInfo.thrdPid);
    PRINTK("R0         = 0x%x\n", g_excInfo.context->uwR0);
    PRINTK("R1         = 0x%x\n", g_excInfo.context->uwR1);
    PRINTK("R2         = 0x%x\n", g_excInfo.context->uwR2);
    PRINTK("R3         = 0x%x\n", g_excInfo.context->uwR3);
    PRINTK("R4         = 0x%x\n", g_excInfo.context->uwR4);
    PRINTK("R5         = 0x%x\n", g_excInfo.context->uwR5);
    PRINTK("R6         = 0x%x\n", g_excInfo.context->uwR6);
    PRINTK("R7         = 0x%x\n", g_excInfo.context->uwR7);
    PRINTK("R8         = 0x%x\n", g_excInfo.context->uwR8);
    PRINTK("R9         = 0x%x\n", g_excInfo.context->uwR9);
    PRINTK("R10        = 0x%x\n", g_excInfo.context->uwR10);
    PRINTK("R11        = 0x%x\n", g_excInfo.context->uwR11);
    PRINTK("R12        = 0x%x\n", g_excInfo.context->uwR12);
    PRINTK("PriMask    = 0x%x\n", g_excInfo.context->uwPriMask);
    PRINTK("SP         = 0x%x\n", g_excInfo.context->uwSP);
    PRINTK("LR         = 0x%x\n", g_excInfo.context->uwLR);
    PRINTK("PC         = 0x%x\n", g_excInfo.context->uwPC);
    PRINTK("xPSR       = 0x%x\n", g_excInfo.context->uwxPSR);
    return 0;
}

VOID HalDumpMsg(VOID)
{
    UINT32 index = 0;
    for (index = 0; index < (OS_EXC_TYPE_MAX - 1); index++) {
        if (g_excArray[index].uwValid == FALSE) {
            continue;
        }
        g_excArray[index].pFnExcInfoCb(index, g_excArray[index].pArg);
    }
}

LITE_OS_SEC_TEXT_INIT VOID HalExcHandleEntry(UINT32 excType, UINT32 faultAddr, UINT32 pid, EXC_CONTEXT_S *excBufAddr)
{
    UINT16 tmpFlag = (excType >> 16) & OS_NULL_SHORT;
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
    HalDumpMsg();
    HalSysExit();
}

VOID HalExcRegister(ExcInfoType type, EXC_INFO_SAVE_CALLBACK func, VOID *arg)
{
    ExcInfoArray *excInfo = NULL;
    if ((type >= OS_EXC_TYPE_MAX) || (func == NULL)) {
        PRINT_ERR("HalExcRegister ERROR!\n");
        return;
    }
    excInfo = &(g_excArray[type]);
    excInfo->uwType = type;
    excInfo->pFnExcInfoCb = func;
    excInfo->pArg = arg;
    excInfo->uwValid = TRUE;
}

/* ****************************************************************************
 Function    : HalHwiInit
 Description : initialization of the hardware interrupt
 Input       : None
 Output      : None
 Return      : None
 **************************************************************************** */
LITE_OS_SEC_TEXT_INIT VOID HalHwiInit()
{
#if (LOSCFG_USE_SYSTEM_DEFINED_INTERRUPT == 1)
    UINT32 index;
    g_hwiForm[0] = 0;             /* [0] Top of Stack */
    g_hwiForm[1] = Reset_Handler; /* [1] reset */
    for (index = 2; index < OS_VECTOR_CNT; index++) {
        g_hwiForm[index] = (HWI_PROC_FUNC)HalHwiDefaultHandler;
    }
    /* Exception handler register */
    g_hwiForm[NonMaskableInt_IRQn + OS_SYS_VECTOR_CNT]   = HalExcNMI;
    g_hwiForm[HARDFAULT_IRQN + OS_SYS_VECTOR_CNT]        = HalExcHardFault;
    g_hwiForm[MemoryManagement_IRQn + OS_SYS_VECTOR_CNT] = HalExcMemFault;
    g_hwiForm[BusFault_IRQn + OS_SYS_VECTOR_CNT]         = HalExcBusFault;
    g_hwiForm[UsageFault_IRQn + OS_SYS_VECTOR_CNT]       = HalExcUsageFault;
    g_hwiForm[SVCall_IRQn + OS_SYS_VECTOR_CNT]           = HalExcSvcCall;
    g_hwiForm[PendSV_IRQn + OS_SYS_VECTOR_CNT]           = HalPendSV;

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

    HalExcRegister(OS_EXC_TYPE_CONTEXT, (EXC_INFO_SAVE_CALLBACK)HalExcContextDump, NULL);
    HalExcRegister(OS_EXC_TYPE_NVIC, (EXC_INFO_SAVE_CALLBACK)HalExcNvicDump, NULL);

    return;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
