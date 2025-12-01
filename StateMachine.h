#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include <memory>
#include "JsonParser.h"

// 事件类型
using Event = std::string;

// 状态ID类型
using StateId = std::string;

// 守卫条件函数类型
using Guard = std::function<bool()>;

// 动作函数类型
using Action = std::function<void()>;

// 历史状态类型
enum class HistoryType {
    None,
    Shallow,
    Deep
};

// 转移定义
struct Transition {
    Event event;                // 触发事件
    StateId targetState;        // 目标状态
    Guard guard;                 // 守卫条件
    Action action;               // 转移动作
};

// 状态定义
class State {
public:
    State(const StateId& id);
    ~State();

    // 获取状态ID
    StateId getId() const;

    // 设置父状态
    void setParent(State* parent);

    // 获取父状态
    State* getParent() const;

    // 添加子状态
    void addChild(std::unique_ptr<State> child);

    // 获取子状态
    State* getChild(const StateId& id) const;

    // 设置初始子状态
    void setInitialChild(const StateId& id);

    // 获取初始子状态
    State* getInitialChild() const;

    // 设置历史状态类型
    void setHistoryType(HistoryType type);

    // 获取历史状态类型
    HistoryType getHistoryType() const;

    // 设置历史状态
    void setHistoryState(State* state);

    // 获取历史状态
    State* getHistoryState() const;

    // 添加转移
    void addTransition(const Transition& transition);

    // 获取转移
    const Transition* getTransition(const Event& event) const;

    // 设置进入动作
    void setEntryAction(Action action);

    // 执行进入动作
    void executeEntryAction() const;

    // 设置退出动作
    void setExitAction(Action action);

    // 执行退出动作
    void executeExitAction() const;

    // 检查是否为复合状态
    bool isComposite() const;

    // 获取所有子状态的指针
    std::vector<State*> getChildren() const;

private:
    StateId id;                          // 状态ID
    State* parent;                       // 父状态
    std::unordered_map<StateId, std::unique_ptr<State>> children;  // 子状态
    StateId initialChildId;              // 初始子状态ID
    HistoryType historyType;             // 历史状态类型
    State* historyState;                 // 历史状态
    std::unordered_map<Event, Transition> transitions;  // 转移
    Action entryAction;                   // 进入动作
    Action exitAction;                    // 退出动作
};

// 有限状态机类
class StateMachine {
public:
    StateMachine();
    ~StateMachine();

    // 从JSON文件加载状态机定义
    bool loadFromJson(const std::string& filename);

    // 初始化状态机
    bool initialize();

    // 触发事件
    void transition(const Event& event);

    // 获取当前状态
    State* getCurrentState() const;

    // 获取根状态
    State* getRootState() const;

private:
    // 从JSON值创建状态
    std::unique_ptr<State> createStateFromJson(const JsonValue& json);

    // 进入状态（递归处理复合状态）
    void enterState(State* state);

    // 退出状态（递归处理复合状态）
    void exitState(State* state);

    // 辅助函数：查找状态
    State* findState(State* root, const StateId& id);

    // 辅助函数：查找最深的子状态
    State* findDeepestChild(State* state);

    std::unique_ptr<State> rootState;  // 根状态
    State* currentState;                  // 当前状态
};

#endif // STATEMACHINE_H