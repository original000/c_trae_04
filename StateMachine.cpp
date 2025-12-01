#include "StateMachine.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace std;

// State类实现
State::State(const StateId& id)
    : id(id), parent(nullptr), historyType(HistoryType::None), historyState(nullptr) {
}

State::~State() {
}

StateId State::getId() const {
    return id;
}

void State::setParent(State* parent) {
    this->parent = parent;
}

State* State::getParent() const {
    return parent;
}

void State::addChild(std::unique_ptr<State> child) {
    child->setParent(this);
    children[child->getId()] = std::move(child);
}

State* State::getChild(const StateId& id) const {
    auto it = children.find(id);
    if (it != children.end()) {
        return it->second.get();
    }
    return nullptr;
}

void State::setInitialChild(const StateId& id) {
    initialChildId = id;
}

State* State::getInitialChild() const {
    return getChild(initialChildId);
}

void State::setHistoryType(HistoryType type) {
    historyType = type;
}

HistoryType State::getHistoryType() const {
    return historyType;
}

void State::setHistoryState(State* state) {
    historyState = state;
}

State* State::getHistoryState() const {
    return historyState;
}

void State::addTransition(const Transition& transition) {
    transitions[transition.event] = transition;
}

const Transition* State::getTransition(const Event& event) const {
    auto it = transitions.find(event);
    if (it != transitions.end()) {
        return &(it->second);
    }
    return nullptr;
}

void State::setEntryAction(Action action) {
    entryAction = action;
}

void State::executeEntryAction() const {
    if (entryAction) {
        entryAction();
    }
}

void State::setExitAction(Action action) {
    exitAction = action;
}

void State::executeExitAction() const {
    if (exitAction) {
        exitAction();
    }
}

bool State::isComposite() const {
    return !children.empty();
}

std::vector<State*> State::getChildren() const {
    std::vector<State*> childPtrs;
    for (const auto& pair : children) {
        childPtrs.push_back(pair.second.get());
    }
    return childPtrs;
}

// StateMachine类实现
StateMachine::StateMachine()
    : currentState(nullptr) {
}

StateMachine::~StateMachine() {
}

bool StateMachine::loadFromJson(const std::string& filename) {
    try {
        JsonValue json = JsonParser::parseFile(filename);
        if (json.isObject()) {
            const JsonValue& rootJson = json["root"];
            if (rootJson.isObject()) {
                rootState = createStateFromJson(rootJson);
                return true;
            }
        }
    } catch (const exception& e) {
        cerr << "Failed to load state machine from JSON: " << e.what() << endl;
    }
    return false;
}

bool StateMachine::initialize() {
    if (!rootState) {
        return false;
    }
    enterState(rootState.get());
    return true;
}

void StateMachine::transition(const Event& event) {
    if (!currentState) {
        return;
    }
    // 从当前状态开始向上查找转移
    State* current = currentState;
    while (current) {
        const Transition* transition = current->getTransition(event);
        if (transition) {
            // 检查守卫条件
            if (transition->guard && !transition->guard()) {
                break;
            }
            // 退出当前状态
            exitState(currentState);
            // 执行转移动作
            if (transition->action) {
                transition->action();
            }
            // 查找目标状态
            State* targetState = nullptr;
            if (transition->targetState == "") {
                // 自转移
                targetState = currentState;
            } else {
                // 从根状态开始查找目标状态
                targetState = findState(getRootState(), transition->targetState);
            }
            if (targetState) {
                // 进入目标状态
                enterState(targetState);
            }
            break;
        }
        current = current->getParent();
    }
}

State* StateMachine::getCurrentState() const {
    return currentState;
}

State* StateMachine::getRootState() const {
    return rootState.get();
}

