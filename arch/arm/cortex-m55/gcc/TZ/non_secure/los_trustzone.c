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

#include "los_trustzone.h"
#include "los_secure_macros.h"
#include "los_secure_context.h"
#include "los_debug.h"
#include "los_arch_context.h"
#include "los_interrupt.h"

OsSecureContext *g_secureContext = NULL;

VOID HalSecureSVCHandler(UINT32 svcID, UINTPTR arg)
{
    switch (svcID) {
        case OS_SVC_START_SCHEDULE:
            HalSecureContextInit();
            HalStartFirstTask();
            break;
        case OS_SVC_ALLOCATE_SECURE_CONTEXT:
            g_secureContext = HalSecureContextAlloc(arg);
            LOS_ASSERT(g_secureContext != NULL);
            HalSecureContextLoad(g_secureContext);
            break;
        case OS_SVC_FREE_SECURE_CONTEXT:
            HalSecureContextFree(g_secureContext);
            break;
        default:
            PRINT_ERR("Incorrect svc id = %u\n", svcID);
            break;
    }
}

VOID HalStartToRun(VOID)
{
    HalSVCStartSchedule();
}

VOID LOS_SecureContextAlloc(UINT32 secureStackSize)
{
    if (secureStackSize == 0) {
        return;
    }

    LOS_ASSERT((__get_IPSR() == 0) && (__get_PRIMASK() == 0));
    secureStackSize = LOS_Align(secureStackSize, sizeof(UINTPTR));
    HalSVCSecureContextAlloc(secureStackSize);
}

VOID LOS_SecureContextFree(VOID)
{
    HalSVCSecureContextFree();
}

