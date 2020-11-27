/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
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

#ifndef _LOS_HW_H
#define _LOS_HW_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_hw
 * The initialization value of stack space.
 */
#define EMPTY_STACK                 0xCACA

/**
 * @ingroup los_hw
 * Check task schedule.
 */
#define LOS_CHECK_SCHEDULE          ((!g_losTaskLock) && (!OS_INT_ACTIVE))

/**
 * @ingroup los_hw
 * @brief Wait for interrupt.
 *
 * @par Description:
 * <ul>
 * <li>This API is used to suspend execution until interrupt or a debug request occurs.</li>
 * </ul>
 * @attention None.
 *
 * @param None.
 *
 * @retval: None.
 *
 * @par Dependency:
 * los_hw.h: the header file that contains the API declaration.
 * @see None.
 * @since Huawei LiteOS V200R002C00
 */
extern VOID wfi(VOID);

/**
 * @ingroup los_hw
 * @brief: mem fence function.
 *
 * @par Description:
 * This API is used to fence for memory.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 * @param: None.
 *
 * @retval:None.
 * @par Dependency:
 * <ul><li>los_hw.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V200R002C00
 */
extern VOID mb(VOID);

/**
 * @ingroup los_hw
 * @brief: mem fence function.
 *
 * @par Description:
 * This API is same as mb, it just for adaptation.
 *
 * @attention:
 * <ul><li>None.</li></ul>
 *
 * @param: None.
 *
 * @retval:None.
 * @par Dependency:
 * <ul><li>los_hw.h: the header file that contains the API declaration.</li></ul>
 * @see None.
 * @since Huawei LiteOS V200R002C00
 */
extern VOID dsb(VOID);

extern VOID LOS_GetCpuCycle(UINT32 *cntHi, UINT32 *cntLo);
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_HW_H */
