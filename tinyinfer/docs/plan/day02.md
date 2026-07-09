# Day 2｜2026-07-10｜基础类型与 CLI

## 主题

实现 C++ Runtime 最底层的基础类型和 CLI 入口。

## 任务

1. 实现 `Status` 类（成功/失败状态，不用异常）
2. 实现 `DataType` 枚举 + `dtype_size()`
3. 实现 `Shape` 结构体（形状描述、元素数计算）
4. 实现 `CliArgs` 结构体 + `parse_cli_args()` + `print_usage()`
5. 实现 `cli_main.cpp` 入口：解析参数并打印

## 产出

- `include/tinyinfer/status.hpp` — Status 类
- `src/status.cpp` — 实现
- `include/tinyinfer/tensor.hpp` — DataType + Shape
- `src/tensor.cpp` — 实现
- `include/tinyinfer/cli.hpp` — CliArgs + CLI 函数
- `src/cli.cpp` — CLI 解析实现
- `src/cli_main.cpp` — 入口
- `tests/test_tensor.cpp` — 7 个测试
- `CMakeLists.txt` — 编译 target 全部配好

## 验收

```bash
cmake -B build && cmake --build build
./build/tinyinfer_cli --help
./build/tinyinfer_cli --model models/debug_model --input-ids "1,2,3"
cd build && ctest  # 100% passed
```
