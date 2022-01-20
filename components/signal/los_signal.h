/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _LOS_SIGNAL_H
#define _LOS_SIGNAL_H

#include <signal.h>
#include "los_list.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_signal
 * Signal error code: The parameters of interface is error.
 *
 * Value: 0x02003200
 *
 */
#define LOS_ERRNO_SIGNAL_INVALID                    LOS_ERRNO_OS_FATAL(LOS_MOD_SIGNAL, 0x00)

/**
 * @ingroup los_signal
 * Signal error code: The memory requests failed.
 *
 * Value: 0x02003201
 *
 */
#define LOS_ERRNO_SIGNAL_NO_MEMORY                  LOS_ERRNO_OS_ERROR(LOS_MOD_SIGNAL, 0x01)

/**
 * @ingroup los_signal
 * Signal error code: The signal is not set.
 *
 * Value: 0x02003202
 *
 */
#define LOS_ERRNO_SIGNAL_NO_SET                     LOS_ERRNO_OS_ERROR(LOS_MOD_SIGNAL, 0x02)

/**
 * @ingroup los_signal
 * Signal error code: Waiting for signal timeout.
 *
 * Value: 0x02003203
 *
 */
#define LOS_ERRNO_SIGNAL_TIMEOUT                    LOS_ERRNO_OS_ERROR(LOS_MOD_SIGNAL, 0x03)

/**
 * @ingroup los_signal
 * Signal error code: The interface is used before system start.
 *
 * Value: 0x02003204
 *
 */
#define LOS_ERRNO_SIGNAL_CAN_NOT_CALL               LOS_ERRNO_OS_ERROR(LOS_MOD_SIGNAL, 0x04)

/**
* @ingroup los_signal
* Mutex error code: Waiting for signal in interrupt callback.
*
* Value: 0x02003205
*
*/
#define LOS_ERRNO_SIGNAL_PEND_INTERR                LOS_ERRNO_OS_ERROR(LOS_MOD_SIGNAL, 0x05)

/**
 * @ingroup los_signal
 * Add the signal num to the signal set.
 */
#define LOS_SIGNAL_MASK(sigNo)          (1U << ((sigNo) - 1))

/**
 * @ingroup los_signal
 * Maximum signal supported num.
 */
#define LOS_SIGNAL_SUPPORT_MAX           31

/**
 * @ingroup los_signal
 * Signal handler type.
 */
typedef VOID (*SIG_HANDLER)(INT32 sigNo);

typedef struct {
    sigset_t     sigSetFlag;                              /**< installing signals */
    sigset_t     sigPendFlag;                             /**< pending signals */
    sigset_t     sigWaitFlag;                             /**< waiting signals */
    siginfo_t    sigInfo;                                 /**< signal info */
    SIG_HANDLER  sigHandlers[LOS_SIGNAL_SUPPORT_MAX + 1]; /**< signal handler */
    LOS_DL_LIST  sigInfoList;                             /**< signal info list */
    VOID         *sigSaveSP;                              /**< save stack pointer */
    VOID         *sigRestoreSP;                           /**< restore stack pointer */
    UINT32       sigStatus;                               /**< status of signal */
} OsSigCB;

typedef struct {
    LOS_DL_LIST node;
    siginfo_t info;
} OsSigInfoNode;

#define OS_SIGNAL_STATUS_WAIT            0x0001
#define OS_SIGNAL_VALID(sigNo)           (((sigNo) > 0) && ((sigNo) <= LOS_SIGNAL_SUPPORT_MAX))

UINT32 OsSignalInit(VOID);
UINTPTR OsSignalTaskContextRestore(VOID);

