#!/usr/bin/env python3
# ============================================================================
# export_hf_to_tinyinfer.py — 将 Hugging Face 模型转换为 TinyInfer 格式
# ============================================================================
#
# 转换流程：
#   Hugging Face checkpoint（safetensors / pytorch.bin）
#   → 统一 tensor 命名
#   → fp32/bf16 → fp16 转换
#   → 按序写入 weights.bin
#   → 生成 model.json
#
# 用法：
#   python tools/export_hf_to_tinyinfer.py \
#     --hf-model Qwen/Qwen2.5-0.5B-Instruct \
#     --output models/qwen2_5_0_5b_tinyinfer
#
# 依赖：
#   pip install transformers torch safetensors
#
# 后续实现内容：
#   1. 从 HF 加载模型
#   2. 遍历 state_dict，按固定顺序排列 tensor
#   3. 写入 model.json（结构参数 + tensor 索引）
#   4. 写入 weights.bin（raw fp16 bytes）
#   5. 进度提示和校验信息
# ============================================================================

import argparse
import sys

def export_model(hf_model_name: str, output_dir: str) -> None:
    """转换 HF 模型 → TinyInfer 格式"""
    # TODO: 加载 HF 模型
    # TODO: 提取 state_dict
    # TODO: 写 model.json
    # TODO: 写 weights.bin
    pass

def main():
    parser = argparse.ArgumentParser(description="Convert HF model to TinyInfer format")
    parser.add_argument("--hf-model", required=True, help="HF model name or local path")
    parser.add_argument("--output", required=True, help="Output directory")
    args = parser.parse_args()

    print(f"Converting {args.hf_model} -> {args.output}")
    # export_model(args.hf_model, args.output)

if __name__ == "__main__":
    main()
