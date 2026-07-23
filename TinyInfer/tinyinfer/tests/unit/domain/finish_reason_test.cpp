// =============================================================================
// TinyInfer — FinishReason 单元测试
// =============================================================================

#include <gtest/gtest.h>

#include "tinyinfer/domain/finish_reason.h"

namespace tinyinfer {
namespace {

// ── 正常结束 ──

TEST(FinishReasonTest, Eos) {
  EXPECT_EQ(to_string(FinishReason::kEos), "eos");
}

TEST(FinishReasonTest, StopToken) {
  EXPECT_EQ(to_string(FinishReason::kStopToken), "stop_token");
}

TEST(FinishReasonTest, StopString) {
  EXPECT_EQ(to_string(FinishReason::kStopString), "stop_string");
}

TEST(FinishReasonTest, MaxTokens) {
  EXPECT_EQ(to_string(FinishReason::kMaxTokens), "max_tokens");
}

// ── 异常结束 ──

TEST(FinishReasonTest, Cancelled) {
  EXPECT_EQ(to_string(FinishReason::kCancelled), "cancelled");
}

TEST(FinishReasonTest, TimedOut) {
  EXPECT_EQ(to_string(FinishReason::kTimedOut), "timed_out");
}

TEST(FinishReasonTest, Failed) {
  EXPECT_EQ(to_string(FinishReason::kFailed), "failed");
}

TEST(FinishReasonTest, Rejected) {
  EXPECT_EQ(to_string(FinishReason::kRejected), "rejected");
}

// ── 不变量：每个枚举值都有明确的字符串映射 ──

TEST(FinishReasonTest, AllValuesHaveNonEmptyString) {
  for (auto reason : {
         FinishReason::kEos,
         FinishReason::kStopToken,
         FinishReason::kStopString,
         FinishReason::kMaxTokens,
         FinishReason::kCancelled,
         FinishReason::kTimedOut,
         FinishReason::kFailed,
         FinishReason::kRejected,
       }) {
    auto str = to_string(reason);
    EXPECT_FALSE(str.empty());
    EXPECT_NE(str, "unknown");
  }
}

}  // namespace
}  // namespace tinyinfer

// 本文件由 Claude Code 编写
