// 请求终止原因

#pragma once

#include <string_view>

namespace tinyinfer {

    enum class FinishReason {
        // 正常结束
        kEos,           //  模型生成 EOS token
        kStopToken,     //  命中 stop token
        kStopString,    //  命中 stop string
        kMaxTokens,     //  达到 token 上限

        // 异常结束
        kCancelled,     //  客户端取消
        kTimedOut,      //  超过绝对截止时间
        kFailed,        //  模型失败/内部错误
        kRejected,      //  准入阶段拒绝
    };

    /// 返回原因可读文本
    [[nodiscard]] inline constexpr std::string_view to_string(FinishReason reason) noexcept {
        switch (reason) {
            case FinishReason::kEos:        return "eos";
            case FinishReason::kStopToken:  return "stop_token";
            case FinishReason::kStopString: return "stop_string";
            case FinishReason::kMaxTokens:  return "max_tokens";
            case FinishReason::kCancelled:  return "cancelled";
            case FinishReason::kTimedOut:   return "timed_out";
            case FinishReason::kFailed:     return "failed";
            case FinishReason::kRejected:   return "rejected";
        }
        return "unknown";
    } 

} // namespace tinyinfer