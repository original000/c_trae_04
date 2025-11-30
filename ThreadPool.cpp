#include "ThreadPool.h"

// 初始化线程局部变量thread_index_
thread_local size_t ThreadPool::thread_index_ = std::numeric_limits<size_t>::max();

// 构造函数，指定线程数量
ThreadPool::ThreadPool(size_t thread_count) : running_(true), thread_count_(thread_count) {
    // 打印检测信息
    printf("=== 环境检测 ===\n");
    printf("操作系统: %s\n", OS_NAME);
    printf("g++ 版本: %s\n", GCC_VERSION_STRING);
    printf("C++14 支持: %s\n", CPP14_SUPPORT_STRING);
    printf("=================\n\n");

    // 创建线程的本地任务队列
    local_queues_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        local_queues_.emplace_back(std::make_unique<LockFreeQueue>());
    }

    // 创建工作线程
    threads_.reserve(thread_count);
    for (size_t i = 0; i < thread_count; ++i) {
        threads_.emplace_back(&ThreadPool::worker, this, i);
    }
}

// 析构函数，优雅关闭线程池
ThreadPool::~ThreadPool() {
    // 关闭线程池
    running_ = false;

    // 等待所有线程完成
    for (auto& thread : threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads_.clear();
    local_queues_.clear();
}

// 工作线程函数
void ThreadPool::worker(size_t index) {
    // 设置当前线程的索引
    thread_index_ = index;

    // 初始化随机数生成器
    static std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<size_t> dist(0, local_queues_.size() - 1);

    while (running_ || (index < local_queues_.size() && !local_queues_[index]->empty())) {
        std::function<void()> task;

        // 尝试从本地队列弹出任务
        if (index < local_queues_.size() && local_queues_[index]->try_pop(task)) {
            if (task) {
                task();
            }
        } else {
            // 尝试从其他队列窃取任务
            bool stolen = false;
            for (size_t i = 0; i < local_queues_.size(); ++i) {
                size_t steal_index = (index + i + 1) % local_queues_.size();
                if (steal_index < local_queues_.size() && local_queues_[steal_index]->steal(task)) {
                    if (task) {
                        task();
                    }
                    stolen = true;
                    break;
                }
            }
            if (!stolen) {
                // 没有任务可执行，让出CPU时间片
                std::this_thread::yield();
            }
        }
    }

    // 清除当前线程的索引
    thread_index_ = std::numeric_limits<size_t>::max();
}

// 任务窃取函数（不再使用，保留兼容性）
std::function<void()> ThreadPool::steal_task() {
    size_t count = thread_count_;

    // 初始化随机数生成器
    static std::mt19937 gen(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<size_t> dist(0, count - 1);

    for (size_t i = 0; i < count; ++i) {
        size_t index = dist(gen);
        if (index < local_queues_.size()) {
            std::function<void()> task;
            if (local_queues_[index]->steal(task)) {
                return task;
            }
        }
    }

    // 所有队列都没有任务
    return nullptr;
}

// 批量提交任务到线程池
void ThreadPool::submit_batch(const std::vector<std::function<void()>>& tasks) {
    // 如果线程池已经关闭，直接返回
    if (!running_) {
        return;
    }

    size_t num_tasks = tasks.size();
    if (num_tasks == 0) {
        return;
    }

    // 将任务均匀分配到各个线程的本地队列
    for (size_t i = 0; i < num_tasks; ++i) {
        size_t index = i % thread_count_;
        if (index < local_queues_.size()) {
            local_queues_[index]->push(tasks[i]);
        }
    }
}

// 设置线程池的线程数量
void ThreadPool::set_thread_count(size_t thread_count) {
    if (thread_count == thread_count_) {
        return;
    }

    if (thread_count < thread_count_) {
        // 减少线程数量
        thread_count_ = thread_count;

        // 不需要显式终止线程，线程会在检查到thread_count_变化时自动退出
    } else {
        // 增加线程数量
        size_t num_to_add = thread_count - thread_count_;

        // 创建新的本地任务队列
        for (size_t i = 0; i < num_to_add; ++i) {
            local_queues_.emplace_back(std::make_unique<LockFreeQueue>());
        }

        // 创建新的工作线程
        for (size_t i = thread_count_; i < thread_count; ++i) {
            threads_.emplace_back(&ThreadPool::worker, this, i);
        }

        thread_count_ = thread_count;
    }
}

// 获取当前线程池的线程数量
size_t ThreadPool::get_thread_count() const {
    return thread_count_;
}
