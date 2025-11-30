#ifndef COROUTINE_H
#define COROUTINE_H

#include <Windows.h>
#include <functional>
#include <unordered_map>
#include <queue>
#include <chrono>

enum class CoroutineStatus {
    READY,
    RUNNING,
    DEAD
};

class CoroutineScheduler {
public:
    using CoroutineFunc = std::function<void()>;
    using CoroutineId = unsigned int;

    CoroutineScheduler();
    ~CoroutineScheduler();

    CoroutineId create_coroutine(CoroutineFunc func);
    void yield();
    void resume(CoroutineId id);
    CoroutineStatus get_status(CoroutineId id) const;

    void run();

private:
    struct Coroutine {
        CoroutineId id;
        CoroutineFunc func;
        LPVOID fiber;
        CoroutineStatus status;
        char stack[64 * 1024]; // 64KB stack

        Coroutine(CoroutineId i, CoroutineFunc f)
            : id(i), func(f), fiber(nullptr), status(CoroutineStatus::READY) {}
    };

    CoroutineId next_coroutine_id_;
    std::unordered_map<CoroutineId, Coroutine*> coroutines_;
    std::queue<CoroutineId> ready_queue_;
    Coroutine* current_coroutine_;
    LPVOID main_fiber_;

    static void __stdcall fiber_entry(void* param);
};

#endif // COROUTINE_H
