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
#
# End-to-end ws63 coverage report generator (hierarchical, lcov-style).
#
#   1. Parse a ws63 serial capture log -> reconstruct .gcda files next to the
#      matching .gcno notes (delegated to parse_gcov_log).
#   2. For each reconstructed .gcda, run the version-matched
#      riscv32-linux-musl-gcov (7.3) to produce .gcov text reports.
#   3. Parse each .gcov into per-line hit/miss/branch records.
#   4. Emit a browsable HTML tree:
#        <html_out>/index.html              overall + top-level dirs/files
#        <html_out>/<dir>/index.html        per-directory summary, clickable
#        <html_out>/<dir>/<file>.html       source view, line-level coloring
#
# Usage:
#   gen_coverage.py <ws63_capture.log> [build_root] [html_out_dir]
#
# Defaults:
#   build_root  = autodetected from the first .gcda path
#   html_out_dir= ./ws63_coverage_html

import glob
import os
import re
import shutil
import struct
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
# testsuites/src/gcov -> morpheus root
REPO_ROOT = os.path.abspath(os.path.join(HERE, "..", "..", "..", "..", ".."))
# RV_GCOV is inside the repo (relative to REPO_ROOT); ARM_GCOV is on PATH
RV_GCOV = os.path.join(REPO_ROOT, "device/soc/hisilicon/ws63v100/sdk/tools/bin"
                       "/compiler/riscv/cc_riscv32_musl_100/cc_riscv32_musl/bin"
                       "/riscv32-linux-musl-gcov")
ARM_GCOV = shutil.which("arm-none-eabi-gcov") or os.path.join(
    REPO_ROOT, "gcc-arm-none-eabi-10.3-2021.10/bin/arm-none-eabi-gcov")

# gcov version words (GCOV_VERSION encoding) -> matching host gcov tool.
# The word is read from the .gcno next to each .gcda, so one entry point
# serves both toolchains: ws63 (riscv32-linux-musl-gcc 7.3) and qemu arm
# ca7 (arm-none-eabi-gcc 10.3.1).
GCOV_VERSION_73 = 0x4137332A   # "A73*"
GCOV_VERSION_103 = 0x42303372  # "B03r"


def gcov_tool_for(gcda_path):
    """Return the version-matched host gcov for a reconstructed .gcda."""
    try:
        with open(gcda_path[:-5] + ".gcno", "rb") as f:
            header = f.read(8)
    except OSError:
        return RV_GCOV
    if len(header) >= 8:
        version = struct.unpack_from("<I", header, 4)[0]
        if version == GCOV_VERSION_103:
            return ARM_GCOV
    return RV_GCOV

LINE_RE = re.compile(r"Lines executed:(?P<v>[\d.]+)% of (?P<n>\d+)")
FILE_RE = re.compile(r"File '(?P<f>[^']+)'")
BRANCH_RE = re.compile(r"Branches executed:(?P<v>[\d.]+)% of (?P<n>\d+)")


def run_parse(log_path, build_root):
    """Write .gcda files in place at the absolute paths the device emitted
    (next to the matching .gcno); return the written .gcda paths."""
    cmd = [sys.executable, os.path.join(HERE, "parse_gcov_log.py"), log_path]
    subprocess.check_call(cmd)
    root = build_root or "/"
    out = []
    for p in glob.glob(os.path.join(root, "**", "*.gcda"), recursive=True):
        out.append(p)
    return out


def run_gcov(gcda_path):
    """Run the version-matched host gcov; return
    (src_rel, lp, ln, bp, bn, gcov_path) or None.

    gcov resolves the per-TU source path recorded in the .gcno (a path
    relative to the GCC compile cwd) relative to ITS OWN cwd. GN compiled
    every TU from <build_root> (== out/<board>/<product>, the dir
    containing the obj/ tree), so we must run gcov from there for the
    relative source paths to resolve and produce an annotated .gcov. The
    .gcda is found next to the .gcno (passed relative to build_root)."""
    gcov = gcov_tool_for(gcda_path)
    gcno = gcda_path[:-5] + ".gcno"
    if not os.path.exists(gcno):
        return None
    idx = gcda_path.find("/obj/")
    build_root = gcda_path[:idx] if idx != -1 else os.path.dirname(gcda_path)
    gcno_rel = os.path.relpath(gcno, build_root)
    try:
        r = subprocess.run([gcov, "-b", "-c", gcno_rel],
                           cwd=build_root, capture_output=True, text=True,
                           timeout=60)
    except subprocess.TimeoutExpired:
        return None
    out = r.stdout + r.stderr
    fm = FILE_RE.search(out)
    lm = LINE_RE.search(out)
    bm = BRANCH_RE.search(out)
    if not (fm and lm):
        return None
    src = fm.group("f")
    lp = float(lm.group("v"))
    ln = int(lm.group("n"))
    bp = float(bm.group("v")) if bm else None
    bn = int(bm.group("n")) if bm else 0
    gcov_path = os.path.join(build_root, os.path.basename(src) + ".gcov")
    return (src, lp, ln, bp, bn, gcov_path)


