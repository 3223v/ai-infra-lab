#!/usr/bin/env python3
# ============================================================================
# reference_forward.py — Python 参考前向传播，用于 C++ 对齐
# ============================================================================
#
# 这是整个项目正确性验证的核心。
# 所有 C++ 算子的输出都要和这个脚本的输出比对。
#
# 工作流程：
#   1. 加载 debug model（或真实模型）
#   2. 输入一组 token ids
#   3. 跑一次完整 forward，得到 logits
#   4. 保存输入、输出、中间结果到文件
#   5. C++ 侧加载同样的模型和输入，跑 forward，比对输出
#
# 用法：
#   python tools/reference_forward.py --model models/debug_model --input-ids "1,2,3"
#
# 输出文件（保存在模型目录下）：
#   reference_input.bin     — 输入的 token ids
#   reference_logits.bin    — 输出的 logits（每层+最终）
#   reference_kv_cache.bin  — [可选] KV Cache 中间值
#
# 后续实现内容：
#   - 加载 PyTorch 模型
#   - model.eval() + torch.no_grad()
#   - 记录每层 hidden_states 或 logits
#   - 写出为 raw binary 或 numpy .npy 文件
# ============================================================================

import argparse
import sys

def run_reference_forward(model_path: str, input_ids: list[int]) -> None:
    """跑一次前向传播，保存输出"""
    # TODO: 加载模型
    # TODO: 输入 token ids → forward → logits
    # TODO: 保存 logits 到文件
    pass

def main():
    parser = argparse.ArgumentParser(description="Run reference forward pass")
    parser.add_argument("--model", required=True, help="Model directory")
    parser.add_argument("--input-ids", required=True, help="Comma-separated token ids")
    args = parser.parse_args()

    input_ids = [int(x.strip()) for x in args.input_ids.split(",")]
    print(f"Running reference forward: model={args.model}, input_ids={input_ids}")
    # run_reference_forward(args.model, input_ids)

if __name__ == "__main__":
    main()
