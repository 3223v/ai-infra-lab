#pragma once

#include <chrono>
#include <cstdint>
#include <memory>

namespace tinyinfer {

/// 时钟抽象接口，支持生产环境真实时钟、测试环境Fake时钟注入
class IClock {
    public:
        using TimePoint = std::chrono::steady_clock::time_point;
        using Duration = std::chrono::nanoseconds;

        virtual ~IClock() = default;

        /// 获取当前时间
        [[nodiscard]] virtual TimePoint now() const noexcept = 0;

        /// 计算从since到当前的时间差（纳秒）
        [[nodiscard]] int64_t elapsed_ns(TimePoint since) const noexcept {
            return std::chrono::duration_cast<Duration>(now() - since).count();
        }

        /// 计算从since到当前的时间差（微秒）
        [[nodiscard]] int64_t elapsed_us(TimePoint since) const noexcept {
            return std::chrono::duration_cast<std::chrono::microseconds>(now() - since).count();
        }

        /// 计算从since到当前的时间差（毫秒）
        [[nodiscard]] int64_t elapsed_ms(TimePoint since) const noexcept {
            return std::chrono::duration_cast<std::chrono::milliseconds>(now() - since).count();
        }
    };

    /// 生产环境默认单调时钟实现
class Clock : public IClock {
    public:
        [[nodiscard]] TimePoint now() const noexcept override {
            return std::chrono::steady_clock::now();
        }

        /// 全局默认实例便捷方法
        static std::shared_ptr<IClock> shared() {
            static std::shared_ptr<Clock> instance = std::make_shared<Clock>();
            return instance;
        }
};

} // namespace tinyinfer