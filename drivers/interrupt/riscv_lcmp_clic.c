/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2026 Huawei Device Co., Ltd. All rights reserved.
 * Copyright (c) 2026-2026 HiSilicon (Shanghai) Technologies Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of the following disclaimer in the documentation and/or other materials
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
 * Description: CLIC interrupt controller driver for HI3322 (LinxCore).
 *              CLIC uses memory-mapped registers (base CLIC_BASE_ADDR from
 *              the board's soc.h, struct layout in clic.h), non-vector mode,
 *              all traps dispatch via mtvec.
 */

#include "los_interrupt.h"
#include "los_hwi_pri.h"
#include "los_arch_interrupt.h"
#include "soc.h"
#include "clic.h"

/* clicintattr: non-vectored (bit0=0), edge-triggered (bit1=1), M-mode ([7:6]=3).
 * Edge-triggered is required so that software can set clicintip[i] by writing 1
 * (per CLIC spec, level-triggered mode ignores software writes when the hardware
 * source is not asserting). This enables TestHwiTrigger / LOS_HwiTrigger. */
#define CLIC_ATTR_MMODE      (CLIC_ATTR_MODE_M | CLIC_ATTR_EDGE_TRIG)

/* cliccfg: nlbits=4, bit0=0 (non-vector global) */
#define CLIC_CFG_VALUE       (4u << 1)

#define CLIC_INTCTL_DEF      0xFFu

/* mcause CSR layout: bit31 = interrupt (1) vs exception (0);
 * bits 30..0 = interrupt/exception code. */
#define MCAUSE_INT_BIT       0x80000000u
#define MCAUSE_INT_ID_MASK   0x7FFFFFFFu

#define CLIC_CFG_REGISTER_BITS_NUM 8U
#define CLIC_CFG_INTERRUPT_OFFSET  1U
#define SET_SYS_PRIOR(usrPrior) \
    ((UINT8)((UINT8)(LOSCFG_HWI_PRIO_LIMIT - 1) + CLIC_CFG_INTERRUPT_OFFSET - (usrPrior)))

#if LOSCFG_HWI_PRIO_LIMIT > (1U << REG_CLIC_INFO_CLICINTCTLBITS)
#error "LOSCFG_HWI_PRIO_LIMIT exceeds CLIC hardware level bits (REG_CLIC_INFO_CLICINTCTLBITS)."
#endif

STATIC UINT32 g_clicCfgLevelBitsNum = 1;

/* Byte access with fence — CLIC requires a fence around each MMIO access. */
static inline void clic_w8(volatile UINT8 *reg, uint8_t v)
{
    *reg = v;
    __asm__ __volatile__("fence" ::: "memory");
}

static inline uint8_t clic_r8(volatile UINT8 *reg)
{
    __asm__ __volatile__("fence" ::: "memory");
    return *reg;
}

STATIC UINT32 HalIrqClear(UINT32 hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_NOK;
    }
    clic_w8(&CLIC_REG->intCtrl[hwiNum].ip, 0);
    return LOS_OK;
}

STATIC UINT32 HalIrqTrigger(UINT32 hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_NOK;
    }
    clic_w8(&CLIC_REG->intCtrl[hwiNum].ip, 1);
    return LOS_OK;
}

STATIC UINT32 HalIrqUnmask(UINT32 hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_NOK;
    }
    clic_w8(&CLIC_REG->intCtrl[hwiNum].ie, 1);
    return LOS_OK;
}

STATIC UINT32 HalIrqMask(UINT32 hwiNum)
{
    if (hwiNum >= OS_HWI_MAX_NUM) {
        return LOS_NOK;
    }
    clic_w8(&CLIC_REG->intCtrl[hwiNum].ie, 0);
    return LOS_OK;
}

STATIC UINT32 HalIrqSetPrio(UINT32 hwiNum, HWI_PRIOR_T priority)
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
    UINT8 clicIntCtl = clic_r8(&CLIC_REG->intCtrl[hwiNum].ctl);
    clicIntCtl = (UINT8)((clicIntCtl & mask) | localLevel);
    clic_w8(&CLIC_REG->intCtrl[hwiNum].ctl, clicIntCtl);
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

/* Driver-owned ops table (arm_nvic.c pattern): the selected driver owns
 * g_hwiControllerOps + HwiControllerOpsGet; the arch layer only consumes them via
 * HwiControllerOpsGet() (e.g. HalHwiInterruptDone). Statically initialized —
 * complete from the first instruction, no init-order window. */
STATIC HwiControllerOps g_hwiControllerOps = {
    .triggerIrq     = HalIrqTrigger,
    .clearIrq       = HalIrqClear,
    .enableIrq      = HalIrqUnmask,
    .disableIrq     = HalIrqMask,
    .setIrqPriority = HalIrqSetPrio,
    .getCurIrqNum   = HalCurIrqGet,
    .getHandleForm  = HalGetHandleForm,
};

HwiControllerOps *HwiControllerOpsGet(VOID)
{
    return &g_hwiControllerOps;
}

LITE_OS_SEC_TEXT_INIT VOID HalIrqInit(VOID)
{
    clic_w8(&CLIC_REG->cfg, CLIC_CFG_VALUE);

    for (UINT32 i = 0; i < CLIC_CFG_REGISTER_BITS_NUM; i++) {
        if (((UINT8)(LOSCFG_HWI_PRIO_LIMIT - 1) & ((0x80U) >> i)) != 0) {
            g_clicCfgLevelBitsNum = 8U - i;
            break;
        }
    }

    for (UINT32 i = 0; i < OS_HWI_MAX_NUM; i++) {
        clic_w8(&CLIC_REG->intCtrl[i].ie, 0);       /* disable first — prevents re-pend */
        clic_w8(&CLIC_REG->intCtrl[i].ip, 0);       /* clear pending */
        clic_w8(&CLIC_REG->intCtrl[i].attr, CLIC_ATTR_MMODE);
        clic_w8(&CLIC_REG->intCtrl[i].ctl, CLIC_INTCTL_DEF);
    }
    /* mtvec is CPU-level trap configuration — set by the arch HalHwiInit
     * (see arch/risc-v/riscv32/gcc/los_interrupt.c), not by this driver. */
}
