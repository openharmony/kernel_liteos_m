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

/*
 * 用例名称: ItPosixTime039
 * 用例描述: DST 起止边界 + 跳变/重复区间 + mktime/localtime 跨边界往返
 *
 * 覆盖目标:
 *   lib/posix/src/tzdst.c:
 *     CheckDstPeriodInner :519-546 全分支
 *       :524-525 tm_isdst<0 且 seconds ∈ [dstStart, dstStart+forward) → FALSE(被跳过区间)
 *       :529-530 tm_isdst<0 且 seconds ∈ (dstEnd, dstEnd+forward] → TRUE(重复区间)
 *       :532-534 tm_isdst>0 → 恒 TRUE(mktime 采信调用者声明)
 *       :541-545 tm == NULL: dstStart <= seconds < dstEnd → TRUE(localtime 判定路径)
 *     CheckWithinDstPeriod :548-595(tm==NULL 走 gmtime 解码 / tm!=NULL 走 mktime 重算)
 *     dst_enable/dst_disable/dst_inquire 配置面(033/035 已测矩阵, 本轮只做必要配置)
 *   lib/posix/src/time.c:
 *     localtime_r :653-676 的 dst 合成分支(:655-657 CheckWithinDstPeriod → dstsec,
 *       :659 local = utc - tzoff + dstsec, :675 tm_isdst = (dstsec != 0))
 *     mktime :770-781 的 dst 合成分支(:773-777 isdst!=0 时查 DST 并清零, :780 减 dstsec)
 *     OsEffectiveTimezone :631-639(:638 走全局 TIMEZONE 分支, RTC hook 未注册)
 *
 * 默认 DST 规则实测结论: tzdst.c 无内置默认起止日(g_strDstStart/g_strDstEnd 初始为
 * 空串, g_isDstWork=FALSE), 规则完全由 dst_enable 注入。本用例注入:
 *   起: "Mar-01 00:00:00"  止: "Oct-01 00:00:00"  前拨: 3600 秒(1 小时)
 * 断言口径(本地标准时面, 由 dst_inquire 回读验证):
 *   - dstStart 本地面 = 当年 3 月 1 日 00:00:00, dstEnd 本地面 = 当年 10 月 1 日 00:00:00
 *   - localtime 判定区间(UTC 秒直接比较): [dstStartUtc, dstEndUtc)
 *     起始前 1 秒(dstStartUtc-1) → tm_isdst==0; 起始瞬间 → 1
 *     结束前 1 秒(dstEndUtc-1) → 1; 结束瞬间/结束后 1 秒 → 0
 *   - 跳变区间: 本地 03-01 00:00:00~00:59:59 不存在(isdst=-1 输入 00:30 → 输出 01:30)
 *   - 重复区间: 本地 10-01 00:00:00~00:59:59 出现两次(isdst=-1 输入 00:30 按 DST 解释)
 *   - mktime(isdst=1) == mktime(isdst=0) - 3600(期内外均如此, :532-534 恒 TRUE)
 *   - 期内往返: mktime(12:00, isdst=1) 经 localtime_r 回读仍为 12:00 且 isdst==1;
 *     其 UTC 秒比 isdst=0 基准少 3600, gmtime 面恰好慢 1 小时(回读一致性)
 *
 * 恢复: 初始状态为 DST disabled(静态初始化 + 033/035/036 用例均自恢复), EXIT 必须
 * dst_disable() 恢复 disabled, 不改全局 timezone。
 */
#include "It_posix_time.h"
#include "tzdst.h"

/* DST 配置: 3 月 1 日 0 点起, 10 月 1 日 0 点止, 前拨 1 小时 */
#define DST_START_STR   "Mar-01 00:00:00"
#define DST_END_STR     "Oct-01 00:00:00"
#define DST_FORWARD_SEC 3600
/* 2026 年(tm_year 口径 126), 与 033/035 同年 */
#define DST_TEST_YEAR_TM    126

