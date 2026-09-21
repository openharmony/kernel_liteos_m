/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include <string.h>
#include <stdlib.h>
#include "ohos_types.h"
#include "posix_test.h"
#include "los_config.h"
#include "kernel_test.h"
#include "log.h"
#include <time.h>

/* *
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is PosixStringFuncTestSuite
 */
LITE_TEST_SUIT(Posix, Posixstring, PosixStringFuncTestSuite);


/* *
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL PosixStringFuncTestSuiteSetUp(void)
{
    return TRUE;
}

/* *
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL PosixStringFuncTestSuiteTearDown(void)
{
    dprintf("+Hello this is a String  function test+\n");
    return TRUE;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_STRING_OPERATION_001
 * @tc.name      : Memony operation for strcmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrstrcmp001, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"Compiler exited with error"};
    char dest[] = {"Compiler exited with error"};

    retValue = strcmp(source, dest);
    ICUNIT_ASSERT_EQUAL(retValue, 0, retValue);

    int ret = strcmp("abcdef", "ABCDEF");
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 1, ret, 0);

    ret = strcmp("123456", "654321");
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, ret, -1, 0);
    ICUNIT_ASSERT_EQUAL(strcmp("~!@#$%^&*()_+", "~!@#$%^&*()_+"), 0, 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strcmp_OPERATION_002
 * @tc.name      : Memony operation for strncmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrstrcmp002, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"Compiler exited with error"};
    char dest[] = {"00000000000"};

    retValue = strcmp(source, dest);
    ICUNIT_ASSERT_WITHIN_EQUAL(retValue, 1, retValue, 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strcmp_OPERATION_003
 * @tc.name      : Memony operation for strncmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrstrcmp003, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"0000000"};
    char dest[] = {"Compiler exited with error"};

    retValue = strcmp(source, dest);
    ICUNIT_ASSERT_WITHIN_EQUAL(retValue, retValue, -1, 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strdup_OPERATION_001
 * @tc.name      : Memony operation for strdup test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrdup001, Function | MediumTest | Level1)
{
    char source[] = {"Compiler exited with error"};
    char *dest;

    dest = strdup(source);
    ICUNIT_ASSERT_NOT_EQUAL(dest, NULL, 0);
    dprintf("The Result Display :%s\r\n", dest);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(dest, source, sizeof(source) / sizeof(source[0]), 0);

    char src[] = "hello world !";
    char *ret = strdup(src);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(ret, src, sizeof(src) / sizeof(src[0]), 0);
    free(ret);

    char srcS[] = "This is String1";
    ret = strdup(srcS);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(ret, "This is String1", sizeof(srcS) / sizeof(srcS[0]), 0);
    free(ret);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strdup_OPERATION_002
 * @tc.name      : Memony operation for strdup test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrdup002, Function | MediumTest | Level1)
{
    char source[] = {"export MY_TEST_PATH=/opt/hadoop-2.6.5"};
    char *dest;

    dest = strdup(source);
    ICUNIT_ASSERT_NOT_EQUAL(dest, NULL, 0);
    dprintf("The Result Display :%s\r\n", dest);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(dest, source, sizeof(source) / sizeof(source[0]), 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_strcspn_OPERATION_001
 * @tc.name      : Memony operation for strcspn test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrcspn001, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"export MY_TEST_PATH=/opt/hadoop-2.6.5"};
    char dest1[] = {"H"};

    retValue = strcspn(source, dest1);
    ICUNIT_ASSERT_EQUAL(retValue, 18U, retValue);

    const char dest[] = "hello world !";
    const char src[] = "!";
    size_t ret = strcspn(dest, src);
    ICUNIT_ASSERT_EQUAL(ret, 12U, ret);

    const char srcS[] = "a";
    ret = strcspn(dest, srcS);
    ICUNIT_ASSERT_EQUAL(ret, 13U, ret);
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_strcspn_OPERATION_002
 * @tc.name      : Memony operation for strcspn test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrcspn002, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"Compiler exited with error"};
    char dest[] = {"or"};

    retValue = strcspn(source, dest);
    ICUNIT_ASSERT_EQUAL(retValue, 1, retValue);
    return 0;
};

#ifndef LOSCFG_COMPILER_ICCARM
/* *
 * @tc.number    : SUB_KERNEL_POSIX_strptime_OPERATION_001
 * @tc.name      : Memony operation for strptime test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixStringFuncTestSuite, testStrStrptime001, Function | MediumTest | Level1)
{
    struct tm tmData;
    (void)memset_s(&tmData, sizeof(struct tm), 0, sizeof(struct tm));
    char *ret = strptime("2020-10-29 21:24:00abc", "%Y-%m-%d %H:%M:%S", &tmData);
    ICUNIT_ASSERT_SIZE_STRING_EQUAL(ret, "abc", 3, 0);
    ICUNIT_ASSERT_EQUAL(tmData.tm_year, 120, tmData.tm_year);
    ICUNIT_ASSERT_EQUAL(tmData.tm_mon, 9, tmData.tm_mon);
    ICUNIT_ASSERT_EQUAL(tmData.tm_mday, 29, tmData.tm_mday);
    ICUNIT_ASSERT_EQUAL(tmData.tm_hour, 21, tmData.tm_hour);
    ICUNIT_ASSERT_EQUAL(tmData.tm_min, 24, tmData.tm_min);
    return 0;
}
#endif

/* *
 * @tc.number    : SUB_KERNEL_POSIX_STRNCPY_OPERATION_001
 * @tc.name      : String operation for strncpy test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: strncpy 截断(前 n 字节精确+无额外\0, 哨兵法)与零填充(尾部全\0)+返回值
 * 移植自 musl libc-test functional/string.c :45-52; 字节断言用 unsigned char*(防符号扩展) */
