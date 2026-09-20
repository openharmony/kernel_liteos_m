/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
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
 *
 * Description: LiteOS gcov dump glue for bare-metal targets (standalone
 * runtime, the first proven implementation).
 *
 *   The kernel is compiled with -fprofile-arcs -ftest-coverage but does NOT
 *   link the toolchain's libgcov.a. This file supplies the gcov runtime
 *   symbols the instrumented translation units reference (__gcov_init /
 *   __gcov_merge_add) and, at test end, walks the registered gcov_info list
 *   itself and streams the counters over the console UART as base64-framed
 *   text for host-side .gcda reconstruction (no on-target filesystem and no
 *   libgcov serialization needed).
 *
 *   The compiler-emitted structs are read through fixed byte offsets, which
 *   are ABI-stable per toolchain and were verified against both target
 *   toolchains (struct layouts from gcov-io.h, cross-checked by
 *   disassembling each libgcov's gcov_do_dump/write_one_data):
 *
 *     ws63: riscv32-linux-musl-gcc 7.3, gcov version word 0x4137332A:
 *       struct gcov_info {
 *         gcov_unsigned_t version;               // +0
 *         struct gcov_info *next;                // +4  (list link, runtime)
 *         gcov_unsigned_t stamp;                 // +8
 *         const char *filename;                  // +12
 *         gcov_merge_fn merge[GCOV_COUNTERS=9];  // +16..+51
 *         gcov_unsigned_t n_functions;           // +52
 *         const struct gcov_fn_info *const *functions; // +56 (ptr table)
 *       };
 *     qemu arm ca7: arm-none-eabi-gcc 10.3.1, gcov version word 0x42303372:
 *       same until merge[GCOV_COUNTERS=8], so n_functions // +48
 *       n_functions // +48 and functions // +52 (still a pointer table).
 *     struct gcov_fn_info (both toolchains):
 *       const struct gcov_info *key;   // +0  (runtime back-ptr)
 *       gcov_unsigned_t ident;         // +4
 *       gcov_unsigned_t lineno_checksum; // +8
 *       gcov_unsigned_t cfg_checksum;  // +12
 *       struct gcov_ctr_info ctrs[1];  // +16 { num(4), values(4) }
 *     Only the arc counter set is ever allocated (-fprofile-arcs), so
 *     ctrs[0] holds every counter of a function. The host-side
 *     parse_gcov_log.py rebuilds the .gcda from the emitted payload and
 *     takes the .gcda version word from the matching .gcno, so both
 *     toolchains' gcov can read the result.
 *
 *   The payload (little-endian) per GCOV_BEGIN..GCOV_END frame is:
 *     stamp:u32  n_functions:u32
 *     per function: ident:u32 lineno_checksum:u32 cfg_checksum:u32
 *                   n_ctrs:u32  n_ctrs * gcov_type(i64)
 */

#include "los_compiler.h"
#include "los_printf.h"
#include "los_interrupt.h"
#include "test_gcov.h"

/* ---- gcov types matching GCC gcov-io.h ---- */
typedef unsigned int gcov_unsigned_t;
typedef long long gcov_type_t;

struct gcov_info;
struct gcov_fn_info;

/* Field accessors by fixed byte offset (ABI-stable per GCC release).
 * Reading the compiler-emitted structs via offsets avoids struct padding
 * guesses and keeps this file free of compiler headers. */
#define GCOV_INFO_VERSION(p)     (*(const gcov_unsigned_t *)((const CHAR *)(p) + 0))
#define GCOV_INFO_NEXT(p)        (*(struct gcov_info **)((CHAR *)(p) + 4))
#define GCOV_INFO_STAMP(p)       (*(const gcov_unsigned_t *)((const CHAR *)(p) + 8))
#define GCOV_INFO_FILENAME(p)    (*(const char *const *)((const CHAR *)(p) + 12))
#if (__GNUC__ >= 10)
/* GCC 10.x: GCOV_COUNTERS == 8 (verified on arm-none-eabi 10.3.1) */
#define GCOV_INFO_NFUNCTIONS_OFF 48
#define GCOV_INFO_FUNCTIONS_OFF  52
#else
/* GCC 7.x: GCOV_COUNTERS == 9 (verified on riscv32-linux-musl 7.3.0) */
#define GCOV_INFO_NFUNCTIONS_OFF 52
#define GCOV_INFO_FUNCTIONS_OFF  56
#endif
#define GCOV_INFO_NFUNCTIONS(p)  (*(const gcov_unsigned_t *)((const CHAR *)(p) + GCOV_INFO_NFUNCTIONS_OFF))
#define GCOV_INFO_FUNCTIONS(p)   (*(const struct gcov_fn_info *const **)((const CHAR *)(p) + GCOV_INFO_FUNCTIONS_OFF))

#define GCOV_FN_KEY(p)     (*(const struct gcov_info *const *)((const CHAR *)(p) + 0))
#define GCOV_FN_IDENT(p)   (*(const gcov_unsigned_t *)((const CHAR *)(p) + 4))
#define GCOV_FN_LNCK(p)    (*(const gcov_unsigned_t *)((const CHAR *)(p) + 8))
#define GCOV_FN_CFGCK(p)   (*(const gcov_unsigned_t *)((const CHAR *)(p) + 12))
#define GCOV_FN_NCTRS(p)   (*(const gcov_unsigned_t *)((const CHAR *)(p) + 16))
#define GCOV_FN_VALUES(p)  (*(const gcov_type_t *const *)((const CHAR *)(p) + 20))

/* ---- gcov_info registration list ----
 * Registered from each translation unit's .init_array constructor. The +4
 * "next" slot of gcov_info is reserved by GCC for exactly this list link
 * (libgcov's own __gcov_init uses it the same way). */
static struct gcov_info *g_gcovHead = NULL;
static volatile int g_gcovDumped = 0;

/* ============================================================================
 * gcov runtime symbols expected by -fprofile-arcs instrumented code.
 * ========================================================================== */

VOID __gcov_init(struct gcov_info *info)
{
    UINT32 intSave;

    if (info == NULL) {
        return;
    }
    intSave = LOS_IntLock();
    GCOV_INFO_NEXT(info) = g_gcovHead;
    g_gcovHead = info;
    LOS_IntRestore(intSave);
}

/* __gcov_merge_add is only stored as a function pointer in
 * gcov_info.merge[] by instrumented code; it is invoked by libgcov's
 * merge-on-read path, which is not used here. A no-op satisfies the
 * reference. */
VOID __gcov_merge_add(gcov_type_t *counters, gcov_unsigned_t n)
{
    (VOID)counters;
    (VOID)n;
}

/* __gcov_exit is called by the GCC-emitted .fini_array destructor of every
 * instrumented translation unit. Bare-metal builds never iterate
 * .fini_array, but the linker still collects the section, so the reference
 * must link. Routing it to the explicit dump keeps libgcov's "dump at
 * exit" semantics should destructors ever be run (idempotent). */
VOID __gcov_exit(VOID)
{
    TEST_GcovDump();
}

/* ============================================================================
 * Public API
 * ========================================================================== */

/* ---- .init_array section bounds, provided by the linker script ----
 * Declared weak: a board whose linker script has no .init_array sections
 * leaves these unresolved to 0, TEST_GcovRunConstructors detects that and
 * becomes a no-op, and the build still links (weak-fallback policy). */
extern VOID (*__init_array_start)(VOID) __attribute__((weak));
extern VOID (*__init_array_end)(VOID) __attribute__((weak));

VOID TEST_GcovRunConstructors(VOID)
{
    VOID (**ctor)(VOID);
    UINT32 intSave;

    if ((&__init_array_start == NULL) || (&__init_array_end == NULL)) {
        return; /* linker script has no .init_array: nothing registered */
    }
    intSave = LOS_IntLock();
    for (ctor = &__init_array_start; ctor < &__init_array_end; ctor++) {
        if (*ctor != NULL) {
            (*ctor)();
        }
    }
    LOS_IntRestore(intSave);
}

/* ---- streaming base64 encoder over the console UART ---- */
static const char g_b64Tab[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

typedef struct {
    unsigned int buf;    /* 3-byte accumulation (low 24 bits used) */
    unsigned int nbits;  /* number of bits pending (0/8/16) */
    unsigned int lineLen;
} B64Ctx;

static void B64PutStr(const char *s, unsigned int len)
{
    UartPuts(s, len, UART_WITH_LOCK);
}

static void B64Push(B64Ctx *c, const unsigned char *p, unsigned int n)
{
    unsigned int i;

    for (i = 0; i < n; i++) {
        c->buf = (c->buf << 8) | p[i];
        c->nbits += 8;
        if (c->nbits == 24) {
            char out[4];
            out[0] = g_b64Tab[(c->buf >> 18) & 0x3f];
            out[1] = g_b64Tab[(c->buf >> 12) & 0x3f];
            out[2] = g_b64Tab[(c->buf >> 6) & 0x3f];
            out[3] = g_b64Tab[c->buf & 0x3f];
            B64PutStr(out, 4);
            c->lineLen += 4;
            if (c->lineLen >= 72) {
                B64PutStr("\n", 1);
                c->lineLen = 0;
            }
            c->buf = 0;
            c->nbits = 0;
        }
    }
}

static void B64Final(B64Ctx *c)
{
    char out[4];

    if (c->nbits == 0) {
        return;
    }
    /* pad whole 6-bit groups for the trailing partial 24-bit group */
    c->buf <<= (24 - c->nbits);
    out[0] = g_b64Tab[(c->buf >> 18) & 0x3f];
    out[1] = g_b64Tab[(c->buf >> 12) & 0x3f];
    out[2] = (c->nbits == 8) ? '=' : g_b64Tab[(c->buf >> 6) & 0x3f];
    out[3] = '='; /* padding: nbits is 8 (==) or 16 (=) here */
    B64PutStr(out, 4);
    c->lineLen += 4;
    c->buf = 0;
    c->nbits = 0;
}

static void B64PushU32(B64Ctx *c, gcov_unsigned_t v)
{
    unsigned char b[4];
    b[0] = (unsigned char)(v & 0xff);
    b[1] = (unsigned char)((v >> 8) & 0xff);
    b[2] = (unsigned char)((v >> 16) & 0xff);
    b[3] = (unsigned char)((v >> 24) & 0xff);
    B64Push(c, b, 4);
}

static void B64PushU64(B64Ctx *c, gcov_type_t v)
{
    unsigned long long uv = (unsigned long long)v;
    unsigned char b[8];
    int i;
    for (i = 0; i < 8; i++) {
        b[i] = (unsigned char)((uv >> (8 * i)) & 0xff);
    }
    B64Push(c, b, 8);
}

/* Local strlen: keeps this file free of libc headers. */
static unsigned int GcovStrLen(const char *s)
{
    unsigned int n = 0;

    while (s[n] != '\0') {
        n++;
    }
    return n;
}

/* ---- dump one gcov_info: filename + base64 payload ---- */
static void GcovDumpOne(struct gcov_info *info)
{
    const char *fn = GCOV_INFO_FILENAME(info);
    gcov_unsigned_t nFn = GCOV_INFO_NFUNCTIONS(info);
    const struct gcov_fn_info *const *fns = GCOV_INFO_FUNCTIONS(info);
    B64Ctx ctx;
    gcov_unsigned_t fi;

    if ((fn == NULL) || (fns == NULL)) {
        return;
    }

    B64PutStr("GCOV_BEGIN ", 11);
    B64PutStr(fn, GcovStrLen(fn));
    B64PutStr("\n", 1);

    ctx.buf = 0;
    ctx.nbits = 0;
    ctx.lineLen = 0;
    B64PutStr("GCOV_DATA\n", 10);

    /* payload: stamp, n_functions, then per-function:
     * ident, lineno_checksum, cfg_checksum, n_ctrs, n_ctrs * gcov_type */
    B64PushU32(&ctx, GCOV_INFO_STAMP(info));
    B64PushU32(&ctx, nFn);
    for (fi = 0; fi < nFn; fi++) {
        const struct gcov_fn_info *fnp = fns[fi];
        gcov_unsigned_t nCt;
        const gcov_type_t *vals;
        gcov_unsigned_t ci;

        if (fnp == NULL) {
            B64PushU32(&ctx, 0);
            B64PushU32(&ctx, 0);
            B64PushU32(&ctx, 0);
            B64PushU32(&ctx, 0);
            continue;
        }
        nCt = GCOV_FN_NCTRS(fnp);
        vals = GCOV_FN_VALUES(fnp);
        B64PushU32(&ctx, GCOV_FN_IDENT(fnp));
        B64PushU32(&ctx, GCOV_FN_LNCK(fnp));
        B64PushU32(&ctx, GCOV_FN_CFGCK(fnp));
        B64PushU32(&ctx, nCt);
        if ((vals != NULL) && (nCt > 0)) {
            for (ci = 0; ci < nCt; ci++) {
                B64PushU64(&ctx, vals[ci]);
            }
        }
    }
    B64Final(&ctx);
    if (ctx.lineLen > 0) {
        B64PutStr("\n", 1);
    }
    B64PutStr("GCOV_END\n", 9);
}

VOID TEST_GcovDump(VOID)
{
    struct gcov_info *info;
    UINT32 intSave;

    intSave = LOS_IntLock();
    if (g_gcovDumped != 0) {
        LOS_IntRestore(intSave);
        return;
    }
    g_gcovDumped = 1;
    info = g_gcovHead;
    LOS_IntRestore(intSave);

    B64PutStr("\nGCOV_DUMP_START\n", 17);
    while (info != NULL) {
        GcovDumpOne(info);
        intSave = LOS_IntLock();
        info = GCOV_INFO_NEXT(info);
        LOS_IntRestore(intSave);
    }
    B64PutStr("GCOV_DUMP_DONE\n", 15);
}
