#include "Coroutine.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <windows.h>

using namespace std;

// 设置控制台输出为UTF-8
void set_console_utf8() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
}

const int NUM_COROUTINES = 10;
const int YIELD_COUNT = 1000;

int main() {
    set_console_utf8();
    CoroutineScheduler scheduler;

    // 简单的协程函数，只是循环yield
    auto coroutine_func = [&](int id) {
        for (int i = 0; i < YIELD_COUNT; ++i) {
            // 这里可以添加一些简单的操作
            scheduler.yield();
        }
    };

    // 创建10个协程
    vector<CoroutineScheduler::CoroutineId> coroutine_ids;
    for (int i = 0; i < NUM_COROUTINES; ++i) {
        coroutine_ids.push_back(scheduler.create_coroutine(
            [&, i]() { coroutine_func(i); }
        ));
    }

    // 记录开始时间
    auto start_time = chrono::high_resolution_clock::now();

    // 运行协程调度器
    scheduler.run();

    // 记录结束时间
    auto end_time = chrono::high_resolution_clock::now();

    // 计算总耗时
    auto duration = chrono::duration_cast<chrono::milliseconds>(end_time - start_time);

    // 输出结果
    cout << "所有协程执行完毕！" << endl;
    cout << "协程数量：" << NUM_COROUTINES << endl;
    cout << "每个协程yield次数：" << YIELD_COUNT << endl;
    cout << "总耗时：" << duration.count() << " 毫秒" << endl;

    // 检查所有协程是否都已结束
    bool all_dead = true;
    for (auto id : coroutine_ids) {
        if (scheduler.get_status(id) != CoroutineStatus::DEAD) {
            all_dead = false;
            break;
        }
    }
    if (all_dead) {
        cout << "所有协程都已正常结束。" << endl;
    } else {
        cout << "存在未正常结束的协程。" << endl;
    }

    return 0;
}
