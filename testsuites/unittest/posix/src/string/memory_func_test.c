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

#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <errno.h>
#include <stdint.h>
#include "ohos_types.h"
#include "posix_test.h"
#include "los_config.h"
#include "kernel_test.h"
#include "log.h"

/* 内核扩展接口：lib/posix/src/malloc.c 与 errno.c 中实现但无公共头文件声明，测试侧 extern 引用 */
extern void *zalloc(size_t size);
extern void set_errno(int errCode);
extern int get_errno(void);
/* 内核扩展接口：lib/posix/src/libc.c 中实现，声明头 lib/posix/include/libc.h 未纳入
 * 测试编译 Include 路径，照上方 set_errno 先例 extern 引用 */
extern const char *libc_get_version_string(void);
extern int libc_get_version(void);

/* *
 * @tc.desc      : register a test suite, this suite is used to test basic flow and interface dependency
 * @param        : subsystem name is utils
 * @param        : module name is utilsFile
 * @param        : test suit name is PosixMemFuncTestSuite
 */
LITE_TEST_SUIT(Posix, PosixMemory, PosixMemFuncTestSuite);


/* *
 * @tc.setup     : setup for all testcases
 * @return       : setup result, TRUE is success, FALSE is fail
 */
static BOOL PosixMemFuncTestSuiteSetUp(void)
{
    return TRUE;
}

/* *
 * @tc.teardown  : teardown for all testcases
 * @return       : teardown result, TRUE is success, FALSE is fail
 */
static BOOL PosixMemFuncTestSuiteTearDown(void)
{
    dprintf("+Hello this is a memory function test+\n");
    return TRUE;
}