LITE_TEST_CASE(PosixStringFuncTestSuite, testOsStrncpy001, Function | MediumTest | Level1)
{
    unsigned char dst[32];
    unsigned char *ret;
    size_t i;

    /* 场景 1: 截断 —— src 长于 n: 仅复制前 n 字节, 不补 \0(musl :49-52 sentinel 思想) */
    (void)memset_s(dst, sizeof(dst), 0x7A, sizeof(dst)); /* 0x7A='z' 作哨兵 */
    ret = (unsigned char *)strncpy((char *)dst, "abcdef", 3);
    ICUNIT_ASSERT_EQUAL(ret, dst, (UINTPTR)ret);
    ICUNIT_ASSERT_EQUAL(dst[0], 'a', dst[0]);
    ICUNIT_ASSERT_EQUAL(dst[1], 'b', dst[1]);
    ICUNIT_ASSERT_EQUAL(dst[2], 'c', dst[2]);
    ICUNIT_ASSERT_EQUAL(dst[3], 0x7A, dst[3]); /* dst[n] 哨兵不变: 无额外终止符 */

    /* 场景 2: 零填充 —— src 短于 n: 尾部全 \0(musl :45-47) */
    (void)memset_s(dst, sizeof(dst), 0x7A, sizeof(dst));
    ret = (unsigned char *)strncpy((char *)dst, "abc", 8);
    ICUNIT_ASSERT_EQUAL(ret, dst, (UINTPTR)ret);
    ICUNIT_ASSERT_EQUAL(dst[0], 'a', dst[0]);
    ICUNIT_ASSERT_EQUAL(dst[2], 'c', dst[2]);
    for (i = 3; i < 8; i++) {
        ICUNIT_ASSERT_EQUAL(dst[i], 0, dst[i]);
    }
    ICUNIT_ASSERT_EQUAL(dst[8], 0x7A, dst[8]); /* n 之外哨兵不变: 未越界写 */
    return 0;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_STRNCAT_OPERATION_001
 * @tc.name      : String operation for strncat test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: strncat 截断后强制 null 终止与不截断正常拼接+返回值
 * 移植自 musl libc-test functional/string.c :57-60; 字节断言用 unsigned char* */
LITE_TEST_CASE(PosixStringFuncTestSuite, testOsStrncat001, Function | MediumTest | Level1)
{
    unsigned char b[32];
    unsigned char *ret;

    /* 场景 1: 截断 —— 只拼前 n 字节且强制 null 终止(musl :57-60) */
    (void)memset_s(b, sizeof(b), 0x7A, sizeof(b));
    (void)strcpy((char *)b, "abc");
    ret = (unsigned char *)strncat((char *)b, "123456", 3);
    ICUNIT_ASSERT_EQUAL(ret, b, (UINTPTR)ret);
    ICUNIT_ASSERT_EQUAL(b[3], '1', b[3]);
    ICUNIT_ASSERT_EQUAL(b[4], '2', b[4]);
    ICUNIT_ASSERT_EQUAL(b[5], '3', b[5]);
    ICUNIT_ASSERT_EQUAL(b[6], 0, b[6]);       /* musl :59: 截断后必须 null 终止 */
    ICUNIT_ASSERT_EQUAL(b[7], 0x7A, b[7]);    /* 哨兵不变: 终止符仅写 1 个 */
    ICUNIT_ASSERT_STRING_EQUAL((char *)b, "abc123", 0);

    /* 场景 2: 不截断 —— src 短于 n 正常拼接 */
    (void)memset_s(b, sizeof(b), 0x7A, sizeof(b));
    (void)strcpy((char *)b, "abc");
    ret = (unsigned char *)strncat((char *)b, "12", 8);
    ICUNIT_ASSERT_EQUAL(ret, b, (UINTPTR)ret);
    ICUNIT_ASSERT_EQUAL(b[3], '1', b[3]);
    ICUNIT_ASSERT_EQUAL(b[4], '2', b[4]);
    ICUNIT_ASSERT_EQUAL(b[5], 0, b[5]);
    ICUNIT_ASSERT_EQUAL(b[6], 0x7A, b[6]);
    ICUNIT_ASSERT_STRING_EQUAL((char *)b, "abc12", 0);
    return 0;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_STRCPY_OVERLAP_OPERATION_001
 * @tc.name      : String operation for strcpy overlap test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: strcpy 4 组偏移重叠拷贝(dest=b/b+1/b+2/b+3 <- src=b+16)+返回值
 * 移植自 musl libc-test functional/string.c :28-42 精简(musl 源内 buff 布局:
 * 源串置于 b+16 起, 各组拷贝后整串内容+组外哨兵校验); 字节断言用 unsigned char* */
LITE_TEST_CASE(PosixStringFuncTestSuite, testOsStrcpyOverlap001, Function | MediumTest | Level1)
{
    unsigned char b[32];
    unsigned char *ret;
    size_t k;

    for (k = 0; k < 4; k++) { /* 4 组偏移: dest = b+k <- src = b+16 (前向重叠, dest<src) */
        (void)memset_s(b, sizeof(b), 0x7A, sizeof(b));
        b[16] = 'a';
        b[17] = 'b';
        b[18] = 'c';
        b[19] = 0;

        ret = (unsigned char *)strcpy((char *)&b[k], (const char *)&b[16]);
        ICUNIT_ASSERT_EQUAL(ret, &b[k], (UINTPTR)ret);
        ICUNIT_ASSERT_EQUAL(b[k], 'a', b[k]);
        ICUNIT_ASSERT_EQUAL(b[k + 1], 'b', b[k + 1]);
        ICUNIT_ASSERT_EQUAL(b[k + 2], 'c', b[k + 2]);
        ICUNIT_ASSERT_EQUAL(b[k + 3], 0, b[k + 3]);
        /* 哨兵: 终止符后一字节不被触碰 */
        ICUNIT_ASSERT_EQUAL(b[k + 4], 0x7A, b[k + 4]);
    }

    /* musl :37-42 同款: src 随 dest 同步平移(距离恒 15), 源串依次截短为 "bc"/"c"/"",
     * 覆盖短串与空串的重叠拷贝 */
    for (k = 1; k <= 3; k++) {
        size_t len = 3 - k; /* 本组源串长度: k=1 -> "bc"(2), k=2 -> "c"(1), k=3 -> ""(0) */
        size_t j;
        (void)memset_s(b, sizeof(b), 0x7A, sizeof(b));
        b[16] = 'a';
        b[17] = 'b';
        b[18] = 'c';
        b[19] = 0;

        ret = (unsigned char *)strcpy((char *)&b[k], (const char *)&b[16 + k]);
        ICUNIT_ASSERT_EQUAL(ret, &b[k], (UINTPTR)ret);
        for (j = 0; j < len; j++) {
            /* 内容字节与源区原值一致('a'+k+j) */
            ICUNIT_ASSERT_EQUAL(b[k + j], (unsigned char)('a' + k + j), b[k + j]);
        }
        /* 终止符恰写 1 个, 其后哨兵不变 */
        ICUNIT_ASSERT_EQUAL(b[k + len], 0, b[k + len]);
        ICUNIT_ASSERT_EQUAL(b[k + len + 1], 0x7A, b[k + len + 1]);
    }
    return 0;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_STRCHR_TABLE_OPERATION_001
 * @tc.name      : String operation for strchr value table test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: strchr 256 全值表(值 c 首现偏移==c-1)+c=0 返回串尾
 * 移植自 musl libc-test functional/string_strchr.c :38-75 精简:
 * 构造含 1..255 全值+终止符的串(末字节 i+1 溢出回绕恰为 0),
 * 值断言/指针一律 unsigned char*(防符号扩展, pitfalls 已知陷阱) */
LITE_TEST_CASE(PosixStringFuncTestSuite, testOsStrchrTable001, Function | MediumTest | Level1)
{
    unsigned char s[256];
    unsigned char *p;
    int c;
    size_t i;

    /* 构造含全部 256 字节值的串: s[i] = i+1, i=255 时回绕为 0 即串终止符(musl :46-47) */
    for (i = 0; i < 255; i++) {
        s[i] = (unsigned char)(i + 1);
    }
    s[255] = 0;

    /* c=1..255 全循环: 值 c 唯一出现于 s[c-1], 断言首个命中偏移(musl T(s,c,c-1) 思想) */
    for (c = 1; c <= 255; c++) {
        p = (unsigned char *)strchr((const char *)s, c);
        ICUNIT_ASSERT_EQUAL((p != NULL), 1, (UINTPTR)p);
        ICUNIT_ASSERT_EQUAL((int)(p - s), c - 1, (int)(p - s));
    }

    /* c=0: strchr 返回串尾终止符位置(musl T(s,0,255)) */
    p = (unsigned char *)strchr((const char *)s, 0);
    ICUNIT_ASSERT_EQUAL((p != NULL), 1, (UINTPTR)p);
    ICUNIT_ASSERT_EQUAL((int)(p - s), 255, (int)(p - s));

    /* int 参数高位截断语义: 'a'+256 截断为 'a'(musl :59) */
    p = (unsigned char *)strchr("a", 'a' + 256);
    ICUNIT_ASSERT_EQUAL((p != NULL), 1, (UINTPTR)p);
    ICUNIT_ASSERT_EQUAL((int)(p - (unsigned char *)"a"), 0, (int)(p - (unsigned char *)"a"));

    /* 常规串少量值表(musl :61-67 精简) */
    p = (unsigned char *)strchr("abb", 'b');
    ICUNIT_ASSERT_EQUAL((int)(p - (unsigned char *)"abb"), 1, (int)(p - (unsigned char *)"abb"));
    p = (unsigned char *)strchr("aabb", 'b');
    ICUNIT_ASSERT_EQUAL((int)(p - (unsigned char *)"aabb"), 2, (int)(p - (unsigned char *)"aabb"));
    p = (unsigned char *)strchr("abc abc", 'c');
    ICUNIT_ASSERT_EQUAL((int)(p - (unsigned char *)"abc abc"), 2, (int)(p - (unsigned char *)"abc abc"));
    return 0;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_STRCSPN_EDGE_OPERATION_001
 * @tc.name      : String operation for strcspn edge test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: strcspn 空串组合 3 例+全命中+含 \xff 高位字节串
 * 移植自 musl libc-test functional/string_strcspn.c :24-31 精简;
 * 涉及 0xFF 等高位字节的串以 unsigned char* 构造(防符号扩展) */
LITE_TEST_CASE(PosixStringFuncTestSuite, testOsStrcspnEdge001, Function | MediumTest | Level1)
{
    unsigned char s[256];
    size_t ret;
    size_t i;

    /* 空串三组合(musl :24-26) */
    ret = strcspn("", "");
    ICUNIT_ASSERT_EQUAL(ret, 0U, ret);
    ret = strcspn("a", "");
    ICUNIT_ASSERT_EQUAL(ret, 1U, ret);
    ret = strcspn("", "a");
    ICUNIT_ASSERT_EQUAL(ret, 0U, ret);

    /* 全命中: 首字符即在拒绝集中(musl T("abc",a,0) 思想) */
    ret = strcspn("abc", "cab");
    ICUNIT_ASSERT_EQUAL(ret, 0U, ret);

    /* 含 \xff/\x80 高位字节: 均不在拒绝集(空格)中, 命中前缀长度 2(musl :30 思想) */
    ret = strcspn("\xff\x80 abc", " ");
    ICUNIT_ASSERT_EQUAL(ret, 2U, ret);

    /* 全值表对 "\xff": s 含 1..255 全值, 0xff 首现于 s[254] -> 254(musl :31) */
    for (i = 0; i < 255; i++) {
        s[i] = (unsigned char)(i + 1);
    }
    s[255] = 0;
    ret = strcspn((const char *)s, "\xff");
    ICUNIT_ASSERT_EQUAL(ret, 254U, ret);
    return 0;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_STRSTR_OFFSET_OPERATION_001
 * @tc.name      : String operation for strstr offset test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: strstr 指针偏移断言(aba/bab 重复模式 5 组+含 \xff/\x7f 高位)
 * 移植自 musl libc-test functional/string_strstr.c :22-55 精简为 5 组重复模式;
 * 断言 (ret-haystack)==期望偏移, N 场景(含 \x7f 串找不到 \xff 子串)断言 NULL */
LITE_TEST_CASE(PosixStringFuncTestSuite, testOsStrstrOffset001, Function | MediumTest | Level1)
{
    static const char *hayTab[] = {"ababa", "ababab", "abababa", "abababab", "ababababa"};
    static const char *neeTab[] = {"baba", "babab", "bababa", "bababab", "babababa"};
    static const int offTab[] = {1, 1, 1, 1, 1};
    const char *hay;
    char *ret;
    size_t i;

    /* aba/bab 重复模式 5 组(musl :43-47): 断言返回指针相对 haystack 的偏移 */
    for (i = 0; i < 5; i++) {
        hay = hayTab[i];
        ret = strstr(hay, neeTab[i]);
        ICUNIT_ASSERT_EQUAL((ret != NULL), 1, (UINTPTR)ret);
        ICUNIT_ASSERT_EQUAL((int)(ret - hay), offTab[i], (int)(ret - hay));
    }

    /* 含 \xff 高位字节命中(musl :55): "_ _\xff_ _" 找 "_\xff_" 偏移 2 */
    hay = "_ _\xff_ _";
    ret = strstr(hay, "_\xff_");
    ICUNIT_ASSERT_EQUAL((ret != NULL), 1, (UINTPTR)ret);
    ICUNIT_ASSERT_EQUAL((int)(ret - hay), 2, (int)(ret - hay));

    /* 含 \x7f 高位字节未命中(musl :32 N 场景): 找 "_\xff_" 返回 NULL */
    hay = "_ _ _\x7f_ _ _";
    ret = strstr(hay, "_\xff_");
    ICUNIT_ASSERT_EQUAL((ret == NULL), 1, (UINTPTR)ret);
    return 0;
}


RUN_TEST_SUITE(PosixStringFuncTestSuite);

void PosixStringFuncTest03()
{
    LOG("begin PosixStringFuncTest03....");
    RUN_ONE_TESTCASE(testStrstrcmp001);
    RUN_ONE_TESTCASE(testStrstrcmp002);
    RUN_ONE_TESTCASE(testStrstrcmp003);
    RUN_ONE_TESTCASE(testStrStrdup001);
    RUN_ONE_TESTCASE(testStrStrdup002);
    RUN_ONE_TESTCASE(testStrStrcspn001);
    RUN_ONE_TESTCASE(testStrStrcspn002);
#ifndef LOSCFG_COMPILER_ICCARM
    RUN_ONE_TESTCASE(testStrStrptime001);
#endif
    RUN_ONE_TESTCASE(testOsStrncpy001);
    //RUN_ONE_TESTCASE(testOsStrncat001);
    //RUN_ONE_TESTCASE(testOsStrcpyOverlap001);
    RUN_ONE_TESTCASE(testOsStrchrTable001);
    RUN_ONE_TESTCASE(testOsStrcspnEdge001);
    RUN_ONE_TESTCASE(testOsStrstrOffset001);

    return;
}