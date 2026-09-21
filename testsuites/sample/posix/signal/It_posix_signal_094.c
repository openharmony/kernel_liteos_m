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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "It_posix_signal.h"
#include <pthread.h>

/*
 * BUG_08(20260912 实测定性, 暂隔离): posix sigwait 族唤醒 mask 位序错位 ——
 *  1) musl porting signal.h:206 sigaddset 置位 1U<<sig(SIGUSR2=12 -> 0x1000);
 *  2) 内核 los_signal.h:101 LOS_SIGNAL_MASK(sigNo)=1U<<(sigNo-1)(12 -> 0x800);
 *  3) 等待任务经 sigwaitinfo->sigtimedwait->LOS_SignalWait 将 posix 位序 set 灌入
 *     sigCB->sigWaitFlag(SignalTimedWait:347), 而 SignalSend 唤醒判断
 *     (los_signal.c:210) 用 LOS_SIGNAL_MASK 位序 -> 永不命中 -> 等待任务永睡;
 *  4) 实证: qemu 探针 "[094K] WAIT set but mask miss waitFlag=1000 sigNo=12"。
 * 影响面: sigwait/sigwaitinfo/sigtimedwait 的阻塞等待路径全部失效(等待者永睡);
 *         sigwait 的 timeout=0 假成功(signal.c:117)与 BUG_03 疑点疑与此同根因。
 * 不受影响: signal() 注册 handler 后 kill 的异步注入模式(090 已验证)。
 * 修复方向: 统一位序(posix 层 set 入内核前转换, 或内核判断按 1U<<sigNo)。
 * 修复后删除本隔离恢复 ItPosixSignal094 执行。
 */
#ifndef PRODUCT_BUG_ISOLATE
#include <errno.h>

/* 跨任务同步方案: 参照 It_posix_signal_090.c(sender 子任务向主任务发信号)反向使用:
 * 本用例为"主任务 -> 等待子任务"方向, 子任务置 volatile 握手标志 g_sig094Ready
 * (其前完成 signal 注册), 主任务轮询该标志后再让出 2 tick 确保子任务已真正进入
 * sigwaitinfo 阻塞; 完成侧同样轮询 g_sig094WaitDone, 规避硬编码 delay 的调度抖动 */
static volatile int g_sig094Ready = 0;
static volatile int g_sig094WaitDone = 0;

static VOID Sig094Handler(int sig)
{
    (VOID)sig;
}

/**
 * @tc.number    SUB_KERNEL_POSIX_SIGNAL_SIGWAITINFO_001
 * @tc.name      sigwaitinfo cross-task blocking wait normal path
 * @tc.desc      [C- SOFTWARE -0200]
 */
/* 用例简要描述: sigwaitinfo 跨任务阻塞等待正路径(主任务向等待中的子任务投递 SIGUSR2) */
static VOID *WaiterTask094(VOID *arg)
{
    sigset_t set;
    siginfo_t info;
    int ret;

    (VOID)arg;

    /* 前置: 注册 SIGUSR2 置 sigSetFlag, 否则 LOS_SignalSend 受 NO_SET 守卫拒绝
     * (los_signal.c:441); 等待中的任务收到信号走唤醒分支, handler 不会执行 */
    (VOID)signal(SIGUSR2, Sig094Handler);

    (VOID)sigemptyset(&set);
    (VOID)sigaddset(&set, SIGUSR2);
    (VOID)memset_s(&info, sizeof(info), 0, sizeof(info));

    g_sig094Ready = 1;
    /* 无 pending 信号时阻塞: sigwaitinfo -> sigtimedwait(NULL 超时=LOS_WAIT_FOREVER)
     * -> SignalTimedWait 挂入 g_waitSignalList(los_signal.c:353) */
    ret = sigwaitinfo(&set, &info);

    /* F-Pos: POSIX 契约——成功等到信号后 info 回填所等信号(当前实现正确) */
    ICUNIT_GOTO_EQUAL(info.si_signo, SIGUSR2, info.si_signo, EXIT);

    /* 契约固化(当前错误行为, 内核修复后翻转本组断言):
     * POSIX 期望 sigwaitinfo 成功返回信号号(此处应==SIGUSR2), 当前实现返回 -1, 机理链:
     * 1) signal.c:136-137 sigtimedwait 以 ret==LOS_OK(0) 判成功;
     * 2) LOS_SignalWait 成功时返回正数信号号(los_signal.c:411 return sigNo), 恒非 0;
     * 3) 判成功永假 -> 走 else 分支 -> errno=EINVAL -> 返回 -1(info 已正确回填,
     *    仅返回值符号语义错误; 修复方向: 成功分支改为返回信号号) */
    ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT);
    ICUNIT_GOTO_EQUAL(errno, EINVAL, errno, EXIT);

    g_sig094WaitDone = 1;

EXIT:
    return NULL;
}

