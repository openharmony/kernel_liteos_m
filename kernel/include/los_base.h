/*
 * Copyright (c) 2013-2023 Huawei Technologies Co., Ltd. All rights reserved.
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

/**
 * @defgroup kernel Kernel
 * @defgroup los_base Basic definitions
 * @ingroup kernel
 */

#ifndef _LOS_BASE_H
#define _LOS_BASE_H

#include "los_typedef.h"
#include "los_compiler.h"
#include "los_config.h"
#include "los_debug.h"
#include "los_list.h"
#include "los_error.h"
#include "los_hw.h"
#include "los_tick.h"
#include "los_io.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_base
 * Define the timeout interval as LOS_NO_WAIT.
 */
#define LOS_NO_WAIT                                 0

/**
 * @ingroup los_base
 * Define the timeout interval as LOS_WAIT_FOREVER.
 */
#define LOS_WAIT_FOREVER                            0xFFFFFFFF

/**
 * @ingroup los_base
 * @brief Sleep the current task.
 *
 * @par Description:
 * This API is used to delay the execution of the current task. The task is able to be scheduled after it is delayed
 * for a specified milliseconds.
 *
 * @attention
 * <ul>
 * <li>The task fails to be delayed if it is being delayed during interrupt processing or it is locked.</li>
 * <li>If 0 is passed in and the task scheduling is not locked, execute the next task in the queue of tasks with the
 * priority of the current task.
 * If no ready task with the priority of the current task is available, the task scheduling will not occur, and the
 * current task continues to be executed.</li>
 * <li>The parameter passed in can not be equal to LOS_WAIT_FOREVER(0xFFFFFFFF).
 * If that happens, the task will not sleep 0xFFFFFFFF milliseconds or sleep forever but sleep 0xFFFFFFFF Ticks.</li>
 * <li>This API is not an accurate delay function. The actual delay time is longer than that of parameter msecs.</li>
 * </ul>
 *
 * @param msecs [IN] Type #UINT32 Milliseconds for which the task is delayed.
 *
 * @retval None
 * @par Dependency:
 * <ul><li>los_base.h: the header file that contains the API declaration.</li></ul>
 * @see LOS_Mdelay | LOS_TaskDelay
 */
extern VOID LOS_Msleep(UINT32 msecs);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_BASE_H */
