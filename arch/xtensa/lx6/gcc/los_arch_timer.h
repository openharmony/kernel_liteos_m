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

#ifndef _LOS_ARCH_TIMER_H
#define _LOS_ARCH_TIMER_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_timer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

VOID SysTick_Handler(VOID);

#define TIM0_GROUP0             0x3FF5F000
#define TIM0_GROUP1             0x3FF60000
#define TIM1_GROUP0             0x3FF5F024
#define TIM1_GROUP1             0x3FF60024

#define TIM0_INT_ENABLE_GROUP0  0x3FF5F098
#define TIM0_INT_ENABLE_GROUP1  0x3FF60098
#define TIM0_INT_CLEAR_GROUP0   0x3FF5F0A4
#define TIM0_INT_CLEAR_GROUP1   0x3FF600A4

typedef struct {
    UINT32 CTRL;
    UINT64 VAL;
    UINT32 UPDATE;
    UINT64 ALARM;
    UINT64 LOAD;
    UINT32 LOAD_TRI;
} Systick_t;

STATIC INLINE UINT32 GetCcount(VOID)
{
    UINT32 intSave;
    __asm__ __volatile__("rsr %0, ccount" : "=a"(intSave) :);
    return intSave;
}

STATIC INLINE VOID ResetCcount(VOID)
{
    __asm__ __volatile__("wsr %0, ccount; rsync" : :"a"(0));
}

STATIC INLINE UINT32 GetCcompare(VOID)
{
    UINT32 intSave;
    __asm__ __volatile__("rsr %0, ccompare0" : "=a"(intSave) :);
    return intSave;
}

STATIC INLINE VOID SetCcompare(UINT32 newCompareVal)
{
    __asm__ __volatile__("wsr %0, ccompare0; rsync" : : "a"(newCompareVal));
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_ARCH_TIMER_H */
