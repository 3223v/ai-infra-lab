# TinyInfer

轻量级 LLM 推理 Runtime · C++20 · CPU → CUDA

目标：在 RTX 3050 4GB 显存上跑通 1B 以下开源 decoder-only 模型。

## 当前状态

Week 1 · 工程骨架搭建中

## 构建

```bash
cmake -B build
cmake --build build
```

## 运行

```bash
./build/tinyinfer_cli --help
```

## 测试

```bash
cd build && ctest
```

## 目录

```
tinyinfer/
├── CMakeLists.txt              # 构建配置
├── README.md                   # 本文件
├── docs/
│   ├── plan.md                 # 8 周完整计划
│   └── current_scope.md        # 当前阶段目标
├── include/tinyinfer/          # 公开头文件
│   ├── tensor.hpp              # DataType / Shape / Tensor
│   ├── status.hpp              # 错误处理
│   ├── model_config.hpp        # 模型配置加载
│   └── cli.hpp                 # 命令行参数
├── src/                        # 实现
│   ├── cli_main.cpp            # 入口
│   ├── tensor.cpp
│   ├── status.cpp
│   └── model_config.cpp
├── tools/                      # Python 辅助脚本
│   ├── inspect_hf_model.py     # 打印模型结构
│   ├── export_debug_model.py   # 导出 debug 模型
│   ├── reference_forward.py    # Python 参考前向
│   └── export_hf_to_tinyinfer.py # HF → TinyInfer 转换
├── models/                     # 模型文件（.gitignore）
└── tests/                      # 测试
    ├── test_tensor.cpp
    └── test_model_config.cpp
```

## 参考资料

- [llama.cpp](https://github.com/ggml-org/llama.cpp)
- [Qwen2.5-0.5B-Instruct](https://huggingface.co/Qwen/Qwen2.5-0.5B-Instruct)
- [SmolLM2-360M-Instruct](https://huggingface.co/HuggingFaceTB/SmolLM2-360M-Instruct)
