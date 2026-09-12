/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

#include "osTest.h"
#include "los_config.h"
#include "los_swtmr_pri.h"
#include "los_resleak_test.h"
#include "icunit_filter.h"
#if (LITEOS_CMSIS_TEST == 1)
#include "cmsis_os.h"
#endif
#if (LOS_POSIX_TEST == 1)
#include "posix_test.h"
#endif
#if (LOS_XTS_TEST == 1)
#include "xts_test.h"
#endif

UINT32 volatile g_testCount;
UINT32 g_testTskHandle;
UINT32 g_testTaskID01;
UINT32 g_testTaskID02;
UINT32 g_testTaskID03;
UINT32 g_testTaskID04;

EVENT_CB_S g_eventCB01;
EVENT_CB_S g_eventCB02;
EVENT_CB_S g_eventCB03;
UINT32 g_mutexTest;
EVENT_CB_S g_exampleEvent;

UINT32 g_hwiNum1;
UINT32 g_hwiNum2;
UINT32 g_usSemID;
UINT32 g_usSemID2;
UINT32 g_cpupTestCount;
UINT32 g_cmsisRobinCount1;
UINT32 g_cmsisCount;

UINT16 g_usSwTmrID;

UINT32 g_testQueueID01;
UINT32 g_testQueueID02;
UINT32 g_testQueueID03;

UINT16 g_index;
UINT32 g_loopCycle = 0xFFFFF;

UINT32 g_passResult = 0;
UINT32 g_failResult = 0;
UINT32 g_testTskHandle;

UINT32 g_leavingTaskNum;
UINT32 g_testTaskIdArray[LOSCFG_BASE_CORE_TSK_LIMIT] = {0};
UINT32 g_uwGetTickConsume = 0;

UINT32 g_usSemID3[LOSCFG_BASE_IPC_SEM_CONFIG + 1];

#define TST_RAMADDRSTART 0x20000000
#define TST_RAMADDREND 0x20010000

extern SWTMR_CTRL_S *g_swtmrCBArray;
UINT32 SwtmrCountGetTest(VOID)
{
    UINT32 loop;
    UINT32 swTmrCnt = 0;
    UINT32 intSave;
    SWTMR_CTRL_S *swTmrCB = (SWTMR_CTRL_S *)NULL;

    intSave = LOS_IntLock();
    swTmrCB = g_swtmrCBArray;
    for (loop = 0; loop < LOSCFG_BASE_CORE_SWTMR_LIMIT; loop++, swTmrCB++) {
        if (swTmrCB->ucState != OS_SWTMR_STATUS_UNUSED) {
            swTmrCnt++;
        }
    }
    (VOID)LOS_IntRestore(intSave);
    return swTmrCnt;
}

extern LosQueueCB *g_allQueue;

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
extern LosQueueCB *g_staticQueue;
#endif

UINT32 QueueUsedCountGet(VOID)
{
    UINT32 intSave;
    UINT32 count = 0;
    UINT32 index;

    intSave = LOS_IntLock();
    for (index = 0; index < LOSCFG_BASE_IPC_QUEUE_LIMIT; index++) {
        LosQueueCB *queueNode = ((LosQueueCB *)g_allQueue) + index;
        if (queueNode->queueState == OS_QUEUE_INUSED) {
            count++;
        }
    }

#if (LOSCFG_QUEUE_STATIC_ALLOCATION == 1)
    for (index = 0; index < LOSCFG_BASE_IPC_STATIC_QUEUE_LIMIT; index++) {
        LosQueueCB *queueNode = ((LosQueueCB *)g_staticQueue) + index;
        if (queueNode->queueState == OS_QUEUE_INUSED) {
            count++;
        }
    }
#endif

    LOS_IntRestore(intSave);

    return count;
}

extern LosTaskCB *g_taskCBArray;
UINT32 TaskUsedCountGet(VOID)
{
    UINT32 intSave;
    UINT32 count = 0;

    intSave = LOS_IntLock();
    for (UINT32 index = 0; index < LOSCFG_BASE_CORE_TSK_LIMIT; index++) {
        LosTaskCB *taskCB = ((LosTaskCB *)g_taskCBArray) + index;
        if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
            count++;
        }
    }
    LOS_IntRestore(intSave);

    return (LOSCFG_BASE_CORE_TSK_LIMIT - count);
}

