#include "Coroutine.h"

CoroutineScheduler::CoroutineScheduler()
    : next_coroutine_id_(1), current_coroutine_(nullptr), main_fiber_(nullptr) {
    // 初始化主纤维
    main_fiber_ = ConvertThreadToFiber(nullptr);
}

CoroutineScheduler::~CoroutineScheduler() {
    // 清理所有协程
    for (auto& pair : coroutines_) {
        if (pair.second->fiber != nullptr) {
            DeleteFiber(pair.second->fiber);
        }
        delete pair.second;
    }
    coroutines_.clear();

    // 恢复线程
    ConvertFiberToThread();
}

CoroutineScheduler::CoroutineId CoroutineScheduler::create_coroutine(CoroutineFunc func) {
    CoroutineId id = next_coroutine_id_++;
    Coroutine* coroutine = new Coroutine(id, func);

    // 创建纤维
    coroutine->fiber = CreateFiberEx(
        0, // 初始栈大小
        64 * 1024, // 提交栈大小
        FIBER_FLAG_FLOAT_SWITCH, // 切换时保存浮点状态
        fiber_entry, // 纤维入口函数
        this // 传递给纤维的参数
    );

    coroutines_[id] = coroutine;
    ready_queue_.push(id);

    return id;
}

void CoroutineScheduler::yield() {
    if (current_coroutine_ == nullptr) {
        return; // 没有当前运行的协程
    }

    // 将当前协程状态设置为READY并加入就绪队列
    current_coroutine_->status = CoroutineStatus::READY;
    ready_queue_.push(current_coroutine_->id);

    // 切换回主纤维
    SwitchToFiber(main_fiber_);
}

void CoroutineScheduler::resume(CoroutineId id) {
    auto it = coroutines_.find(id);
    if (it == coroutines_.end()) {
        return; // 协程不存在
    }

    Coroutine* coroutine = it->second;
    if (coroutine->status != CoroutineStatus::READY) {
        return; // 协程不是READY状态
    }

    // 保存当前协程状态
    Coroutine* previous_coroutine = current_coroutine_;

    // 设置当前协程并运行
    current_coroutine_ = coroutine;
    current_coroutine_->status = CoroutineStatus::RUNNING;

    // 切换到目标纤维
    SwitchToFiber(current_coroutine_->fiber);

    // 切换回来后，检查之前的协程是否需要恢复
    if (previous_coroutine != nullptr && previous_coroutine->status == CoroutineStatus::RUNNING) {
        current_coroutine_ = previous_coroutine;
    }
}

CoroutineStatus CoroutineScheduler::get_status(CoroutineId id) const {
    auto it = coroutines_.find(id);
    if (it == coroutines_.end()) {
        return CoroutineStatus::DEAD; // 协程不存在，视为DEAD
    }
    return it->second->status;
}

void CoroutineScheduler::run() {
    while (!ready_queue_.empty()) {
        CoroutineId id = ready_queue_.front();
        ready_queue_.pop();

        resume(id);
    }
}

void __stdcall CoroutineScheduler::fiber_entry(void* param) {
    CoroutineScheduler* scheduler = static_cast<CoroutineScheduler*>(param);

    while (true) {
        if (scheduler->current_coroutine_ != nullptr) {
            // 运行协程函数
            scheduler->current_coroutine_->func();

            // 协程函数执行完毕，设置状态为DEAD
            scheduler->current_coroutine_->status = CoroutineStatus::DEAD;

            // 切换回主纤维
            SwitchToFiber(scheduler->main_fiber_);
        }
    }
}
