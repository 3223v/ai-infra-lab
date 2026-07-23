// =============================================================================
// TinyInfer — Common 模块单元测试
// =============================================================================

#include <gtest/gtest.h>

#include "tinyinfer/common/clock.h"
#include "tinyinfer/common/error.h"
#include "tinyinfer/common/noncopyable.h"
#include "tinyinfer/common/result.h"
#include "tinyinfer/common/scope_guard.h"
#include "tinyinfer/common/types.h"

// ---------------------------------------------------------------------------
// Error 测试
// ---------------------------------------------------------------------------
TEST(ErrorTest, OkByDefault) {
  tinyinfer::Error err(tinyinfer::ErrorCode::kOk, "");
  EXPECT_TRUE(err.ok());
}

TEST(ErrorTest, FactoryMethods) {
  auto err = tinyinfer::Error::InvalidArgument("bad input");
  EXPECT_EQ(err.code(), tinyinfer::ErrorCode::kInvalidArgument);
  EXPECT_EQ(err.message(), "bad input");
  EXPECT_FALSE(err.ok());
}

TEST(ErrorTest, InternalError) {
  auto err = tinyinfer::Error::Internal("invariant broken");
  EXPECT_EQ(err.code(), tinyinfer::ErrorCode::kInvariantViolation);
}

// ---------------------------------------------------------------------------
// Result 测试
// ---------------------------------------------------------------------------
TEST(ResultTest, SuccessValue) {
  tinyinfer::Result<int> r{42};
  EXPECT_TRUE(r.ok());
  EXPECT_TRUE(r);
  EXPECT_EQ(r.value(), 42);
  EXPECT_EQ(*r, 42);
}

TEST(ResultTest, ErrorValue) {
  tinyinfer::Result<int> r{tinyinfer::Error::Internal("fail")};
  EXPECT_FALSE(r.ok());
  EXPECT_FALSE(r);
  EXPECT_EQ(r.error().code(), tinyinfer::ErrorCode::kInvariantViolation);
  EXPECT_EQ(r.value_or(0), 0);
}

TEST(ResultTest, VoidSuccess) {
  tinyinfer::Result<void> r;
  EXPECT_TRUE(r.ok());
}

TEST(ResultTest, VoidError) {
  tinyinfer::Result<void> r{tinyinfer::Error::Internal("oops")};
  EXPECT_FALSE(r.ok());
  EXPECT_EQ(r.error().message(), "oops");
}

// ---------------------------------------------------------------------------
// Clock 测试
// ---------------------------------------------------------------------------
TEST(ClockTest, NowIsMonotonic) {
  auto clock = tinyinfer::Clock::shared();
  auto t1 = clock->now();
  auto t2 = clock->now();
  // 不要求严格递增（可能在同一节拍内），但不应倒退
  EXPECT_GE(t2, t1);
}

TEST(ClockTest, ElapsedPositive) {
  auto clock = tinyinfer::Clock::shared();
  auto start = clock->now();
  auto elapsed = clock->elapsed_ns(start);
  EXPECT_GE(elapsed, 0);
}

// ---------------------------------------------------------------------------
// NonCopyable 测试
// ---------------------------------------------------------------------------
TEST(NonCopyableTest, DefaultConstructible) {
  tinyinfer::NonCopyable nc;
  (void)nc;
}

// ---------------------------------------------------------------------------
// ScopeGuard 测试
// ---------------------------------------------------------------------------
TEST(ScopeGuardTest, ExecuteOnScopeExit) {
  int counter = 0;
  {
    auto guard = tinyinfer::ScopeGuard{[&] { ++counter; }};
    EXPECT_EQ(counter, 0);
  }
  EXPECT_EQ(counter, 1);
}

TEST(ScopeGuardTest, DismissPreventsExecution) {
  int counter = 0;
  {
    auto guard = tinyinfer::ScopeGuard{[&] { ++counter; }};
    guard.dismiss();
  }
  EXPECT_EQ(counter, 0);
}

TEST(ScopeGuardTest, MoveDismissesSource) {
  int counter = 0;
  {
    auto guard1 = tinyinfer::ScopeGuard{[&] { ++counter; }};
    auto guard2 = std::move(guard1);
  }
  EXPECT_EQ(counter, 1);
}
