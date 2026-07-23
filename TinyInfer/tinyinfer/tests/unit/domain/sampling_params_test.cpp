// =============================================================================
// TinyInfer — SamplingParams 单元测试
// =============================================================================

#include <gtest/gtest.h>

#include "tinyinfer/domain/sampling_params.h"

namespace tinyinfer {
namespace {

// ── 默认构造 ──

TEST(SamplingParamsTest, DefaultValues) {
  SamplingParams p;
  EXPECT_EQ(p.max_new_tokens(), 512);
  EXPECT_FLOAT_EQ(p.temperature(), 1.0f);
  EXPECT_EQ(p.top_k(), 40);
  EXPECT_FLOAT_EQ(p.top_p(), 0.95f);
  EXPECT_EQ(p.seed(), -1);
  EXPECT_FLOAT_EQ(p.repetition_penalty(), 1.0f);
  EXPECT_TRUE(p.stop_token_ids().empty());
  EXPECT_TRUE(p.stop_strings().empty());
  EXPECT_FALSE(p.include_stop_string());
}

// ── 自定义构造 ──

TEST(SamplingParamsTest, CustomValues) {
  SamplingParams p(100, 0.5f, 20, 0.9f, 42, 1.2f);
  EXPECT_EQ(p.max_new_tokens(), 100);
  EXPECT_FLOAT_EQ(p.temperature(), 0.5f);
  EXPECT_EQ(p.top_k(), 20);
  EXPECT_FLOAT_EQ(p.top_p(), 0.9f);
  EXPECT_EQ(p.seed(), 42);
  EXPECT_FLOAT_EQ(p.repetition_penalty(), 1.2f);
}

// ── 校验：默认参数应合法 ──

TEST(SamplingParamsTest, DefaultIsValid) {
  SamplingParams p;
  EXPECT_TRUE(p.valid());
}

// ── 校验：各边界 ──

TEST(SamplingParamsTest, MaxTokensZeroIsInvalid) {
  SamplingParams p(0);
  EXPECT_FALSE(p.valid());
}

TEST(SamplingParamsTest, MaxTokensNegativeIsInvalid) {
  SamplingParams p(-1);
  EXPECT_FALSE(p.valid());
}

TEST(SamplingParamsTest, TemperatureZeroIsValid) {
  SamplingParams p(512, 0.0f);
  EXPECT_TRUE(p.valid());  // temperature=0 表示贪婪采样，合法
}

TEST(SamplingParamsTest, TemperatureNegativeIsInvalid) {
  SamplingParams p(512, -0.1f);
  EXPECT_FALSE(p.valid());
}

TEST(SamplingParamsTest, TopKZeroIsValid) {
  SamplingParams p(512, 1.0f, 0);
  EXPECT_TRUE(p.valid());  // top_k=0 表示不做 top_k 筛选
}

TEST(SamplingParamsTest, TopKNegativeIsInvalid) {
  SamplingParams p(512, 1.0f, -1);
  EXPECT_FALSE(p.valid());
}

TEST(SamplingParamsTest, TopPExactlyOneIsValid) {
  SamplingParams p(512, 1.0f, 40, 1.0f);
  EXPECT_TRUE(p.valid());
}

TEST(SamplingParamsTest, TopPAboveOneIsInvalid) {
  SamplingParams p(512, 1.0f, 40, 1.01f);
  EXPECT_FALSE(p.valid());
}

TEST(SamplingParamsTest, TopPNegativeIsInvalid) {
  SamplingParams p(512, 1.0f, 40, -0.1f);
  EXPECT_FALSE(p.valid());
}

TEST(SamplingParamsTest, RepetitionPenaltyOneIsValid) {
  SamplingParams p(512, 1.0f, 40, 0.95f, -1, 1.0f);
  EXPECT_TRUE(p.valid());
}

TEST(SamplingParamsTest, RepetitionPenaltyBelowOneIsInvalid) {
  SamplingParams p(512, 1.0f, 40, 0.95f, -1, 0.9f);
  EXPECT_FALSE(p.valid());
}

// ── 停止条件 ──

TEST(SamplingParamsTest, AddStopToken) {
  SamplingParams p;
  p.add_stop_token(2);
  p.add_stop_token(3);
  ASSERT_EQ(p.stop_token_ids().size(), 2u);
  EXPECT_EQ(p.stop_token_ids()[0], 2);
  EXPECT_EQ(p.stop_token_ids()[1], 3);
}

TEST(SamplingParamsTest, AddStopString) {
  SamplingParams p;
  p.add_stop_string("</s>");
  p.add_stop_string("\n\n");
  ASSERT_EQ(p.stop_strings().size(), 2u);
  EXPECT_EQ(p.stop_strings()[0], "</s>");
  EXPECT_EQ(p.stop_strings()[1], "\n\n");
}

TEST(SamplingParamsTest, SetIncludeStopString) {
  SamplingParams p;
  EXPECT_FALSE(p.include_stop_string());
  p.set_include_stop_string(true);
  EXPECT_TRUE(p.include_stop_string());
}

// ── 不可变性：getter 返回的引用不能让调用者修改内部数据 ──

TEST(SamplingParamsTest, StopTokenIdsGetterIsReadOnly) {
  SamplingParams p;
  p.add_stop_token(42);
  // const & 确保下面这行编译不通过：
  // p.stop_token_ids().push_back(99);  // ❌ 编译错误
  EXPECT_EQ(p.stop_token_ids().size(), 1u);
}

}  // namespace
}  // namespace tinyinfer

// 本文件由 Claude Code 编写
