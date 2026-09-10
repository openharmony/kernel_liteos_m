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

#include "osTest.h"
#include "It_los_mem.h"


#if (LOSCFG_MEM_LEAKCHECK == 1)

static UINT32 TestCase(VOID)
{
    UINT32 size;
    void *p[2] = {NULL};

    MemStart();

    MemInit();

    /* Allocate some nodes */
    size = 0x80;
    p[0] = LOS_MemAlloc(g_memPool, size);
    ICUNIT_GOTO_NOT_EQUAL(p[0], NULL, 0, EXIT);

    p[1] = LOS_MemAlloc(g_memPool, size);
    ICUNIT_GOTO_NOT_EQUAL(p[1], NULL, 0, EXIT);

    /* Show used nodes - should display call stack info for allocated nodes */
    LOS_MemUsedNodeShow(g_memPool);

    /* Free one node and show again */
    (VOID)LOS_MemFree(g_memPool, p[0]);
    LOS_MemUsedNodeShow(g_memPool);

    /* Free remaining node and show */
    (VOID)LOS_MemFree(g_memPool, p[1]);
    LOS_MemUsedNodeShow(g_memPool);

EXIT:
    MemFree();
    MemEnd();
    return LOS_OK;
}

#endif /* LOSCFG_MEM_LEAKCHECK == 1 */

VOID ItLosMem052(void)
{
#if (LOSCFG_MEM_LEAKCHECK == 1)
    TEST_ADD_CASE("ItLosMem052", TestCase, TEST_LOS, TEST_MEM, TEST_LEVEL1, TEST_FUNCTION);
#endif
}
