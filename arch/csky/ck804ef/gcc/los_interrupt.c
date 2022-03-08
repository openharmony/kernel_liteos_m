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
#include <stdarg.h>
#include <csi_core.h>
#include "securec.h"
#include "los_context.h"
#include "los_arch_context.h"
#include "los_arch_interrupt.h"
#include "los_debug.h"
#include "los_hook.h"
#include "los_task.h"
#include "los_sched.h"
#include "los_memory.h"
#include "los_membox.h"

#define INT_OFFSET       6
#define PRI_OFF_PER_INT  8
#define PRI_PER_REG      4
#define PRI_OFF_IN_REG   6
#define PRI_BITS         2
#define PRI_HI           0
#define PRI_LOW          7
#define MASK_8_BITS      0xFF
#define MASK_32_BITS     0xFFFFFFFF
#define BYTES_OF_128_INT 4

#define OS_USER_HWI_MIN                 0
#define OS_USER_HWI_MAX                 (LOSCFG_PLATFORM_HWI_LIMIT - 1)
#define HWI_ALIGNSIZE                   0x400

UINT32 volatile g_intCount = 0;
CHAR g_trapStackBase[OS_TRAP_STACK_SIZE];

STATIC VIC_TYPE *VIC_REG = (VIC_TYPE *)VIC_REG_BASE;

UINT32 HwiNumValid(UINT32 num)
{
    return ((num) >= OS_USER_HWI_MIN) && ((num) <= OS_USER_HWI_MAX);
}

UINT32 HalGetPsr(VOID)
{
    UINT32 intSave;
    __asm__ volatile("mfcr %0, psr" : "=r" (intSave) : : "memory");
    return intSave;
}

UINT32 HalSetVbr(UINT32 intSave)
{
    __asm__ volatile("mtcr %0, vbr" : : "r"(intSave)  : "memory");
    return intSave;
}

UINT32 ArchIntLock(VOID)
{
    UINT32 flags = __get_PSR();
    __disable_irq();
    return flags;
}

UINT32 ArchIntUnLock(VOID)
{
    UINT32 flags = __get_PSR();
    __enable_irq();
    return flags;
}

VOID ArchIntRestore(UINT32 intSave)
{
    __asm__ __volatile__(
    "mtcr   %0, psr \n"
    :
    :"r" (intSave)
    :"memory"
    );
}

UINT32 ArchIntLocked(VOID)
{
    UINT32 intSave;
    __asm__ volatile("mfcr %0, psr" : "=r" (intSave) : : "memory");
    return !(intSave & (1 << INT_OFFSET));
}

UINT32 ArchIsIntActive(VOID)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    if (g_intCount > 0) {
        LOS_IntRestore(intSave);
        return 1;
    }
    LOS_IntRestore(intSave);
    return 0;
}

#if (LOSCFG_PLATFORM_HWI_WITH_ARG == 1)

/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector, VOID *arg)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        csi_vic_set_vector(num, vector);
    }
}
#else
/* *
 * @ingroup los_hwi
 * Set interrupt vector table.
 */
VOID OsSetVector(UINT32 num, HWI_PROC_FUNC vector)
{
    if ((num + OS_SYS_VECTOR_CNT) < OS_VECTOR_CNT) {
        csi_vic_set_vector(num, vector);
    }
}
#endif

int csi_kernel_intrpt_enter(void)
{
    UINT32 intSave;
    intSave = LOS_IntLock();
    g_intCount++;
    LOS_IntRestore(intSave);
    return 0;
}
int csi_kernel_intrpt_exit(void)
{
    UINT32 intSave;

    intSave = LOS_IntLock();
    if (g_intCount > 0) {
        g_intCount--;
    }
    HalIrqEndCheckNeedSched();
    LOS_IntRestore(intSave);
    return 0;
}

