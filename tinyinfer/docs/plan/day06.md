# Day 6｜2026-07-14｜Python Reference 前向传播

## 主题

用 PyTorch 跑一次 debug model 的完整前向传播，保存输入输出作为 C++ 对齐的参考。

## 任务

1. 写 `tools/reference_forward.py`
   - 加载 debug model 权重（或直接重建相同模型）
   - 输入一组固定 token ids（如 [1, 2, 3, 4]）
   - 跑一次 forward，得到 logits
   - 保存输入 token ids → `reference_input.bin`
   - 保存最终 logits → `reference_logits.bin`
   - [可选] 保存每层 hidden_states 到 `reference_hidden_N.bin`
2. 文件格式与 weights.bin 一致：raw fp32 bytes，配合 shape 信息

## 验收

```bash
python tools/reference_forward.py --model models/debug_model --input-ids "1,2,3,4"
# 生成 reference_input.bin + reference_logits.bin
# 后续 C++ 用相同输入跑 forward，logits 误差应 < 1e-5
```