def normalize_src(src):
    """Strip leading ../ sequences so the path is relative to the repo root,
    giving a clean tree (e.g. kernel/liteos_m/.../los_task.c)."""
    while src.startswith("../"):
        src = src[3:]
    return src


def html_escape(s):
    return (s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
             .replace("'", "&#39;").replace('"', "&quot;"))


# ---- .gcov text report parsing ----
GCOV_LINE_RE = re.compile(r"^\s*(.*?)\s*:\s*(\d+)\s*:\s?(.*)$")


def parse_gcov_file(gcov_path):
    """Parse a .gcov text report into a list of source-line records.

    Each record: (count_token, line_no, source_text, annotations)
      count_token: '-' | '#####' | '=====' | '<integer>'
      annotations: list of str (branch/call lines that follow this source line)
    """
    records = []
    cur = None
    try:
        with open(gcov_path, "r", errors="replace") as f:
            for raw in f:
                line = raw.rstrip("\n")
                m = GCOV_LINE_RE.match(line)
                if m:
                    tok, no, src = m.group(1), int(m.group(2)), m.group(3)
                    if no == 0 and src.startswith(("Source:", "Graph:", "Data:",
                                                    "Runs:", "Programs:")):
                        continue  # gcov header metadata
                    cur = [tok, no, src, []]
                    records.append(cur)
                else:
                    # annotation line: 'function ...', 'call ...', 'branch ...'
                    s = line.strip()
                    if s and cur is not None:
                        cur[3].append(s)
    except OSError:
        return []
    return records


def count_value(tok):
    """Return (hit_count:int|None, is_code:bool)."""
    if tok == "-":
        return None, False
    if tok in ("#####", "====="):
        return 0, True
    try:
        return int(tok), True
    except ValueError:
        return None, False


# ---- directory tree ----
class Node:
    __slots__ = ("dirs", "files", "lines_total", "lines_hit",
                 "branch_total", "branch_hit")

    def __init__(self):
        self.dirs = {}   # name -> Node
        self.files = []  # list of file row dicts
        self.lines_total = 0
        self.lines_hit = 0
        self.branch_total = 0
        self.branch_hit = 0


def build_tree(rows):
    """rows: list of (src_rel, lp, ln, bp, bn, gcov_path). Returns root Node."""
    root = Node()
    for src_rel, lp, ln, bp, bn, gcov_path in rows:
        parts = [p for p in src_rel.split("/") if p]
        node = root
        for d in parts[:-1]:
            node = node.dirs.setdefault(d, Node())
        hit = int(round(ln * lp / 100.0))
        node.files.append({
            "name": parts[-1],
            "src_rel": src_rel,
            "lp": lp,
            "ln": ln,
            "hit": hit,
            "bp": bp,
            "bn": bn,
            "gcov_path": gcov_path,
        })
        node.lines_total += ln
        node.lines_hit += hit
        if bp is not None and bn:
            node.branch_total += bn
            node.branch_hit += int(round(bn * bp / 100.0))
    # aggregate up the tree
    _aggregate(root)
    return root


def _aggregate(node):
    for child in node.dirs.values():
        _aggregate(child)
        node.lines_total += child.lines_total
        node.lines_hit += child.lines_hit
        node.branch_total += child.branch_total
        node.branch_hit += child.branch_hit


def pct_color(p):
    if p >= 80:
        return "h"
    if p >= 40:
        return "m"
    return "l"


