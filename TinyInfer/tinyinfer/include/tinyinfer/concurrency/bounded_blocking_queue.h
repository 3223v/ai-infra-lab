// =============================================================================
// TinyInfer — 线程安全的有界阻塞队列
//
// 用于 OutputChannel：Scheduler（单生产者）写入事件，HTTP 层（单消费者）读取。
//
// 关闭语义（排空后关闭）：
//  - Close() 后不能再写入，所有写入方法立即返回 false。
//  - Close() 后仍可继续读取已有元素，直到队列排空，此后读取返回 nullopt。
//    这样消费者能消费完 Close 前已入队的事件，常用于"结束流"。
//
// 方法命名遵循 rule.md §1.4（PascalCase）。
// 写入返回 bool（成功/失败）、读取返回 std::optional<T>（空或关闭后返回 nullopt），
// 这是为了正确表达"关闭"这一运行时事件，避免静默丢数据或永久阻塞。
// =============================================================================

#pragma once

#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <optional>
#include <utility>

namespace tinyinfer {

template <typename T>
class BoundedBlockingQueue {
 public:
    explicit BoundedBlockingQueue(size_t capacity) : capacity_(capacity) {}

    BoundedBlockingQueue(const BoundedBlockingQueue&) = delete;
    BoundedBlockingQueue& operator=(const BoundedBlockingQueue&) = delete;
    BoundedBlockingQueue(BoundedBlockingQueue&&) = delete;
    BoundedBlockingQueue& operator=(BoundedBlockingQueue&&) = delete;

    /// 阻塞写入，直到成功或队列被关闭。
    /// 返回 true 表示写入成功；false 表示队列已关闭（item 被丢弃）。
    bool BlockingPush(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_push_.wait(lock, [this] { return queue_.size() < capacity_ || closed_; });
        if (closed_) {
            return false;
        }
        queue_.push_back(std::move(item));
        cv_pop_.notify_one();
        return true;
    }

    /// 带超时的写入。成功返回 true；超时或队列已关闭返回 false。
    /// 用途：Scheduler 写 OutputEvent 时做背压保护，避免被慢客户端永久阻塞。
    bool TryPushFor(T item, std::chrono::nanoseconds timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        const bool ready =
            cv_push_.wait_for(lock, timeout, [this] { return queue_.size() < capacity_ || closed_; });
        if (!ready || closed_) {
            return false;
        }
        queue_.push_back(std::move(item));
        cv_pop_.notify_one();
        return true;
    }

    /// 非阻塞写入。成功 true；队列已满或已关闭返回 false。
    bool TryPush(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (closed_ || queue_.size() >= capacity_) {
            return false;
        }
        queue_.push_back(std::move(item));
        cv_pop_.notify_one();
        return true;
    }

    /// 阻塞读取，直到有元素或队列关闭且排空。
    /// 关闭且排空后返回 nullopt，表示流正常结束。
    [[nodiscard]] std::optional<T> BlockingPop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_pop_.wait(lock, [this] { return !queue_.empty() || closed_; });
        if (queue_.empty()) {
            // closed_ && empty：流结束
            return std::nullopt;
        }
        T item = std::move(queue_.front());
        queue_.pop_front();
        cv_push_.notify_one();
        return item;
    }

    /// 非阻塞读取。队列为空（含关闭且排空）时返回 nullopt。
    [[nodiscard]] std::optional<T> TryPop() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return std::nullopt;
        }
        T item = std::move(queue_.front());
        queue_.pop_front();
        cv_push_.notify_one();
        return item;
    }

    /// 关闭队列，唤醒所有阻塞的生产者与消费者。
    /// 幂等：重复调用无副作用。
    void Close() {
        std::unique_lock<std::mutex> lock(mutex_);
        if (closed_) {
            return;
        }
        closed_ = true;
        cv_push_.notify_all();
        cv_pop_.notify_all();
    }

    [[nodiscard]] size_t Size() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size();
    }

    [[nodiscard]] size_t Capacity() const noexcept { return capacity_; }

    [[nodiscard]] bool IsClosed() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return closed_;
    }

    [[nodiscard]] bool IsEmpty() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    [[nodiscard]] bool IsFull() const noexcept {
        std::unique_lock<std::mutex> lock(mutex_);
        return queue_.size() >= capacity_;
    }

 private:
    std::deque<T> queue_;
    const size_t capacity_;
    bool closed_ = false;
    mutable std::mutex mutex_;
    std::condition_variable cv_push_;  // 等待"有空位"
    std::condition_variable cv_pop_;   // 等待"有元素"
};

}  // namespace tinyinfer
