/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "osTest.h"
#include "It_los_lmk.h"
#include "los_list.h"

STATIC UINT32 release_OK(VOID)
{
    return LOS_OK;
}

STATIC UINT32 release_NOK(VOID)
{
    return LOS_NOK;
}

STATIC UINT32 restore_OK(VOID)
{
    return LOS_OK;
}

STATIC UINT32 restore_NOK(VOID)
{
    return LOS_NOK;
}

STATIC UINT32 TestCase(VOID)
{
    UINT32 ret;
    LosLmkOpsNode firstOpsNode = {
        .priority = LMK_PRIORITY_LOW,
        .freeMem = release_NOK,
        .restoreTask = restore_NOK,
    };

    LosLmkOpsNode anotherOpsNode = {
        .priority = LMK_PRIORITY_MEDIUM,
        .freeMem = release_NOK,
        .restoreTask = restore_NOK,
    };

    LosLmkOpsNode thirdOpsNode = {
        .priority = LMK_PRIORITY_HIGH,
        .freeMem = release_OK,
        .restoreTask = restore_OK,
    };

    ret = LOS_LmkOpsNodeRegister(&firstOpsNode);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_LmkOpsNodeRegister(&anotherOpsNode);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    ret = LOS_LmkTasksKill();
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_LMK_FREE_MEMORY_FAILURE, ret);
    ret = LOS_LmkTasksRestore();
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_LMK_RESTORE_TASKS_FAILURE, ret);

    ret = LOS_LmkOpsNodeRegister(&thirdOpsNode);
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_LmkTasksKill();
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_LMK_FREE_MEMORY_FAILURE, ret);
    ret = LOS_LmkTasksKill();
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_LMK_MEMORY_ALREADY_FREED, ret);
    ret = LOS_LmkTasksRestore();
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_LMK_RESTORE_TASKS_FAILURE, ret);
    ret = LOS_LmkTasksRestore();
    ICUNIT_ASSERT_EQUAL(ret, LOS_ERRNO_LMK_RESTORE_NOT_NEEDED, ret);

    firstOpsNode.freeMem = release_OK;
    firstOpsNode.restoreTask =restore_OK;
    anotherOpsNode.freeMem = release_OK;
    anotherOpsNode.restoreTask =restore_OK;
    ret = LOS_LmkTasksKill();
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);
    ret = LOS_LmkTasksRestore();
    ICUNIT_ASSERT_EQUAL(ret, LOS_OK, ret);

    (VOID)LOS_LmkOpsNodeUnregister(&firstOpsNode);
    (VOID)LOS_LmkOpsNodeUnregister(&anotherOpsNode);
    (VOID)LOS_LmkOpsNodeUnregister(&thirdOpsNode);

    return LOS_OK;
}

VOID ItLosLmk003(VOID)
{
    TEST_ADD_CASE("ItLosLmk003", TestCase, TEST_LOS, TEST_COMP, TEST_LEVEL0, TEST_FUNCTION);
}
