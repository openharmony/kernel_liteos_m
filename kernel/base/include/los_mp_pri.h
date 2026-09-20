/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 */

#ifndef _LOS_MP_PRI_H
#define _LOS_MP_PRI_H

#include "los_compiler.h"
#include "los_interrupt.h"
#include "los_percpu_pri.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define OS_MP_CPU_ALL       LOSCFG_KERNEL_CPU_MASK

#define OS_MP_GC_PERIOD     100 /* ticks */

#define OS_HWI_IPI_BASE     0

typedef enum {
    LOS_MP_IPI_WAKEUP = OS_HWI_IPI_BASE,
    LOS_MP_IPI_SCHEDULE,
    LOS_MP_IPI_HALT,
} MP_IPI_TYPE;

#ifdef LOSCFG_KERNEL_SMP
extern VOID LOS_MpSchedule(UINT32 target);
extern VOID OsMpWakeHandler(VOID);
extern VOID OsMpScheduleHandler(VOID);
extern VOID OsMpHaltHandler(VOID);
extern UINT32 OsMpInit(VOID);
extern VOID OsMpCollectTasks(VOID);
#else
STATIC INLINE VOID LOS_MpSchedule(UINT32 target)
{
    (VOID)target;
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _LOS_MP_PRI_H */