void TestKernel(void)
{
#if (LOS_KERNEL_ATOMIC_TEST == 1)
    ItSuiteLosAtomic();
#endif
#if (LOS_KERNEL_CORE_TASK_TEST == 1)
    ItSuiteLosTask();
#endif
#if (LOS_KERNEL_IPC_QUEUE_TEST == 1)
    ItSuiteLosQueue();
#endif
#if (LOS_KERNEL_IPC_MUX_TEST == 1)
    ItSuiteLosMux();
#endif
#if (LOS_KERNEL_IPC_EVENT_TEST == 1)
    ItSuiteLosEvent();
#endif
#if (LOS_KERNEL_IPC_SEM_TEST == 1)
    ItSuiteLosSem();
#endif
#if (LOSCFG_BASE_IPC_RWSEM == 1)
    ItSuiteLosRwsem();
#endif
#if (LOS_KERNEL_CORE_SWTMR_TEST == 1)
    ItSuiteLosSwtmr();
#endif
#if (LOS_KERNEL_HWI_TEST == 1)
    ItSuiteLosHwi();
#endif
#if (LOS_KERNEL_MEM_TEST == 1)
    ItSuiteLosMem();
#endif
#if (LOS_KERNEL_CORE_CPUP_TEST == 1)
    ItSuiteLosCpup();
#endif
#if (LOS_KERNEL_DYNLINK_TEST == 1)
    ItSuiteLosDynlink();
#endif

#if (LOS_KERNEL_LMS_TEST == 1)
    ItSuiteLosLms();
#endif

#if (LOS_KERNEL_PM_TEST == 1)
    ItSuiteLosPm();
#endif

#if (LOS_KERNEL_LMK_TEST == 1)
    ItSuiteLosLmk();
#endif

#if (LOS_KERNEL_SIGNAL_TEST == 1)
    ItSuiteLosSignal();
#endif
#if (LOS_KERNEL_MISC_TEST == 1)
    ItSuiteLosMisc();
#endif

#if (LOS_KERNEL_TRACE_TEST == 1)
    ItSuiteLosTrace();
#endif

#if (LOS_KERNEL_EXC_TEST == 1)
    ItSuiteLosExc();
#endif

#if (LOS_KERNEL_SHELL_TEST == 1)
    ItSuiteLosShell();
#endif
}

#if (CMSIS_OS_VER == 2)
void TestCmsis2(void)
{
#if (LOS_CMSIS2_CORE_TASK_TEST == 1)
    ItSuite_Cmsis_Los_Task();
#endif

#if (LOS_CMSIS2_IPC_EVENT_TEST == 1)
    ItSuite_Cmsis_Los_Event();
#endif
#if (LOS_CMSIS2_CORE_SWTMR_TEST == 1)
    ItSuite_Cmsis_Los_Swtmr();
#endif
#if (LOS_CMSIS2_IPC_SEM_TEST == 1)
    ItSuite_Cmsis_Los_Sem();
#endif
#if (LOS_CMSIS2_IPC_MUX_TEST == 1)
    ItSuite_Cmsis_Los_Mux();
#endif
#if (LOS_CMSIS2_HWI_TEST == 1)
    ItSuite_Cmsis_Los_Hwi();
#endif
#if (LOS_CMSIS2_IPC_MSG_TEST == 1)
    ItSuite_Cmsis_Los_Msg();
#endif
}
#endif

VOID TestTaskEntry(VOID)
{
    dprintf("\t\n --- Test Start --- \n\n");
    ICunitInit();

    OsResLeakInit();

    OsTestCalibrateBusyDelay();

    TestKernel();

#if (LOS_POSIX_TEST == 1)
    ItSuitePosix();
#endif

#if (LOS_CMSIS_TEST == 1)
    CmsisFuncTestSuite();
#endif

#if(LOS_XTS_TEST == 1)
    XtsTestSuite();
#endif

    /* The log is used for testing entrance guard, please do not make any changes. */
    dprintf("\nfailed count:%d, success count:%d\n", g_failResult, g_passResult);
    dprintf("[Crash-Skip] count:%d\n", g_crashResult);
    dprintf("[ResLeak] total resource leak count: %u\n", OsResLeakGetTotalLeakCnt());
    IcFilterReportNotFound();
    IcResumeReport();
    ICunitPrintFailLogs();
    dprintf("--- Test End ---\n");
}

UINT32 los_TestInit(VOID)
{
    UINT32 ret;
    TSK_INIT_PARAM_S osTaskInitParam = { 0 };

    osTaskInitParam.pfnTaskEntry = (TSK_ENTRY_FUNC)TestTaskEntry;
    osTaskInitParam.uwStackSize = OS_TSK_TEST_STACK_SIZE;
    osTaskInitParam.pcName = "IT_TST_INI";
    osTaskInitParam.usTaskPrio = TASK_PRIO_TEST;
    osTaskInitParam.uwResved = LOS_TASK_ATTR_JOINABLE;

    ret = LOS_TaskCreate(&g_testTskHandle, &osTaskInitParam);
    if (LOS_OK != ret) {
        dprintf("LosTestInit  error\n");
    }
    return ret;
}

