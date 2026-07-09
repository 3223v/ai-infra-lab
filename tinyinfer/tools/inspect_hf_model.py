#!/usr/bin/env python3
# ============================================================================
# inspect_hf_model.py — 打印 Hugging Face 模型结构和权重信息
# ============================================================================
#
# 用法：
#   python tools/inspect_hf_model.py --model Qwen/Qwen2.5-0.5B-Instruct
#   python tools/inspect_hf_model.py --model HuggingFaceTB/SmolLM2-360M-Instruct
#
# 依赖：
#   pip install transformers torch
# ============================================================================

import argparse
from transformers import AutoModelForCausalLM, AutoConfig


def inspect_model(model_name: str) -> None:
    # ---- 1. 加载配置（不加载权重，速度很快） ----
    print(f"Loading config for: {model_name}")
    config = AutoConfig.from_pretrained(model_name, trust_remote_code=True)

    print("\n" + "=" * 72)
    print("MODEL CONFIG")
    print("=" * 72)
    # 不同类型的模型字段名可能略有差异，用 getattr 安全获取
    fields = [
        ("model_type",           "Model type"),
        ("hidden_size",          "Hidden size"),
        ("num_hidden_layers",    "Number of layers"),
        ("num_attention_heads",  "Attention heads"),
        ("num_key_value_heads",  "KV heads (GQA)"),
        ("head_dim",             "Head dimension"),
        ("intermediate_size",    "Intermediate size (MLP)"),
        ("vocab_size",           "Vocabulary size"),
        ("max_position_embeddings", "Max position embeddings"),
        ("rope_theta",           "RoPE theta"),
        ("rms_norm_eps",         "RMSNorm epsilon"),
        ("tie_word_embeddings",  "Tie word embeddings"),
    ]
    for attr, label in fields:
        val = getattr(config, attr, None)
        if val is not None:
            print(f"  {label:30s} = {val}")

    # 计算 head_dim（如果没有直接提供）
    if getattr(config, "head_dim", None) is None:
        head_dim = config.hidden_size // config.num_attention_heads
        config.head_dim = head_dim
        print(f"  {'Head dimension (derived)':30s} = {head_dim}")

    # ---- 2. 加载模型（仅配置，不加载权重，节省内存和时间） ----
    # 这里只需要知道结构，不需要权重
    # 注意：trust_remote_code=True 用于 Qwen 等需要自定义代码的模型
    print(f"\nLoading model structure (weights on disk, not in memory)...")
    model = AutoModelForCausalLM.from_config(config, trust_remote_code=True)

    print("\n" + "=" * 72)
    print("WEIGHT LIST")
    print("=" * 72)
    print(f"  {'NAME':60s} {'SHAPE':25s} {'PARAMS':>12s}  DTYPE")
    print("  " + "-" * 68)

    total_params = 0
    weight_count = 0

    # 遍历 state_dict：key 是权重名，param 是 torch.Tensor
    for name, param in model.state_dict().items():
        shape = tuple(param.shape)
        numel = param.numel()      # 元素总数
        dtype = str(param.dtype)    # torch.float32 / torch.bfloat16 等
        # 移除 "torch." 前缀，输出更干净
        if dtype.startswith("torch."):
            dtype = dtype[6:]

        total_params += numel
        weight_count += 1

        # 格式化 shape 显示
        shape_str = str(shape)
        print(f"  {name:60s} {shape_str:25s} {numel:>12,d}  {dtype}")

    # ---- 3. 汇总 ----
    print("\n" + "=" * 72)
    print("SUMMARY")
    print("=" * 72)
    print(f"  Total weight tensors:  {weight_count}")
    print(f"  Total parameters:      {total_params:,} ({total_params/1e6:.2f}M)")

    # fp16: 每个参数 2 字节
    fp16_size = total_params * 2
    fp32_size = total_params * 4

    def fmt_bytes(b):
        if b >= 1024**3:
            return f"{b/1024**3:.2f} GB"
        elif b >= 1024**2:
            return f"{b/1024**2:.2f} MB"
        elif b >= 1024:
            return f"{b/1024:.2f} KB"
        return f"{b} B"

    print(f"  Weight size (fp16):    {fmt_bytes(fp16_size)}")
    print(f"  Weight size (fp32):    {fmt_bytes(fp32_size)}")
    print()

    # ---- 4. 算子清单 ----
    # 从权重名称推断需要的算子
    ops = set()
    for name, _ in model.state_dict().items():
        if "embed" in name:
            ops.add("Embedding lookup")
        if "layernorm" in name or "norm" in name:
            ops.add("RMSNorm")
        if "q_proj" in name or "k_proj" in name or "v_proj" in name or "o_proj" in name:
            ops.add("Linear (matmul) — Q/K/V/O projection")
        if "gate_proj" in name or "up_proj" in name or "down_proj" in name:
            ops.add("Linear (matmul) — MLP gate/up/down")
        if "lm_head" in name:
            ops.add("Linear (matmul) — LM Head")
        if "q_proj" in name or "k_proj" in name:
            ops.add("RoPE (rotary position embedding)")

    print("=" * 72)
    print("REQUIRED OPERATORS (for C++ implementation)")
    print("=" * 72)
    for op in sorted(ops):
        print(f"  - {op}")
    print()


def main():
    parser = argparse.ArgumentParser(description="Inspect HF model structure")
    parser.add_argument("--model", default="Qwen/Qwen2.5-0.5B-Instruct",
                        help="HF model name or local path")
    args = parser.parse_args()

    inspect_model(args.model)


if __name__ == "__main__":
    main()
