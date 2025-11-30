#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <random>
#include <utility>
#include <limits>
#include "LockFreeQueue.h"

// 检测代码
#ifdef _WIN32
    #define OS_NAME "Windows"
#elif _UNIX
    #define OS_NAME "Linux"
#else
    #define OS_NAME "Unknown"
#endif

#ifdef __GNUC__
    #define GCC_VERSION_STRING "Unknown"
#else
    #define GCC_VERSION_STRING "Unknown"
#endif

#ifdef CPP14_SUPPORTED
    #define CPP14_SUPPORT_STRING "Yes"
#else
    #define CPP14_SUPPORT_STRING "No"
#endif

// 线程池类
class ThreadPool {
public:
    // 构造函数，指定线程数量
    explicit ThreadPool(size_t thread_count = std::thread::hardware_concurrency());

    // 析构函数，优雅关闭线程池
    ~ThreadPool();

    // 提交任务到线程池，返回std::future
    template <typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

    // 批量提交任务到线程池
    void submit_batch(const std::vector<std::function<void()>>& tasks);

    // 设置线程池的线程数量
    void set_thread_count(size_t thread_count);

    // 获取当前线程池的线程数量
    size_t get_thread_count() const;

private:
    // 工作线程函数
    void worker(size_t index);

    // 任务窃取函数
    std::function<void()> steal_task();

    // 线程池状态
    std::atomic<bool> running_;
    // 线程列表
    std::vector<std::thread> threads_;
    // 每个线程的本地任务队列
    std::vector<std::unique_ptr<LockFreeQueue>> local_queues_;
    // 线程池的线程数量
    std::atomic<size_t> thread_count_;
    // 当前线程的索引（线程局部变量）
    static thread_local size_t thread_index_;
};

// 提交任务到线程池，返回std::future
template <typename F, typename... Args>
auto ThreadPool::submit(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();

    // 将任务添加到随机选择的线程的本地队列
    static std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<size_t> dist(0, local_queues_.size() - 1);
    size_t index = dist(gen);
    local_queues_[index]->push([task]() { (*task)(); });

    return res;
}
