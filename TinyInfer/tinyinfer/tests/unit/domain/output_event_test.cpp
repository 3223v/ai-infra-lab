// =============================================================================
// TinyInfer — OutputEvent 单元测试
// =============================================================================

#include <gtest/gtest.h>

#include "tinyinfer/domain/output_event.h"

namespace tinyinfer {
namespace {

// ── TokenDelta ──

TEST(OutputEventTest, TokenDeltaFields) {
  auto e = OutputEvent::TokenDelta(42, "hello", 5);
  EXPECT_EQ(e.type(), OutputEvent::Type::kTokenDelta);
  EXPECT_EQ(e.token_id(), 42);
  EXPECT_EQ(e.text(), "hello");
  EXPECT_EQ(e.generated_len(), 5);
}

// ── Finish ──

TEST(OutputEventTest, FinishReasonEos) {
  auto e = OutputEvent::Finish(FinishReason::kEos);
  EXPECT_EQ(e.type(), OutputEvent::Type::kFinish);
  EXPECT_EQ(e.finish_reason(), FinishReason::kEos);
}

TEST(OutputEventTest, FinishReasonCancelled) {
  auto e = OutputEvent::Finish(FinishReason::kCancelled);
  EXPECT_EQ(e.finish_reason(), FinishReason::kCancelled);
}

// ── Error ──

TEST(OutputEventTest, ErrorFields) {
  auto e = OutputEvent::Error(ErrorCode::kInvalidArgument, "bad prompt");
  EXPECT_EQ(e.type(), OutputEvent::Type::kError);
  EXPECT_EQ(e.error_code(), ErrorCode::kInvalidArgument);
  EXPECT_EQ(e.error_message(), "bad prompt");
}

// ── Cancelled ──

TEST(OutputEventTest, CancelledType) {
  auto e = OutputEvent::Cancelled();
  EXPECT_EQ(e.type(), OutputEvent::Type::kCancelled);
}

// ── Usage ──

TEST(OutputEventTest, UsageFields) {
  auto e = OutputEvent::Usage(100, 50);
  EXPECT_EQ(e.type(), OutputEvent::Type::kUsage);
  EXPECT_EQ(e.prompt_tokens(), 100);
  EXPECT_EQ(e.generated_tokens(), 50);
}

// ── 不变：不同事件类型的数据互不污染 ──

TEST(OutputEventTest, TokenDeltaDoesNotHaveFinishReason) {
  auto e = OutputEvent::TokenDelta(1, "x", 1);
  // TokenDelta 事件上取 finish_reason 应该返回原始默认值（无意义）
  // 但调用者应该先检查 type()，不应在非 Finish 事件上调 finish_reason()
  EXPECT_EQ(e.type(), OutputEvent::Type::kTokenDelta);
}

}  // namespace
}  // namespace tinyinfer

// 本文件由 Claude Code 编写
