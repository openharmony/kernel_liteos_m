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

#include "los_secure_context.h"
#include "los_secure_macros.h"
#include "los_secure_heap.h"

OS_CMSE_NS_ENTRY VOID HalSecureContextInit(VOID)
{
    UINT32 ipsr;

    OS_IPSR_READ(ipsr);
    if (!ipsr) {
        return;
    }

    HalSecureContextInitAsm();
}

OS_CMSE_NS_ENTRY OsSecureContext *HalSecureContextAlloc(UINT32 size)
{
    OsSecureContext *secureContext = NULL;
    UINT32 ipsr;

    OS_IPSR_READ(ipsr);
    if (!ipsr) {
        return NULL;
    }

    secureContext = HalSecureMalloc(sizeof(OsSecureContext));
    if (secureContext == NULL) {
        return NULL;
    }

    secureContext->stackLimit = HalSecureMalloc(size);
    if (secureContext->stackLimit == NULL) {
        HalSecureFree(secureContext);
        return NULL;
    }

    secureContext->stackStart = secureContext->stackLimit + size;
    secureContext->curStackPointer = secureContext->stackStart;

    return secureContext;
}

OS_CMSE_NS_ENTRY VOID HalSecureContextFree(OsSecureContext *secureContext)
{
    UINT32 ipsr;

    OS_IPSR_READ(ipsr);
    if (!ipsr) {
        return;
    }

    HalSecureFree(secureContext->stackLimit);
    secureContext->stackLimit = NULL;
    HalSecureFree(secureContext);
}

OS_CMSE_NS_ENTRY VOID HalSecureContextLoad(OsSecureContext *secureContext)
{
    HalSecureContextLoadAsm(secureContext);
}

OS_CMSE_NS_ENTRY VOID HalSecureContextSave(OsSecureContext *secureContext)
{
    HalSecureContextSaveAsm(secureContext);
}

