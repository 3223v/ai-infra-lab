# Day 5｜2026-07-13｜CPU Tensor 与权重加载

## 主题

实现 C++ 侧 Tensor 类和从 weights.bin 加载权重。

## 任务

1. `include/tinyinfer/tensor.hpp` — 添加 `class Tensor`
   - 成员：dtype / shape / `std::vector<uint8_t>` 数据
   - 方法：`data<T>()` 返回类型化指针、`print_first(n)` 打印前 n 个值
2. `src/tensor.cpp` — 实现 Tensor
3. 实现权重加载函数：根据 TensorEntry 的 offset/size 从 weights.bin 读取
4. CLI `--model` 加载后打印前几个 tensor 的前 5 个值
5. 对比 Python 导出值，验证一致

## 验收

```bash
cmake --build build
./build/tinyinfer_cli --model models/debug_model
# 输出 tensor 列表 + 每个 tensor 前几个 float 值
# 与 Python export_debug_model.py 的打印结果一致
```
