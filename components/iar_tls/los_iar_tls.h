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

#include "los_compiler.h"

/* Using task extended field to realize IAR TLS function in target_config.h include this header file. */
#define LOSCFG_TASK_STRUCT_EXTENSION    VOID *iarTlsArea;

#if (__VER__ < 8000000)
#include <yvals.h>

#define LOSCFG_TASK_CREATE_EXTENSION_HOOK(taskCB)

#define LOSCFG_TASK_DELETE_EXTENSION_HOOK(taskCB)   do {     \
    if (taskCB->iarTlsArea != NULL) {                        \
        __iar_dlib_perthread_deallocate(taskCB->iarTlsArea); \
        taskCB->iarTlsArea = NULL;                           \
    }                                                        \
} while (0);
#else
#include <Dlib_threads.h>

void *IarPerThreadTlsAreaAllocate(void);
void IarPerThreadTlsAreaDeallocate(void *tlsArea);

#define LOSCFG_TASK_CREATE_EXTENSION_HOOK(taskCB)

#define LOSCFG_TASK_DELETE_EXTENSION_HOOK(taskCB)  do {    \
    if (taskCB->iarTlsArea != NULL) {                      \
        IarPerThreadTlsAreaDeallocate(taskCB->iarTlsArea); \
        taskCB->iarTlsArea = NULL;                         \
    }                                                      \
} while (0);
#endif

/* In the main initialization phase, the macro should be called. */
#define IAR_INIT_LOCKS()  __iar_Initlocks()
