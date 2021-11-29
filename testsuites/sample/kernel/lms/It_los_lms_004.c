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
#define PTR_NUM 100
#define SIZE_NUM 2
    CHAR *str[PTR_NUM] = {NULL};
    UINT32 size[SIZE_NUM] = {20, 10}; /* mem size 20, 10 */
    UINT32 index;
    UINT32 i, j, k;
    UINT32 boundary;
    for (k = 0; k < SIZE_NUM; k++) {
        index = 0;
        for (j = 0; j < 10; j++) {             /* loop 10 times each size */
            for (i = 2; i < 8; i++, index++) { /* boundary loop from 2 to 8 */
                boundary = 1 << i;
                PRINT_DEBUG("size = %d, boundary = %d\n", size[k], boundary);
                str[index] = (char *)LOS_MemAllocAlign(m_aucSysMem0, size[k], boundary);
                ICUNIT_GOTO_NOT_EQUAL(str[index], NULL, str[index], EXIT);
            }
        }
        for (i = 0; i < index; i++) {
            (VOID)LOS_MemFree(m_aucSysMem0, str[i]);
        }
    }

    return LOS_OK;
EXIT:
    for (i = 0; i < index; i++) {
        (VOID)LOS_MemFree(m_aucSysMem0, str[i]);
    }
    return LOS_OK;
}

/* LmsTestMemAlignFree */
VOID ItLosLms004(void)
{
    TEST_ADD_CASE("ItLosLms004", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}