/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCPY_OPERATION_001
 * @tc.name      : Memony operation for memcpy test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcpy001, Function | MediumTest | Level1)
{
    void *retValue = NULL;
    char source[] = {"This File is About Memony Operation Test , Please Carefully Check Result As Below\r\n"};
    char dest[1024] = {0};
    retValue = memcpy(dest, source, sizeof(source) / sizeof(source[0]));

    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);

    retValue = memcpy(source, dest, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);

    ICUNIT_ASSERT_STRING_EQUAL(dest, source, 0);

    int len;
    char chr = 'A';
    char buf[1024];

    len = sizeof(buf) / sizeof(buf[0]);
    for (int i = 0; i < len; i++) {
        buf[i] = chr + rand() % 26;
    }

    memcpy(&buf[16], &buf[0], 16);
    for (int i = 0; i < 16; i++) {
        ICUNIT_ASSERT_EQUAL(buf[i + 16], buf[i], 0);
        if (buf[i + 16] != buf[i]) {
            dprintf("String Copy error \r\n");
            break;
        }
    }
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCPY_OPERATION_002
 * @tc.name      : Memony operation for memcpy test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcpy002, Function | MediumTest | Level1)
{
    void *retValue = NULL;

    char source[] = {"memory refers to the computer hardware devices used to store information for"
        " immediate use in a computer\r\n"};
    char dest[1024] = {0};
    retValue = memcpy(dest, source, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);

    retValue = memcpy(source, dest, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);
    ICUNIT_ASSERT_STRING_EQUAL(dest, source, 0);

    char chr = 'A';
    int i, len, failure;
    char src[1024];
    char dst[1024];

    len = rand() % 1024;
    for (i = 0; i < len; i++) {
        src[i] = chr + i % 26;
    }

    memcpy(dst, src, len);
    failure = 0;
    for (i = 0; i < len; i++) {
        if (dst[i] != src[i]) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMSET_OPERATION_001
 * @tc.name      : Memony operation for memset test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemset001, Function | MediumTest | Level1)
{
    void *retValue = NULL;
    char source[1024] = {"memory refers to the computer hardware devices used to store information for"
        " immediate use in a computer\r\n"};
    char ch = rand() % 26 + 'A';
    retValue = memset(source, ch, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);
    ICUNIT_ASSERT_EQUAL(retValue, source, 0);

    for (int i = 0; i < (sizeof(source) / sizeof(source[0])); i++) {
        ICUNIT_ASSERT_EQUAL(source[i], ch, 0);
    }
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCMP_OPERATION_001
 * @tc.name      : Memony operation for memcmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcmp001, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"memory refers to the computer hardware devices used to store information for "
        "immediate use in a computer\r\n"};
    char dest[] = {"memory refers to the computer hardware devices used to store information for "
        "immediate use in a computer\r\n"};

    retValue = memcmp(source, dest, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_EQUAL(retValue, 0, retValue);


    char orign[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    char lt[8]    = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x77};
    char eq[8]    = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    char gt[8]    = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x99};

    int ret;
    int len = sizeof(orign);

    ret = memcmp(orign, lt, len);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 1, ret, 0);

    ret = memcmp(eq, orign, len);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);

    ret = memcmp(orign, gt, len);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, ret, -1, 0);

    ret = memcmp(gt, orign, 0);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCMP_OPERATION_002
 * @tc.name      : Memony operation for memcmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcmp002, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"memory refers to the computer hardware devices used to store information for "
        "immediate use in a computer\r\n"};
    char dest[] = {"Hello, Richard, how are you?\r\n"};
    retValue = memcmp(source, dest, sizeof(dest) / sizeof(dest[0]));
    ICUNIT_ASSERT_WITHIN_EQUAL(retValue, 1, retValue, 0);

    int ret = memcmp(L"CBCDEFG", L"BBCDEFG", 7);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, 1, ret, 0);

    ret = memcmp(L"ABCDEFG", L"abcdefg", 2);
    ICUNIT_ASSERT_WITHIN_EQUAL(ret, ret, -1, 0);

    ret = memcmp(L"ABCDEFG", L"ABCDEFG", 6);
    ICUNIT_ASSERT_EQUAL(ret, 0, ret);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMCMP_OPERATION_003
 * @tc.name      : Memony operation for memcmp test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemcmp003, Function | MediumTest | Level1)
{
    int retValue = 0;
    char source[] = {"00000\r\n"};
    char dest[] = {"memory refers to the computer hardware devices used to store information for "
        "immediate use in a computer\r\n"};
    retValue = memcmp(source, dest, sizeof(source) / sizeof(source[0]));
    ICUNIT_ASSERT_WITHIN_EQUAL(retValue, retValue, -1, 0);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_REALLOC_OPERATION_001
 * @tc.name      : Memony operation for realloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemRealloc001, Function | MediumTest | Level1)
{
    char *source = (char *)malloc(20);
    source = (char *)realloc(source, 0);
    ICUNIT_ASSERT_EQUAL(source, NULL, 0);

    size_t k, len, mlen, rlen;
    for (int i = 0; i < 5; i++) {
        char *mem = NULL;
        char *data = NULL;
        int failure = 0;
        const char testChar = 0x36;

        mlen = 10;
        mem = malloc(mlen);
        ICUNIT_ASSERT_NOT_EQUAL(mem, NULL, 0);

        (void)memset_s(mem, mlen, testChar, mlen);
        rlen = rand() % (1024) + mlen;
        char *mem1 = realloc(mem, rlen);
        if (mem1 == NULL) {
            free(mem);
        }
        ICUNIT_ASSERT_NOT_EQUAL(mem1, NULL, 0);

        len = mlen <= rlen ? mlen : rlen;

        data = (char *)mem1;
        for (k = 0; k < len; k++) {
            if (data[k] != testChar) {
                failure = 1;
            }
        }

        free(mem1);
        ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    }
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_REALLOC_OPERATION_002
 * @tc.name      : Memony operation for realloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemRealloc002, Function | MediumTest | Level1)
{
    char *source = (char *)malloc(20);

    char *newData = (char *)realloc(source, 40);
    ICUNIT_ASSERT_NOT_EQUAL(newData, NULL, 0);
    if (newData != NULL) {
        source = newData;
    }
    free(source);

    const size_t len = 1024;
    const size_t large = 4096;

    void *mem = malloc(len);
    ICUNIT_ASSERT_NOT_EQUAL(mem, NULL, 0);

    void *reMem = realloc(mem, large);
    ICUNIT_ASSERT_NOT_EQUAL(reMem, NULL, 0);

    if (reMem != NULL) {
        mem = reMem;
    }
    free(mem);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_REALLOC_OPERATION_003
 * @tc.name      : Memony operation for realloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemRealloc003, Function | MediumTest | Level1)
{
    char *retValue = NULL;

    retValue = (char *)realloc(retValue, 20);
    ICUNIT_ASSERT_NOT_EQUAL(retValue, NULL, 0);
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_REALLOC_OPERATION_004
 * @tc.name      : Memony operation for realloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemRealloc004, Function | MediumTest | Level1)
{
    char *source = (char *)malloc(20);

    char *newData = (char *)realloc(source, 10);
    ICUNIT_ASSERT_NOT_EQUAL(newData, NULL, 0);

    if (newData != NULL) {
        source = newData;
    }
    free(source);
    return 0;
};


