# 内核代码覆盖率统计操作指南

**开启宏：`LOSCFG_TEST_KERNEL_COVERAGE=y`**
（写在产品的 `kernel_configs/debug.config` 里。两板均已配置：
qemu=`vendor/ohemu/qemu_mini_system_demo`，ws63=`vendor/hihope/ws63_m_testsuits`；
改过宏后需重新编译。）

前置条件：`lcov` 已安装（`sudo apt-get install -y lcov`）。

qemu 与 ws63 流程完全相同，仅脚本与产物路径不同，按下表替换即可。

---

## 操作步骤

### 步骤 1：编译

```bash
# qemu
bash ~/.config/opencode/skills/board-test-runner/scripts/qemu-system-arm/compile.sh
# ws63
bash ~/.config/opencode/skills/board-test-runner/scripts/ws63/compile.sh
```

### 步骤 2：运行测试（自动捕获串口日志）

```bash
# qemu（日志 logs/qemu_test_<时间戳>.log）
bash ~/.config/opencode/skills/board-test-runner/scripts/qemu-system-arm/run.sh
# ws63（日志 logs/ws63_capture_<时间戳>.txt）
bash ~/.config/opencode/skills/board-test-runner/scripts/ws63/run.sh
```

### 步骤 3：生成覆盖率报告

```bash
bash kernel/liteos_m/testsuites/src/gcov/gen_coverage.sh <步骤2的日志文件>
```

一条命令自动完成：重建 .gcda → 按板子匹配 gcov 工具 → lcov → genhtml。
报告输出：`logs/coverage_html/lcov/<板名>/index.html`
（`<板名>` 与中间数据 `<板名>.info` 由脚本自动推导；原始 .gcda 落盘在
`out/<板>/<产品>/obj/**/*.gcda`）。

---

## 检查要点

| 检查 | 方法 |
|---|---|
| 覆盖率宏已生效 | `grep LOSCFG_TEST_KERNEL_COVERAGE out/<板>/<产品>/config.h` 应有定义 |
| dump 正常完成 | 日志末尾有 `GCOV_DUMP_START` ... `GCOV_DUMP_DONE`，中间每组文件一个 `GCOV_BEGIN`/`GCOV_END` 帧 |
| 改了 .h 不生效 | 工具链不跟踪头文件依赖，必须 touch 对应 .c（或全部源文件）后重编 |
| 报告为 0% / 版本报错 | 不要手工指定 `--gcov-tool`，`gen_coverage.sh` 已自动匹配 |

备用工具：`gen_coverage.py`（自研层级 HTML，用法
`python3 gen_coverage.py <日志> [构建目录] [输出目录]`，gcov 工具同样自动匹配）。
