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
    CHAR *src;
    CHAR *buf;

    src = LOS_MemAlloc(m_aucSysMem0, 9);
    ICUNIT_ASSERT_NOT_EQUAL(src, NULL, src);

    (void)memset_s(src, 9, 0, 9);
    src[0] = 49;
    src[1] = 50;
    src[2] = 51;
    src[3] = 52;
    src[4] = 53;
    src[5] = 54;
    src[6] = 55;
    src[7] = 56;
    PRINTK("strlen(src) = %d\n", strlen(src));

    buf = LOS_MemAlloc(m_aucSysMem0, 7);
    ICUNIT_ASSERT_NOT_EQUAL(buf, NULL, buf);
    buf[0] = 0;
    strncat(buf, src, 8); /* trigger buf overflow */
    buf[0] = 0;
    strncat(buf, src, 9); /* trigger buf overflow */
    buf[0] = 0;
    strncat(buf, src, 20); /* trigger buf overflow */
    buf[0] = 0;
    strncat(buf, src, 21); /* trigger buf overflow */
    PRINTK("\n######%s stop ######\n", __FUNCTION__);

    return LOS_OK;
}

/* LmsTestStrncatOverflow */
VOID ItLosLms018(void)
{
    TEST_ADD_CASE("ItLosLms018", TestCase, TEST_LOS, TEST_LMS, TEST_LEVEL1, TEST_FUNCTION);
}

