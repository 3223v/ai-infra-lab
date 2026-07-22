// =============================================================================
// TinyInfer — 单调时钟
// =============================================================================

#pragma once

#include <chrono>

namespace tinyinfer {

class Clock {
 public:
  using TimePoint = std::chrono::steady_clock::time_point;
  using Duration  = std::chrono::nanoseconds;

  // 当前时间
  [[nodiscard]] static TimePoint now() noexcept {
    return std::chrono::steady_clock::now();
  }

  // 时间差（纳秒）
  [[nodiscard]] static int64_t elapsed_ns(TimePoint since) noexcept {
    return std::chrono::duration_cast<Duration>(now() - since).count();
  }

  // 时间差（毫秒）
  [[nodiscard]] static int64_t elapsed_ms(TimePoint since) noexcept {
    return elapsed_ns(since) / 1'000'000;
  }

  // 时间差（微秒）
  [[nodiscard]] static int64_t elapsed_us(TimePoint since) noexcept {
    return elapsed_ns(since) / 1'000;
  }
};

}  // namespace tinyinfer
