# TinyInfer — 当前 8 周目标

起止：2026-07-09 ~ 2026-09-02（56 天）

## 一句话

用 C++ 写一个能在 RTX 3050 4GB 上跑通 1B 以下开源模型的推理 runtime。

## 做什么

| 阶段 | 周 | 目标 |
|---|---|---|
| 工程骨架 | W1 | CMake、基础类型、CLI、模型配置、权重格式 |
| CPU 正确性 | W2 | debug model 上跑通完整 Transformer forward |
| 真实模型 | W3 | 接入 Qwen2.5-0.5B 或 SmolLM2-360M 权重 |
| CUDA 算子 | W4 | GPU backend：RMSNorm、RoPE、Attention、MLP |
| 完整推理 | W5 | 真实模型 GPU prefill + generate |
| KV Cache | W6 | prefill 写缓存 → decode 读缓存 → 连续生成 |
| 稳定性 | W7 | CLI、脚本、错误处理、README |
| 验收 | W8 | RTX 3050 4GB 上稳定运行、从零可复现 |

## 不做什么（本阶段）

- 模拟器、调度器（BlockServe-Sim 已冻结）
- 训练（TinyGPT、100M）
- 多机多卡、HTTP 服务、FlashAttention
- 复杂图表、Benchmark 报告

## 技术选型

- **语言**：C++20
- **构建**：CMake + FetchContent (Catch2)
- **模型**：Qwen2.5-0.5B-Instruct（主），SmolLM2-360M（备）
- **权重**：自定义 model.json + weights.bin
- **Tokenizer**：Python 侧处理（transformers）
- **GPU**：CUDA + cuBLAS

## 验收标准

```text
1. C++ 加载真实模型权重
2. CUDA backend 执行推理
3. RTX 3050 4GB 上稳定生成文本
4. Python 只做 tokenizer 和权重转换
5. 从零可复现
```

详见 `docs/plan.md`。
