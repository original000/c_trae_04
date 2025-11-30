#include "AhoCorasick.h"

AhoCorasick::AhoCorasick(bool case_insensitive) 
    : case_insensitive(case_insensitive) {
    // 初始化根节点
    nodes.push_back(new Node());
}

AhoCorasick::~AhoCorasick() {
    // 释放所有节点内存
    for (Node* node : nodes) {
        delete node;
    }
}

int AhoCorasick::addRule(const std::string& pattern) {
    Node* current = nodes[0];
    
    for (char c : pattern) {
        if (case_insensitive) {
            c = toLower(c);
        }
        
        if (current->children.find(c) == current->children.end()) {
            // 创建新节点
            nodes.push_back(new Node());
            current->children[c] = nodes.size() - 1;
        }
        
        current = nodes[current->children[c]];
    }
    
    // 将规则ID添加到输出链表
    int rule_id = patterns.size();
    current->outputs.push_back(rule_id);
    patterns.push_back(pattern);
    
    return rule_id;
}

void AhoCorasick::build() {
    std::queue<int> q;
    Node* root = nodes[0];
    
    // 初始化根节点的所有子节点的失败指针为根节点
    for (auto& pair : root->children) {
        int child_id = pair.second;
        nodes[child_id]->fail = 0;
        q.push(child_id);
    }
    
    // BFS构建失败转移指针和输出链表
    while (!q.empty()) {
        int current_id = q.front();
        q.pop();
        Node* current = nodes[current_id];
        
        for (auto& pair : current->children) {
            char c = pair.first;
            int child_id = pair.second;
            Node* child = nodes[child_id];
            
            // 找到当前节点的失败指针指向的节点
            int fail_id = current->fail;
            Node* fail_node = nodes[fail_id];
            
            // 沿着失败指针链查找，直到找到根节点或包含字符c的子节点
            while (fail_id != 0 && fail_node->children.find(c) == fail_node->children.end()) {
                fail_id = fail_node->fail;
                fail_node = nodes[fail_id];
            }
            
            // 如果找到包含字符c的子节点，则将子节点的失败指针指向该节点
            if (fail_node->children.find(c) != fail_node->children.end() && 
                fail_node->children[c] != child_id) {
                child->fail = fail_node->children[c];
            } else {
                child->fail = 0;
            }
            
            // 将失败指针指向的节点的输出链表合并到当前子节点的输出链表
            Node* fail_child = nodes[child->fail];
            child->outputs.insert(child->outputs.end(), 
                                    fail_child->outputs.begin(), 
                                    fail_child->outputs.end());
            
            // 将子节点加入队列，继续BFS
            q.push(child_id);
        }
    }
}

std::vector<std::pair<int, int>> AhoCorasick::match(const std::string& text) {
    std::vector<std::pair<int, int>> results;
    Node* current = nodes[0];
    
    for (int i = 0; i < text.size(); ++i) {
        char c = text[i];
        if (case_insensitive) {
            c = toLower(c);
        }
        
        // 沿着失败指针链查找，直到找到根节点或包含字符c的子节点
        while (current->children.find(c) == current->children.end() && current != nodes[0]) {
            current = nodes[current->fail];
        }
        
        // 如果找到包含字符c的子节点，则移动到该子节点
        if (current->children.find(c) != current->children.end()) {
            current = nodes[current->children[c]];
        }
        
        // 如果当前节点有输出，则将所有匹配到的规则ID和位置加入结果
        if (!current->outputs.empty()) {
            for (int rule_id : current->outputs) {
                int start_pos = i - patterns[rule_id].size() + 1;
                results.emplace_back(rule_id, start_pos);
            }
        }
    }
    
    return results;
}

int AhoCorasick::getRuleCount() const {
    return patterns.size();
}

char AhoCorasick::toLower(char c) const {
    if (c >= 'A' && c <= 'Z') {
        return c + ('a' - 'A');
    }
    return c;
}