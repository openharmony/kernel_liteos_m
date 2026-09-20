/*
 * Copyright (c) 2026 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted that the following conditions are met:
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
#include "It_los_rwsem.h"

/* 用例简要描述: LOS_Rwsem* 错误路径一揽子：创建循环至池耗尽→ALL_BUSY；
 * 越界句柄→INVALID；读计数增减与写计数路径；写持锁下读 NO_WAIT→UNAVAILABLE；
 * 写锁降级成功路径；调度锁内 pend→PEND_IN_LOCK；删除后陈旧句柄五操作→INVALID。
 * 只调对外接口。 */

#define TEST_RWSEM_CREATE_LOOP 16 // 16, upper bound of create attempts (> LOSCFG_BASE_IPC_RWSEM_LIMIT).

static UINT32 Testcase(VOID)
{
    UINT32 ret;
    UINT32 semH[TEST_RWSEM_CREATE_LOOP];
    UINT32 created = 0;
    UINT32 i;

    /* F-Inv: 持续创建直至池耗尽 → ALL_BUSY（覆盖空闲链表耗尽分支）。 */
    for (i = 0; i < TEST_RWSEM_CREATE_LOOP; i++) {
        ret = LOS_RwsemCreate(&semH[i]);
        if (ret == LOS_ERRNO_RWSEM_ALL_BUSY) {
            break;
        }
        ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
        created++;
    }
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_ALL_BUSY, ret, EXIT);
    ICUNIT_GOTO_NOT_EQUAL(created, 0, created, EXIT);

    /* F-Inv: 越界句柄（索引恒等比较超限）→ INVALID。 */
    ret = LOS_RwsemPendRead(0xFFFF, LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret, EXIT);
    ret = LOS_RwsemPostRead(0xFFFF);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret, EXIT);

    /* F-Pos: 读计数增减（无竞争下连续读锁/读解锁）。 */
    ret = LOS_RwsemPendRead(semH[0], LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_RwsemPendRead(semH[0], LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_RwsemPostRead(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_RwsemPostRead(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Pos: 写锁获取（写计数路径）。 */
    ret = LOS_RwsemPendWrite(semH[0], LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Inv: 写持锁下读 NO_WAIT → UNAVAILABLE。 */
    ret = LOS_RwsemPendRead(semH[0], LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_UNAVAILABLE, ret, EXIT);

    ret = LOS_RwsemPostWrite(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Pos: 写锁降级为读锁（DowngradeWrite 成功路径）。 */
    ret = LOS_RwsemPendWrite(semH[0], LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_RwsemDowngradeWrite(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_RwsemPostRead(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    /* F-Inv: 调度锁内 pend → PEND_IN_LOCK。 */
    LOS_TaskLock();
    ret = LOS_RwsemPendRead(semH[0], 10); // 10, timeout ticks.
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_PEND_IN_LOCK, ret, EXIT);
    LOS_TaskUnlock();

    /* F-Inv: 删除后陈旧句柄 → 五个操作均 INVALID（状态为 UNUSED）。 */
    ret = LOS_RwsemDelete(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);
    ret = LOS_RwsemPendRead(semH[0], LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret, EXIT);
    ret = LOS_RwsemPendWrite(semH[0], LOS_NO_WAIT);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret, EXIT);
    ret = LOS_RwsemPostRead(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret, EXIT);
    ret = LOS_RwsemPostWrite(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret, EXIT);
    ret = LOS_RwsemDowngradeWrite(semH[0]);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_RWSEM_INVALID, ret, EXIT);

EXIT:
    for (i = 0; i < created; i++) {
        (VOID)LOS_RwsemDelete(semH[i]);
    }
    return LOS_OK;
}

VOID ItLosRwsem016(VOID)
{
    TEST_ADD_CASE("ItLosRwsem016", Testcase, TEST_LOS, TEST_RWSEM, TEST_LEVEL1, TEST_FUNCTION);
}
