#!/usr/bin/env python3
# Copyright (c) 2026-2026 Huawei Device Co., Ltd. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this list of
# conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice, this list
# of conditions and the following disclaimer in the documentation and/or other materials
# provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific prior written
# permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
## ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# Parse a ws63 serial capture log: extract the GCOV_DUMP_START..GCOV_DUMP_DONE
# section, decode the base64-framed per-file payloads emitted by the gcov
# glue (kernel/liteos_m/testsuites/src/gcov/los_gcov.c), and write one
# .gcda per frame next to its .gcno notes so that lcov / gcov can produce a
# coverage report.
#
# Wire format (see los_gcov.c):
#   GCOV_DUMP_START
#   GCOV_BEGIN <filename>          # absolute build-time .gcda path
#   GCOV_DATA
#   <base64 lines...>
#   GCOV_END
#   ... (repeat per translation unit) ...
#   GCOV_DUMP_DONE
#
# Payload: the raw .gcda byte stream that libgcov serialized through the
# --wrap redirected __gcov_write_* primitives. libgcov (version-matched to
# the compiler) owns the format, so the host only base64-decodes and writes
# the bytes verbatim. Logs from the legacy hand-rolled implementation
# (payload = stamp + n_functions + per-function records) are detected by the
# missing "adcg" magic and reconstructed with the legacy builder.

import base64
import os
import struct
import sys

GCOV_DATA_MAGIC = 0x67636461
GCOV_VERSION = 0x4137332A
TAG_FUNCTION = 0x01000000
TAG_COUNTER_ARCS = 0x01A10000


def u32(b, o):
    return struct.unpack_from('<I', b, o)[0]

def i64(b, o):
    return struct.unpack_from('<q', b, o)[0]


def build_gcda_legacy(stamp, functions, version=None):
    """Legacy payload -> .gcda (hand-rolled format, GCC 7.3 validated).

    version: the .gcda version word (GCOV_VERSION encoding, e.g. 0x4137332A
    for GCC 7.3 / 0x42303372 for GCC 10.3). It must match the compiler that
    produced the .gcno, otherwise the gcov tool rejects the file. When not
    given, GCOV_VERSION (GCC 7.3) is used as fallback."""
    out = bytearray()
    out += struct.pack('<I', GCOV_DATA_MAGIC)
    out += struct.pack('<I', version if version is not None else GCOV_VERSION)
    out += struct.pack('<I', stamp)
    for ident, lnck, cfgck, ctrs in functions:
        out += struct.pack('<I', TAG_FUNCTION)
        out += struct.pack('<I', 3)
        out += struct.pack('<III', ident, lnck, cfgck)
        out += struct.pack('<I', TAG_COUNTER_ARCS)
        out += struct.pack('<I', 2 * len(ctrs))
        for c in ctrs:
            out += struct.pack('<q', c)
    out += struct.pack('<I', 0)  # EOF tag
    return bytes(out)


def decode_payload_legacy(blob):
    """Legacy payload: stamp, n_functions, per-fn
    {ident, lnck, cfgck, n_ctrs, n_ctrs * gcov_type}."""
    if len(blob) < 8:
        return None
    stamp = u32(blob, 0)
    nfn = u32(blob, 4)
    off = 8
    fns = []
    for _ in range(nfn):
        if off + 16 > len(blob):
            break
        ident = u32(blob, off)
        lnck = u32(blob, off + 4)
        cfgck = u32(blob, off + 8)
        nctrs = u32(blob, off + 12)
        off += 16
        ctrs = []
        for _ in range(nctrs):
            if off + 8 > len(blob):
                break
            ctrs.append(i64(blob, off))
            off += 8
        fns.append((ident, lnck, cfgck, ctrs))
    return stamp, fns


def payload_to_gcda(blob, version=None):
    """Return .gcda bytes for a decoded payload, handling both the current
    raw-passthrough format and the legacy hand-rolled format."""
    if len(blob) >= 4 and u32(blob, 0) == GCOV_DATA_MAGIC:
        return blob  # current format: raw libgcov stream, write verbatim
    legacy = decode_payload_legacy(blob)
    if legacy is not None:
        stamp, fns = legacy
        return build_gcda_legacy(stamp, fns, version)
    return None


def gcno_version(gcda_path):
    """Read the version word from the .gcno next to the target .gcda.

    The .gcno header is {magic "oncg", version, stamp}; the version word
    uses the same GCOV_VERSION encoding as the .gcda and identifies the
    compiler that instrumented the code (needed because the legacy payload
    itself carries no version). Returns None when no .gcno is found."""
    if not gcda_path.endswith(".gcda"):
        return None
    gcno = gcda_path[:-5] + ".gcno"
    try:
        with open(gcno, 'rb') as f:
            header = f.read(8)
    except OSError:
        return None
    if len(header) < 8 or u32(header, 0) != 0x67636E6F:  # "oncg"
        return None
    return u32(header, 4)


def parse_log(log_path):
    with open(log_path, 'r', errors='replace') as f:
        lines = f.readlines()

    files = {}   # filename -> bytes(gcda)
    in_dump = False
    cur_file = None
    cur_b64 = []

    for raw in lines:
        line = raw.rstrip('\n').rstrip('\r')
        if line == 'GCOV_DUMP_START':
            in_dump = True
            continue
        if line == 'GCOV_DUMP_DONE':
            break
        if not in_dump:
            continue
        if line.startswith('GCOV_BEGIN '):
            cur_file = line[len('GCOV_BEGIN '):].strip()
            cur_b64 = []
            continue
        if line == 'GCOV_DATA':
            continue
        if line == 'GCOV_END':
            if cur_file is not None:
                raw = ''.join(cur_b64)
                # Tolerate a historical device-side base64 padding bug where
                # a 1-byte trailing group was emitted as "XY=A" not "XY==".
                if raw.endswith('=A'):
                    raw = raw[:-1] + '='
                blob = base64.b64decode(raw) if raw else b''
                data = payload_to_gcda(blob, gcno_version(cur_file))
                if data is not None:
                    files[cur_file] = data
            cur_file = None
            cur_b64 = []
            continue
        if cur_file is not None:
            cur_b64.append(line.strip())

    return files


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: parse_gcov_log.py <ws63_capture.log> [out_root]")
    log_path = sys.argv[1]
    out_root = sys.argv[2] if len(sys.argv) > 2 else None

    files = parse_log(log_path)
    if not files:
        sys.exit("no GCOV data found in log (is LOSCFG_KERNEL_COVERAGE=y and "
                 "did the test reach '--- Test End ---'?)")

    n_written = 0
    n_skip = 0
    for fn, data in files.items():
        path = fn
        if out_root:
            # remap leading build root onto out_root if desired
            if path.startswith('/'):
                path = os.path.join(out_root, path.lstrip('/'))
        d = os.path.dirname(path)
        try:
            os.makedirs(d, exist_ok=True)
            with open(path, 'wb') as f:
                f.write(data)
            n_written += 1
        except OSError as e:
            print("skip %s: %s" % (path, e), file=sys.stderr)
            n_skip += 1

    print("reconstructed %d .gcda files (%d skipped)" % (n_written, n_skip))


if __name__ == '__main__':
    main()