/**
 * @ingroup los_signal
 * @brief Register the handler for the specified signal.
 *
 * @par Description:
 * This API is used to register the handler for the specified signal, otherwise it is the default handler.
 *
 * @attention None.
 *
 * @param sigNo   [IN] The specified signal num.
 * @param handler [IN] The handler for this signal, which is either SIG_IGN, SIG_DFL,
 * or the address of a programmer-defined function.
 *
 * @retval: SIG_ERR Type#SIG_HANDLER: error code.
 * @retval: old     Type#SIG_HANDLER: success, the previous handler is returned.
 * <ul><li>los_signal.h: the header file that contains the API declaration.</li></ul>
 * @see None
 */
SIG_HANDLER LOS_SignalSet(INT32 sigNo, SIG_HANDLER handler);

/**
 * @ingroup los_signal
 * @brief Shield the specified signal set.
 *
 * @par Description:
 * This API is used to shield the specified signal set and get the current signal set.
 *
 * @attention None.
 *
 * @param how    [IN] The behavior of the call is dependent on the value of how, which is either SIG_BLOCK,
 * SIG_UNBLOCK, SIG_SETMASK.
 * @param set    [IN] The new signal set.
 * @param oldSet [OUT] The old signal set.
 *
 * @retval: LOS_ERRNO_SIGNAL_CAN_NOT_CALL Type#UINT32: The interface is used before system start.
 * @retval: LOS_ERRNO_SIGNAL_NO_MEMORY Type#UINT32: The memory requests failed.
 * @retval: LOS_ERRNO_SIGNAL_INVALID Type#UINT32: The parameters of interface is error.
 * @retval: LOS_OK Type#UINT32: success.
 * <ul><li>los_signal.h: the header file that contains the API declaration.</li></ul>
 * @see None
 */
UINT32 LOS_SignalMask(INT32 how, const sigset_t *set, sigset_t *oldSet);

/**
 * @ingroup los_signal
 * @brief Suspend execution of the calling thread until one of the signals specified in
 * the signal set becomes pending.
 *
 * @par Description:
 * This API is used to suspend execution of the calling thread until one of the signals
 * specified in the signal set becomes pending and return the signal number in sig.
 *
 * @attention None.
 *
 * @param set     [IN] The specified signal set which waiting for.
 * @param info    [OUT] The info of signal becomes pending.
 * @param timeout [IN] The waiting time.
 *
 * @retval: LOS_ERRNO_SIGNAL_INVALID Type#UINT32: The parameters of interface is error.
 * @retval: LOS_ERRNO_SIGNAL_CAN_NOT_CALL Type#UINT32: The interface is used before system start.
 * @retval: LOS_ERRNO_SIGNAL_NO_MEMORY Type#UINT32: The memory requests failed.
 * @retval: LOS_ERRNO_SIGNAL_PEND_INTERR Type#UINT32: Waiting for signal in interrupt callback.
 * @retval: LOS_ERRNO_SIGNAL_TIMEOUT Type#UINT32: Waiting for signal timeout.
 * @retval: signo Type#UINT32: success, returning the signal num which becomes pending.
 * <ul><li>los_signal.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_SignalSend
 */
UINT32 LOS_SignalWait(const sigset_t *set, siginfo_t *info, UINT32 timeout);

/**
 * @ingroup los_signal
 * @brief Send the specified signal to the specified task.
 *
 * @par Description:
 * This API is used to send the specified signal to the specified task.
 *
 * @attention None.
 *
 * @param taskID   [IN] Send a signal to this task.
 * @param sigNo    [IN] The signal num.
 *
 * @retval: LOS_ERRNO_SIGNAL_NO_MEMORY Type#UINT32: The memory requests failed.
 * @retval: LOS_ERRNO_SIGNAL_INVALID Type#UINT32: The parameters of interface is error.
 * @retval: LOS_ERRNO_SIGNAL_NO_SET Type#UINT32: The signal is not set.
 * @retval: LOS_OK Type#UINT32: success.
 * <ul><li>los_signal.h: the header file that contains the API declaration.</li></ul>
 * @see None
 */
UINT32 LOS_SignalSend(UINT32 taskID, INT32 sigNo);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_SIGNAL_H */