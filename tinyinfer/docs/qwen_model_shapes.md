# Qwen2.5-0.5B-Instruct 模型结构

来源：`python tools/inspect_hf_model.py --model Qwen/Qwen2.5-0.5B-Instruct`

## 配置

| 参数 | 值 |
|---|---|
| model_type | qwen2 |
| hidden_size | 896 |
| num_layers | 24 |
| num_attention_heads | 14 |
| num_kv_heads (GQA) | 2 |
| head_dim | 64 |
| intermediate_size (MLP) | 4864 |
| vocab_size | 151,936 |
| max_position_embeddings | 32,768 |
| rope_theta | 1,000,000.0 |
| rms_norm_eps | 1e-06 |
| tie_word_embeddings | True |
| 原始 dtype | bfloat16 |

## 参数详解

### model_type
模型架构标识。`qwen2` 表示 Qwen2.x 系列。这个字段在 C++ 侧用于判断模型结构——不同 model_type 的层结构、激活函数可能不同。当前只支持 `qwen2`（和类似架构如 Llama）。

### hidden_size
**896** — 每一层隐藏状态的维度，也是整个模型内部流通的向量宽度。

所有中间结果（token embedding、attention 输出、MLP 输出）都是这个维度。hidden_size 决定了：
- 权重大小：Q/K/V/O 投影矩阵都是 `[hidden_size, hidden_size]` 量级
- 每层计算量：和 hidden_size² 成正比
- 显存占用：每层的 activation buffer 大小

896 是一个相对小的值（Llama-7B 是 4096），这使得 0.5B 模型能在 4GB 显存上运行。

### num_layers
**24** — Decoder Layer 的重复次数。每一层结构完全相同，但权重独立。

输入 token → Embedding → [Layer 0 → Layer 1 → ... → Layer 23] → RMSNorm → LM Head → logits

层数越多 → 模型越深 → 表达能力越强 → 需要的内存和计算越多。

### num_attention_heads
**14** — Multi-Head Attention 的 head 数量。

每个 token 的 hidden_state（896 维）被分成 14 个 head，每个 head 64 维（= 896 / 14 = head_dim）。不同 head 关注不同位置的关系——有的看邻近词、有的看远距离依赖。

### num_kv_heads (GQA)
**2** — Key 和 Value 的 head 数量。

这是 **Grouped Query Attention (GQA)** 的关键参数。14 个 Q head 共享 2 个 KV head，即每 7 个 Q head 使用同一组 K/V。

为什么要这样做？
- **省显存**：K/V cache 大小和 num_kv_heads 成正比。2 比 14 省了 7 倍 KV cache
- **推理更快**：decode 时 K/V 矩阵更小，访存更少
- **精度影响小**：KV 的信息可以被多个 Q head 共享

我们的 K/V 投影 shape 是 `[128, 896]`，其中 128 = num_kv_heads × head_dim = 2 × 64。

### head_dim
**64** — 每个 attention head 的维度。

满足 `hidden_size = num_attention_heads × head_dim = 14 × 64 = 896`。

Attention 计算中用 `sqrt(head_dim) = 8` 做 scaling：`softmax(QK^T / sqrt(64))`。

### intermediate_size (MLP)
**4864** — MLP 中间层的维度。

每个 token 经过 Attention 后进入 MLP，维度从 896 扩展到 4864，处理完后压缩回 896。

SwiGLU 结构需要两个扩展矩阵（gate_proj 和 up_proj），所以 MLP 部分有 3 个权重矩阵：
- `gate_proj: [4864, 896]` — 门控
- `up_proj:   [4864, 896]` — 投影
- `down_proj: [896, 4864]` — 压缩回 hidden_size

4864 / 896 ≈ 5.4，这是 Qwen 的扩展比。Llama 通常是 8/3 ≈ 2.7（intermediate ≈ hidden × 8/3），Qwen 的更大。

### vocab_size
**151,936** — 词表大小，即模型认识多少个不同的 token。

注意几个数字：
- 151,936 >> 常见英文词表（~50K），因为 Qwen 针对多语言优化
- embedding 矩阵 `[151936, 896]` 有 **1.36 亿参数**，占模型总参数的 ~28%
- 这是 0.5B 模型的主要参数来源
- `tie_word_embeddings = True` 意味着 embedding 和 lm_head 共享同一份权重，节省一倍

### max_position_embeddings
**32,768** — 模型训练时支持的最大上下文长度。

推理时可以设置更小的值来省 KV cache。我们的目标是 context=256 起步，远小于这个上限。

### rope_theta
**1,000,000.0** — RoPE 的基础频率。

RoPE（Rotary Position Embedding）用三角函数给每个位置编码：
- 低频维度 → 区分远距离位置
- 高频维度 → 区分邻近位置

