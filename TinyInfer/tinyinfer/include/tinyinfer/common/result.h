// =============================================================================
// TinyInfer — Result<T, Error> 返回值类型
// =============================================================================
//
// 使用方式:
//   Result<int> r = compute();
//   if (!r) { handle_error(r.error()); }
//   int value = *r;
//
// 设计原则：
//  - 参数/资源/Backend 可恢复错误：返回 Result
//  - 不变量破坏：Debug 断言，Release Fatal 日志
//  - 异常不得穿透 Scheduler 主循环
// =============================================================================

#pragma once

#include "tinyinfer/common/error.h"

#include <cassert>
#include <optional>
#include <type_traits>
#include <variant>

namespace tinyinfer {

// ---------------------------------------------------------------------------
//  Result<T> — 成功时包含 T，失败时包含 Error
// ---------------------------------------------------------------------------
template <typename T>
class Result {
  static_assert(!std::is_reference_v<T>, "Result<T> does not support references");
  static_assert(!std::is_same_v<T, void>, "Use Result<void> pattern or just return Error");

 public:
  // 隐式构造：从值
  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(T value) : storage_(std::move(value)) {}

  // 隐式构造：从 Error
  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(Error error) : storage_(std::move(error)) {}

  // 判断是否成功
  [[nodiscard]] explicit operator bool() const noexcept {
    return std::holds_alternative<T>(storage_);
  }

  [[nodiscard]] bool ok() const noexcept {
    return std::holds_alternative<T>(storage_);
  }

  // 访问值 (调用方必须确认成功)
  [[nodiscard]] T& value() noexcept {
    assert(ok() && "Result::value() called on error result");
    return std::get<T>(storage_);
  }

  [[nodiscard]] const T& value() const noexcept {
    assert(ok() && "Result::value() called on error result");
    return std::get<T>(storage_);
  }

  // 按指针访问
  [[nodiscard]] T* operator->() noexcept { return &value(); }
  [[nodiscard]] const T* operator->() const noexcept { return &value(); }

  [[nodiscard]] T& operator*() noexcept { return value(); }
  [[nodiscard]] const T& operator*() const noexcept { return value(); }

  // 访问错误
  [[nodiscard]] const Error& error() const noexcept {
    assert(!ok() && "Result::error() called on success result");
    return std::get<Error>(storage_);
  }

  // 获取值，若有错误则返回默认值
  [[nodiscard]] T value_or(T default_value) const noexcept {
    return ok() ? std::get<T>(storage_) : std::move(default_value);
  }

 private:
  std::variant<T, Error> storage_;
};

// ---------------------------------------------------------------------------
// Result<void> 特化
// ---------------------------------------------------------------------------
template <>
class Result<void> {
 public:
  Result() = default;

  // NOLINTNEXTLINE(google-explicit-constructor)
  Result(Error error) : error_(std::move(error)) {}

  [[nodiscard]] explicit operator bool() const noexcept { return !error_.has_value(); }
  [[nodiscard]] bool ok() const noexcept { return !error_.has_value(); }

  [[nodiscard]] const Error& error() const noexcept {
    assert(error_.has_value() && "Result::error() called on success result");
    return *error_;
  }

 private:
  std::optional<Error> error_;
};

}  // namespace tinyinfer
