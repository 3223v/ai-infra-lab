# Day 3｜2026-07-11｜模型结构调研

## 主题

用 Python 分析目标模型结构，搞清楚需要实现哪些算子、每种权重的 shape。

## 任务

1. 写 `tools/inspect_hf_model.py`，用 transformers 库加载 Qwen2.5-0.5B
2. 打印模型配置每项参数
3. 遍历所有权重，记录 name / shape / dtype / 参数量
4. 汇总总参数量、fp16 预估显存
5. 根据权重命名反推需要哪些算子
6. 写 `docs/qwen_model_shapes.md`，记录所有发现

## 产出

- `tools/inspect_hf_model.py` — 模型结构检查工具
- `docs/qwen_model_shapes.md` — 完整模型结构文档

## 关键发现

| 参数 | 值 |
|---|---|
| hidden_size | 896 |
| num_layers | 24 |
| num_attention_heads | 14 |
| num_kv_heads (GQA) | 2 |
| vocab_size | 151,936 |
| 权重 dtype | bfloat16 |
| 总参数 | ~494M |
| fp16 大小 | ~1.17 GB |

需要的算子：Embedding、Linear(matmul)、RMSNorm、RoPE、SiLU、Causal Attention、Greedy Decode。

## 验收

```bash
python tools/inspect_hf_model.py --model Qwen/Qwen2.5-0.5B-Instruct
# 输出完整模型结构和所有权重
```