std::unique_ptr<State> StateMachine::createStateFromJson(const JsonValue& json) {
    if (!json.isObject()) {
        return nullptr;
    }
    // 获取状态ID
    StateId id = "";
    if (json["id"].isString()) {
        id = json["id"].asString();
    }
    if (id.empty()) {
        return nullptr;
    }
    auto state = std::make_unique<State>(id);
    // 设置进入动作
    if (json["entry"].isString()) {
        // 这里可以根据字符串映射到实际的动作函数
        // 为了简化，这里暂时不实现动作函数的映射
        // state->setEntryAction([]() { /* 动作实现 */ });
    }
    // 设置退出动作
    if (json["exit"].isString()) {
        // 同理，这里暂时不实现动作函数的映射
        // state->setExitAction([]() { /* 动作实现 */ });
    }
    // 设置历史状态类型
    if (json["history"].isString()) {
        std::string history = json["history"].asString();
        if (history == "shallow") {
            state->setHistoryType(HistoryType::Shallow);
        } else if (history == "deep") {
            state->setHistoryType(HistoryType::Deep);
        }
    }
    // 设置初始子状态
    if (json["initial"].isString()) {
        state->setInitialChild(json["initial"].asString());
    }
    // 添加转移
    if (json["transitions"].isArray()) {
        const auto& transitionsJson = json["transitions"].asArray();
        for (const auto& transitionJson : transitionsJson) {
            if (transitionJson.isObject()) {
                Transition transition;
                // 获取事件
                if (transitionJson["event"].isString()) {
                    transition.event = transitionJson["event"].asString();
                }
                // 获取目标状态
                if (transitionJson["target"].isString()) {
                    transition.targetState = transitionJson["target"].asString();
                }
                // 获取守卫条件
                if (transitionJson["guard"].isString()) {
                    // 同理，这里暂时不实现守卫函数的映射
                    // transition.guard = []() { return true; }; // 默认返回true
                }
                // 获取转移动作
                if (transitionJson["action"].isString()) {
                    // 同理，这里暂时不实现动作函数的映射
                }
                state->addTransition(transition);
            }
        }
    }
    // 添加子状态
    if (json["children"].isArray()) {
        const auto& childrenJson = json["children"].asArray();
        for (const auto& childJson : childrenJson) {
            auto child = createStateFromJson(childJson);
            if (child) {
                state->addChild(std::move(child));
            }
        }
    }
    return state;
}

void StateMachine::enterState(State* state) {
    if (!state) {
        return;
    }
    // 进入状态
    state->executeEntryAction();
    // 如果是复合状态，进入初始子状态或历史状态
    if (state->isComposite()) {
        State* nextState = nullptr;
        if (state->getHistoryType() != HistoryType::None && state->getHistoryState()) {
            nextState = state->getHistoryState();
        } else {
            nextState = state->getInitialChild();
        }
        if (nextState) {
            enterState(nextState);
            return;
        }
    }
    // 更新当前状态
    currentState = state;
}

void StateMachine::exitState(State* state) {
    if (!state) {
        return;
    }
    // 如果是复合状态，先退出子状态
    if (state->isComposite()) {
        State* child = findDeepestChild(state);
        if (child) {
            exitState(child);
        }
    }
    // 保存历史状态
    State* parent = state->getParent();
    if (parent) {
        if (parent->getHistoryType() == HistoryType::Deep) {
            parent->setHistoryState(state);
        } else if (parent->getHistoryType() == HistoryType::Shallow) {
            // 浅历史只保存直接子状态
            parent->setHistoryState(state);
        }
    }
    // 退出状态
    state->executeExitAction();
}

// 辅助函数：查找状态
State* StateMachine::findState(State* root, const StateId& id) {
    if (!root) {
        return nullptr;
    }
    if (root->getId() == id) {
        return root;
    }
    for (State* child : root->getChildren()) {
        State* found = findState(child, id);
        if (found) {
            return found;
        }
    }
    return nullptr;
}

// 辅助函数：查找最深的子状态
State* StateMachine::findDeepestChild(State* state) {
    if (!state) {
        return nullptr;
    }
    if (!state->isComposite()) {
        return state;
    }
    for (State* child : state->getChildren()) {
        State* deepest = findDeepestChild(child);
        if (deepest) {
            return deepest;
        }
    }
    return nullptr;
}
