// ============================================================================
// model_config.hpp — 模型配置定义与 JSON 解析
// ============================================================================
//
// 模型配置对应 model.json 文件，描述模型结构参数和权重索引。
//
// model.json 格式（示例）：
// {
//   "model_type": "qwen2_like",
//   "hidden_size": 896,
//   "num_layers": 24,
//   "num_attention_heads": 14,
//   "num_kv_heads": 2,
//   "head_dim": 64,
//   "vocab_size": 151936,
//   "dtype": "fp16",
//   "tensors": [
//     {"name": "model.embed_tokens.weight", "offset": 0, "size": 272170496, "shape": [151936, 896]},
//     ...
//   ]
// }
//
// 后续在此文件中添加：
//   struct TensorEntry { std::string name; size_t offset; size_t size; Shape shape; };
//   struct ModelConfig { ... };
//   ModelConfig load_model_config(const std::string& json_path);
// ============================================================================

#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace tinyinfer {

// ---- TensorEntry -----------------------------------------------------------
// 描述 weights.bin 中的一个 tensor
// offset 和 size 用于在 .bin 文件中定位

// ---- ModelConfig -----------------------------------------------------------
// 包含：模型类型、hidden_size、层数、head 数、vocab_size 等
// 以及一个 TensorEntry 列表（每个权重的名称、shape、offset、size）

// ---- load_model_config -----------------------------------------------------
// 解析 model.json，返回 ModelConfig 或错误
// 依赖：需要 JSON 解析——先用简单的手写解析器（只支持上述字段），
// 后续如果需要可以引入 nlohmann/json

} // namespace tinyinfer
