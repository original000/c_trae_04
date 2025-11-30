#include "RingBuffer.h"
#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <cassert>
#include <algorithm>

using namespace std;
using namespace std::chrono;

constexpr size_t BUFFER_CAPACITY = 1024;
constexpr size_t TOTAL_OPERATIONS = 10000;
constexpr size_t PRODUCER_THREADS = 8;

int main() {
    // 首先打印g++版本
    cout << "=== g++ Version ===\n";
    system("g++ --version");
    cout << endl;

    // 使用MPSC模式的环形缓冲区
    RingBuffer<int, RingBufferMode::MPSC> ring_buffer(BUFFER_CAPACITY);

    atomic<size_t> produced_count{0};
    atomic<size_t> consumed_count{0};
    atomic<bool> done{false};

    // 生产者线程函数
    auto producer_func = [&]() {
        size_t local_count = 0;
        while (local_count < TOTAL_OPERATIONS / PRODUCER_THREADS) {
            int value = static_cast<int>(produced_count.fetch_add(1, memory_order_relaxed));
            
            // 尝试push值，如果失败则自旋等待
            while (!ring_buffer.try_push(value)) {
                for (int i = 0; i < 100; ++i) {
                    __asm__ volatile ("pause");
                }
            }
            
            local_count++;
        }
    };

    // 消费者线程函数
    auto consumer_func = [&]() {
        vector<int> received_values;
        received_values.reserve(TOTAL_OPERATIONS);

        while (consumed_count.load(memory_order_relaxed) < TOTAL_OPERATIONS) {
            int value;
            if (ring_buffer.try_pop(value)) {
                received_values.push_back(value);
                consumed_count.fetch_add(1, memory_order_relaxed);
            } else {
                // 短暂自旋等待
                for (int i = 0; i < 100; ++i) {
                    __asm__ volatile ("pause");
                }
            }
        }

        // 验证接收值的正确性
        sort(received_values.begin(), received_values.end());
        for (size_t i = 0; i < received_values.size(); ++i) {
            assert(received_values[i] == static_cast<int>(i));
        }
        cout << "✓ 数据正确性验证通过！所有值均按预期接收。\n";
    };

    // 启动计时
    auto start_time = high_resolution_clock::now();

    // 创建并启动生产者线程
    vector<thread> producer_threads;
    producer_threads.reserve(PRODUCER_THREADS);
    for (size_t i = 0; i < PRODUCER_THREADS; ++i) {
        producer_threads.emplace_back(producer_func);
    }

    // 创建并启动消费者线程
    thread consumer_thread(consumer_func);

    // 等待所有生产者线程完成
    for (auto& thread : producer_threads) {
        thread.join();
    }

    // 等待消费者线程完成
    consumer_thread.join();

    // 停止计时
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<nanoseconds>(end_time - start_time);

    // 计算QPS
    double ops_per_second = static_cast<double>(TOTAL_OPERATIONS) / (duration.count() / 1e9);
    double billion_ops_per_second = ops_per_second / 1e9;

    cout << endl;
    cout << "=== 性能测试结果 ===\n";
    cout << "总操作次数: " << TOTAL_OPERATIONS << endl;
    cout << "执行时间: " << duration.count() << " 纳秒\n";
    cout << "QPS: " << ops_per_second << " ops/sec\n";
    cout << "约 " << billion_ops_per_second << " billion ops/sec\n";

    return 0;
}