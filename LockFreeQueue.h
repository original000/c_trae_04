#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

// 线程安全队列
class LockFreeQueue {
public:
    LockFreeQueue() = default;
    ~LockFreeQueue() = default;

    // 禁止拷贝和移动
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    LockFreeQueue(LockFreeQueue&&) = delete;
    LockFreeQueue& operator=(LockFreeQueue&&) = delete;

    // 从队列尾部添加元素
    bool push(const std::function<void()>& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(value);
        return true;
    }

    bool push(std::function<void()>&& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(value));
        return true;
    }

    // 从队列头部弹出元素
    bool try_pop(std::function<void()>& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(queue_.front());
        queue_.pop_front();
        return true;
    }

    // 从队列尾部窃取元素（用于任务窃取）
    bool steal(std::function<void()>& value) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(queue_.back());
        queue_.pop_back();
        return true;
    }

    // 检查队列是否为空
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

private:
    std::deque<std::function<void()>> queue_;
    mutable std::mutex mutex_;
};
