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

#include "los_iar_tls.h"
#include <stdio.h>
#include "los_task.h"
#include "los_mux.h"
#include "los_memory.h"

/* IAR version is less than 8. */
#if (__VER__ < 8000000)
void __DLIB_TLS_MEMORY *__iar_dlib_perthread_access(void _DLIB_TLS_MEMORY *symbp)
{
    if (!LOS_TaskIsRuning()) {
        CHAR _DLIB_TLS_MEMORY *tlsAreaPtr = (char _DLIB_TLS_MEMORY *)__segment_begin("__DLIB_PERTHREAD");
        tlsAreaPtr += __IAR_DLIB_PERTHREAD_SYMBOL_OFFSET(symbp);
        return (void __DLIB_TLS_MEMORY *)tlsAreaPtr;
    } else {
        UINT32 taskID = LOS_CurTaskIDGet();
        LosTaskCB *task = OS_TCB_FROM_TID(taskID);
        if (task->iarTlsArea == NULL) {
            task->iarTlsArea = __iar_dlib_perthread_allocate();
        }
        return (void __DLIB_TLS_MEMORY *)task->iarTlsArea;
    }
}

#else /* IAR version 8 or above. */
#pragma section="__iar_tls$$DATA"
void *__aeabi_read_tp(void)
{
    if (!LOS_TaskIsRunning()) {
        return __section_begin("__iar_tls$$DATA");
    } else {
        UINT32 taskID = LOS_CurTaskIDGet();
        LosTaskCB *task = OS_TCB_FROM_TID(taskID);
        if (task->iarTlsArea == NULL) {
            task->iarTlsArea = IarPerThreadTlsAreaAllocate();
        }
        return task->iarTlsArea;
    }
}

void *IarPerThreadTlsAreaAllocate(void)
{
    UINT32 tlsAreaSize = __iar_tls_size();
    VOID *tlsAreaPtr = LOS_MemAlloc(m_aucSysMem0, tlsAreaSize);
    if (tlsAreaPtr == NULL) {
        return NULL;
    }
    
    __iar_tls_init(tlsAreaPtr);
    return tlsAreaPtr;
}

void IarPerThreadTlsAreaDeallocate(void *tlsArea)
{
    __call_thread_dtors();
    LOS_MemFree(m_aucSysMem0, tlsArea);
}
#endif

#ifndef _MAX_LOCK
#define _MAX_LOCK       4
#endif

#ifndef _MAX_FLOCK
#define _MAX_FLOCK      FOPEN_MAX
#endif

struct IarMutexInfo {
    UINT32 muxID;
    BOOL usedFlag;
};

STATIC struct IarMutexInfo g_iarSysMutex[_MAX_LOCK] = {0};
STATIC struct IarMutexInfo g_iarFileMutex[_MAX_FLOCK] = {0};

STATIC __iar_Rmtx IarMtxCreate(struct IarMutexInfo *mutexArray, UINT32 size)
{
    UINT32 i;
    
    for (i = 0; i < size; i++) {
        if (mutexArray[i].usedFlag == FALSE) {
            UINT32 ret = LOS_MuxCreate(&mutexArray[i].muxID);
            if (ret == LOS_OK) {
                mutexArray[i].usedFlag = TRUE;
                return (__iar_Rmtx)&mutexArray[i];
            } else {
                break;
            }
        }
    }
    
    return NULL;
}

void __iar_system_Mtxinit(__iar_Rmtx *m)
{
    if (m == NULL) {
        return;
    }

    *m = IarMtxCreate(g_iarSysMutex, _MAX_LOCK);
}

void __iar_system_Mtxdst(__iar_Rmtx *m)
{
    if (m == NULL) {
        return;
    }
    struct IarMutexInfo *muxInfo = (struct IarMutexInfo *)*m;
    
    (void)LOS_MuxDelete(muxInfo->muxID);
    muxInfo->usedFlag = FALSE;
    *m = (__iar_Rmtx)NULL;
}

void __iar_system_Mtxlock(__iar_Rmtx *m)
{
    if (m == NULL) {
        return;
    }
    struct IarMutexInfo *muxInfo = (struct IarMutexInfo *)*m;
    
    if (LOS_TaskIsRunning()) {
        (void)LOS_MuxPend(muxInfo->muxID, LOS_WAIT_FOREVER);
    }
}

void __iar_system_Mtxunlock(__iar_Rmtx *m)
{
    if (m == NULL) {
        return;
    }
    struct IarMutexInfo *muxInfo = (struct IarMutexInfo *)*m;
    
    if (LOS_TaskIsRunning()) {
        (void)LOS_MuxPost(muxInfo->muxID);
    }
}

void __iar_file_Mtxinit(__iar_Rmtx *m)
{
    if (m == NULL) {
        return;
    }

    *m = IarMtxCreate(g_iarFileMutex, _MAX_FLOCK);
}

void __iar_file_Mtxdst(__iar_Rmtx *m)
{
    if (m == NULL) {
        return;
    }
    struct IarMutexInfo *muxInfo = (struct IarMutexInfo *)*m;
    
    (void)LOS_MuxDelete(muxInfo->muxID);
    muxInfo->usedFlag = FALSE;
    *m = (__iar_Rmtx)NULL;
}

void __iar_file_Mtxlock(__iar_Rmtx *m)
{
    if (m == NULL) {
        return;
    }
    struct IarMutexInfo *muxInfo = (struct IarMutexInfo *)*m;
    
    if (LOS_TaskIsRunning()) {
        (void)LOS_MuxPend(muxInfo->muxID, LOS_WAIT_FOREVER);
    }
}

void __iar_file_Mtxunlock(__iar_Rmtx *m)
{
    if (m == NULL) {
        return;
    }
    struct IarMutexInfo *muxInfo = (struct IarMutexInfo *)*m;
    
    if (LOS_TaskIsRunning()) {
        (void)LOS_MuxPost(muxInfo->muxID);
    }
}