theta 越大 → 低频越低 → 能区分的距离越远。1,000,000 是 Qwen2.5 的默认值，支持更长上下文。

### rms_norm_eps
**1e-06** — RMSNorm 的 epsilon 值。

归一化时在分母加的一个极小值，防止除零：
```
RMSNorm(x) = x / sqrt(mean(x²) + eps) * weight
```

太小 → 数值不稳定；太大 → 影响归一化效果。1e-6 是标准取值。

### tie_word_embeddings
**True** — 输入 embedding 和输出 lm_head 共享权重。

这意味着：
- `model.embed_tokens.weight` 和 `lm_head.weight` 是**同一份权重**（同一个 tensor）
- 总参数量减去 136M（embedding 的大小）
- weights.bin 中只存一份
- C++ 加载时只需要加载一次，embedding 和 lm_head 引用同一块内存

如果不共享（tie=False），embedding 和 lm_head 各占 136M，总共多 136M 参数。

## 参数量

| 指标 | 值 |
|---|---|
| 权重 tensor 数量 | 291 |
| 总参数（含重复） | 630,167,424 (~630M) |
| 总参数（去重，embed/lm_head 共享） | ~494M |
| fp16 权重文件大小 | ~1.17 GB |
| fp32 权重文件大小 | ~2.35 GB |

## 每层结构（共 24 层，layer index 0~23）

```
model.layers.N.self_attn.q_proj.weight   [896, 896]
model.layers.N.self_attn.q_proj.bias     [896]
model.layers.N.self_attn.k_proj.weight   [128, 896]    ← kv_heads × head_dim = 2×64
model.layers.N.self_attn.k_proj.bias     [128]
model.layers.N.self_attn.v_proj.weight   [128, 896]
model.layers.N.self_attn.v_proj.bias     [128]
model.layers.N.self_attn.o_proj.weight   [896, 896]
model.layers.N.mlp.gate_proj.weight      [4864, 896]
model.layers.N.mlp.up_proj.weight        [4864, 896]
model.layers.N.mlp.down_proj.weight      [896, 4864]
model.layers.N.input_layernorm.weight    [896]
model.layers.N.post_attention_layernorm.weight [896]
```

每层共 13 个 tensor，含 **3 个 bias**（q/k/v projection）。

## 非层权重

```
model.embed_tokens.weight   [151936, 896]  ← 136M 参数
model.norm.weight           [896]          ← 最终 RMSNorm
lm_head.weight              [151936, 896]  ← 与 embed_tokens 共享（tie）
```

## GQA 分析

- 14 个 query head，2 个 KV head
- Q 维度：14 × 64 = 896（= hidden_size）
- K/V 维度：2 × 64 = 128
- 推理时 K/V 需要在 14 个 head 间广播（或 repeat）

## Weight 命名规范

| 名称片段 | 含义 | 对应算子 |
|---|---|---|
| `embed_tokens` | Token 嵌入表 | Embedding lookup |
| `input_layernorm` | Attention 前 RMSNorm | RMSNorm |
| `post_attention_layernorm` | MLP 前 RMSNorm | RMSNorm |
| `self_attn.q_proj` | Query 投影 | Linear (matmul) |
| `self_attn.k_proj` | Key 投影 | Linear (matmul) |
| `self_attn.v_proj` | Value 投影 | Linear (matmul) |
| `self_attn.o_proj` | Attention 输出投影 | Linear (matmul) |
| `mlp.gate_proj` | SwiGLU gate | Linear + SiLU |
| `mlp.up_proj` | SwiGLU up | Linear |
| `mlp.down_proj` | SwiGLU down | Linear |
| `norm` | 最终输出 RMSNorm | RMSNorm |
| `lm_head` | 输出 logits | Linear (matmul) |

## 需要实现的 C++ 算子

1. **Embedding lookup** — token id → hidden state
2. **Linear (matmul)** — 含 bias，矩阵乘法
3. **RMSNorm** — 均方根归一化
4. **RoPE** — 旋转位置编码，应用于 Q 和 K
5. **SiLU** — SwiGLU 激活函数
6. **Causal Self-Attention** — Q·K^T / √d + mask + softmax × V，支持 GQA
7. **Argmax / Greedy decode** — 从 logits 取 top-1 token

## 显存估算（RTX 3050 4GB）

| 项目 | 大小 |
|---|---|
| 权重 (fp16) | ~1.17 GB |
| KV Cache (context=256, fp16) | ~2 × 24 × 2 × 64 × 256 × 2B = ~3.1 MB |
| Activations (粗略) | ~200 MB |
| 余量 | ~2.5 GB ✅ |

4GB 显存足够运行，context 可以开到 256~512。