CSS = """
body{font-family:monospace;margin:1em}
table{border-collapse:collapse;width:100%}
td,th{border:1px solid #ccc;padding:2px 8px;text-align:right;white-space:nowrap}
td.f,td.s{text-align:left;white-space:normal}
td.s{width:100%}
td.c{width:80px}
td.n{width:50px}
.h{background:#e8f5e9}.m{background:#fff8e1}.l{background:#ffebee}
.hit{color:#1b5e20;font-weight:bold}
.miss{color:#b71c1c;font-weight:bold}
.nocode{color:#999}
.annot{color:#666;font-size:90%;padding-left:24px}
a{text-decoration:none}
a:hover{text-decoration:underline}
.crumb{font-size:110%;margin-bottom:1em}
.bar{height:14px;display:inline-block;vertical-align:middle;border:1px solid #999}
.bar span{display:inline-block;height:14px}
"""


def render_header(title, crumb_html):
    return ("<!doctype html><html><head><meta charset='utf-8'>"
            "<title>%s</title><style>%s</style></head><body>"
            "<div class='crumb'>%s</div>"
            % (html_escape(title), CSS, crumb_html))


def render_footer():
    return "</body></html>"


def crumb_html_for(rel_dir):
    """rel_dir: '' for root, 'a/b' for subdir. Returns breadcrumb HTML with
    RELATIVE paths from the current directory (so 'a/b/index.html' links to
    its ancestors via '../../index.html', not root-relative 'a/index.html'
    which a browser would mis-resolve against the current page's dir).

    For an ancestor at depth i (0-based) when the current depth is D, the
    href is: (D-i-1) levels of '../' + ancestor_path + '/index.html'."""
    parts = [p for p in rel_dir.split("/") if p]
    depth = len(parts)
    crumbs = ['<a href="%sindex.html">root</a>' % ("../" * depth)]
    cur = ""
    for i, p in enumerate(parts):
        cur = (cur + "/" + p) if cur else p
        levels_up = depth - i - 1
        href = ("../" * levels_up) + cur + "/index.html"
        crumbs.append(' / <a href="%s">%s</a>' % (href, html_escape(p)))
    return "".join(crumbs)


def pct_str(num, den):
    return ("%.2f%%" % (100.0 * num / den)) if den else "-"


def render_index_page(node, rel_dir, html_dir):
    """Write index.html for one directory level."""
    out_path = os.path.join(html_dir, rel_dir) if rel_dir else html_dir
    os.makedirs(out_path, exist_ok=True)
    crumb = crumb_html_for(rel_dir)
    title = "coverage: %s" % (rel_dir or "root")

    parts = [render_header(title, crumb)]
    parts.append("<h1>%s</h1>" % html_escape(rel_dir or "coverage"))
    parts.append("<p>line coverage: <b>%s</b> (%d / %d)"
                 % (pct_str(node.lines_hit, node.lines_total),
                    node.lines_hit, node.lines_total))
    if node.branch_total:
        parts.append("&nbsp;&nbsp;branch coverage: <b>%s</b> (%d / %d)</p>"
                     % (pct_str(node.branch_hit, node.branch_total),
                        node.branch_hit, node.branch_total))
    else:
        parts.append("</p>")

    parts.append("<table><tr><th class='f'>name</th>"
                 "<th>lines %</th><th>lines hit/total</th>"
                 "<th>branches %</th><th>branches hit/total</th></tr>")
    # subdirectories
    for name in sorted(node.dirs):
        child = node.dirs[name]
        lp = pct_str(child.lines_hit, child.lines_total)
        bp = pct_str(child.branch_hit, child.branch_total)
        cls = pct_color((100.0 * child.lines_hit / child.lines_total)
                        if child.lines_total else 0)
        parts.append(
            "<tr class='%s'><td class='f'><a href='%s/index.html'>%s/</a></td>"
            "<td>%s</td><td>%d/%d</td><td>%s</td><td>%d/%d</td></tr>"
            % (cls, html_escape(name), html_escape(name),
               lp, child.lines_hit, child.lines_total,
               bp, child.branch_hit, child.branch_total))
    # files
    for fr in sorted(node.files, key=lambda x: x["name"]):
        lp = "%.2f%%" % fr["lp"]
        bp = ("%.2f%%" % fr["bp"]) if (fr["bp"] is not None) else "-"
        cls = pct_color(fr["lp"])
        link = html_escape(fr["name"] + ".html")
        parts.append(
            "<tr class='%s'><td class='f'><a href='%s'>%s</a></td>"
            "<td>%s</td><td>%d/%d</td><td>%s</td><td>%s</td></tr>"
            % (cls, link, html_escape(fr["name"]),
               lp, fr["hit"], fr["ln"],
               bp, ("%d/%d" % (int(round(fr["bn"] * fr["bp"] / 100.0)),
                               fr["bn"])) if (fr["bp"] is not None and fr["bn"])
               else "-"))
    parts.append("</table>")
    parts.append(render_footer())
    with open(os.path.join(out_path, "index.html"), "w") as f:
        f.write("".join(parts))


