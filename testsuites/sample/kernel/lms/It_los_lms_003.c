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
    UINT32 boundary;
    CHAR *str = NULL;
    UINT32 i;
    UINT32 size = 0x8 + 0x2;        /* mem size 0x8 + 0x2 */
    for (i = 2; i < 8; i++) { /* boundary loop from 2 to 8 */
        boundary = 1 << i;
        str = (CHAR *)LOS_MemAllocAlign(m_aucSysMem0, size, boundary);
        ICUNIT_ASSERT_NOT_EQUAL(str, NULL, str);
        PRINTK("str = 0x%x, boundary = %d\n", str, boundary);
        PRINTK("0x%x\n", str[size + 1]); /* not trigger read overflow at size + 1 */
        PRINTK("0x%x\n", str[size + 2]); /* trigger read overflow at size + 2 */
        (VOID)LOS_MemFree(m_aucSysMem0, str);
    }

    return LOS_OK;
}

/* LmsTestMemAlignOverflow_Not4Align */
VOID ItLosLms003(void)
{
    TEST_ADD_CASE("ItLosLms003", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}

