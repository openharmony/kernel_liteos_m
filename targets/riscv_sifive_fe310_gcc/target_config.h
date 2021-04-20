/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
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

#ifndef _TARGETS_CONFIG_H
#define _TARGETS_CONFIG_H

#include "soc.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define OS_SYS_CLOCK                                  10000000UL

#define LOSCFG_BASE_CORE_TICK_PER_SECOND              1000

/****************************** System clock module configuration ****************************/
#define LOSCFG_BASE_CORE_TIMER_NUM                      7

/****************************** Task module configuration ********************************/
#define LOSCFG_BASE_CORE_TSK_LIMIT                      20            // max num task
#define LOSCFG_BASE_CORE_TSK_IDLE_STACK_SIZE            (0x500U)      // IDLE task stack
#define LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE         (0x2D0U)      // default stack
#define LOSCFG_BASE_CORE_TSK_MIN_STACK_SIZE             (0x130U)
#define LOSCFG_BASE_CORE_TIMESLICE                      1             // task-ROBIN moduel cutting switch
#define LOSCFG_BASE_CORE_TIMESLICE_TIMEOUT              10
/****************************** Semaphore module configuration ******************************/
#define LOSCFG_BASE_IPC_SEM                             1
#define LOSCFG_BASE_IPC_SEM_LIMIT                       10              // the max sem-numb
/****************************** mutex module configuration ******************************/
#define LOSCFG_BASE_IPC_MUX                             1
#define LOSCFG_BASE_IPC_MUX_LIMIT                       10              // the max mutex-num
/****************************** Queue module configuration ********************************/
#define LOSCFG_BASE_IPC_QUEUE                           1
#define LOSCFG_BASE_IPC_QUEUE_LIMIT                     10              //the max queue-numb

/****************************** Software timer module configuration **************************/
#define LOSCFG_BASE_CORE_SWTMR                          1
#define LOSCFG_BASE_CORE_SWTMR_LIMIT                    10				// the max SWTMR numb

/****************************** Memory module configuration **************************/
#define LOSCFG_MEM_MUL_POOL                             1
#define OS_SYS_MEM_NUM                                  20
/*=============================================================================
                                       Exception module configuration
=============================================================================*/
#define LOSCFG_PLATFORM_EXC                             0
/* =============================================================================
                                       printf module configuration
============================================================================= */
#define LOSCFG_KERNEL_PRINTF                            1

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */


#endif /* _TARGETS_CONFIG_H */
