/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
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

#include "It_los_lms.h"

char g_testLmsPool[2 * PAGE_SIZE];
CHAR g_lmsSandboxBuf[LMS_TEST_SANDBOX_SIZE];
BOOL g_lmsSandboxFirstCall = TRUE;

STATIC VOID testPoolInit(void)
{
#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
    UINT32 ret = LOS_MemPoolInit(g_testLmsPool, 2 * PAGE_SIZE, 0);
#else
    UINT32 ret = LOS_MemInit(g_testLmsPool, 2 * PAGE_SIZE);
#endif
    if (ret != 0) {
        PRINT_ERR("%s failed, ret = 0x%x\n", __FUNCTION__, ret);
        return;
    }
}

VOID ItSuiteLosLms(void)
{
    testPoolInit();
    ItLosLms001();
    ItLosLms002();
    ItLosLms003();
    ItLosLms004();
    ItLosLms005();
    ItLosLms006();
    ItLosLms007();
    ItLosLms008();
    ItLosLms009();
    ItLosLms010();
    ItLosLms011();
#if !(defined(LOSCFG_KERNEL_MEM_BESTFIT) && defined(LOSCFG_MEM_DEBUG))
    ItLosLms012();
#endif
    ItLosLms013();
    ItLosLms014();
    ItLosLms015();
    ItLosLms016();
    /* ItLosLms017(); */
    ItLosLms018();
    ItLosLms019();
    ItLosLms020();
    ItLosLms021();
    ItLosLms022();
    // ItLosLms023();
    // ItLosLms024();
    ItLosLms025();
    ItLosLms026();
    ItLosLms027();
#if !(defined(LOSCFG_KERNEL_MEM_BESTFIT) && defined(LOSCFG_MEM_DEBUG))
    ItLosLms028();
#endif
}
