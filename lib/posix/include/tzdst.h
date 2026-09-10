/* ----------------------------------------------------------------------------
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: LiteOS Time Zone and DST API
 * Author: Huawei LiteOS Team
 * Create: 2019-11-01
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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
 * --------------------------------------------------------------------------- */

#ifndef _TZDST_H
#define _TZDST_H

#include "time.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @ingroup tzdst
 * @par Description:
 * This API is used to set the timezone string in the format of
 * "tzn[+/-]hh[:mm[:ss]][dzn]", for example "CST+8" means UTC+8 (the
 * sign is east-positive, the opposite of the POSIX TZ string).
 *
 * @attention
 * <ul>
 * <li>Only the timezone part is parsed. DST string configures dst_enable.</li>
 * </ul>
 *
 * @retval #None
 *
 * @par Dependency:
 * <ul><li>tzdst.h</li></ul>
 * @see dst_enable | dst_disable | dst_inquire
 */
void settimezone(const char *);

/**
 * @ingroup tzdst
 * @par Description:
 * This API is used to disable the DST function.
 *
 * @retval #0  Succeed.
 * @retval #-1 Fail.
 *
 * @par Dependency:
 * <ul><li>tzdst.h</li></ul>
 * @see dst_enable | dst_inquire
 */
int dst_disable(void);

/**
 * @ingroup tzdst
 * @par Description:
 * This API is used to enable the DST function.
 * The time string format is "MMM-dd HH:MM:SS" or "MMM-nWw-WWW HH:MM:SS",
 * for example "Feb-03 03:00:00" or "Oct-1st-Fri 02:59:59".
 *
 * @attention
 * <ul>
 * <li>The swForwardSeconds must be in [0, 24*3600).</li>
 * </ul>
 *
 * @retval #0  Succeed.
 * @retval #-1 Fail.
 *
 * @par Dependency:
 * <ul><li>tzdst.h</li></ul>
 * @see dst_disable | dst_inquire
 */
int dst_enable(const char *dstStartTime, const char *dstEndTime, int forwardSeconds);

/**
 * @ingroup tzdst
 * @par Description:
 * This API is used to inquire the DST start and end time of the specified year.
 *
 * @retval #0  Succeed.
 * @retval #-1 Fail.
 *
 * @par Dependency:
 * <ul><li>tzdst.h</li></ul>
 * @see dst_enable | dst_disable
 */
int dst_inquire(int year, struct tm *dstStart, struct tm *dstEnd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _TZDST_H */
