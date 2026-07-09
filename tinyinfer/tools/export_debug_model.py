#!/usr/bin/env python3
# ============================================================================
# export_debug_model.py — 导出一个极小的 debug 模型，用于 C++ 正确性验证
# ============================================================================
#
# 为什么要 debug model 而不是直接上 0.5B？
#   - 0.5B 模型权重文件 ~1GB，加载慢、比对难
#   - debug model 只有几十 KB，加载瞬间完成
#   - 用随机权重 + 固定 seed，保证 C++ 和 Python 可复现
#   - 先把注意力放在"结构对不对"上，再放大到真实模型
#
# 导出格式：
#   models/debug_model/
#   ├── model.json     — 模型结构参数
#   └── weights.bin    — 按顺序排列的权重 float32 原始数据
#
# 用法：
#   python tools/export_debug_model.py --hidden-size 64 --num-layers 2 --output models/debug_model
#
# 后续实现内容：
#   1. 用 PyTorch 构建一个 GPT-like decoder-only 模型（小尺寸）
#   2. 初始化随机权重（固定 seed=42）
#   3. 写出 model.json
#   4. 写出 weights.bin（raw float32 bytes）
#   5. 打印每个 tensor 的名称和 shape
# ============================================================================

import argparse
import sys

def export_debug_model(hidden_size: int, num_layers: int, num_heads: int, output_dir: str) -> None:
    """构建小模型 → 导出自定义格式"""
    # TODO: 构建 GPT-like decoder-only 模型
    # TODO: 用 torch.randn 生成随机权重（seed=42 固定）
    # TODO: 写出 model.json
    # TODO: 按序写出 weights.bin
    pass

def main():
    parser = argparse.ArgumentParser(description="Export a tiny debug model")
    parser.add_argument("--hidden-size", type=int, default=64, help="Hidden dimension")
    parser.add_argument("--num-layers", type=int, default=2, help="Number of decoder layers")
    parser.add_argument("--num-heads", type=int, default=4, help="Number of attention heads")
    parser.add_argument("--output", default="models/debug_model", help="Output directory")
    args = parser.parse_args()

    print(f"Exporting debug model: hidden={args.hidden_size}, layers={args.num_layers}, heads={args.num_heads}")
    # export_debug_model(args.hidden_size, args.num_layers, args.num_heads, args.output)

if __name__ == "__main__":
    main()