static UINT32 Testcase(VOID)
{
    struct tm dstStart = {0};
    struct tm dstEnd = {0};
    struct tm inTm = {0};
    struct tm back = {0};
    struct tm gmtT = {0};
    struct tm gmtBase = {0};
    time_t dstStartUtc;
    time_t dstEndUtc;
    time_t base;
    time_t t;
    time_t timeVal;
    INT32 ret;
    struct tm *tmRet = NULL;

    /* 初始状态: DST disabled(dst_inquire 返回 -1); 失败则先恢复再退出 */
    ret = dst_inquire(DST_TEST_YEAR_TM, &dstStart, &dstEnd);
    ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT);

    /* 注入 DST 规则并回读起止(本地标准时面) */
    ret = dst_enable(DST_START_STR, DST_END_STR, DST_FORWARD_SEC);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ret = dst_inquire(DST_TEST_YEAR_TM, &dstStart, &dstEnd);
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    /* 起止面: 3 月 1 日 00:00:00 / 10 月 1 日 00:00:00 */
    ICUNIT_GOTO_EQUAL(dstStart.tm_mon, 2, dstStart.tm_mon, EXIT);  /* 2: March */
    ICUNIT_GOTO_EQUAL(dstStart.tm_mday, 1, dstStart.tm_mday, EXIT);
    ICUNIT_GOTO_EQUAL(dstStart.tm_hour, 0, dstStart.tm_hour, EXIT);
    ICUNIT_GOTO_EQUAL(dstEnd.tm_mon, 9, dstEnd.tm_mon, EXIT);      /* 9: October */
    ICUNIT_GOTO_EQUAL(dstEnd.tm_mday, 1, dstEnd.tm_mday, EXIT);
    ICUNIT_GOTO_EQUAL(dstEnd.tm_hour, 0, dstEnd.tm_hour, EXIT);

    /* 起止的 UTC 秒 = mktime(isdst=0), 与 DstConfigDecode 同口径 */
    dstStart.tm_isdst = 0;
    dstStartUtc = mktime(&dstStart);
    ICUNIT_GOTO_NOT_EQUAL(dstStartUtc, (time_t)-1, dstStartUtc, EXIT);
    dstEnd.tm_isdst = 0;
    dstEndUtc = mktime(&dstEnd);
    ICUNIT_GOTO_NOT_EQUAL(dstEndUtc, (time_t)-1, dstEndUtc, EXIT);

    /* --- 边界 1: DST 起始日前 1 秒 → 不生效(CheckDstPeriodInner :541-542 FALSE) --- */
    timeVal = dstStartUtc - 1;
    tmRet = localtime_r(&timeVal, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 0, back.tm_isdst, EXIT);

    /* --- 边界 2: DST 起始瞬间 → 生效(:545 TRUE), 墙钟面从 00:00:00 跳到 01:00:00 --- */
    timeVal = dstStartUtc;
    tmRet = localtime_r(&timeVal, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 1, back.tm_isdst, EXIT);

    /* --- 边界 3: DST 结束前 1 秒 → 仍生效(本地面 00:59:59) --- */
    timeVal = dstEndUtc - 1;
    tmRet = localtime_r(&timeVal, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 1, back.tm_isdst, EXIT);

    /* --- 边界 4: DST 结束瞬间(结束日当天 00:00:00) → 不生效(:541 seconds >= dstEnd) --- */
    timeVal = dstEndUtc;
    tmRet = localtime_r(&timeVal, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 0, back.tm_isdst, EXIT);

    /* --- 边界 5: 结束日后 1 秒 → 不生效 --- */
    timeVal = dstEndUtc + 1;
    tmRet = localtime_r(&timeVal, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 0, back.tm_isdst, EXIT);

    /* --- 期内往返: mktime(isdst=1) 与 isdst=0 基准差恰为前拨量(:532-534 恒 TRUE + :780) --- */
    inTm.tm_year = DST_TEST_YEAR_TM;
    inTm.tm_mon = 5;    /* 5: June, 期内 */
    inTm.tm_mday = 15;  /* 15: 月中, 与起止日错开 */
    inTm.tm_hour = 12;  /* 12: 正午, 跨日安全 */
    inTm.tm_isdst = 0;
    base = mktime(&inTm);           /* 无 DST 偏移的基准 UTC 秒 */
    ICUNIT_GOTO_NOT_EQUAL(base, (time_t)-1, base, EXIT);

    inTm.tm_year = DST_TEST_YEAR_TM;
    inTm.tm_mon = 5;
    inTm.tm_mday = 15;
    inTm.tm_hour = 12;
    inTm.tm_isdst = 1;              /* 声明为 DST 时间 → 减前拨量 */
    t = mktime(&inTm);
    ICUNIT_GOTO_EQUAL(t, base - DST_FORWARD_SEC, t, EXIT);

    /* localtime 回读: 仍是 2026-06-15 12:00:00 且 isdst==1(OsEffectiveTimezone 合成) */
    tmRet = localtime_r(&t, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_year, DST_TEST_YEAR_TM, back.tm_year, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_mon, 5, back.tm_mon, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_mday, 15, back.tm_mday, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_hour, 12, back.tm_hour, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_min, 0, back.tm_min, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_sec, 0, back.tm_sec, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 1, back.tm_isdst, EXIT);

    /* gmtime 回读一致性: t = base - 3600 → UTC 面比基准恰好慢 1 小时(环差, 跨日稳健) */
    tmRet = gmtime_r(&t, &gmtT);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    tmRet = gmtime_r(&base, &gmtBase);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL((gmtBase.tm_hour - gmtT.tm_hour + 24) % 24, 1, gmtT.tm_hour, EXIT);

    /* --- 跳变区间(isdst=-1): 本地 03-01 00:30:00 不存在, mktime 归一到 01:30:00 --- */
    inTm.tm_year = DST_TEST_YEAR_TM;
    inTm.tm_mon = 2;    /* 2: March, 起始日当天 */
    inTm.tm_mday = 1;
    inTm.tm_hour = 0;
    inTm.tm_min = 30;   /* 30: 落在 [dstStart, dstStart+3600) 被跳过区间 */
    inTm.tm_sec = 0;
    inTm.tm_isdst = -1; /* 未知, 交给实现判定(:524-525 返回 FALSE, 不加前拨) */
    t = mktime(&inTm);
    ICUNIT_GOTO_NOT_EQUAL(t, (time_t)-1, t, EXIT);
    tmRet = localtime_r(&t, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_mday, 1, back.tm_mday, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_hour, 1, back.tm_hour, EXIT);  /* 00:30 → 01:30 */
    ICUNIT_GOTO_EQUAL(back.tm_min, 30, back.tm_min, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 1, back.tm_isdst, EXIT);

    /* --- 重复区间(isdst=-1): 本地 10-01 00:30:00 第二次出现, 按 DST 解释(:529-530 TRUE) --- */
    inTm.tm_year = DST_TEST_YEAR_TM;
    inTm.tm_mon = 9;    /* 9: October, 结束日当天 */
    inTm.tm_mday = 1;
    inTm.tm_hour = 0;
    inTm.tm_min = 30;   /* 30: 落在 (dstEnd, dstEnd+3600] 重复区间 */
    inTm.tm_sec = 0;
    inTm.tm_isdst = -1;
    t = mktime(&inTm);
    ICUNIT_GOTO_NOT_EQUAL(t, (time_t)-1, t, EXIT);
    tmRet = localtime_r(&t, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_mday, 1, back.tm_mday, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_hour, 0, back.tm_hour, EXIT);  /* 面值 00:30 保持 */
    ICUNIT_GOTO_EQUAL(back.tm_min, 30, back.tm_min, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 1, back.tm_isdst, EXIT);  /* 但被判定为 DST */

    /* --- 期外往返: isdst=0 输入不加不减, 回读一致 --- */
    inTm.tm_year = DST_TEST_YEAR_TM;
    inTm.tm_mon = 11;   /* 11: December, 期外 */
    inTm.tm_mday = 15;
    inTm.tm_hour = 12;
    inTm.tm_min = 0;
    inTm.tm_sec = 0;
    inTm.tm_isdst = 0;
    base = mktime(&inTm);
    ICUNIT_GOTO_NOT_EQUAL(base, (time_t)-1, base, EXIT);
    tmRet = localtime_r(&base, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_mon, 11, back.tm_mon, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_mday, 15, back.tm_mday, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_hour, 12, back.tm_hour, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 0, back.tm_isdst, EXIT);

    /* --- 期外 isdst=1 输入(行为固化): :532-534 恒 TRUE, 仍减前拨量 --- */
    inTm.tm_year = DST_TEST_YEAR_TM;
    inTm.tm_mon = 11;
    inTm.tm_mday = 15;
    inTm.tm_hour = 12;
    inTm.tm_min = 0;
    inTm.tm_sec = 0;
    inTm.tm_isdst = 1;
    t = mktime(&inTm);
    ICUNIT_GOTO_EQUAL(t, base - DST_FORWARD_SEC, t, EXIT);

    /* --- 收尾: 关闭 DST 并确认清除 --- */
    ret = dst_disable();
    ICUNIT_GOTO_EQUAL(ret, 0, ret, EXIT);
    ret = dst_inquire(DST_TEST_YEAR_TM, &dstStart, &dstEnd);
    ICUNIT_GOTO_EQUAL(ret, -1, ret, EXIT);
    timeVal = dstStartUtc;
    tmRet = localtime_r(&timeVal, &back);
    ICUNIT_GOTO_NOT_EQUAL(tmRet, NULL, tmRet, EXIT);
    ICUNIT_GOTO_EQUAL(back.tm_isdst, 0, back.tm_isdst, EXIT);

EXIT:
    /* 恢复: DST 必须回到默认 disabled(幂等) */
    (VOID)dst_disable();
    return LOS_OK;
}

VOID ItPosixTime039(void)
{
    TEST_ADD_CASE("ItPosixTime039", Testcase, TEST_LIB, TEST_POSIX, TEST_LEVEL0, TEST_FUNCTION);
}
