# CMake 指南（本项目）

## 当前项目的 CMake 结构

```
ai-infra-lab/
├── CMakeLists.txt              # 根：项目级配置
└── blockserve/
    ├── CMakeLists.txt          # 子项目：可执行文件 + 测试
    ├── include/blockserve/     # 头文件（PUBLIC 接口）
    ├── src/                    # 源文件
    └── tests/                  # 测试文件
```

### 根 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.20)       # CMake 最低版本
project(ai_infra_lab LANGUAGES CXX)        # 项目名 + 语言

set(CMAKE_CXX_STANDARD 20)                 # 使用 C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON)        # 编译器必须支持 C++20
set(CMAKE_CXX_EXTENSIONS OFF)             # 禁用编译器扩展（保证可移植）

add_subdirectory(blockserve)               # 引入子目录
enable_testing()                           # 开启测试支持（ctest）
```

| 指令 | 作用 |
|---|---|
| `project()` | 声明项目名和语言，设置 `PROJECT_NAME` 等变量 |
| `set()` | 设置 CMake 变量 |
| `add_subdirectory()` | 递归处理子目录的 CMakeLists.txt |
| `enable_testing()` | 启用 `ctest` 命令 |

### blockserve/CMakeLists.txt

```cmake
# 1. 公共对象库 — 源码只编译一次，主程序和测试共享
add_library(blockserve_lib OBJECT
    src/request.cpp
    src/request_queue.cpp
    src/workload_loader.cpp
    src/simulator.cpp
    src/log.cpp
)

target_include_directories(blockserve_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(blockserve_lib PRIVATE
    -Wall -Wextra -Wpedantic
)

# 2. 主可执行文件
add_executable(blockserve src/main.cpp)
target_link_libraries(blockserve PRIVATE blockserve_lib)

# 3. 测试（Catch2 自动下载）
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)

add_executable(blockserve_tests
    tests/test_request.cpp
)
target_link_libraries(blockserve_tests PRIVATE
    blockserve_lib
    Catch2::Catch2WithMain
)
target_include_directories(blockserve_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
catch_discover_tests(blockserve_tests)
```

---

## 关键概念

### 1. OBJECT Library（`add_library(... OBJECT ...)`）

OBJECT library 只编译 `.cpp` 成 `.o`，不链接。然后多个 target（主程序、测试）可以链接同一个 OBJECT library —— 源码只编译一次。

```
src/request.cpp ─┐
src/queue.cpp  ──┤── 编译成 .o ──┬── 链接 → blockserve（主程序）
src/simulator.cpp─┘               └── 链接 → blockserve_tests（测试）
```

没有 OBJECT library 的话，两个 target 都要列一遍源文件，编译两遍。

### 2. FetchContent（自动下载依赖）

```cmake
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1
)
FetchContent_MakeAvailable(Catch2)
```

- 首次 `cmake -B build` 时自动 clone Catch2 到 `build/_deps/`
- 不需要 `brew install`、不需要 `git submodule`
- 只要网络通，CMake 全自动处理

### 3. Catch2::Catch2WithMain vs Catch2::Catch2

| Target | 说明 |
|---|---|
| `Catch2::Catch2WithMain` | 自动提供 `main()` 函数，你只需写 `TEST_CASE` |
| `Catch2::Catch2` | 不提供 main，你需要自己写 |

选 `WithMain` 省事。

### 4. catch_discover_tests

```cmake
catch_discover_tests(blockserve_tests)
```

编译后自动扫描 test 可执行文件，把每个 `TEST_CASE` 注册为独立的 ctest 测试。这样跑 `ctest` 能看到每个用例分别 PASS/FAIL，而不是一个大的二进制 pass/fail。

---

## 常用命令

```bash
# 配置（首次 或 CMakeLists 变更后）
cmake -B build

# 编译
cmake --build build

# 运行主程序
./build/blockserve/blockserve

# 运行所有测试
cd build && ctest

# 运行测试（打印失败详情）
cd build && ctest --output-on-failure

# 运行匹配名称的测试（支持通配符）
cd build && ctest -R "request"

# 查看测试列表（不运行）
cd build && ctest -N

# 清理重配（遇到奇怪问题时）
rm -rf build && cmake -B build
```

---

## 添加新测试文件

1. 在 `blockserve/tests/` 下新建 `test_xxx.cpp`
2. 在 `blockserve/CMakeLists.txt` 的 `add_executable(blockserve_tests ...)` 里加上文件名
3. 重新 `cmake -B build && cmake --build build && cd build && ctest`

## 添加新源文件

1. 在 `blockserve/src/` 下新建 `xxx.cpp`
2. 在 `blockserve/include/blockserve/` 下新建 `xxx.hpp`
3. 在 `blockserve/CMakeLists.txt` 的 `add_library(blockserve_lib OBJECT ...)` 里加上 `src/xxx.cpp`
4. 主程序和测试都会自动链接到新文件

---

## 参考

- CMake 官方教程：https://cmake.org/cmake/help/latest/guide/tutorial/
- Catch2 文档：https://github.com/catchorg/Catch2/blob/devel/docs/tutorial.md
- CMake FetchContent：https://cmake.org/cmake/help/latest/module/FetchContent.html