UINT32 LosAppInit(VOID)
{
    UINT32 ret;

    ret = los_TestInit();
    if (ret != LOS_OK) {
        return ret;
    }
    return LOS_OK;
}

#ifdef __RISC_V__
#ifdef LOS_HIMIDEERV100
#define HWI_TRIG_BASE 0x20c20
#define HWI_CLEAN_TRI 0x20c20
#define HIW_SYS_COUNT OS_RISCV_SYS_VECTOR_CNT
#elif defined(LOS_HIFONEV320_RV32)
#define HWI_TRIG_BASE 0xF8B31000
#define HWI_CLEAN_TRI (HWI_TRIG_BASE + 0x04)
#define HWI_MASK_IRQ (HWI_TRIG_BASE + 0x0c)
#define HIW_SYS_COUNT (26 + 6)
#endif

extern VOID HalIrqEnable(UINT32 vector);

#ifdef LOSCFG_PLATFORM_WS63_M

/* WS63 TIMER1: base 0x44002200, load_count0@+0x00, load_count1@+0x04,
 * control@+0x10, eoi@+0x14.
 * control: bit0=enable, bits[2:1]=mode (00=one-shot, 01=periodic). TIMER_CLOCK=80MHz.
 * eoi: write 1 to clear interrupt pending flag. */
#define WS63_TIMER1_BASE         0x44002200UL
#define WS63_TIMER1_LOAD_COUNT0  (WS63_TIMER1_BASE + 0x00)
#define WS63_TIMER1_LOAD_COUNT1  (WS63_TIMER1_BASE + 0x04)
#define WS63_TIMER1_CONTROL      (WS63_TIMER1_BASE + 0x10)
#define WS63_TIMER1_EOI          (WS63_TIMER1_BASE + 0x14)
#define TIMER1_CTRL_ENABLE        0x1   /* enable + mode 00 (one-shot) */
#define TIMER1_CTRL_DISABLE       0x0

VOID TestHwiClear(UINT32 hwiNum)
{
    if (hwiNum == TIMER_1_IRQN) {
        volatile UINT32 *eoi = (volatile UINT32 *)WS63_TIMER1_EOI;
        *eoi = 1;   /* clear timer1 interrupt pending flag */
    }
    dsb();
    LOS_HwiClear(hwiNum);
}

static VOID HalTimerStart(UINT32 hwiNum)
{
    if (hwiNum == TIMER_1_IRQN) {
        volatile UINT32 *load0 = (volatile UINT32 *)WS63_TIMER1_LOAD_COUNT0;
        volatile UINT32 *load1 = (volatile UINT32 *)WS63_TIMER1_LOAD_COUNT1;
        volatile UINT32 *ctrl  = (volatile UINT32 *)WS63_TIMER1_CONTROL;

        HalIrqEnable(hwiNum);             /* enable TIMER_1_IRQN at PLIC */
        *load0 = 80;                 /* ~1µs @ 80MHz (ws63 TIMER_CLOCK) */
        *load1 = 0;                      /* clear high 32-bit (match tick setup) */
        *ctrl  = TIMER1_CTRL_DISABLE;   /* disable timer */
        *ctrl  = TIMER1_CTRL_ENABLE;    /* enable timer, one-shot */
    }
}

VOID TestHwiTrigger(UINT32 hwiNum)
{
    HalTimerStart(hwiNum);
    for (volatile UINT32 i = 0; i < 1000; i++) {} /* let timer1 fire + ISR dispatch */
}

UINT32 TestHwiDelete(UINT32 hwiNum)
{
    if (hwiNum == TIMER_1_IRQN) {
        volatile UINT32 *ctrl = (volatile UINT32 *)WS63_TIMER1_CONTROL;
        volatile UINT32 *eoi  = (volatile UINT32 *)WS63_TIMER1_EOI;
        *ctrl = TIMER1_CTRL_DISABLE;   /* stop timer1 */
        *eoi  = 1;                      /* clear pending interrupt */
    }
    UINT32 ret = LOS_HwiDelete(hwiNum, NULL);
    return (ret == LOS_OK) ? LOS_OK : LOS_NOK;
}

