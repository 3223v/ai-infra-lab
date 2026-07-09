// ============================================================================
// model_config.cpp — 模型配置加载
// ============================================================================
//
// 解析 model.json，提取模型结构参数和权重索引。
//
// JSON 解析策略（按优先级）：
//   1. 先手写简单解析器（只支持上述字段，不引入第三方依赖）
//   2. 如果后续字段太复杂，考虑引入 nlohmann/json（header-only, MIT）
//
// 后续在此文件中添加：
//   - 简单 JSON 解析辅助函数（跳过空白、读取字符串、读取数字）
//   - TensorEntry 结构体
//   - ModelConfig 结构体 + load_model_config() 函数
//   - 错误处理：文件不存在、JSON 格式错误、字段缺失
// ============================================================================

#include "tinyinfer/model_config.hpp"

namespace tinyinfer {

// ---- 简单 JSON 解析（内部使用） ---------------------------------------------
// 只解析 model.json 中需要的字段，不做完整 JSON parser
// 函数清单（后续逐步实现）：
//   skip_whitespace(s, &pos)
//   read_string(s, &pos) → std::optional<std::string>
//   read_number(s, &pos) → std::optional<int64_t>
//   expect_char(s, &pos, expected) → bool

// ---- ModelConfig -----------------------------------------------------------
// 存储：
//   std::string model_type;         // "qwen2_like"
//   int64_t hidden_size;
//   int64_t num_layers;
//   int64_t num_attention_heads;
//   int64_t num_kv_heads;
//   int64_t head_dim;
//   int64_t vocab_size;
//   std::string dtype;              // "fp16" 或 "fp32"
//   std::vector<TensorEntry> tensors;

// ---- load_model_config -----------------------------------------------------
// 1. 打开 model.json 文件，读入全部内容到 std::string
// 2. 逐字段解析 JSON，填入 ModelConfig
// 3. 校验必填字段存在且合法
// 4. 返回 ModelConfig 或 Status::Error

} // namespace tinyinfer
