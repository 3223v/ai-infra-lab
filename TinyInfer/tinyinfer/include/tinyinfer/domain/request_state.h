// =============================================================================
// TinyInfer — 请求状态机
//
// 定义一个请求从创建到终止的完整生命周期状态，以及合法的状态转换。
// 状态图与转换约束参见 design.md §8。
//
// 设计要点：
//  - 所有状态转换必须显式经过 Transition()，禁止任意跳转。
//  - 非法转换返回 Error(kIllegalStateTransition)，让问题在 Debug 期立刻暴露。
//  - 终态（kFinished/kCancelled/kTimedOut/kFailed/kRejected）不可转出。
// =============================================================================

#pragma once

#include <string>
#include <string_view>

#include "tinyinfer/common/error.h"
#include "tinyinfer/common/result.h"

namespace tinyinfer {

// 请求生命周期状态
enum class RequestState {
    // ── 活跃态：可继续转换 ──
    kCreated,          // 请求对象已创建，Prompt 尚未 tokenize
    kTokenizing,       // 正在 tokenize Prompt
    kWaiting,          // Prompt 已就绪，进入调度器等待队列
    kRunningPrefill,   // 正在处理 Prompt 的 prefill
    kRunningDecode,    // prefill 完成，正在逐 token 生成

    // ── 终止态：不可再转出 ──
    kFinished,         // 正常结束（EOS / stop token / stop string / max_tokens）
    kCancelled,        // 客户端主动取消
    kTimedOut,         // 超过请求绝对截止时间
    kFailed,           // 模型执行 / 内存分配 / 内部逻辑错误
    kRejected,         // 准入阶段被拒绝（prompt 太长 / 队列满 / 参数非法）
};

/// 状态可读文本，用于日志与序列化
[[nodiscard]] inline constexpr std::string_view to_string(RequestState state) noexcept {
    switch (state) {
        case RequestState::kCreated:        return "created";
        case RequestState::kTokenizing:     return "tokenizing";
        case RequestState::kWaiting:        return "waiting";
        case RequestState::kRunningPrefill: return "running_prefill";
        case RequestState::kRunningDecode:  return "running_decode";
        case RequestState::kFinished:       return "finished";
        case RequestState::kCancelled:      return "cancelled";
        case RequestState::kTimedOut:       return "timed_out";
        case RequestState::kFailed:         return "failed";
        case RequestState::kRejected:       return "rejected";
    }
    return "unknown";
}

/// 是否为终止态（不可再转出）
[[nodiscard]] inline constexpr bool IsTerminal(RequestState state) noexcept {
    switch (state) {
        case RequestState::kFinished:
        case RequestState::kCancelled:
        case RequestState::kTimedOut:
        case RequestState::kFailed:
        case RequestState::kRejected:
            return true;
        case RequestState::kCreated:
        case RequestState::kTokenizing:
        case RequestState::kWaiting:
        case RequestState::kRunningPrefill:
        case RequestState::kRunningDecode:
            return false;
    }
    return false;
}

/// 判断 from → to 是否为合法转换（仅查询，不改变状态）
[[nodiscard]] inline constexpr bool CanTransition(RequestState from, RequestState to) noexcept {
    switch (from) {
        case RequestState::kCreated:
            return to == RequestState::kTokenizing;

        case RequestState::kTokenizing:
            return to == RequestState::kWaiting ||
                   to == RequestState::kFailed;

        case RequestState::kWaiting:
            return to == RequestState::kRunningPrefill ||
                   to == RequestState::kCancelled ||
                   to == RequestState::kTimedOut ||
                   to == RequestState::kRejected;

        case RequestState::kRunningPrefill:
            return to == RequestState::kRunningDecode ||
                   to == RequestState::kFailed ||
                   to == RequestState::kCancelled ||
                   to == RequestState::kTimedOut;

        case RequestState::kRunningDecode:
            return to == RequestState::kRunningDecode ||
                   to == RequestState::kFinished ||
                   to == RequestState::kFailed ||
                   to == RequestState::kCancelled ||
                   to == RequestState::kTimedOut;

        // 终态不可转出
        case RequestState::kFinished:
        case RequestState::kCancelled:
        case RequestState::kTimedOut:
        case RequestState::kFailed:
        case RequestState::kRejected:
            return false;
    }
    return false;
}

/// 执行受约束的状态转换：合法则返回新状态，非法则返回 Error。
///
/// 调用方应当通过它修改请求状态，而不是直接赋值，从而保证状态机不变量。
[[nodiscard]] inline Result<RequestState> Transition(RequestState from, RequestState to) {
    if (!CanTransition(from, to)) {
        return Error{
            ErrorCode::kIllegalStateTransition,
            std::string{"illegal state transition: "} +
                std::string{to_string(from)} + " -> " + std::string{to_string(to)},
        };
    }
    return to;
}

}  // namespace tinyinfer
