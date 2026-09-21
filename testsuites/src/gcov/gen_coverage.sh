#!/usr/bin/env bash
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
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Description: End-to-end coverage report generator using the standard
#              lcov + genhtml (qemu and ws63, one entry point).
#
# Usage:       gen_coverage.sh <capture.log> [html_out_dir] [info_out_path]
#

set -euo pipefail

if [ $# -lt 1 ]; then
    echo "usage: $0 <capture.log> [html_out_dir] [info_out_path]" >&2
    exit 1
fi

LOG_PATH=$(readlink -f "$1")
HERE=$(cd "$(dirname "$0")" && pwd)
REPO_ROOT=$(cd "$HERE/../../../../.." && pwd)  # testsuites/src/gcov -> morpheus root

# Version-matched host gcov tools (auto-selected in step 2)
# RV_GCOV is inside the repo; ARM_GCOV is on the system PATH (outside repo)
RV_GCOV="$REPO_ROOT/device/soc/hisilicon/ws63v100/sdk/tools/bin/compiler/riscv/cc_riscv32_musl_100/cc_riscv32_musl/bin/riscv32-linux-musl-gcov"
ARM_GCOV="$(command -v arm-none-eabi-gcov || echo "$REPO_ROOT/gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcov")"

echo "=== step 1: reconstruct .gcda from serial log ==="
python3 "$HERE/parse_gcov_log.py" "$LOG_PATH"

# Locate the build dir from the first GCOV_BEGIN path (the .gcda path baked
# at compile time: <build_dir>/obj/<...>/<name>.gcda)
FIRST_GCDA=$(grep -m1 '^GCOV_BEGIN ' "$LOG_PATH" | sed 's/^GCOV_BEGIN //;s/\r$//')
if [ -z "$FIRST_GCDA" ]; then
    echo "no GCOV_BEGIN frame in $LOG_PATH" >&2
    exit 1
fi
case "$FIRST_GCDA" in
    */obj/*) BUILD_DIR="${FIRST_GCDA%%/obj/*}" ;;
    *)       BUILD_DIR=$(dirname "$FIRST_GCDA") ;;
esac

# Read the .gcno version word (LE u32 at offset 4) and pick the gcov tool
GCNO="${FIRST_GCDA%.gcda}.gcno"
GCOV_VER=$(od -An -tx4 -j4 -N4 "$GCNO" 2>/dev/null | tr -d ' \n')
case "$GCOV_VER" in
    4137332a) GCOV_TOOL="$RV_GCOV"; TOOL_DESC="riscv32-linux-musl-gcov 7.3" ;;
    42303372) GCOV_TOOL="$ARM_GCOV"; TOOL_DESC="arm-none-eabi-gcov 10.3.1" ;;
    *) echo "unknown .gcno version word: '$GCOV_VER' ($GCNO)" >&2; exit 1 ;;
esac

# Board name for default output paths
case "$BUILD_DIR" in
    *arm_mps2_an386*) BOARD=qemu ;;
    *ws63_m*)         BOARD=ws63 ;;
    *)                BOARD=$(basename "$BUILD_DIR") ;;
esac
HTML_DIR=${2:-"$REPO_ROOT/logs/coverage_html/lcov/$BOARD"}
INFO_OUT=${3:-"$REPO_ROOT/logs/coverage_html/lcov/$BOARD.info"}
mkdir -p "$(dirname "$INFO_OUT")"

if [ ! -x "$GCOV_TOOL" ]; then
    echo "gcov tool not found/executable: $GCOV_TOOL" >&2
    exit 1
fi

echo "=== step 2: lcov capture ($TOOL_DESC, board=$BOARD) ==="
echo "    build dir: $BUILD_DIR"
lcov --capture \
     --gcov-tool "$GCOV_TOOL" \
     --directory "$BUILD_DIR" \
     --output-file "$INFO_OUT" \
     --rc derive_function_end_line=0 \
     --rc lcov_branch_coverage=1 \
     --ignore-errors mismatch,inconsistent,unused,empty,range,negative \
     2>&1 | tail -8

echo "=== step 3: genhtml ==="
mkdir -p "$HTML_DIR"
genhtml "$INFO_OUT" \
        --output-directory "$HTML_DIR" \
        --title "$BOARD coverage" \
        --branch-coverage \
        --ignore-errors range,inconsistent,corrupt,unsupported \
        2>&1 | tail -8

echo ""
echo "=== done ==="
echo "info file: $INFO_OUT"
echo "HTML report: $HTML_DIR/index.html"
