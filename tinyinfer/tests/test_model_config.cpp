// ============================================================================
// test_model_config.cpp — 模型配置加载测试
// ============================================================================
//
// 当前 model_config 尚未实现，这些测试暂时留空。
// 后续 Day 4 实现 model_config 后填入：
//   1. 加载合法 model.json → 成功
//   2. 文件不存在 → 错误
//   3. JSON 格式错误 → 错误
//   4. 缺少必填字段 → 错误
// ============================================================================
#include <catch2/catch_test_macros.hpp>
#include "tinyinfer/model_config.hpp"
#include "tinyinfer/status.hpp"

using namespace tinyinfer;

// TODO: Day 4 实现 model_config 后填充测试
TEST_CASE("model_config placeholder", "[model]") {
    // 占位：确保测试 target 能编译链接
    CHECK(true);
}
