#include <iostream>
#include <vector>
#include <chrono>
#include "ThreadPool.h"

// 斐波那契函数（递归实现，用于性能测试）
int fib(int n) {
    if (n <= 1) {
        return n;
    }
    return fib(n - 1) + fib(n - 2);
}

int main() {
    try {
        // 创建8线程池
        ThreadPool pool(8);

        // 提交100个斐波那契任务
        const int num_tasks = 100;
        const int fib_n = 30; // 斐波那契数列的第n项

        std::vector<std::future<int>> futures;
        futures.reserve(num_tasks);

        // 记录开始时间
        auto start_time = std::chrono::high_resolution_clock::now();

        // 提交任务
        for (int i = 0; i < num_tasks; ++i) {
            futures.emplace_back(pool.submit(fib, fib_n));
        }

        // 等待所有任务完成并获取结果
        std::vector<int> results;
        results.reserve(num_tasks);

        for (auto& future : futures) {
            results.emplace_back(future.get());
        }

        // 记录结束时间
        auto end_time = std::chrono::high_resolution_clock::now();

        // 计算总耗时
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

        // 打印结果
        std::cout << "=== Performance Benchmark ===\n";
        std::cout << "ThreadPool Size: 8\n";
        std::cout << "Number of Tasks: 100\n";
        std::cout << "Fibonacci Sequence nth term: n = " << fib_n << "\n";
        std::cout << "Total Time for All Tasks: " << duration.count() << " milliseconds\n";
        std::cout << "==============================\n";

        // 验证结果是否正确
        bool all_correct = true;
        int expected_result = fib(fib_n);

        for (int result : results) {
            if (result != expected_result) {
                all_correct = false;
                break;
            }
        }

        if (all_correct) {
            std::cout << "All task results are correct\n";
        } else {
            std::cout << "Task results contain errors\n";
        }

        // 刷新缓冲区，确保所有输出信息都能被显示出来
        std::cout.flush();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }

    return 0;
}
