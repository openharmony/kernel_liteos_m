/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 * Copyright (c) 2026-2026 HiSilicon (Shanghai) Technologies Co., Ltd. All rights reserved.
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

/*
 * Description: CLIC interrupt controller driver for HI3322.
 *              CLIC uses memory-mapped registers at 0xb0000000 (non-vector
 *              mode, all traps dispatch via mtvec).
 */

#include "los_interrupt.h"
#include "los_hwi_pri.h"
#include "los_arch_interrupt.h"
#include "clic.h"
#include "soc.h"

extern VOID HalTrapVector(VOID);

#define CSR_MTVEC  0x305u

#define CLIC_CFG_OFF        0x00u
#define CLIC_INTIP_OFF(x)   (0x1000u + 4u * (x))
#define CLIC_INTIE_OFF(x)   (0x1001u + 4u * (x))
#define CLIC_INTATTR_OFF(x) (0x1002u + 4u * (x))
#define CLIC_INTCTL_OFF(x)  (0x1003u + 4u * (x))

/* clicintattr: bit0=0 (non-vector), bit1=1 (edge-triggered), [7:6]=3 M-mode.
 * Edge-triggered is required so that software can set clicintip[i] by writing 1
 * (per CLIC spec, level-triggered mode ignores software writes when the hardware
 * source is not asserting). This enables TestHwiTrigger / LOS_HwiTrigger. */
#define CLIC_ATTR_MMODE      ((3u << 6) | 0x02u)

/* cliccfg: nlbits=4, bit0=0 (non-vector global) */
#define CLIC_CFG_VALUE       (4u << 1)

#define CLIC_INTCTL_DEF      0xFFu

/* mcause CSR layout: bit31 = interrupt (1) vs exception (0);
 * bits 30..0 = interrupt/exception code. */
#define MCAUSE_INT_BIT       0x80000000u
#define MCAUSE_INT_ID_MASK   0x7FFFFFFFu

/*
 * Use hardcoded base instead of read_custom_csr(0xFBF) — re-reading the
 * CSR each call returned inconsistent values on this chip (writes went to
 * one address, reads from another).
 */
#define CLIC_REG_BASE        0xb0000000u

#define CLIC_CFG_REGISTER_BITS_NUM 8U
#define CLIC_CFG_INTERRUPT_OFFSET  1U
#define SET_SYS_PRIOR(usrPrior) \
    ((UINT8)((UINT8)(LOSCFG_HWI_PRIO_LIMIT - 1) + CLIC_CFG_INTERRUPT_OFFSET - (usrPrior)))

#if LOSCFG_HWI_PRIO_LIMIT > (1U << REG_CLIC_INFO_CLICINTCTLBITS)
#error "LOSCFG_HWI_PRIO_LIMIT exceeds CLIC hardware level bits (REG_CLIC_INFO_CLICINTCTLBITS)."
#endif

STATIC UINT32 g_clicCfgLevelBitsNum = 1;

static inline void clic_w8(UINT32 off, uint8_t v)
{
    *(volatile uint8_t *)(uintptr_t)(CLIC_REG_BASE + off) = v;
    __asm__ __volatile__("fence" ::: "memory");
}

static inline uint8_t clic_r8(UINT32 off)
{
    __asm__ __volatile__("fence" ::: "memory");
    return *(volatile uint8_t *)(uintptr_t)(CLIC_REG_BASE + off);
}

STATIC UINT32 HalIrqClear(UINT32 hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_NOK;
    }
    clic_w8(CLIC_INTIP_OFF(hwiNum), 0);
    return LOS_OK;
}

STATIC UINT32 HalIrqTrigger(UINT32 hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_NOK;
    }
    clic_w8(CLIC_INTIP_OFF(hwiNum), 1);
    return LOS_OK;
}

STATIC UINT32 HalIrqUnmask(UINT32 hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_NOK;
    }
    clic_w8(CLIC_INTIE_OFF(hwiNum), 1);
    return LOS_OK;
}

STATIC UINT32 HalIrqMask(UINT32 hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_NOK;
    }
    clic_w8(CLIC_INTIE_OFF(hwiNum), 0);
    return LOS_OK;
}

STATIC UINT32 HalIrqSetPrio(UINT32 hwiNum, UINT8 priority)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_ERRNO_HWI_NUM_INVALID;
    }
    if (priority >= LOSCFG_HWI_PRIO_LIMIT) {
        return LOS_ERRNO_HWI_PRIO_INVALID;
    }
    UINT8 localLevel = SET_SYS_PRIOR(priority);
    localLevel = (UINT8)((UINT32)localLevel << (8U - g_clicCfgLevelBitsNum));
    UINT8 mask = ((UINT8)(-1)) >> g_clicCfgLevelBitsNum;
    UINT8 clicIntCtl = clic_r8(CLIC_INTCTL_OFF(hwiNum));
    clicIntCtl = (UINT8)((clicIntCtl & mask) | localLevel);
    clic_w8(CLIC_INTCTL_OFF(hwiNum), clicIntCtl);
    return LOS_OK;
}

STATIC UINT32 HalCurIrqGet(VOID)
{
    UINT32 mcause;
    __asm__ volatile("csrr %0, mcause" : "=r"(mcause));
    if ((mcause & MCAUSE_INT_BIT) == 0) {
        return 0;
    }
    return mcause & MCAUSE_INT_ID_MASK;
}

LITE_OS_SEC_TEXT_INIT VOID HalIrqInit(VOID)
{
    HwiControllerOps *ops = ArchIntOpsGet();

    /* Register CLIC ops into the kernel's HwiControllerOps framework */
    ops->clearIrq       = HalIrqClear;
    ops->triggerIrq     = HalIrqTrigger;
    ops->enableIrq      = HalIrqUnmask;
    ops->disableIrq     = HalIrqMask;
    ops->setIrqPriority = HalIrqSetPrio;
    ops->getCurIrqNum   = HalCurIrqGet;

    clic_w8(CLIC_CFG_OFF, CLIC_CFG_VALUE);

    for (UINT32 i = 0; i < CLIC_CFG_REGISTER_BITS_NUM; i++) {
        if (((UINT8)(LOSCFG_HWI_PRIO_LIMIT - 1) & ((0x80U) >> i)) != 0) {
            g_clicCfgLevelBitsNum = 8U - i;
            break;
        }
    }

    for (UINT32 i = 0; i < OS_HWI_MAX_NUM; i++) {
        clic_w8(CLIC_INTIP_OFF(i), 0);       /* clear pending */
        clic_w8(CLIC_INTIE_OFF(i), 0);       /* disabled */
        clic_w8(CLIC_INTATTR_OFF(i), CLIC_ATTR_MMODE);
        clic_w8(CLIC_INTCTL_OFF(i), CLIC_INTCTL_DEF);
    }

    /* Point mtvec at HalTrapVector (assembly trap handler) */
    UINT32 trap_addr = (UINT32)(uintptr_t)HalTrapVector;
    __asm__ volatile ("csrw %0, %1" :: "i"(CSR_MTVEC), "r"(trap_addr) : "memory");
}
