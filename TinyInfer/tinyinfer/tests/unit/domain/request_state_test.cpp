// =============================================================================
// TinyInfer — RequestState 单元测试
// =============================================================================

#include <gtest/gtest.h>

#include "tinyinfer/common/error.h"
#include "tinyinfer/domain/request_state.h"

namespace tinyinfer {
namespace {

// ── to_string ──

TEST(RequestStateTest, ToString) {
    EXPECT_EQ(to_string(RequestState::kCreated), "created");
    EXPECT_EQ(to_string(RequestState::kTokenizing), "tokenizing");
    EXPECT_EQ(to_string(RequestState::kWaiting), "waiting");
    EXPECT_EQ(to_string(RequestState::kRunningPrefill), "running_prefill");
    EXPECT_EQ(to_string(RequestState::kRunningDecode), "running_decode");
    EXPECT_EQ(to_string(RequestState::kFinished), "finished");
    EXPECT_EQ(to_string(RequestState::kCancelled), "cancelled");
    EXPECT_EQ(to_string(RequestState::kTimedOut), "timed_out");
    EXPECT_EQ(to_string(RequestState::kFailed), "failed");
    EXPECT_EQ(to_string(RequestState::kRejected), "rejected");
}

// ── IsTerminal ──

TEST(RequestStateTest, ActiveStatesAreNotTerminal) {
    EXPECT_FALSE(IsTerminal(RequestState::kCreated));
    EXPECT_FALSE(IsTerminal(RequestState::kTokenizing));
    EXPECT_FALSE(IsTerminal(RequestState::kWaiting));
    EXPECT_FALSE(IsTerminal(RequestState::kRunningPrefill));
    EXPECT_FALSE(IsTerminal(RequestState::kRunningDecode));
}

TEST(RequestStateTest, TerminalStatesAreTerminal) {
    EXPECT_TRUE(IsTerminal(RequestState::kFinished));
    EXPECT_TRUE(IsTerminal(RequestState::kCancelled));
    EXPECT_TRUE(IsTerminal(RequestState::kTimedOut));
    EXPECT_TRUE(IsTerminal(RequestState::kFailed));
    EXPECT_TRUE(IsTerminal(RequestState::kRejected));
}

// ── CanTransition：合法路径 ──

TEST(RequestStateTest, CanTransitionHappyPath) {
    EXPECT_TRUE(CanTransition(RequestState::kCreated, RequestState::kTokenizing));
    EXPECT_TRUE(CanTransition(RequestState::kTokenizing, RequestState::kWaiting));
    EXPECT_TRUE(CanTransition(RequestState::kWaiting, RequestState::kRunningPrefill));
    EXPECT_TRUE(CanTransition(RequestState::kRunningPrefill, RequestState::kRunningDecode));
    // decode 自循环（逐 token 生成）
    EXPECT_TRUE(CanTransition(RequestState::kRunningDecode, RequestState::kRunningDecode));
    EXPECT_TRUE(CanTransition(RequestState::kRunningDecode, RequestState::kFinished));
}

TEST(RequestStateTest, CanTransitionErrorBranches) {
    // 任何活跃态都可以转入失败 / 取消 / 超时
    EXPECT_TRUE(CanTransition(RequestState::kTokenizing, RequestState::kFailed));
    EXPECT_TRUE(CanTransition(RequestState::kWaiting, RequestState::kCancelled));
    EXPECT_TRUE(CanTransition(RequestState::kWaiting, RequestState::kTimedOut));
    EXPECT_TRUE(CanTransition(RequestState::kWaiting, RequestState::kRejected));
    EXPECT_TRUE(CanTransition(RequestState::kRunningPrefill, RequestState::kFailed));
    EXPECT_TRUE(CanTransition(RequestState::kRunningDecode, RequestState::kCancelled));
}

// ── CanTransition：非法路径 ──

TEST(RequestStateTest, CannotSkipStages) {
    // 不允许跳过中间阶段
    EXPECT_FALSE(CanTransition(RequestState::kCreated, RequestState::kWaiting));
    EXPECT_FALSE(CanTransition(RequestState::kCreated, RequestState::kRunningDecode));
    EXPECT_FALSE(CanTransition(RequestState::kWaiting, RequestState::kRunningDecode));
    EXPECT_FALSE(CanTransition(RequestState::kTokenizing, RequestState::kRunningPrefill));
    // prefill 不能直接结束，必须先进 decode
    EXPECT_FALSE(CanTransition(RequestState::kRunningPrefill, RequestState::kFinished));
    // 不能反向回退
    EXPECT_FALSE(CanTransition(RequestState::kRunningDecode, RequestState::kWaiting));
    EXPECT_FALSE(CanTransition(RequestState::kWaiting, RequestState::kCreated));
}

TEST(RequestStateTest, TerminalStatesCannotTransitionOut) {
    const RequestState terminals[] = {
        RequestState::kFinished, RequestState::kCancelled, RequestState::kTimedOut,
        RequestState::kFailed, RequestState::kRejected,
    };
    for (auto t : terminals) {
        EXPECT_FALSE(CanTransition(t, RequestState::kFinished)) << "from " << to_string(t);
        EXPECT_FALSE(CanTransition(t, RequestState::kWaiting)) << "from " << to_string(t);
        EXPECT_FALSE(CanTransition(t, RequestState::kCreated)) << "from " << to_string(t);
    }
}

// ── Transition：合法返回新状态 ──

TEST(RequestStateTest, TransitionLegalReturnsNewState) {
    auto r = Transition(RequestState::kCreated, RequestState::kTokenizing);
    ASSERT_TRUE(r.ok()) << r.error().message();
    EXPECT_EQ(*r, RequestState::kTokenizing);

    auto r2 = Transition(RequestState::kRunningDecode, RequestState::kFinished);
    ASSERT_TRUE(r2.ok()) << r2.error().message();
    EXPECT_EQ(*r2, RequestState::kFinished);
}

// ── Transition：非法返回错误 ──

TEST(RequestStateTest, TransitionIllegalReturnsError) {
    auto r = Transition(RequestState::kCreated, RequestState::kFinished);
    ASSERT_FALSE(r.ok());
    EXPECT_EQ(r.error().code(), ErrorCode::kIllegalStateTransition);
    // 错误信息应包含起止状态名，便于排障
    EXPECT_NE(r.error().message().find("created"), std::string::npos);
    EXPECT_NE(r.error().message().find("finished"), std::string::npos);
}

TEST(RequestStateTest, TransitionFromTerminalFails) {
    auto r = Transition(RequestState::kFinished, RequestState::kRunningDecode);
    EXPECT_FALSE(r.ok());
    EXPECT_EQ(r.error().code(), ErrorCode::kIllegalStateTransition);
}

}  // namespace
}  // namespace tinyinfer
