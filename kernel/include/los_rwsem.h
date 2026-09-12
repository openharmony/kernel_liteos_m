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

/**
 * @defgroup los_rwsem RW Semaphore
 * @ingroup kernel
 */

#ifndef _LOS_RWSEM_H
#define _LOS_RWSEM_H

#include "los_error.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: Invalid parameter.
 *
 * Value: 0x02002200.
 *
 * Solution: Change the passed-in invalid parameter value to a valid value.
 */
#define LOS_ERRNO_RWSEM_INVALID          LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x00)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: Null pointer.
 *
 * Value: 0x02002201.
 *
 * Solution: Change the passed-in null pointer to a valid non-null pointer.
 */
#define LOS_ERRNO_RWSEM_PTR_NULL         LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x01)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: No RW semaphore control structure is available.
 *
 * Value: 0x02002202.
 *
 * Solution: Perform corresponding operations based on the requirements in the code context.
 */
#define LOS_ERRNO_RWSEM_ALL_BUSY         LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x02)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: The API is called during an interrupt, which is forbidden.
 *
 * Value: 0x02002203.
 *
 * Solution: Do not call the API during an interrupt.
 */
#define LOS_ERRNO_RWSEM_PEND_INTERR      LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x03)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: The task is unable to request a semaphore because task scheduling is locked.
 *
 * Value: 0x02002204.
 *
 * Solution: Do not call the API when task scheduling is locked.
 */
#define LOS_ERRNO_RWSEM_PEND_IN_LOCK     LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x04)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: The request for a RW semaphore times out.
 *
 * Value: 0x02002205.
 *
 * Solution: Change the passed-in parameter value to the value within the valid range.
 */
#define LOS_ERRNO_RWSEM_TIMEOUT          LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x05)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: The waiting queue for the RW semaphore is not empty.
 *
 * Value: 0x02002206.
 *
 * Solution: Delete the RW semaphore after awaking all tasks that are waiting on the semaphore.
 */
#define LOS_ERRNO_RWSEM_PENDED           LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x06)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: Invalid status.
 *
 * Value: 0x02002207.
 *
 * Solution: Use the pend and post APIs in pairs.
 */
#define LOS_ERRNO_RWSEM_INVALID_STATUS   LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x07)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: The RW semaphore is not immediately available.
 *
 * Value: 0x02002208.
 *
 * Solution: Obtain the RW semaphore only when the semaphore is available.
 */
#define LOS_ERRNO_RWSEM_UNAVAILABLE      LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x08)

/**
 * @ingroup los_rwsem
 * RW Semaphore error code: The API is called during an exception, which is forbidden.
 *
 * Value: 0x02002209.
 *
 * Solution: Do not call the API during an exception.
 */
#define LOS_ERRNO_RWSEM_PEND_EXCERR      LOS_ERRNO_OS_ERROR(LOS_MOD_RWSEM, 0x09)

/**
 * @ingroup los_rwsem
 * @brief Create a RW semaphore.
 *
 * @par Description:
 * This API is used to create a RW semaphore control structure and return the ID of this semaphore control structure.
 * @attention
 * None.
 *
 * @param semHandle   [OUT] ID of the RW semaphore control structure that is initialized.
 *
 * @retval #LOS_ERRNO_RWSEM_PTR_NULL  The passed-in semHandle value is NULL.
 * @retval #LOS_ERRNO_RWSEM_ALL_BUSY  No RW semaphore control structure is available.
 * @retval #LOS_OK   The RW semaphore is successfully created.
 * @par Dependency:
 * los_rwsem.h: the header file that contains the API declaration.
 * @see LOS_RwsemDelete
 */
extern UINT32 LOS_RwsemCreate(UINT32 *semHandle);

/**
 * @ingroup los_rwsem
 * @brief Delete a RW semaphore.
 *
 * @par Description:
 * This API is used to delete a RW semaphore control structure that has an ID specified by semHandle.
 * @attention
 * The specified rwsem id must be created first before deleting it.
 *
 * @param semHandle   [IN] ID of the RW semaphore control structure to be deleted. The ID of the RW semaphore
 *                         control structure is obtained from semaphore creation.
 *
 * @retval #LOS_ERRNO_RWSEM_INVALID  The passed-in RW semHandle value is invalid.
 * @retval #LOS_ERRNO_RWSEM_PENDED   The waiting queue for the RW semaphore is not empty.
 * @retval #LOS_OK   The RW semaphore control structure is successfully deleted.
 * @par Dependency:
 * los_rwsem.h: the header file that contains the API declaration.
 * @see LOS_RwsemCreate
 */
extern UINT32 LOS_RwsemDelete(UINT32 semHandle);

/**
 * @ingroup los_rwsem
 * @brief Request a RW semaphore.
 *
 * @par Description:
 * This API is used to request a RW semaphore based on the semaphore control structure ID specified by semHandle and the
 * parameter that specifies the timeout period.
 * @attention
 * Do not pend rwsem during an interrupt.
 * Do not pend rwsem in a system task, such as idle or swtmr task.
 * The specified rwsem id must be created first.
 * Do not recommend to use this API in software timer callback.
 *
 * @param semHandle [IN] ID of the RW semaphore control structure to be requested. The ID of the RW semaphore control
 *                       structure is obtained from semaphore creation.
 * @param timeout   [IN] Timeout interval for waiting on the RW semaphore. The value range is [0, 0xFFFFFFFF].
 *                       If the value is set to 0, the RW semaphore is not waited on. If the value is set to 0xFFFFFFFF,
 *                       the RW semaphore is waited on forever(unit: Tick).
 *
 * @retval #LOS_ERRNO_RWSEM_INVALID             The passed-in semHandle value is invalid.
 * @retval #LOS_ERRNO_RWSEM_PEND_INTERR         The API is called during an interrupt, which is forbidden.
 * @retval #LOS_ERRNO_RWSEM_PEND_IN_LOCK        The task is unable to request a RW semaphore because task scheduling
 *                                              is locked.
 * @retval #LOS_ERRNO_RWSEM_TIMEOUT             The request for the RW semaphore times out.
 * @retval #LOS_OK   The RW semaphore request succeeds.
 * @par Dependency:
 * los_rwsem.h: the header file that contains the API declaration.
 * @see LOS_RwsemPostRead | LOS_RwsemCreate
 */
