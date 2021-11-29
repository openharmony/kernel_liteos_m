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

#include "osTest.h"
#include "It_los_lms.h"

static UINT32 TestCase(VOID)
{
    UINT32 size = 20; /* mem size 20 */
    CHAR *backStr;

    CHAR *str = (CHAR *)LOS_MemAlloc(g_testLmsPool, size);
    ICUNIT_ASSERT_NOT_EQUAL(str, NULL, str);
    PRINT_DEBUG("str = 0x%x,%d\n", str, __LINE__);

    CHAR *newPtr = LOS_MemRealloc(g_testLmsPool, str, 0x8+0x2); /* reaSize 0x2 is not 4Align */
    ICUNIT_GOTO_NOT_EQUAL(newPtr, NULL, newPtr, EXIT);
    PRINT_DEBUG("newPtr = 0x%x,%d\n", newPtr, __LINE__);

    PRINTK("0x%x\n", newPtr[0x8+0x3]); /* not trigger overflow at newPtr[0x3] */

    PRINTK("Trigger write overflow\n");
    newPtr[0x8+0x4] = 0x01; /* write 0x1,trigger overflow at newPtr[0x4] */

    PRINTK("Trigger read overflow\n");
    PRINTK("0x%x\n", newPtr[0x8+0x4]); /* trigger read overflow at newPtr[0x4] */

    backStr = (CHAR *)(newPtr - 1); /* Add offset -1 */
    PRINTK("Trigger back offset write overflow\n");
    *backStr = 0x01;

    PRINTK("Trigger back offset read overflow\n");
    PRINTK("0x%x\n", *backStr); /* trigger overflow */

    PRINTK("Trigger read overflow\n"); /* trigger read overflow */
    PRINTK("0x%x\n", newPtr[0x8+0x5]); /* trigger overflow at newPtr[0x5] */

    (VOID)LOS_MemFree(g_testLmsPool, newPtr);

    return LOS_OK;
EXIT:
    (VOID)LOS_MemFree(g_testLmsPool, str);
    return LOS_OK;
}

/* LmsTestReallocTest6 */
VOID ItLosLms010(void)
{
    TEST_ADD_CASE("ItLosLms010", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}

