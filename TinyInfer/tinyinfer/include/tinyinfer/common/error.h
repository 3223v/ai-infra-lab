// 错误类型定义

#pragma once

#include <string>
#include <string_view>

namespace tinyinfer{
    // 错误码枚举
    enum class ErrorCode{
        // 成功
        kOk = 0,

        // 客户端错误 (1xx)
        kInvalidArgument = 100,
        kPromptTooLong = 101,
        kUnsupportedField = 102,
        kRequestCancelled = 103,

        // 资源错误 (2xx)
        kQueueFull = 200,
        kSlotExhausted = 201,
        kContextExceeded = 202,
        kOutputChannelBackpressure = 203,

        // 模型错误 (3xx)
        kWeightMissing = 300,
        kWeightShapeMismatch = 301,
        kBackendForwardFailed = 302,
        kNaNOrInfDetected = 303,
        kTokenizerError = 304,

        // 内部错误 (4xx)
        kIllegalStateTransition = 400,
        kSlotDoubleFree = 401,
        kStaleHandleAccess = 402,
        kBatchResultCountMismatch = 403,
        kInvariantViolation = 404,
        kTimeout = 405,
    };

    // Error 错误对象
    class Error{
        public:
            Error(ErrorCode code, std::string message)
                : code_(code), message_(std::move(message)) {}

            [[nodiscard]] ErrorCode code() const noexcept { return code_; }
            [[nodiscard]] const std::string& message() const noexcept { return message_; }
            [[nodiscard]] bool ok() const noexcept { return code_ == ErrorCode::kOk; }

            // 工厂方法
            static Error InvalidArgument(std::string msg) {
                return Error(ErrorCode::kInvalidArgument, std::move(msg));
            }

            static Error Internal(std::string msg) {
                return Error(ErrorCode::kInvariantViolation, std::move(msg));
            }
            static Error Ok() {
                return Error(ErrorCode::kOk, "");
            }
        
        private:
            ErrorCode code_;
            std::string message_;
    };
}   // namespace tinyinfer