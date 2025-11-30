#ifndef RING_BUFFER_H
#define RING_BUFFER_H

#include <atomic>
#include <type_traits>
#include <chrono>
#include <cstddef>
#include <cassert>
#include <new>
#include <utility>

enum class RingBufferMode {
    SPSC,  // Single Producer Single Consumer
    MPSC   // Multiple Producers Single Consumer
};

// 工业级无锁环形缓冲区
// 容量必须是2的幂
// 支持SPSC和MPSC模式
template <typename T, RingBufferMode Mode = RingBufferMode::SPSC>
class RingBuffer {
public:
    explicit RingBuffer(size_t capacity) 
        : capacity_(round_up_to_power_of_two(capacity))
        , mask_(capacity_ - 1)
        , buffer_(new T[capacity_])
        , head_(0)
        , tail_(0)
    {
        assert(is_power_of_two(capacity_));
    }

    ~RingBuffer()
    {
        // 销毁所有未弹出的元素
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t head = head_.load(std::memory_order_relaxed);
        
        while (tail != head) {
            buffer_[tail & mask_].~T();
            tail++;
        }
        
        delete[] buffer_;
    }

    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator=(RingBuffer&&) = delete;

    // 尝试插入元素
    template <typename U = T>
    bool try_push(U&& value)
    {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_acquire);
        
        if ((head - tail) >= capacity_) {
            return false; // 缓冲区满
        }
        
        // MPSC模式下需要自旋锁保护
        if (Mode == RingBufferMode::MPSC) {
            while (spinlock_.test_and_set(std::memory_order_acquire)) {
                // 自旋等待锁释放
            }
            
            // 再次检查缓冲区是否满（防止在等待锁期间被其他生产者填满）
            head = head_.load(std::memory_order_relaxed);
            tail = tail_.load(std::memory_order_acquire);
            
            if ((head - tail) >= capacity_) {
                spinlock_.clear(std::memory_order_release);
                return false;
            }
        }
        
        // 构造元素
        new (&buffer_[head & mask_]) T(std::forward<U>(value));
        
        // 发布新的head位置
        head_.store(head + 1, std::memory_order_release);
        
        // MPSC模式下释放自旋锁
        if (Mode == RingBufferMode::MPSC) {
            spinlock_.clear(std::memory_order_release);
        }
        
        return true;
    }

    // 尝试弹出元素
    bool try_pop(T& value)
    {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t head = head_.load(std::memory_order_acquire);
        
        if (tail == head) {
            return false; // 缓冲区空
        }
        
        // 复制元素
        value = std::move(buffer_[tail & mask_]);
        
        // 销毁原元素
        buffer_[tail & mask_].~T();
        
        // 发布新的tail位置
        tail_.store(tail + 1, std::memory_order_release);
        
        return true;
    }

    // 原地构造元素
    template <typename... Args>
    bool emplace(Args&&... args)
    {
        size_t head = head_.load(std::memory_order_relaxed);
        size_t tail = tail_.load(std::memory_order_acquire);
        
        if ((head - tail) >= capacity_) {
            return false; // 缓冲区满
        }
        
        // MPSC模式下需要自旋锁保护
        if (Mode == RingBufferMode::MPSC) {
            while (spinlock_.test_and_set(std::memory_order_acquire)) {
                // 自旋等待锁释放
            }
            
            // 再次检查缓冲区是否满（防止在等待锁期间被其他生产者填满）
            head = head_.load(std::memory_order_relaxed);
            tail = tail_.load(std::memory_order_acquire);
            
            if ((head - tail) >= capacity_) {
                spinlock_.clear(std::memory_order_release);
                return false;
            }
        }
        
        // 原地构造元素
        new (&buffer_[head & mask_]) T(std::forward<Args>(args)...);
        
        // 发布新的head位置
        head_.store(head + 1, std::memory_order_release);
        
        // MPSC模式下释放自旋锁
        if (Mode == RingBufferMode::MPSC) {
            spinlock_.clear(std::memory_order_release);
        }
        
        return true;
    }

    // 带超时的等待弹出
    template <typename Rep, typename Period>
    bool wait_pop(T& value, const std::chrono::duration<Rep, Period>& timeout)
    {
        auto start_time = std::chrono::steady_clock::now();
        
        while (true) {
            if (try_pop(value)) {
                return true;
            }
            
            auto elapsed_time = std::chrono::steady_clock::now() - start_time;
            if (elapsed_time >= timeout) {
                return false;
            }
            
            // 短暂的自旋等待
            for (int i = 0; i < 100; ++i) {
                __asm__ volatile ("pause");
            }
        }
    }

    // 线程安全的容量查询
    size_t capacity() const noexcept
    {
        return capacity_;
    }

    // 线程安全的大小查询
    size_t size() const noexcept
    {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return head - tail;
    }

    // 线程安全的空查询
    bool empty() const noexcept
    {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    // 线程安全的满查询
    bool full() const noexcept
    {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return (head - tail) >= capacity_;
    }

private:
    static size_t round_up_to_power_of_two(size_t n)
    {
        if (n == 0) {
            return 1;
        }
        
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        
        #ifdef __SIZEOF_SIZE_T__
            #if __SIZEOF_SIZE_T__ == 8
                n |= n >> 32;
            #endif
        #endif
        
        return n + 1;
    }

    static bool is_power_of_two(size_t n)
    {
        return n != 0 && (n & (n - 1)) == 0;
    }

    const size_t capacity_;
    const size_t mask_;

    T* buffer_;

    // 生产者索引
    std::atomic<size_t> head_;

    // 消费者索引
    std::atomic<size_t> tail_;

    // MPSC模式下的自旋锁回退
    alignas(64) std::atomic_flag spinlock_ = ATOMIC_FLAG_INIT;
};

// 显式实例化常见类型和模式
template class RingBuffer<int, RingBufferMode::SPSC>;
template class RingBuffer<int, RingBufferMode::MPSC>;
template class RingBuffer<long, RingBufferMode::SPSC>;
template class RingBuffer<long, RingBufferMode::MPSC>;

#endif // RING_BUFFER_H