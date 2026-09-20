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

#ifndef _HISOC_CLIC_H
#define _HISOC_CLIC_H

#include "los_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* ---- CLIC register layout (struct description, GIC/NVIC driver pattern) ----
 * The board's soc.h must define CLIC_BASE_ADDR before CLIC_REG is used
 * (e.g. device/board/hihope/hi3322/.../asm/soc.h).
 *
 * Memory map (CLIC spec + LCMP implementation):
 *   0x000  cliccfg   (RW, 8-bit)
 *   0x004  clicinfo  (RO, 32-bit)
 *   0x1000 per-interrupt block: 4 bytes per interrupt —
 *          clicintip / clicintie / clicintattr / clicintctl at +0/+1/+2/+3
 */

/* Per-interrupt register group (4 bytes, at 0x1000 + 4*n). */
struct ClicIntCtrl {
    volatile UINT8 ip;      /* +0 clicintip: pending (SW-writable when edge-triggered) */
    volatile UINT8 ie;      /* +1 clicintie: enable */
    volatile UINT8 attr;    /* +2 clicintattr: mode[7:6], trig[1], vectored[0] */
    volatile UINT8 ctl;     /* +3 clicintctl: priority/level bits */
};

struct ClicRegs {
    volatile UINT8 cfg;                 /* 0x000 cliccfg */
    UINT8 reserved0[3];                 /* 0x001-0x003 */
    volatile UINT32 info;               /* 0x004 clicinfo (RO) */
    UINT8 reserved1[0x1000 - 0x008];    /* 0x008-0x0FFF */
    struct ClicIntCtrl intCtrl[];       /* 0x1000+, count = clicinfo.NUM_INTERRUPT */
};

#define CLIC_REG   ((volatile struct ClicRegs *)(UINTPTR)CLIC_BASE_ADDR)

/* clicinfo.CLICINTCTLBITS: priority bits implemented in clicintctl (LCMP: 3). */
#define REG_CLIC_INFO_CLICINTCTLBITS 3

/* clicintattr bits */
#define CLIC_ATTR_VECTORED       (1U << 0)   /* 0 = non-vectored (this driver) */
#define CLIC_ATTR_EDGE_TRIG      (1U << 1)   /* edge-triggered; required so SW can set clicintip */
#define CLIC_ATTR_MODE_SHIFT     6
#define CLIC_ATTR_MODE_M         (3U << CLIC_ATTR_MODE_SHIFT)  /* machine mode */

extern VOID HalIrqInit(VOID);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
