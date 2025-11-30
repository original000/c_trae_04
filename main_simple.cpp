#include "Coroutine.h"
#include <iostream>

using namespace std;

int main() {
    cout << "Starting coroutine scheduler..." << endl;

    CoroutineScheduler scheduler;

    // 创建一个简单的协程
    auto coroutine_func = [&]() {
        cout << "Coroutine started!" << endl;
        for (int i = 0; i < 5; ++i) {
            cout << "Coroutine iteration: " << i << endl;
            scheduler.yield();
        }
        cout << "Coroutine finished!" << endl;
    };

    CoroutineScheduler::CoroutineId id = scheduler.create_coroutine(coroutine_func);
    cout << "Created coroutine with ID: " << id << endl;

    cout << "Running scheduler..." << endl;
    scheduler.run();

    cout << "Scheduler finished!" << endl;

    return 0;
}
