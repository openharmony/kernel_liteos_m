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

#ifndef _LOS_LMK_H
#define _LOS_LMK_H

#include "los_config.h"
#include "los_compiler.h"
#include "los_list.h"
#include "los_error.h"

typedef UINT32 (*FreeMemByKillingTask)(VOID);
typedef UINT32 (*RestoreKilledTask)(VOID);

/**
 * @ingroup los_lmk
 * Lmk error code: Invalid parameter.
 *
 * Value: 0x02002101
 *
 */
#define LOS_ERRNO_LMK_INVALID_PARAMETER        LOS_ERRNO_OS_ERROR(LOS_MOD_LMK, 0x01)

/**
 * @ingroup los_lmk
 * Lmk error code: LosLmkOpsNode already registered.
 *
 * Value: 0x02002102
 *
 */
#define LOS_ERRNO_LMK_ALREADY_REGISTERED        LOS_ERRNO_OS_ERROR(LOS_MOD_LMK, 0x02)

/**
 * @ingroup los_lmk
 * Lmk error code: LosLmkOpsNode not yet registered.
 *
 * Value: 0x02002103
 *
 */
#define LOS_ERRNO_LMK_NOT_REGISTERED            LOS_ERRNO_OS_ERROR(LOS_MOD_LMK, 0x03)

/**
 * @ingroup los_lmk
 * Lmk error code: Failed to free memory by invoking the registered functions.
 *
 * Value: 0x02002104
 *
 */
#define LOS_ERRNO_LMK_FREE_MEMORY_FAILURE       LOS_ERRNO_OS_ERROR(LOS_MOD_LMK, 0x04)

/**
 * @ingroup los_lmk
 * Lmk error code: The registered free memory functions have been invoked.
 *
 * Value: 0x02002105
 *
 */
#define LOS_ERRNO_LMK_MEMORY_ALREADY_FREED     LOS_ERRNO_OS_ERROR(LOS_MOD_LMK, 0x05)

/**
 * @ingroup los_lmk
 * Lmk error code: Failed to restore the killed tasks by invoking the registered functions.
 *
 * Value: 0x02002106
 *
 */
#define LOS_ERRNO_LMK_RESTORE_TASKS_FAILURE     LOS_ERRNO_OS_ERROR(LOS_MOD_LMK, 0x06)

/**
 * @ingroup los_lmk
 * Lmk error code: No need to restore when no free memory functions have been invoked.
 *
 * Value: 0x02002107
 *
 */
#define LOS_ERRNO_LMK_RESTORE_NOT_NEEDED        LOS_ERRNO_OS_ERROR(LOS_MOD_LMK, 0x07)

typedef struct {
    UINT32 priority;             /**< The priority in the LMK list, the higher priority with a smaller number. */
    UINT32 (*freeMem)(VOID);     /**< Release the memory of tasks in the LMK list. Return LOS_OK for a successful release. */
    UINT32 (*restoreTask)(VOID); /**< Restore the tasks killed by freeMem(). Return LOS_OK for a successful restore. */
    LOS_DL_LIST node;            /**< LosLmkOpsNode node.  */
} LosLmkOpsNode;

typedef struct {
    LOS_DL_LIST lmkOpsList;     /**< The registered LosLmkOpsNode will be inserted in this list. */
    BOOL isMemFreed;            /**< Flag that if LOS_LmkTasksKill has been invoked. */
} LosLmkOps;

/**
 * @ingroup los_lmk
 * @brief Register a low memory killer node.
 *
 * @par Description:
 * This API is used to register a low memory killer node. A LosLmkOpsNode node
 * can be registered only once.
 *
 * @attention None.
 *
 * @param  lmkNode [IN] The LosLmkOpsNode node to be registered.
 *
 * @retval LOS_OK The LosLmkOpsNode node is registered successfully.
 * @retval LOS_ERRNO_LMK_INVALID_PARAMETER  The parameter is invalid.
 * @retval LOS_ERRNO_LMK_ALREADY_REGISTERED The LosLmkOpsNode node already registered.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
UINT32 LOS_LmkOpsNodeRegister(LosLmkOpsNode *lmkNode);

/**
 * @ingroup los_lmk
 * @brief Unregister a low memory killer node.
 *
 * @par Description:
 * This API is used to unregister a low memory killer node.
 *
 * @attention None.
 *
 * @param  lmkNode [IN] The LosLmkOpsNode node to be registered.
 *
 * @retval LOS_OK The LosLmkOpsNode node is unregistered successfully.
 * @retval LOS_ERRNO_LMK_NOT_REGISTERED The LosLmkOpsNode node is not yet registered.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
UINT32 LOS_LmkOpsNodeUnregister(LosLmkOpsNode *lmkNode);

/**
 * @ingroup los_lmk
 * @brief Initialize low memory killer framework.
 *
 * @par Description:
 * This API is used to initialize the low memory killer framework.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
VOID OsLmkInit(VOID);

/**
 * @ingroup los_lmk
 * @brief Restore the tasks killed by the task which triggers low memory killer.
 *
 * @par Description:
 * This API is used to restore the tasks killed by the task which triggers low memory killer.
 * This function will be invoked by the developer as needed.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval LOS_OK  All the restore killed tasks functions are invoked successfully.
 * @retval LOS_ERRNO_LMK_RESTORE_NOT_NEEDED No need to restore since no tasks killed to free memory.
 * @retval LOS_ERRNO_LMK_RESTORE_TASKS_FAILURE Failed to restore the killed tasks by invoking the registered functions.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
UINT32 LOS_LmkTasksRestore(VOID);

/**
 * @ingroup los_lmk
 * @brief Kill the tasks to release the used memory.
 *
 * @par Description:
 *  This API is used to kill the tasks to release the used memory when low memory killer is triggered. 
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval LOS_OK All the free memory functions are invoked successfully.
 * @retval LOS_ERRNO_LMK_MEMORY_ALREADY_FREED The registered free memory functions have been invoked.
 * @retval LOS_ERRNO_LMK_FREE_MEMORY_FAILURE Failed to free memory by invoking the registered functions.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
UINT32 LOS_LmkTasksKill(VOID);

#if (LOSCFG_KERNEL_LMK_DEBUG == 1)
/**
 * @ingroup los_lmk
 * @brief Output the low memory killer node priorities.
 *
 * @par Description:
 *  This API is used to output the low memory killer node priorities.
 *
 * @attention None.
 *
 * @param  None.
 *
 * @retval None.
 * @par Dependency:
 * <ul><li>los_lmk.h: the header file that contains the API declaration.</li></ul>
 * @see
 */
VOID LOS_LmkOpsNodeInfoShow(VOID);
#endif
#endif
