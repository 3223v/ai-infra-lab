// =============================================================================
// TinyInfer — 输出事件（调度器 → HTTP 层的消息）
// =============================================================================

#pragma once

#include <cstdint>
#include <string>

#include "tinyinfer/common/error.h"
#include "tinyinfer/common/types.h"
#include "tinyinfer/domain/finish_reason.h"

namespace tinyinfer {

class OutputEvent {
 public:
  // 事件类型
  enum class Type {
    kTokenDelta,  // 生成一个新 token
    kFinish,      // 请求正常结束
    kError,       // 请求失败
    kCancelled,   // 客户端取消
    kUsage,       // 用量统计
  };

  // ── 工厂方法：每种事件类型一个 static 函数 ──

  static OutputEvent TokenDelta(TokenId token, std::string text, int32_t generated_len) {
    OutputEvent e;
    e.type_ = Type::kTokenDelta;
    e.token_id_ = token;
    e.text_ = std::move(text);
    e.generated_len_ = generated_len;
    return e;
  }

  static OutputEvent Finish(FinishReason reason) {
    OutputEvent e;
    e.type_ = Type::kFinish;
    e.finish_reason_ = reason;
    return e;
  }

  static OutputEvent Error(ErrorCode code, std::string message) {
    OutputEvent e;
    e.type_ = Type::kError;
    e.error_code_ = code;
    e.error_message_ = std::move(message);
    return e;
  }

  static OutputEvent Cancelled() {
    OutputEvent e;
    e.type_ = Type::kCancelled;
    return e;
  }

  static OutputEvent Usage(int32_t prompt_tokens, int32_t generated_tokens) {
    OutputEvent e;
    e.type_ = Type::kUsage;
    e.prompt_tokens_ = prompt_tokens;
    e.generated_tokens_ = generated_tokens;
    return e;
  }

  // ── getter ──

  [[nodiscard]] Type type() const noexcept { return type_; }

  // TokenDelta 专用
  [[nodiscard]] TokenId token_id() const noexcept { return token_id_; }
  [[nodiscard]] const std::string& text() const noexcept { return text_; }
  [[nodiscard]] int32_t generated_len() const noexcept { return generated_len_; }

  // Finish 专用
  [[nodiscard]] FinishReason finish_reason() const noexcept { return finish_reason_; }

  // Error 专用
  [[nodiscard]] ErrorCode error_code() const noexcept { return error_code_; }
  [[nodiscard]] const std::string& error_message() const noexcept { return error_message_; }

  // Usage 专用
  [[nodiscard]] int32_t prompt_tokens() const noexcept { return prompt_tokens_; }
  [[nodiscard]] int32_t generated_tokens() const noexcept { return generated_tokens_; }

  /// 是否为终止事件（Finish / Error / Cancelled）。
  /// 终止事件标志流的结束，OutputChannel 保证终止事件幂等（重复写不报错、不重复入队）。
  [[nodiscard]] bool IsTerminal() const noexcept {
    return type_ == Type::kFinish || type_ == Type::kError || type_ == Type::kCancelled;
  }

 private:
  Type type_;

  // TokenDelta
  TokenId token_id_ = kInvalidTokenId;
  std::string text_;
  int32_t generated_len_ = 0;

  // Finish
  FinishReason finish_reason_ = FinishReason::kEos;

  // Error
  ErrorCode error_code_ = ErrorCode::kOk;
  std::string error_message_;

  // Usage
  int32_t prompt_tokens_ = 0;
  int32_t generated_tokens_ = 0;
};

}  // namespace tinyinfer

// 本文件由 Claude Code 编写