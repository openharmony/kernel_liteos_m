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
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of
 * conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#include "osTest.h"
#include "it_los_hwi.h"

static VOID HwiF047(VOID)
{
    TestHwiClear(HWI_NUM_TEST);
}

/* 用例简要描述: LOS_HwiCreate NULL handler/越界号/重复创建错误码+删后重建 */
static UINT32 Testcase(VOID)
{
    HWI_PRIOR_T hwiPrio = 3;
    HWI_MODE_T mode = 0;
    HwiIrqParam irqParam;
    UINT32 ret;
    UINT32 created = 0; /* 已注册中断标记,EXIT 统一注销 */

    /* F-Inv: NULL 处理函数 -> LOS_ERRNO_HWI_PROC_FUNC_NULL(内核级守卫,los_hwi.c) */
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, NULL, NULL);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_HWI_PROC_FUNC_NULL, ret, EXIT);

    /* F-Inv: 越界中断号 -> LOS_ERRNO_HWI_NUM_INVALID(两架构 ArchHwiCreate 守卫:
       arm/riscv 均 hwiNum >= OS_HWI_MAX_NUM 早返) */
    ret = LOS_HwiCreate(0xFFFF, hwiPrio, mode, (HWI_PROC_FUNC)HwiF047, NULL);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_HWI_NUM_INVALID, ret, EXIT);

    (VOID)memset_s(&irqParam, sizeof(HwiIrqParam), 0, sizeof(HwiIrqParam));
    irqParam.pDevId = 0;

    /* F-Pos: 合法创建 */
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF047, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    created = 1;

    /* F-Inv: 同号重复创建 -> LOS_ERRNO_HWI_ALREADY_CREATED(两架构均断言既有钩子) */
    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF047, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_HWI_ALREADY_CREATED, ret, EXIT);

    /* F-Pos: 删除后重建成功(资源回收闭环;重复 Delete 两架构语义不同不测) */
    ret = LOS_HwiDelete(HWI_NUM_TEST, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    created = 0;

    ret = LOS_HwiCreate(HWI_NUM_TEST, hwiPrio, mode, (HWI_PROC_FUNC)HwiF047, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    created = 1;

    ret = LOS_HwiDelete(HWI_NUM_TEST, &irqParam);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    created = 0;

    return LOS_OK;

EXIT:
    if (created) { (VOID)LOS_HwiDelete(HWI_NUM_TEST, &irqParam); }
    return LOS_OK;
}

VOID ItLosHwi047(VOID)
{
    TEST_ADD_CASE("ItLosHwi047", Testcase, TEST_LOS, TEST_HWI, TEST_LEVEL0, TEST_FUNCTION);
}