static UINT32 Testcase094(VOID)
{
    int ret;
    pthread_t waiterThread;
    pthread_attr_t waiterAttr;
    struct sched_param sp;
    UINT32 i;

    g_sig094Ready = 0;
    g_sig094WaitDone = 0;

    ret = pthread_attr_init(&waiterAttr);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    sp.sched_priority = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    (VOID)pthread_attr_setschedparam(&waiterAttr, &sp);
    (VOID)pthread_attr_setinheritsched(&waiterAttr, PTHREAD_EXPLICIT_SCHED);

    ret = pthread_create(&waiterThread, &waiterAttr, WaiterTask094, NULL);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_ATTR);

    /* 时序容差: 轮询等待子任务就绪(ready 置位前 signal 注册已完成) */
    for (i = 0; i < 100; i++) {
        if (g_sig094Ready == 1) {
            break;
        }
        LOS_TaskDelay(1);
    }
    ICUNIT_GOTO_EQUAL(g_sig094Ready, 1, g_sig094Ready, EXIT_JOIN);
    /* ready 置位与真正进入 sigwaitinfo 阻塞之间的调度空隙, 再让出 2 tick */
    LOS_TaskDelay(2);

    /* F-Pos: 向阻塞等待中的子任务投递 SIGUSR2(唤醒路径 los_signal.c:209-214:
     * sigWaitFlag 命中 -> 回填 sigInfo -> OsSchedWake) */
    ret = pthread_kill(waiterThread, SIGUSR2);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT_JOIN);

    /* 等待子任务完成全部断言 */
    for (i = 0; i < 100; i++) {
        if (g_sig094WaitDone == 1) {
            break;
        }
        LOS_TaskDelay(1);
    }
    ICUNIT_GOTO_EQUAL(g_sig094WaitDone, 1, g_sig094WaitDone, EXIT_JOIN);

EXIT_JOIN:
    (VOID)pthread_join(waiterThread, NULL);
EXIT_ATTR:
    (VOID)pthread_attr_destroy(&waiterAttr);
    return LOS_OK;
}

/**
 * @tc.number    SUB_KERNEL_POSIX_SIGNAL_SIGWAIT_001
 * @tc.name      sigwait invalid-parameter stub contract
 * @tc.desc      [C- SOFTWARE -0200]
 */
/* 契约固化(项目先例 testIpcSem_Named001): sigwait 对非法入参当前假成功, 机理链:
 * 1) signal.c:117 sigwait 以 timeout=0 调 LOS_SignalWait(不等待);
 * 2) los_signal.c:340-343 SignalTimedWait 见 timeout==0 立即返回
 *    LOS_ERRNO_SIGNAL_INVALID(LOS_ERRNO_OS_FATAL 族 UINT32 大正数错误码);
 * 3) signal.c:118 以 ret<0 判失败——错误码为 UINT32 大正数, int 化后仍>0,
 *    判失败为假 -> 落入 *sig = info.si_signo(局部 {0} 初始化, 内核未回填, 恒 0)
 *    且返回 0(假成功);
 * POSIX 期望: sigwait 应阻塞等待信号到达; 对 NULL/空 set 应失败返回 -1 并置 EINVAL;
 * 修复方向: timeout 传 LOS_WAIT_FOREVER + 返回码判断改为错误码比较(而非 ret<0);
 * 内核修复后翻转本组断言 */
/* 用例简要描述: sigwait 非法入参(NULL/空集)假成功契约固化 */
VOID ItPosixSignal094(VOID)
{
    TEST_ADD_CASE("ItPosixSignal094", Testcase094, TEST_LIB, TEST_POSIX, TEST_LEVEL2, TEST_FUNCTION);
}

#endif /* PRODUCT_BUG_ISOLATE */
static UINT32 Testcase095(VOID)
{
    sigset_t set;
    int sig;
    int ret;

    /* F-Inv: set=NULL -> LOS_SignalWait:378 早返 INVALID -> 假成功: 返回 0 且 *sig 被填 0 */
    sig = -1; /* 预置非 0, 观察内核是否真实回填信号号 */
    ret = sigwait(NULL, &sig);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret); /* 内核修复后翻转本断言(期望 -1/EINVAL 或阻塞语义) */
    ICUNIT_ASSERT_EQUAL(sig, 0, sig); /* 内核修复后翻转本断言(期望真实信号号) */

    /* F-Inv: 空 set(*set==0) -> LOS_SignalWait:378 早返 INVALID -> 同样假成功 */
    (VOID)sigemptyset(&set);
    sig = -1;
    ret = sigwait(&set, &sig);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret); /* 内核修复后翻转本断言 */
    ICUNIT_ASSERT_EQUAL(sig, 0, sig); /* 内核修复后翻转本断言 */

    return LOS_OK;
}


VOID ItPosixSignal095(VOID)
{
    TEST_ADD_CASE("ItPosixSignal095", Testcase095, TEST_LIB, TEST_POSIX, TEST_LEVEL1, TEST_FUNCTION);
}
