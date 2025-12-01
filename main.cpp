#include "StateMachine.h"
#include <iostream>
#include <vector>
#include <random>
#include <chrono>

int main() {
    // 创建状态机实例
    StateMachine fsm;

    // 从JSON文件加载状态机定义
    if (!fsm.loadFromJson("door_lock.json")) {
        std::cerr << "Failed to load state machine from door_lock.json" << std::endl;
        return 1;
    }

    // 初始化状态机
    if (!fsm.initialize()) {
        std::cerr << "Failed to initialize state machine" << std::endl;
        return 1;
    }

    // 输出初始状态
    State* initialState = fsm.getCurrentState();
    std::cout << "Initial state: " << (initialState ? initialState->getId() : "None") << std::endl;

    // 定义随机事件列表
    std::vector<Event> events = {
        "PIN_ENTERED",
        "DOOR_OPENED",
        "DOOR_CLOSED",
        "LOCK_BUTTON_PRESSED",
        "EMERGENCY",
        "TIMEOUT"
    };

    // 初始化随机数生成器
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_int_distribution<int> distribution(0, events.size() - 1);

    // 模拟1000次随机事件
    const int numEvents = 1000;
    for (int i = 0; i < numEvents; ++i) {
        // 随机选择一个事件
        int eventIndex = distribution(generator);
        Event event = events[eventIndex];

        // 触发事件
        fsm.transition(event);

        // 每100次事件输出一次当前状态
        if ((i + 1) % 100 == 0) {
            State* currentState = fsm.getCurrentState();
            std::cout << "After " << (i + 1) << " events, current state: " 
                      << (currentState ? currentState->getId() : "None") << std::endl;
        }
    }

    // 输出最终状态
    State* finalState = fsm.getCurrentState();
    std::cout << "Final state after " << numEvents << " events: " 
              << (finalState ? finalState->getId() : "None") << std::endl;

    // 验证最终状态是否合理（门锁系统最终状态应该是Locked或Unlocked）
    if (finalState) {
        std::string finalStateId = finalState->getId();
        if (finalStateId == "Locked" || finalStateId == "Unlocked") {
            std::cout << "Test passed: Final state is reasonable (" << finalStateId << ")" << std::endl;
        } else {
            std::cout << "Test warning: Final state is " << finalStateId << ", which may not be reasonable" << std::endl;
        }
    } else {
        std::cout << "Test failed: Final state is None" << std::endl;
        return 1;
    }

    return 0;
}
