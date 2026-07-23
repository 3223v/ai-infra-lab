// NonCopyable / NonMovable 基类

#pragma once

namespace tinyinfer{
    class NonCopyable{
        public:
            NonCopyable() = default;
            NonCopyable(const NonCopyable&) = delete;
            NonCopyable& operator = (const NonCopyable&) = delete;
    };

    class NonMovable{
        public:
            NonMovable() = default;
            NonMovable(NonMovable&&) = delete;
            NonMovable& operator=(NonMovable&&) = delete;
    };

    class NonCopyableNonMovable{
        public:
            NonCopyableNonMovable() = default;
            NonCopyableNonMovable(const NonCopyableNonMovable&) = delete;
            NonCopyableNonMovable& operator=(const NonCopyableNonMovable&) = delete;
            NonCopyableNonMovable(NonCopyableNonMovable&&) = delete;
            NonCopyableNonMovable& operator=(NonCopyableNonMovable&&) = delete;

    };
}