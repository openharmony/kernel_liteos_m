/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
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

#ifndef _RTC_TIME_HOOK_H
#define _RTC_TIME_HOOK_H

#include "los_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */


/*
 * RTC interfaces for improving the accuracy of the time, need implement and
 * register by user, implementation method is as follows:
 *
 * UINT64 RtcGetTickHook(VOID)
 * brief: supplement expanded ticks
 * input: VOID
 * return: UINT64 ticks
 *
 * INT32 RtcGetTimeHook(UINT64 *usec)
 * brief: get RTC time
 * output: UINT64 *usec, pointer to get the time, value is counted in microsecond(us)
 * return: non-0 if error, 0 if success.
 *
 * INT32 RtcSetTimeHook(UINT64 msec, UINT64 *usec)
 * brief: set RTC time
 * input: UINT64 msec, the time to be set counted in millisecond(ms).
 * input: UINT64 *usec, the time to be set counted in microsecond(us)
 * return: non-0 if error, 0 if success.
 *
 * INT32 RtcGetTimezoneHook(INT32 *tz)
 * brief: get RTC timezone
 * output: INT32 *tz, pointer to get the timezone, value is counted in second
 * return: non-0 if error, 0 if success.
 *
 * INT32 RtcSetTimezoneHook(INT32 tz)
 * brief: set RTC timezone
 * input: INT32 tz, the timezone to be set, value is counted in second
 * return: non-0 if error, 0 if success.
 */
struct RtcTimeHook {
    UINT64  (*RtcGetTickHook)(VOID);
    INT32   (*RtcGetTimeHook)(UINT64 *usec);
    INT32   (*RtcSetTimeHook)(UINT64 msec, UINT64 *usec);
    INT32   (*RtcGetTimezoneHook)(INT32 *tz);
    INT32   (*RtcSetTimezoneHook)(INT32 tz);
};

/*
 * @brief let user register a RTC getter hook to improve time accuracy for time-related POSIX methods.
 * @param cfg, a structure, member is pointer to function to be registered
 * @return VOID.
 */
VOID LOS_RtcHookRegister(struct RtcTimeHook *cfg);

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