#elif LOSCFG_RISCV_LCMP_CLIC
/*
 * CLIC-based platforms (hi3322) have working software-trigger (INTIP
 * pending bit) and proper create/delete/unmask via the HwiControllerOps
 * framework.  Use the real LOS_Hwi* APIs so HWI tests function correctly.
 */
VOID TestHwiTrigger(UINT32 hwiNum)
{
    LOS_HwiEnable(hwiNum);
    LOS_HwiTrigger(hwiNum);
}

UINT32 TestHwiDelete(UINT32 hwiNum)
{
    UINT32 ret = LOS_HwiDelete(hwiNum, NULL);
    if (ret != LOS_OK) {
        return LOS_NOK;
    }
    return LOS_OK;
}

VOID TestHwiClear(UINT32 hwiNum)
{
    LOS_HwiClear(hwiNum);
}
#else
/*
 * ws63 PLIC (himideerv200) has no software-trigger mechanism (LOCIPD is
 * read-only, HalIrqTrigger is a stub). Calling HalIrqEnable on a real PLIC
 * source would enable a hardware interrupt whose handler dispatch is broken
 * (OsIntHandle is a stub), causing an interrupt storm and crash. Make
 * TestHwiTrigger a no-op so HWI tests fail gracefully without crashing.
 */
VOID TestHwiTrigger(UINT32 hwiNum)
{
    (VOID)hwiNum;
#if (LOS_FEATURE_ADAPTED == 1)
    HalIrqEnable(hwiNum);
#endif
}

UINT32 TestHwiDelete(UINT32 hwiNum)
{
    (VOID)hwiNum;
    return LOS_OK;
}

VOID TestHwiClear(UINT32 hwiNum)
{
    (VOID)hwiNum;
}
#endif /* LOSCFG_RISCV_LCMP_CLIC */

#define HIGH_SHIFT 32
UINT64 LosCpuCycleGet(VOID)
{
    UINT64 timeCycle;

    timeCycle = LOS_SysCycleGet();
    return timeCycle;
}
#else

#ifdef __ARM_ARCH_7A__
/* Cortex-A (GIC): spin iterations after software-pend. GIC delivery takes
 * ~1µs; 10000 volatile iterations ≈ 70µs @ ~900MHz — 70x margin, and only
 * costs time when the IRQ is masked (test fails anyway in that case). */
#define OS_HWI_TRIG_SPIN 10000
#endif

#define OS_NVIC_SETPEND 0xE000E200
#define OS_NVIC_CLRPEND 0xE000E280
#define HWI_SHIFT_NUM 5
#define HWI_BIT 2

VOID TestHwiTrigger(UINT32 hwiNum)
{
    LOS_HwiTrigger(hwiNum);
#ifdef __ARM_ARCH_7A__
    /* GIC software-pend is asynchronous: the ISPENDR write completes before
     * the distributor forwards the IRQ, so the handler runs only after this
     * function returns. Test cases assert on g_testCount immediately after
     * trigger, assuming the NVIC synchronous semantics. Spin to let the IRQ
     * preempt us and the handler finish (same approach as the ws63 timer1
     * busy-wait above). If the task is suspended by the handler (hwi_019),
     * we simply resume and finish the loop later — harmless. */
    for (volatile UINT32 i = 0; i < OS_HWI_TRIG_SPIN; i++) {}
#endif
}

VOID TestHwiUnTrigger(UINT32 hwiNum)
{
    LOS_HwiClear(hwiNum);
}
#define OS_NVIC_CLRENA_BASE 0xE000E180
#define NVIC_CLR_IRQ(uwHwiNum)                                                                       \
    do {                                                                                             \
        *(volatile UINT32 *)(OS_NVIC_CLRENA_BASE + (((uwHwiNum) >> HWI_SHIFT_NUM) << HWI_BIT)) = 1 << ((uwHwiNum) & 0x1F); \
    } while (0)

UINT32 TestHwiDelete(UINT32 hwiNum)
{
    UINT32 ret = LOS_HwiDelete(hwiNum, NULL);
    if (ret != LOS_OK) {
        return LOS_NOK;
    }
    return LOS_OK;
}
VOID TestHwiClear(UINT32 hwiNum) {}
#endif

#if (LOSCFG_BASE_IPC_SEM == 1)
UINT32 TestSemDelete(UINT32 semHandle)
{
    return LOS_SemDelete(semHandle);
}
UINT32 g_usSemID;
UINT32 g_usSemID2;
UINT32 g_usSemID3[LOSCFG_BASE_IPC_SEM_LIMIT + 1];
#endif

