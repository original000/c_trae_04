#include "LockProfiler.h"
#include <vector>
#include <thread>
#include <chrono>
#include <random>
#include <memory>
#include <iostream>

std::vector<std::unique_ptr<std::mutex>> mutexes;
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> dis(0, 9);

void ThreadFunction(int threadId) {
    while (LockProfiler::Instance().IsRunning()) {
        int mutexIndex = dis(gen);
        auto start = std::chrono::high_resolution_clock::now();

        mutexes[mutexIndex]->lock();

        auto end = std::chrono::high_resolution_clock::now();
        auto waitTime = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

        if (waitTime > 100) { // 只记录等待时间超过100ns的竞争
            LockProfiler::Instance().OnLockContended(mutexes[mutexIndex].get(), waitTime);
        }

        LockProfiler::Instance().OnLockAcquired(mutexes[mutexIndex].get());

        // 模拟临界区操作
        std::this_thread::sleep_for(std::chrono::microseconds(10));

        LockProfiler::Instance().OnLockReleased(mutexes[mutexIndex].get());
        mutexes[mutexIndex]->unlock();
    }
}

int main() {
    // 初始化10个mutex
    for (int i = 0; i < 10; ++i) {
        mutexes.emplace_back(std::make_unique<std::mutex>());
    }

    // 启动锁分析器
    LockProfiler::Instance().Start();

    // 创建4个线程
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(ThreadFunction, i);
    }

    // 运行5秒
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // 停止分析器和线程
    LockProfiler::Instance().Stop();
    for (auto& thread : threads) {
        thread.join();
    }

    // 输出火焰图数据
    LockProfiler::Instance().OutputFlameGraph("flamegraph.json");

    std::cout << "锁竞争分析完成，已生成 flamegraph.json 文件。" << std::endl;

    return 0;
}