def render_file_page(fr, rel_dir, html_dir):
    """Write the per-file source-annotated HTML."""
    out_path = os.path.join(html_dir, rel_dir) if rel_dir else html_dir
    os.makedirs(out_path, exist_ok=True)
    records = parse_gcov_file(fr["gcov_path"])
    crumb = crumb_html_for(rel_dir)
    title = fr["src_rel"]

    parts = [render_header(title, crumb)]
    parts.append("<h1>%s</h1>" % html_escape(fr["src_rel"]))
    parts.append("<p>line coverage: <b>%.2f%%</b> (%d / %d)"
                 % (fr["lp"], fr["hit"], fr["ln"]))
    if fr["bp"] is not None and fr["bn"]:
        parts.append("&nbsp;&nbsp;branch coverage: <b>%.2f%%</b> (%d / %d)</p>"
                     % (fr["bp"], int(round(fr["bn"] * fr["bp"] / 100.0)),
                        fr["bn"]))
    else:
        parts.append("</p>")
    parts.append("<table><tr><th class='c'>count</th><th class='n'>line</th>"
                 "<th class='s'>source</th></tr>")
    for tok, no, src, annots in records:
        cnt, is_code = count_value(tok)
        if cnt is None:
            parts.append("<tr><td class='c nocode'>-</td><td class='n'>%d</td>"
                         "<td class='s nocode'>%s</td></tr>"
                         % (no, html_escape(src)))
        elif cnt == 0:
            parts.append("<tr class='l'><td class='c miss'>#####</td>"
                         "<td class='n'>%d</td><td class='s'>%s</td></tr>"
                         % (no, html_escape(src)))
        else:
            parts.append("<tr class='h'><td class='c hit'>%d</td>"
                         "<td class='n'>%d</td><td class='s'>%s</td></tr>"
                         % (cnt, no, html_escape(src)))
        for a in annots:
            parts.append("<tr><td class='c'></td><td class='n'></td>"
                         "<td class='s annot'>%s</td></tr>"
                         % (html_escape(a),))
    parts.append("</table>")
    parts.append(render_footer())
    with open(os.path.join(out_path, fr["name"] + ".html"), "w") as f:
        f.write("".join(parts))


def render_tree(node, rel_dir, html_dir):
    render_index_page(node, rel_dir, html_dir)
    for name, child in node.dirs.items():
        child_rel = (rel_dir + "/" + name) if rel_dir else name
        render_tree(child, child_rel, html_dir)
    for fr in node.files:
        render_file_page(fr, rel_dir, html_dir)


def main():
    if len(sys.argv) < 2:
        sys.exit("usage: gen_coverage.py <ws63_capture.log> [build_root] "
                 "[html_out_dir]")
    log_path = os.path.abspath(sys.argv[1])
    build_root = sys.argv[2] if len(sys.argv) > 2 else None
    html_dir = sys.argv[3] if len(sys.argv) > 3 else "ws63_coverage_html"

    gcda_paths = run_parse(log_path, build_root)
    if not gcda_paths:
        sys.exit("no .gcda reconstructed from log")

    rows = []
    ok = 0
    fail = 0
    for p in gcda_paths:
        res = run_gcov(p)
        if res is None:
            fail += 1
            continue
        src, lp, ln, bp, bn, gcov_path = res
        rows.append((normalize_src(src), lp, ln, bp, bn, gcov_path))
        ok += 1

    root = build_tree(rows)
    render_tree(root, "", html_dir)

    print("gcov processed %d files (%d failed)" % (ok, fail))
    if root.lines_total:
        print("overall line coverage: %.2f%% (%d/%d)"
              % (100.0 * root.lines_hit / root.lines_total,
                 root.lines_hit, root.lines_total))
    print("HTML report: %s/index.html" % os.path.abspath(html_dir))


if __name__ == '__main__':
    main()