extern UINT32 LOS_RwsemPendRead(UINT32 semHandle, UINT32 timeout);

/**
 * @ingroup los_rwsem
 * @brief Release a RW semaphore.
 *
 * @par Description:
 * This API is used to release a RW semaphore that has a semaphore control structure ID specified by semHandle.
 * @attention
 * The specified rwsem id must be created first.
 *
 * @param semHandle   [IN] ID of the RW semaphore control structure to be released. The ID of the RW semaphore control
 *                         structure is obtained from semaphore creation.
 *
 * @retval #LOS_ERRNO_RWSEM_INVALID         The passed-in semHandle value is invalid.
 * @retval #LOS_ERRNO_RWSEM_INVALID_STATUS  Invalid status.
 * @retval #LOS_OK                          The RW semaphore is successfully released.
 * @par Dependency:
 * los_rwsem.h: the header file that contains the API declaration.
 * @see LOS_RwsemPendRead | LOS_RwsemCreate
 */
extern UINT32 LOS_RwsemPostRead(UINT32 semHandle);

/**
 * @ingroup los_rwsem
 * @brief Request a RW semaphore.
 *
 * @par Description:
 * This API is used to request a RW semaphore based on the semaphore control structure ID specified by semHandle and the
 * parameter that specifies the timeout period.
 * @attention
 * Do not pend rwsem during an interrupt.
 * Do not pend rwsem in a system task, such as idle or swtmr task.
 * The specified rwsem id must be created first.
 * Do not recommend to use this API in software timer callback.
 *
 * @param semHandle [IN] ID of the RW semaphore control structure to be requested. The ID of the RW semaphore control
 *                       structure is obtained from semaphore creation.
 * @param timeout   [IN] Timeout interval for waiting on the RW semaphore. The value range is [0, 0xFFFFFFFF].
 *                       If the value is set to 0, the RW semaphore is not waited on. If the value is set to 0xFFFFFFFF,
 *                       the RWsemaphore is waited on forever(unit: Tick).
 *
 * @retval #LOS_ERRNO_RWSEM_INVALID             The passed-in semHandle value is invalid.
 * @retval #LOS_ERRNO_RWSEM_PEND_INTERR         The API is called during an interrupt, which is forbidden.
 * @retval #LOS_ERRNO_RWSEM_PEND_IN_LOCK        The task is unable to request a RW semaphore because task scheduling
 *                                              is locked.
 * @retval #LOS_ERRNO_RWSEM_TIMEOUT             The request for the semaphore times out.
 * @retval #LOS_OK   The RW semaphore request succeeds.
 * @par Dependency:
 * los_rwsem.h: the header file that contains the API declaration.
 * @see LOS_RwsemPostWrite | LOS_RwsemCreate
 */
extern UINT32 LOS_RwsemPendWrite(UINT32 semHandle, UINT32 timeout);

/**
 * @ingroup los_rwsem
 * @brief Release a RW semaphore.
 *
 * @par Description:
 * This API is used to release a RW semaphore that has a RW semaphore control structure ID specified by semHandle.
 * @attention
 * The specified rwsem id must be created first.
 *
 * @param semHandle   [IN] ID of the RW semaphore control structure to be released. The ID of the RW semaphore control
 *                         structure is obtained from semaphore creation.
 *
 * @retval #LOS_ERRNO_RWSEM_INVALID         The passed-in semHandle value is invalid.
 * @retval #LOS_ERRNO_RWSEM_INVALID_STATUS  Invalid status.
 * @retval #LOS_OK                          The RW semaphore is successfully released.
 * @par Dependency:
 * los_rwsem.h: the header file that contains the API declaration.
 * @see LOS_RwsemPendWrite | LOS_RwsemCreate
 */
extern UINT32 LOS_RwsemPostWrite(UINT32 semHandle);

/**
 * @ingroup los_rwsem
 * @brief Release a RW semaphore.
 *
 * @par Description:
 * This API is used to downgrade the write semaphore to the read semaphore.
 * @attention
 * The specified rwsem id must be created first.
 *
 * @param semHandle   [IN] ID of the RW semaphore control structure to be downgraded. The ID of the RW semaphore control
 *                         structure is obtained from semaphore creation.
 *
 * @retval #LOS_ERRNO_RWSEM_INVALID         The passed-in RW semHandle value is invalid.
 * @retval #LOS_ERRNO_RWSEM_INVALID_STATUS  Invalid status.
 * @retval #LOS_OK                          The RW semaphore is successfully released.
 * @par Dependency:
 * los_rwsem.h: the header file that contains the API declaration.
 * @see LOS_RwsemPendWrite | LOS_RwsemCreate
 */
extern UINT32 LOS_RwsemDowngradeWrite(UINT32 semHandle);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_RWSEM_H */
