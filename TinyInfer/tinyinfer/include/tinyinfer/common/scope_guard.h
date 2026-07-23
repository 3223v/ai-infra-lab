// ScopeGuard RAII 清理动作

#pragma once

#include <type_traits>
#include <utility>

namespace tinyinfer{
    // ---------------------------------------------------------------------------
    // ScopeGuard — 析构时自动执行函数
    //
    // 用法:
    //   auto guard = ScopeGuard([] { cleanup(); });
    //   guard.dismiss();  // 取消执行
    // ---------------------------------------------------------------------------
    template <typename F>
    class ScopeGuard{
        public:
            explicit ScopeGuard(F&& f) : f_(std::forward<F>(f)), active_(true) {}
            explicit ScopeGuard(const F& f) : f_(f), active_(true) {}
            
            ScopeGuard(ScopeGuard&& other) noexcept
                : f_(std::move(other.f_)), active_(other.active_) {
            other.dismiss();
            }
            
            ScopeGuard& operator=(ScopeGuard&& other) noexcept {
                if (this != &other) {
                    if (active_) f_();
                    f_ = std::move(other.f_);
                    active_ = other.active_;
                    other.dismiss();
                }
                return *this;
            }

            ScopeGuard(const ScopeGuard&) = delete;
            ScopeGuard& operator=(const ScopeGuard&) = delete;
            
             ~ScopeGuard() noexcept(noexcept(f_())) {
                if (active_) f_();
            }

            void dismiss() noexcept { active_ = false; }

            private:
                F f_;
                bool active_;
    };
    // C++17 deduction guide
    template <typename F>
    ScopeGuard(F) -> ScopeGuard<F>;
} // namespace tinyinfer