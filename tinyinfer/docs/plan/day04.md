# Day 4｜2026-07-12｜自定义权重格式 v0

## 主题

设计 TinyInfer 自定义权重格式（model.json + weights.bin），用极小 debug 模型跑通 Python→C++ 数据通道。

## 任务

### 1. Python 侧：导出 debug model

文件：`tools/export_debug_model.py`

- 用 PyTorch 构建微型 GPT（hidden=64, layers=2, heads=4, vocab=256）
- `torch.manual_seed(42)` 固定随机权重
- 生成 `models/debug_model/model.json`：结构参数 + tensor 索引
- 生成 `models/debug_model/weights.bin`：raw fp32 bytes 拼接

### 2. C++ 侧：模型配置定义

文件：`include/tinyinfer/model_config.hpp`

- `struct TensorEntry`：name / offset / size / shape
- `struct ModelConfig`：model_type / hidden_size / num_layers / vocab_size / dtype / tensors
- `ModelConfig load_model_config(const std::string& json_path)`

### 3. C++ 侧：JSON 解析

文件：`src/model_config.cpp`

- 手写简单 JSON 解析器（不引入第三方库）
- 辅助函数：skip_whitespace / read_string / read_int / expect_char
- `load_model_config()` 实现

### 4. CLI 接入

文件：`src/cli_main.cpp`

- `--model` 参数实际调用 `load_model_config()`
- 打印：模型类型、层数、hidden_size、tensor 数量

### 5. 测试

文件：`tests/test_model_config.cpp`

- 加载 debug model 的 model.json
- 验证字段值匹配
- 验证 tensor 数量和指定 tensor 的 shape

## 验收

```bash
python tools/export_debug_model.py                    # 生成 debug model
cmake --build build
./build/tinyinfer_cli --model models/debug_model      # C++ 加载并打印
cd build && ctest --output-on-failure                 # 全绿
```

## 产出

- `tools/export_debug_model.py` — debug 模型导出脚本
- `include/tinyinfer/model_config.hpp` — 模型配置定义
- `src/model_config.cpp` — JSON 解析 + 模型加载
- `tests/test_model_config.cpp` — 配置加载测试
- `models/debug_model/model.json` — 示例模型配置（生成）
- `models/debug_model/weights.bin` — 示例权重（生成）
