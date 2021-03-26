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
#include "los_mpu.h"
#include "los_config.h"
#include "los_context.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#define SIZE_4G_BYTE        0x100000000
#define MPU_MAX_REGION_NUM  8

STATIC UINT8 g_regionNumBeUsed[MPU_MAX_REGION_NUM] = {0};

typedef enum {
    MPU_AP_FORBID_USER_FORBID = 0x0, /* Privileged:No access      Unprivileged:No access */
    MPU_AP_RW_USER_FORBID = 0x1,     /* Privileged:Read/Write     Unprivileged:No access */
    MPU_AP_RW_USER_RO = 0x2,         /* Privileged:Read/Write     Unprivileged:Read-only */
    MPU_AP_RW_USER_RW = 0x3,         /* Privileged:Read/Write     Unprivileged:Read/Write */
    MPU_AP_NA_USER_NA = 0x4,         /* Privileged:UNPREDICTABLE  Unprivileged:UNPREDICTABLE */
    MPU_AP_RO_USER_FORBID = 0x5,     /* Privileged:Read-only      Unprivileged:No access */
    MPU_AP_RO_USER_RO = 0x6,         /* Privileged:Read-only      Unprivileged:Read-only */
} MpuApConfig;

VOID HalMpuEnable(UINT32 defaultRegionEnable)
{
    UINTPTR intSave = HalIntLock();
    MPU->CTRL = (MPU_CTRL_ENABLE_Msk | ((defaultRegionEnable << MPU_CTRL_PRIVDEFENA_Pos) & MPU_CTRL_PRIVDEFENA_Msk));
    SCB->SHCSR |= SCB_SHCSR_MEMFAULTENA_Msk;
    __DSB();
    __ISB();
    HalIntRestore(intSave);
}
VOID HalMpuDisable(VOID)
{
    UINTPTR intSave = HalIntLock();
    MPU->CTRL = 0;
    __DSB();
    __ISB();
    HalIntRestore(intSave);
}

STATIC VOID HalMpuRASRAddMemAttr(MPU_CFG_PARA *para, UINT32 *RASR)
{
    BOOL cachable = 0;
    BOOL buffable = 0;
    switch (para->memType) {
        case MPU_MEM_ON_CHIP_ROM:
        case MPU_MEM_ON_CHIP_RAM:
            cachable = 1;
            buffable = 0;
            break;
        case MPU_MEM_XIP_PSRAM:
            cachable = 1;
            buffable = 1;
            break;
        case MPU_MEM_XIP_NOR_FLASH:
            cachable = 0;
            buffable = 1;
            break;
        default:
            break;
    }
    (*RASR) |= ((cachable << MPU_RASR_C_Pos) | (buffable << MPU_RASR_B_Pos));
}

STATIC UINT32 HalMpuEncodeSize(UINT64 size)
{
    UINT32 encodeSize = 0;
    if (size > SIZE_4G_BYTE) {
        return 0;
    }
    if ((size & 0x1F) != 0) { /* size sould aligned to 32 byte at least. */
        return 0;
    }
    size = (size >> 2);
    while (size != 0) {
        if (((size & 1) != 0) && ((size & 0xFFFFFFFE) != 0)) { /* size != 2^x (5 <= x <= 32)  128B - 4GB */
            return 0;
        }
        size = (size >> 1);
        encodeSize++;
    }
    return encodeSize;
}

STATIC UINT32 HalMpuEncodeAP(MpuAccessPermission permission)
{
    UINT32 ap;
    switch (permission) {
        case MPU_RW_BY_PRIVILEGED_ONLY:
            ap = MPU_AP_RW_USER_FORBID;
            break;
        case MPU_RW_ANY:
            ap = MPU_AP_RW_USER_RW;
            break;
        case MPU_RO_BY_PRIVILEGED_ONLY:
            ap = MPU_AP_RO_USER_FORBID;
            break;
        case MPU_RO_ANY:
            ap = MPU_AP_RO_USER_RO;
            break;
        default:
            ap = MPU_AP_RW_USER_RW;
            break;
    }
    return ap;
}

STATIC UINT32 HalMpuGetRASR(UINT32 encodeSize, MPU_CFG_PARA *para)
{
    UINT32 RASR;
    UINT32 ap;
    ap = HalMpuEncodeAP(para->permission);
    RASR = MPU_RASR_ENABLE_Msk;
    RASR |= ((encodeSize << MPU_RASR_SIZE_Pos) & MPU_RASR_SIZE_Msk);
    RASR |= ((ap << MPU_RASR_AP_Pos) & MPU_RASR_AP_Msk) | ((para->executable << MPU_RASR_XN_Pos) & MPU_RASR_XN_Msk) |
        ((para->shareability << MPU_RASR_S_Pos) & MPU_RASR_S_Msk);
    HalMpuRASRAddMemAttr(para, &RASR);
    return RASR;
}

UINT32 HalMpuSetRegion(UINT32 regionId, MPU_CFG_PARA *para)
{
    UINT32 RASR;
    UINT32 RBAR;
    UINT32 RNR;
    UINT32 encodeSize;
    UINTPTR intSave;
    UINT64 size;

    if ((regionId >= MPU_MAX_REGION_NUM) || (para == NULL)) {
        return LOS_NOK;
    }

    if ((MPU_TYPE_DREGION_Msk & MPU->TYPE) == 0) {
        return LOS_NOK;
    }

    RNR = regionId;
    encodeSize = HalMpuEncodeSize(para->size);
    if (encodeSize == 0) {
        return LOS_NOK;
    }
    size = para->size - 1;              /* size aligned after encode check */
    if ((para->baseAddr & size) != 0) { /* base addr should aligned to region size */
        return LOS_NOK;
    }
    RBAR = para->baseAddr & MPU_RBAR_ADDR_Msk;
    RASR = HalMpuGetRASR(encodeSize, para);
    intSave = HalIntLock();
    if (g_regionNumBeUsed[regionId]) {
        HalIntRestore(intSave);
        return LOS_NOK;
    }
    MPU->RNR = RNR;
    MPU->RBAR = RBAR;
    MPU->RASR = RASR;
    __DSB();
    __ISB();
    g_regionNumBeUsed[regionId] = 1; /* Set mpu region used flag */
    HalIntRestore(intSave);
    return LOS_OK;
}

UINT32 HalMpuDisableRegion(UINT32 regionId)
{
    volatile UINT32 type;
    UINTPTR intSave;

    if (regionId >= MPU_MAX_REGION_NUM) {
        return LOS_NOK;
    }

    intSave = HalIntLock();
    if (!g_regionNumBeUsed[regionId]) {
        HalIntRestore(intSave);
        return LOS_NOK;
    }

    type = MPU->TYPE;
    if ((MPU_TYPE_DREGION_Msk & type) != 0) {
        MPU->RNR = regionId;
        MPU->RASR = 0;
        __DSB();
        __ISB();
    }
    g_regionNumBeUsed[regionId] = 0; /* clear mpu region used flag */
    HalIntRestore(intSave);
    return LOS_OK;
}

INT32 HalMpuUnusedRegionGet(VOID)
{
    INT32 id;
    UINTPTR intSave = HalIntLock();
    for (id = 0; id < MPU_MAX_REGION_NUM; id++) {
        if (!g_regionNumBeUsed[id]) {
            break;
        }
    }
    HalIntRestore(intSave);

    if (id == MPU_MAX_REGION_NUM) {
        return -1;
    } else {
        return id;
    }
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */
