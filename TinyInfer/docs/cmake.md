# CMake 快捷指令参考

> **TinyInfer 项目编译 / 运行 / 测试 / 清理 一站速查。**

---

## 目录

| 操作 | 跳转 |
|------|------|
| [一、首次配置与编译](#一首次配置与编译) | 一行命令从零到可运行 |
| [二、各 Preset 启动命令](#二各-preset-启动命令) | debug / release / asan / tsan |
| [三、增量编译](#三增量编译) | 改代码后快速重新编译 |
| [四、运行测试](#四运行测试) | 全部测试 / 单个测试 / 过滤 |
| [五、运行 CLI](#五运行-cli) | CLI 入口命令 |
| [六、清理](#六清理) | 完全清理 / 部分清理 |
| [七、Sanitizer 专项](#七sanitizer-专项) | ASan / UBSan / TSan |
| [八、clang-format / clang-tidy](#八clang-format--clang-tidy) | 格式化和静态检查 |
| [九、常见问题速查](#九常见问题速查) | 报错排查 |
| [十、一键脚本](#十一键脚本) | 复制即用的脚本 |

---

## 一、首次配置与编译

```bash
# 从项目根目录开始（TinyInfer/tinyinfer/）

# 步骤 1：进入 tinyinfer 目录
cd TinyInfer/tinyinfer

# 步骤 2：CMake 配置（首次，会自动下载 GoogleTest）
cmake --preset debug-core -B build/debug-core

# 步骤 3：编译（-j 按 CPU 核数并行）
cmake --build build/debug-core -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# 步骤 4：验证
./build/debug-core/tinyinfer_cli --version
# 输出: tinyinfer version 0.1.0 (release)
```

---

## 二、各 Preset 启动命令

```bash
# ── 日常开发（最常用）──
cmake --preset debug-core -B build/debug-core
cmake --build build/debug-core -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# ── Release 构建 ──
cmake --preset release-core -B build/release-core
cmake --build build/release-core -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# ── ASan + UBSan（内存 + 未定义行为检测）──
cmake --preset asan-core -B build/asan-core
cmake --build build/asan-core -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# ── TSan（线程竞态检测）──
cmake --preset tsan-mock -B build/tsan-mock
cmake --build build/tsan-mock -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# ── 含 LibTorch 的构建（后阶段使用）──
cmake --preset debug-cpu-torch -B build/debug-cpu-torch
cmake --build build/debug-cpu-torch -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)
```

---

## 三、增量编译

改代码后不需要重新 `cmake --preset`，直接：

```bash
# 增量编译（只重编译改动的文件）
cmake --build build/debug-core -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# 如果新增了 .cpp 文件 → 需要重新配置
cmake --preset debug-core -B build/debug-core
cmake --build build/debug-core -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)

# 如果改了 CMakeLists.txt → 同样重新配置
```

**判断规则：**
```
只改 .h / .cpp 内容          → cmake --build 即可
新增/删除/重命名 .cpp 文件    → 重新 cmake --preset
改了 CMakeLists.txt          → 重新 cmake --preset
改了 Dependencies.cmake 等   → 重新 cmake --preset
```

---

## 四、运行测试

```bash
# 运行全部测试
ctest --test-dir build/debug-core --output-on-failure

# 只运行 Common 模块测试
ctest --test-dir build/debug-core -R "common/" --output-on-failure

# 只运行某个具体测试
ctest --test-dir build/debug-core -R "ErrorTest" --output-on-failure

# 详细输出（调试时用）
ctest --test-dir build/debug-core --output-on-failure -V

# 直接运行可执行文件（调试单个测试更方便）
./build/debug-core/tests/tinyinfer_test_common

# GoogleTest 参数过滤
./build/debug-core/tests/tinyinfer_test_common --gtest_filter="*Error*"
```

---

## 五、运行 CLI

```bash
# 版本
./build/debug-core/tinyinfer_cli --version
# 输出: tinyinfer version 0.1.0 (release)

# 帮助
./build/debug-core/tinyinfer_cli --help

# Release 构建
./build/release-core/tinyinfer_cli --version
```

---

## 六、清理

```bash
# 完全清理一个构建目录
rm -rf build/debug-core

# 清理所有构建产物
rm -rf build/

# 只清理编译产物，保留 CMake 缓存（下次构建更快）
cmake --build build/debug-core --target clean

# 重新来过（最彻底）
rm -rf build/
cmake --preset debug-core -B build/debug-core
cmake --build build/debug-core -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)
```

---

## 七、Sanitizer 专项

```bash
# ── ASan + UBSan 构建与测试 ──
cmake --preset asan-core -B build/asan-core
cmake --build build/asan-core -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)
ctest --test-dir build/asan-core --output-on-failure

# ── TSan 构建与测试（检测线程竞态）──
cmake --preset tsan-mock -B build/tsan-mock
cmake --build build/tsan-mock -j$(sysctl -n hw.ncpu 2>/dev/null || nproc)
ctest --test-dir build/tsan-mock --output-on-failure

# ── ASan 环境变量（按需）──
ASAN_OPTIONS=detect_leaks=1:abort_on_error=1 \
  ctest --test-dir build/asan-core --output-on-failure
```

---

## 八、clang-format / clang-tidy

```bash
# ── 格式化单个文件 ──
clang-format -i path/to/file.h

# ── 格式化所有 .h/.cpp ──
find . -name '*.h' -o -name '*.cpp' | xargs clang-format -i

# ── 检查格式（不改文件，CI 用）──
find . -name '*.h' -o -name '*.cpp' | xargs clang-format --dry-run -Werror

# ── clang-tidy 静态检查 ──
# 需要先 cmake 生成 compile_commands.json
cmake --preset debug-core -B build/debug-core
clang-tidy -p build/debug-core path/to/file.cpp
```

---

## 九、常见问题速查

| 问题 | 原因 | 解决 |
|------|------|------|
| `cmake: command not found` | 未安装 CMake | `brew install cmake` |
| `CMake Error: Could not find preset` | 不在正确的目录 | `cd TinyInfer/tinyinfer` |
| `No rule to make target` | 新增了 .cpp 但未重新配置 | 重新 `cmake --preset` |
| `undefined reference to` | 链接库缺失 | 检查 `target_link_libraries` |
| `could not find GoogleTest` | 网络问题 | 检查网络，重试 `cmake --preset` |
| `ld: library not found for -lgtest` | 缓存问题 | `rm -rf build/` 重来 |
| `AddressSanitizer: heap-use-after-free` | ASan 发现内存 bug | 查看 ASan 报告中源码位置 |
| `ThreadSanitizer: data race` | TSan 发现竞态 | 查看 TSan 报告中两个访问位置 |

---

## 十、一键脚本

以下脚本可存放为 `scripts/build.sh`，一键配置 + 编译 + 测试：

```bash
#!/bin/bash
set -euo pipefail

# TinyInfer 一键构建与测试
cd "$(dirname "$0")/../TinyInfer/tinyinfer"

PRESET="${1:-debug-core}"
BUILD_DIR="build/${PRESET}"

echo "=== 配置 (preset: $PRESET) ==="
cmake --preset "$PRESET" -B "$BUILD_DIR"

echo "=== 编译 ==="
NPROC=$(sysctl -n hw.ncpu 2>/dev/null || nproc)
cmake --build "$BUILD_DIR" -j"$NPROC"

echo "=== 运行测试 ==="
ctest --test-dir "$BUILD_DIR" --output-on-failure

echo "=== 完成 ==="
"$BUILD_DIR/tinyinfer_cli" --version
```

使用：

```bash
chmod +x scripts/build.sh

./scripts/build.sh                 # 默认 debug-core
./scripts/build.sh release-core    # Release 构建
./scripts/build.sh asan-core       # Sanitizer 构建
```

---

## 附录：构建产物结构

```
TinyInfer/tinyinfer/build/
└── debug-core/                  ← 每个 preset 对应一个子目录
    ├── CMakeCache.txt           ← CMake 缓存
    ├── compile_commands.json    ← clang-tidy 需要
    ├── tinyinfer_cli            ← CLI 可执行文件
    ├── tests/
    │   └── tinyinfer_test_common  ← 测试可执行文件
    └── _deps/                   ← FetchContent 依赖
```