/* *
 * @tc.number    : SUB_KERNEL_POSIX_CALLOC_OPERATION_001
 * @tc.name      : Memony operation for calloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemCalloc001, Function | MediumTest | Level1)
{
    /* 移植自 musl libc-test malloc_calloc_test.cpp calloc_002：分配 n*size 字节并逐字节校验清零 */
    const size_t n = 10;
    const size_t size = 40;
    char *mem = NULL;
    char *mem1 = NULL;
    mem = (char *)calloc(n, size);
    ICUNIT_GOTO_EQUAL((mem != NULL), 1, (UINTPTR)mem, EXIT);
    for (size_t i = 0; i < n * size; i++) {
        ICUNIT_GOTO_EQUAL(mem[i], 0, mem[i], EXIT);
    }
    free(mem);
    mem = NULL;

    /* 单元素分配同样清零 */
    mem1 = (char *)calloc(1, 100);
    ICUNIT_GOTO_EQUAL((mem1 != NULL), 1, (UINTPTR)mem1, EXIT);
    for (size_t i = 0; i < 100; i++) {
        ICUNIT_GOTO_EQUAL(mem1[i], 0, mem1[i], EXIT);
    }
    free(mem1);
    mem1 = NULL;
EXIT:
    if (mem != NULL) { free(mem); mem = NULL; }
    if (mem1 != NULL) { free(mem1); mem1 = NULL; }
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_CALLOC_OPERATION_002
 * @tc.name      : Memony operation for calloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemCalloc002, Function | MediumTest | Level1)
{
    /* 多次分配的内容独立性：两块内存分别写入不同模式后互不干扰
     * 注: 指针须为 unsigned char*, 0xAA 等 >0x7F 字节与 signed char 比较会因符号扩展恒不等 */
    const size_t len = 64;
    unsigned char *mem1 = NULL;
    unsigned char *mem2 = NULL;
    mem1 = (unsigned char *)calloc(16, 4);
    ICUNIT_GOTO_EQUAL((mem1 != NULL), 1, (UINTPTR)mem1, EXIT);
    mem2 = (unsigned char *)calloc(16, 4);
    ICUNIT_GOTO_EQUAL((mem2 != NULL), 1, (UINTPTR)mem2, EXIT);

    (void)memset(mem1, 0xAA, len);
    (void)memset(mem2, 0x55, len);
    for (size_t i = 0; i < len; i++) {
        ICUNIT_GOTO_EQUAL(mem1[i], 0xAA, mem1[i], EXIT);
        ICUNIT_GOTO_EQUAL(mem2[i], 0x55, mem2[i], EXIT);
    }

    free(mem1);
    mem1 = NULL;
    free(mem2);
    mem2 = NULL;
EXIT:
    if (mem1 != NULL) { free(mem1); mem1 = NULL; }
    if (mem2 != NULL) { free(mem2); mem2 = NULL; }
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_CALLOC_OPERATION_003
 * @tc.name      : Memony operation for calloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemCalloc003, Function | MediumTest | Level1)
{
    /* 移植自 musl libc-test malloc_calloc_test.cpp calloc_001/003~005，按内核实现裁剪：
     * 内核 calloc 对 nitems/size 为 0 及乘法溢出（nitems > UINT32_MAX/size）均返回 NULL，
     * 且不设置 errno（musl 侧的 errno==ENOMEM 断言不适用） */
    void *mem = calloc((size_t)-1, 1);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);

    mem = calloc(SIZE_MAX / 2, 3);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);

    mem = calloc(1, (size_t)-1);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);

    /* 内核语义：nitems 或 size 为 0 返回 NULL（与 musl 返回最小可用块的行为不同） */
    mem = calloc(0, 8);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);

    mem = calloc(8, 0);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_FREE_OPERATION_001
 * @tc.name      : Memony operation for free test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemFree001, Function | MediumTest | Level1)
{
    int *mem = NULL;
    unsigned char *mem1 = NULL;
    unsigned char *mem2 = NULL;

    /* free(NULL) 容忍：内核 free 对 NULL 直接返回，不崩溃（用例执行到末尾即通过） */
    free(NULL);

    /* 移植自 musl libc-test malloc_free_test.cpp free_001：malloc 后写入校验再释放 */
    mem = (int *)malloc(sizeof(int));
    ICUNIT_GOTO_EQUAL((mem != NULL), 1, (UINTPTR)mem, EXIT);
    *mem = 5;
    ICUNIT_GOTO_EQUAL(*mem, 5, *mem, EXIT);
    free(mem);
    mem = NULL;

    /* malloc→free→malloc 复用链路：释放后再分配应成功且可正常写入读取（unsigned char 防 0xA5 符号扩展） */
    mem1 = (unsigned char *)malloc(32);
    ICUNIT_GOTO_EQUAL((mem1 != NULL), 1, (UINTPTR)mem1, EXIT);
    (void)memset(mem1, 0x5A, 32);
    ICUNIT_GOTO_EQUAL(mem1[0], 0x5A, mem1[0], EXIT);
    free(mem1);
    mem1 = NULL;

    mem2 = (unsigned char *)malloc(32);
    ICUNIT_GOTO_EQUAL((mem2 != NULL), 1, (UINTPTR)mem2, EXIT);
    (void)memset(mem2, 0xA5, 32);
    ICUNIT_GOTO_EQUAL(mem2[31], 0xA5, mem2[31], EXIT);
    free(mem2);
    mem2 = NULL;
EXIT:
    if (mem != NULL) { free(mem); mem = NULL; }
    if (mem1 != NULL) { free(mem1); mem1 = NULL; }
    if (mem2 != NULL) { free(mem2); mem2 = NULL; }
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMALIGN_OPERATION_001
 * @tc.name      : Memony operation for memalign test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemalign001, Function | MediumTest | Level1)
{
    /* 移植自 musl libc-test memalign.c memalign_0100/0200：按 2 的幂对齐分配，地址需为 align 的整数倍 */
    const size_t aligns[] = {8, 16, 32, 64, 512};
    const size_t len = 128;
    const size_t count = sizeof(aligns) / sizeof(aligns[0]);
    char *mem = NULL;
    for (size_t k = 0; k < count; k++) {
        size_t align = aligns[k];
        mem = (char *)memalign(align, len);
        ICUNIT_GOTO_EQUAL((mem != NULL), 1, (UINTPTR)mem, EXIT);
        ICUNIT_GOTO_EQUAL(((UINTPTR)mem % align), 0, (UINTPTR)mem, EXIT);

        int failure = 0;
        (void)memset(mem, 0x3C, len);
        for (size_t i = 0; i < len; i++) {
            if (mem[i] != 0x3C) {
                failure = 1;
            }
        }
        ICUNIT_GOTO_EQUAL(failure, 0, failure, EXIT);
        free(mem);
        mem = NULL;
    }
EXIT:
    if (mem != NULL) { free(mem); mem = NULL; }
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMALIGN_OPERATION_002
 * @tc.name      : Memony operation for memalign test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemalign002, Function | MediumTest | Level1)
{
    /* 移植自 musl libc-test memalign.c memalign_0300/0400/0500，按内核实现裁剪：
     * LOS_MemAllocAlign 拒绝 boundary 为 0/非 2 的幂；包装层对 size 为 0 直接返回 NULL；
     * 超大 size 因内存不足返回 NULL */
    void *mem = memalign(0, 128);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);

    /* 非 2 的幂对齐值（musl 用 getpagesize()-1，此处取更小的 3） */
    mem = memalign(3, 128);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);

    mem = memalign(16, 0);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);

    mem = memalign(8, (size_t)-1);
    ICUNIT_ASSERT_EQUAL(mem, NULL, (UINTPTR)mem);
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_ZALLOC_OPERATION_001
 * @tc.name      : Memony operation for zalloc test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemZalloc001, Function | MediumTest | Level1)
{
    /* zalloc 为内核扩展接口（lib/posix/src/malloc.c）：分配并清零，逻辑类比 calloc 的清零校验 */
    const size_t len = 64;
    char *mem = NULL;
    void *mem1 = NULL;
    mem = (char *)zalloc(len);
    ICUNIT_GOTO_EQUAL((mem != NULL), 1, (UINTPTR)mem, EXIT);
    for (size_t i = 0; i < len; i++) {
        ICUNIT_GOTO_EQUAL(mem[i], 0, mem[i], EXIT);
    }

    int failure = 0;
    (void)memset(mem, 0x77, len);
    for (size_t i = 0; i < len; i++) {
        if (mem[i] != 0x77) {
            failure = 1;
        }
    }
    ICUNIT_GOTO_EQUAL(failure, 0, failure, EXIT);
    free(mem);
    mem = NULL;

    /* size 为 0 返回 NULL */
    mem1 = zalloc(0);
    ICUNIT_GOTO_EQUAL(mem1, NULL, (UINTPTR)mem1, EXIT);
EXIT:
    if (mem != NULL) { free(mem); mem = NULL; }
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_ERRNO_OPERATION_001
 * @tc.name      : Errno operation for set_errno and get_errno test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsSetErrno001, Function | MediumTest | Level1)
{
    /* lib/posix/src/errno.c 的 set_errno/get_errno 为任务级 errno 存取接口；
     * 用例运行前 errno 可能已被前置用例修改，仅记录原值用于恢复 */
    int oldErrno = get_errno();
    LOG("initial errno = %d\n", oldErrno);

    /* set→get 往返 */
    set_errno(EINVAL);
    ICUNIT_ASSERT_EQUAL(get_errno(), EINVAL, get_errno());
    set_errno(ENOMEM);
    ICUNIT_ASSERT_EQUAL(get_errno(), ENOMEM, get_errno());

    /* set_errno(0) 为 no-op：不清除当前值（errno.c 实现语义：errCode 为 0 直接返回） */
    set_errno(0);
    ICUNIT_ASSERT_EQUAL(get_errno(), ENOMEM, get_errno());

    /* errno 宏与 set/get_errno 同源：均经 __errno_location 访问任务级存储 */
    errno = EINTR;
    ICUNIT_ASSERT_EQUAL(get_errno(), EINTR, get_errno());
    set_errno(ESRCH);
    ICUNIT_ASSERT_EQUAL(errno, ESRCH, errno);

    /* 恢复原值（set_errno(0) 不生效，原值为 0 时无法恢复为 0，属实现限制） */
    if (oldErrno != 0) {
        set_errno(oldErrno);
    }
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMMOVE_OPERATION_001
 * @tc.name      : Memony operation for memmove test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: memmove 四场景(同址/非重叠/前向重叠/后向重叠)内容校验+返回值
 * 移植自 musl libc-test functionalext/supplement/string/memmove.c memmove_0100~0400,
 * 数据量加大为 256B、重叠偏移 16B; 字节值断言一律 unsigned char*(防符号扩展) */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemmove001, Function | MediumTest | Level1)
{
    unsigned char buf[512];
    unsigned char backup[512];
    unsigned char *ret;
    size_t i;
    int failure;

    /* 场景 1: 同址(dest==src) —— 返回 dest 且内容不变(musl memmove_0100 思想) */
    for (i = 0; i < sizeof(buf); i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }
    (void)memcpy_s(backup, sizeof(backup), buf, sizeof(buf));
    ret = (unsigned char *)memmove(&buf[16], &buf[16], 256); /* 16: 偏移, 256: 数据长度 */
    ICUNIT_ASSERT_EQUAL(ret, &buf[16], (UINTPTR)ret);
    failure = 0;
    for (i = 0; i < sizeof(buf); i++) {
        if (buf[i] != backup[i]) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);

    /* 场景 2: 非重叠 —— 源 [0,256) 拷到 [256,512), 源区不变(musl memmove_0200 思想) */
    for (i = 0; i < 256; i++) {
        buf[i] = (unsigned char)(i & 0xFF);
    }
    (void)memset_s(&buf[256], 256, 0x5A, 256);
    (void)memcpy_s(backup, sizeof(backup), buf, sizeof(buf));
    ret = (unsigned char *)memmove(&buf[256], &buf[0], 256);
    ICUNIT_ASSERT_EQUAL(ret, &buf[256], (UINTPTR)ret);
    failure = 0;
    for (i = 0; i < 256; i++) {
        if ((buf[256 + i] != backup[i]) || (buf[i] != backup[i])) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);

    /* 场景 3: 前向重叠(dest=src+16, dest<src) —— 正向拷贝, 结果区==原源区
     * (musl memmove_0300 思想; 源串尾部数据布局: 源 [16,272) 尾部延伸入重叠区) */
    (void)memset_s(buf, sizeof(buf), 0xA5, sizeof(buf));
    for (i = 0; i < 256; i++) {
        buf[16 + i] = (unsigned char)(i & 0xFF);
    }
    (void)memcpy_s(backup, sizeof(backup), buf, sizeof(buf));
    ret = (unsigned char *)memmove(&buf[32], &buf[16], 256);
    ICUNIT_ASSERT_EQUAL(ret, &buf[32], (UINTPTR)ret);
    failure = 0;
    for (i = 0; i < 256; i++) {
        if (buf[32 + i] != (unsigned char)(i & 0xFF)) {
            failure = 1;
            break;
        }
    }
    /* 重叠未触及区域([0,16) 与 [288,512))须保持原样 */
    for (i = 0; i < 16; i++) {
        if ((buf[i] != backup[i]) || (buf[288 + i] != backup[288 + i])) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);

    /* 场景 4: 后向重叠(dest=src-16, dest>src) —— 反向拷贝, 结果区==原源区
     * (musl memmove_0400 思想; 源 [16,272) 尾部数据须先于覆盖被搬走) */
    (void)memset_s(buf, sizeof(buf), 0xA5, sizeof(buf));
    for (i = 0; i < 256; i++) {
        buf[16 + i] = (unsigned char)(i & 0xFF);
    }
    (void)memcpy_s(backup, sizeof(backup), buf, sizeof(buf));
    ret = (unsigned char *)memmove(&buf[0], &buf[16], 256);
    ICUNIT_ASSERT_EQUAL(ret, &buf[0], (UINTPTR)ret);
    failure = 0;
    for (i = 0; i < 256; i++) {
        if (buf[i] != (unsigned char)(i & 0xFF)) {
            failure = 1;
            break;
        }
    }
    /* 源区之后 [272,512) 须保持原样 */
    for (i = 272; i < sizeof(buf); i++) {
        if (buf[i] != backup[i]) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_MEMMOVE_OPERATION_002
 * @tc.name      : Memony operation for memmove test
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: memmove n=0 边界(合法返回 dest)+256B 长块双向重叠
 * 移植自 musl libc-test functionalext/supplement/string/memmove.c :60-89
 * (memmove_0300/0400)思想并加大为 256B 长块; n=0 时不做任何拷贝仅返回 dest */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsMemMemmove002, Function | MediumTest | Level1)
{
    unsigned char buf[512];
    unsigned char backup[512];
    unsigned char *ret;
    size_t i;
    int failure;

    for (i = 0; i < sizeof(buf); i++) {
        buf[i] = (unsigned char)((i * 7) & 0xFF); /* 非规律填充, 增强区分度 */
    }
    (void)memcpy_s(backup, sizeof(backup), buf, sizeof(buf));

    /* n=0 边界: 返回 dest 且不触碰缓冲区内容 */
    ret = (unsigned char *)memmove(&buf[128], &buf[64], 0);
    ICUNIT_ASSERT_EQUAL(ret, &buf[128], (UINTPTR)ret);
    /* 同址 + n=0 */
    ret = (unsigned char *)memmove(&buf[128], &buf[128], 0);
    ICUNIT_ASSERT_EQUAL(ret, &buf[128], (UINTPTR)ret);
    failure = 0;
    for (i = 0; i < sizeof(buf); i++) {
        if (buf[i] != backup[i]) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);

    /* 256B 长块前向重叠一次(dest=src+16) */
    (void)memset_s(buf, sizeof(buf), 0x5A, sizeof(buf));
    for (i = 0; i < 256; i++) {
        buf[16 + i] = (unsigned char)(i & 0xFF);
    }
    ret = (unsigned char *)memmove(&buf[32], &buf[16], 256);
    ICUNIT_ASSERT_EQUAL(ret, &buf[32], (UINTPTR)ret);
    failure = 0;
    for (i = 0; i < 256; i++) {
        if (buf[32 + i] != (unsigned char)(i & 0xFF)) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);

    /* 256B 长块后向重叠一次(dest=src-16) */
    (void)memset_s(buf, sizeof(buf), 0x5A, sizeof(buf));
    for (i = 0; i < 256; i++) {
        buf[16 + i] = (unsigned char)(i & 0xFF);
    }
    ret = (unsigned char *)memmove(&buf[0], &buf[16], 256);
    ICUNIT_ASSERT_EQUAL(ret, &buf[0], (UINTPTR)ret);
    failure = 0;
    for (i = 0; i < 256; i++) {
        if (buf[i] != (unsigned char)(i & 0xFF)) {
            failure = 1;
            break;
        }
    }
    ICUNIT_ASSERT_EQUAL(failure, 0, failure);
    return 0;
};

/* *
 * @tc.number    : SUB_KERNEL_POSIX_LIBC_VERSION_001
 * @tc.name      : libc_get_version and libc_get_version_string self consistency
 * @tc.desc      : [C- SOFTWARE -0200]
 */
/* 用例简要描述: libc 版本接口自洽性(数值 0x00XXYYZZ 与字符串前 3 段互相印证)
 * 实现位于 lib/posix/src/libc.c:39-51, 数值格式见 libc.c:46 注释(XX=major/YY=minor/ZZ=patch);
 * 版本号可由构建注入(libc_config.h 的 LIBC_VERSION_STR/NUM 可覆盖), 故勿断字面量;
 * 测试环境无 sscanf(全仓未收录), 采用手工解析, 不引入新依赖 */
LITE_TEST_CASE(PosixMemFuncTestSuite, testOsLibcVersion001, Function | MediumTest | Level1)
{
    const char *verStr;
    const char *p;
    int num;
    unsigned int major = 0;
    unsigned int minor = 0;
    unsigned int patch = 0;

    verStr = libc_get_version_string();
    ICUNIT_ASSERT_EQUAL((verStr != NULL), 1, (UINTPTR)verStr);

    num = libc_get_version();

    /* 手工解析 "<major>.<minor>.<patch>[-<platform>[-<desc>]]" 前 3 段数字 */
    p = verStr;
    while ((*p >= '0') && (*p <= '9')) {
        major = major * 10 + (unsigned int)(*p - '0');
        p++;
    }
    ICUNIT_ASSERT_EQUAL(*p, '.', (int)(unsigned char)*p);
    p++;
    while ((*p >= '0') && (*p <= '9')) {
        minor = minor * 10 + (unsigned int)(*p - '0');
        p++;
    }
    ICUNIT_ASSERT_EQUAL(*p, '.', (int)(unsigned char)*p);
    p++;
    while ((*p >= '0') && (*p <= '9')) {
        patch = patch * 10 + (unsigned int)(*p - '0');
        p++;
    }

    /* 自洽校验: 数值 0x00XXYYZZ 各字节段与字符串解析值一致 */
    ICUNIT_ASSERT_EQUAL(((unsigned int)num >> 16) & 0xFF, major, num);
    ICUNIT_ASSERT_EQUAL(((unsigned int)num >> 8) & 0xFF, minor, num);
    ICUNIT_ASSERT_EQUAL((unsigned int)num & 0xFF, patch, num);
    return 0;
};

RUN_TEST_SUITE(PosixMemFuncTestSuite);

void PosixStringMemTest03()
{
    LOG("begin PosixStringMemTest03....");
    RUN_ONE_TESTCASE(testOsMemMemcpy001);
    RUN_ONE_TESTCASE(testOsMemMemcpy002);
    RUN_ONE_TESTCASE(testOsMemMemset001);
    RUN_ONE_TESTCASE(testOsMemMemcmp001);
    RUN_ONE_TESTCASE(testOsMemMemcmp002);
    RUN_ONE_TESTCASE(testOsMemMemcmp003);
    RUN_ONE_TESTCASE(testOsMemRealloc001);
    RUN_ONE_TESTCASE(testOsMemRealloc002);
    RUN_ONE_TESTCASE(testOsMemRealloc003);
    RUN_ONE_TESTCASE(testOsMemRealloc004);
    RUN_ONE_TESTCASE(testOsMemCalloc001);
    RUN_ONE_TESTCASE(testOsMemCalloc002);
    RUN_ONE_TESTCASE(testOsMemCalloc003);
    RUN_ONE_TESTCASE(testOsMemFree001);
    RUN_ONE_TESTCASE(testOsMemMemalign001);
    RUN_ONE_TESTCASE(testOsMemMemalign002);
    RUN_ONE_TESTCASE(testOsMemZalloc001);
    RUN_ONE_TESTCASE(testOsSetErrno001);
    RUN_ONE_TESTCASE(testOsMemMemmove001);
    RUN_ONE_TESTCASE(testOsMemMemmove002);
    RUN_ONE_TESTCASE(testOsLibcVersion001);

    return;
}