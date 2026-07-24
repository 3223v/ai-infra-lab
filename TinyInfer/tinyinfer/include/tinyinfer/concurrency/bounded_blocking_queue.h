// bounded_blocking_queue 线程安全的有界阻塞队列

#pragma once

#include <mutex>
#include <condition_variable>
#include <deque>


namespace tinyinfer {
    template <typename T>

    class BoundedBlockingQueue {
        public:
            explicit BoundedBlockingQueue(size_t capacity);

            // 队尾放一个，满则阻塞
            void push(T item);

            // 队头取一个，空则阻塞
            [[nodiscard]] T pop();

            // 尝试放，不阻塞，满则false
            bool try_push(T item);

            // 尝试取，不阻塞，空则false
            [[nidiscard]] std::optional<T> try_pop();

            // 关闭队列，唤醒所有阻塞的线程
            void close();

            [[nodiscard]] size_t size() const noexcept;
            [[nodiscard]] size_t capacity() const noexcept;
            [[nodiscard]] bool closed() const noexcept;

        private:
            std::deque<T> queue_;
            size_t capacity_;
            bool closed_ = false;
            mutable std::mutex mutex_;
            std::condition_variable cv_push_;
            std::condition_variable cv_pop_;

    }
}